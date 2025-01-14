// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/10/2024.

#include <boost/property_tree/ptree.hpp>
#include <cassert>
#include <cstddef>
#include <format>
#include <include/exceptions.hpp>
#include <include/node-attributes.hpp>
#include <include/node-type.hpp>
#include <lib/ptree/generative-node-source.hpp>
#include <lib/schema-parser/auto-index.hpp>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/schema-parser/parser-phase-two.hpp>
#include <lib/schema-parser/token-type.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/schema.hpp>
#include <memory>
#include <string>

using namespace std::string_literals;
namespace bpt = boost::property_tree;
namespace csp = c4lib::schema_parser;

namespace c4lib::property_tree {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Generative_node_source::Generative_node_source(
    csp::Parser_phase_two& parser, const csp::Token& type, const csp::Token& identifier)
    : m_identifier(identifier), m_parser(parser), m_tokenizer(parser.m_tokenizer), m_type(type)
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initializes the tree associated with m_type.
bool Generative_node_source::init_()
{
    m_root = std::make_unique<Dimension_node>();
    // If the type is a template we need to skip over 4 tokens: type < typename >
    // Otherwise we skip past the single type token.
    const size_t skip_count{m_type.type == csp::Token_type::template_type ? 4UL : 1UL};
    const size_t bracket_index = m_type.index + skip_count;
    assert(bracket_index == m_identifier.index
           || m_tokenizer.at(bracket_index).type == csp::Token_type::open_square_bracket);
    const bool is_success{
        init_node_(m_root.get(), nullptr, m_parser.m_ptree_parent, bracket_index, limits::invalid_size, "", "")};
    m_tokenizer.restore_type_name_token();
    return is_success;
}

bool Generative_node_source::init_node_(Dimension_node* node,
    Dimension_node* parent,
    bpt::ptree* ptree_parent,
    size_t bracket_token_index,
    size_t array_subscript,
    const std::string& array_name,
    const std::string& cumulative_subscript_string)
{
    node->parent = parent;
    node->index = 0;
    std::string node_name;
    if (parent == nullptr) {
        node_name = m_identifier.value;
    }
    else {
        node_name = std::format("[{}]", array_subscript);
    }
    node->ptree = &ptree_parent->add_child(node_name, bpt::ptree{});

    size_t next_bracket_token_index{limits::invalid_size};
    bool is_array{false};
    size_t dimension_size{limits::invalid_size};
    std::string enum_name;
    bool is_capture{false};
    if (!parse_dimension_info_(
            bracket_token_index, next_bracket_token_index, is_array, dimension_size, enum_name, is_capture)) {
        return false;
    }

    bpt::ptree& attributes{node->ptree->put_child(nn_attributes, bpt::ptree{nv_meta})};
    attributes.add(nn_name, node_name);

    // The array name attribute is used for an array and its children.
    if (!array_name.empty()) {
        // This node is a member of an array.  Set the array name attribute to indicate that this
        // node belongs to an array.  This information is used for various purposes.  For example,
        // node readers uses this information to know when to generate the PlayerTypes enumeration.
        attributes.add(nn_array_name, array_name);
    }
    else if (is_array) {
        // This node is an array.  Set its array name attribute using the node's name.
        attributes.add(nn_array_name, node_name);
    }

    if (is_array) {
        if (dimension_size > limits::max_array_dimension) {
            throw make_ex<Node_source_error>(fmt::array_dimension_out_of_range, m_identifier.loc, dimension_size);
        }
        if (dimension_size == 0) {
            // Set node->index to limits::invalid_size to facilitate node traversal.  If the index were
            // to remain 0, node traversal would prematurely end at this node.
            node->index = limits::invalid_size;
        }

        attributes.add(nn_type, node_type_as_string(Node_type::array_type));
        attributes.add(nn_typename, m_type.value);
        const std::string array_subscript_string{std::format("[{}]", dimension_size)};
        attributes.add(nn_subscripts, array_subscript_string);
        node->nodes.reserve(dimension_size);

        for (size_t index = 0; index < dimension_size; ++index) {
            if (is_capture) {
                m_captured_index = index;
            }

            std::string subscript_string;
            if (!enum_name.empty()) {
                const csp::Def_mem& enumerator{
                    m_parser.m_definition_table.get_enumerator(enum_name, gsl::narrow<int>(index))};
                subscript_string = std::format("[{}:{}]", index, enumerator.name);
            }
            else {
                subscript_string = std::format("[{}]", index);
            }

            node->nodes.emplace_back();
            if (!init_node_(&node->nodes.at(index), node, node->ptree, next_bracket_token_index, index, node_name,
                    cumulative_subscript_string + subscript_string)) {
                return false;
            }
        }
    }
    else {
        // This node is a leaf.  We set node->index to limits::invalid_size to facilitate
        // node traversal.
        node->index = limits::invalid_size;

        attributes.add(nn_type, token_type_to_node_type_as_string(m_type.type));
        attributes.add(nn_typename, m_type.value);
        if (!cumulative_subscript_string.empty()) {
            attributes.add(nn_subscripts, cumulative_subscript_string);
        }
        if (m_type.type == csp::Token_type::enum_type) {
            attributes.add(nn_enum, enum_name_from_type(m_type.value));
        }
    }
    return true;
}

bool Generative_node_source::is_query_reader_production_(size_t bracket_token_index) const
{
    // To determine whether the current token stream constitutes a query-reader production, we first save
    // the current stream position and scan ahead in the stream looking for a query-reader token.  We stop
    // our search once we find a query-reader or a close square-bracket token.
    // To prevent run-away searches, we first ensure that the next token is open-square-bracket.
    const csp::auto_index auto_restore_state(m_tokenizer, bracket_token_index);
    bool is_query_reader{false};
    const csp::Token* token{&m_tokenizer.next()};
    if (token->type != csp::Token_type::open_square_bracket) {
        return false;
    }

    while (token->type != csp::Token_type::close_square_bracket) {
        token = &m_tokenizer.next();
        if (token->type == csp::Token_type::query_reader_keyword) {
            is_query_reader = true;
            break;
        }
    }

    return is_query_reader;
}

bool Generative_node_source::is_use_capture_production_(size_t bracket_token_index) const
{
    // To determine whether the current token stream constitutes a use-capture production, we first save
    // the current stream position and scan ahead in the stream looking for a use-capture token.  We stop
    // our search once we find a use-capture or a close square-bracket token.
    // To prevent run-away searches, we first ensure that the next token is open-square-bracket.
    const csp::auto_index auto_restore_state(m_tokenizer, bracket_token_index);
    bool is_use_capture{false};
    const csp::Token* token{&m_tokenizer.next()};
    if (token->type != csp::Token_type::open_square_bracket) {
        return false;
    }

    while (token->type != csp::Token_type::close_square_bracket) {
        token = &m_tokenizer.next();
        if (token->type == csp::Token_type::use_capture_keyword) {
            is_use_capture = true;
            break;
        }
    }

    return is_use_capture;
}

bool Generative_node_source::next_(bpt::ptree*& ptree)
{
    if (!m_root) {
        if (!init_()) {
            return false;
        }
    }

    // Return the next node.  We intentionally skip node's of type array_type by design because
    // such nodes do not require further processing by the parser.  Instead, array_type nodes exist
    // to help generate well-formatted translations.
    bool unused{false};
    do {
        ptree = next_(*m_root, unused);
    }
    while ((ptree != nullptr) && ptree->get<Node_type>(nn_attributes + "."s + nn_type) == Node_type::array_type);
    return true;
}

bpt::ptree* Generative_node_source::next_(Dimension_node& node, bool& increment_caller_index)
{
    // Check if more nodes exist.
    if (node.index == node.nodes.size()) {
        return nullptr;
    }

    // Check if node is a leaf or an empty array node.  If so, return node.ptree and tell caller to increment its index.
    if (node.nodes.empty()) {
        // Set node.index to zero in case this is a simple type.  This will cause subsequent calls to
        // next_ to return nullptr due to the existence check above.
        node.index = 0;

        increment_caller_index = true;
        return node.ptree;
    }

    // Node is a branch.  Descend.
    bool increment_this_index{false};
    bpt::ptree* ptree{next_(node.nodes.at(node.index), increment_this_index)};
    if (increment_this_index) {
        node.index++;
        // Reset our index back to zero and tell caller to increment its index unless this is the root.
        // Once the root reaches nodes.size(), this is the last node and subsequent calls to next_ will
        // return nullptr due to the existence check above.
        if (node.index == node.nodes.size() && node.parent != nullptr) {
            node.index = 0;
            increment_caller_index = true;
        }
    }
    return ptree;
}

bool Generative_node_source::parse_dimension_info_(size_t bracket_token_index,
    size_t& next_bracket_token_index,
    bool& is_array,
    size_t& dimension_size,
    std::string& enum_name,
    bool& is_capture) const
{
    // Check if there's a subscript to parse.
    if (bracket_token_index == m_identifier.index) {
        // No subscript to parse
        is_array = false;
        dimension_size = 0;
        return true;
    }

    is_array = true;

    const bool is_success
        = pr_array_suffix_(bracket_token_index, next_bracket_token_index, dimension_size, enum_name, is_capture);
    return is_success;
}

// <array-suffix> ::= <open-square-bracket> <use-capture-node-reference> <opt-enum-bind> <close-square-bracket> |
//                    <open-square-bracket> <query-reader-keyword> <close-square-bracket> |
//                    <open-square-bracket> <expression> <opt-enum-bind> <opt-index-capture> <close-square-bracket>
bool Generative_node_source::pr_array_suffix_(size_t bracket_token_index,
    size_t& next_bracket_token_index,
    size_t& dimension_size,
    std::string& enum_name,
    bool& is_capture) const
{
    bool is_success{false};
    if (is_use_capture_production_(bracket_token_index)) {
        is_success = pr_array_suffix_with_use_capture_(
            bracket_token_index, next_bracket_token_index, dimension_size, enum_name);
    }
    else if (is_query_reader_production_(bracket_token_index)) {
        is_success = pr_array_suffix_with_query_reader_(bracket_token_index, next_bracket_token_index, dimension_size);
    }
    else {
        is_success = pr_array_suffix_standard_(
            bracket_token_index, next_bracket_token_index, dimension_size, enum_name, is_capture);
    }
    return is_success;
}

// <array-suffix> ::= <open-square-bracket> <expression> <opt-enum-bind> <opt-index-capture> <close-square-bracket>
bool Generative_node_source::pr_array_suffix_standard_(size_t bracket_token_index,
    size_t& next_bracket_token_index,
    size_t& dimension_size,
    std::string& enum_name,
    bool& is_capture) const
{
    const csp::auto_index auto_restore_state(m_tokenizer, bracket_token_index);

    int value{limits::invalid_value};
    const bool is_success{m_parser.pr_open_square_bracket_() && m_parser.pr_expression_(value)
                          && pr_opt_enum_bind_(enum_name) && pr_opt_index_capture_(is_capture)
                          && m_parser.pr_close_square_bracket_()};
    if (!is_success) {
        return false;
    }

    // Parsing succeeded.  Set values of output parameters.
    dimension_size = gsl::narrow<size_t>(value);
    next_bracket_token_index = m_tokenizer.get_index();
    return is_success;
}

// <array-suffix> ::= <open-square-bracket> <query-reader-keyword> <close-square-bracket>
bool Generative_node_source::pr_array_suffix_with_query_reader_(
    size_t bracket_token_index, size_t& next_bracket_token_index, size_t& dimension_size) const
{
    const csp::auto_index auto_restore_state(m_tokenizer, bracket_token_index);

    int value{limits::invalid_value};
    const bool is_success{
        m_parser.pr_open_square_bracket_() && pr_query_reader_keyword_(value) && m_parser.pr_close_square_bracket_()};
    if (!is_success) {
        return false;
    }

    // Parsing succeeded.  Set values of output parameters.
    dimension_size = gsl::narrow<size_t>(value);
    next_bracket_token_index = m_tokenizer.get_index();
    return is_success;
}

// <array-suffix> ::= <open-square-bracket> <use-capture-node-reference> <opt-enum-bind> <close-square-bracket>
bool Generative_node_source::pr_array_suffix_with_use_capture_(
    size_t bracket_token_index, size_t& next_bracket_token_index, size_t& dimension_size, std::string& enum_name) const
{
    const csp::auto_index auto_restore_state(m_tokenizer, bracket_token_index);

    int value{limits::invalid_value};
    const bool is_success{m_parser.pr_open_square_bracket_() && pr_use_capture_node_reference_(value)
                          && pr_opt_enum_bind_(enum_name) && m_parser.pr_close_square_bracket_()};
    if (!is_success) {
        return false;
    }

    // Parsing succeeded.  Set values of output parameters.
    dimension_size = gsl::narrow<size_t>(value);
    next_bracket_token_index = m_tokenizer.get_index();
    return is_success;
}

// <capture-index-keyword> ::= capture_index
bool Generative_node_source::pr_capture_index_keyword_() const
{
    const csp::Token& token{m_tokenizer.next()};
    const bool is_success{token.type == csp::Token_type::capture_index_keyword};
    return is_success;
}

// <enum-name> ::= <identifier>
bool Generative_node_source::pr_enum_name_(std::string& enum_name) const
{
    const bool is_success{m_parser.pr_identifier_()};
    if (is_success) {
        const csp::Token& identifier{m_tokenizer.previous()};
        enum_name = identifier.value;
    }
    else {
        enum_name = "";
    }
    return is_success;
}

// <index-capture> ::= <capture-index-keyword>
bool Generative_node_source::pr_index_capture_(bool& is_capture) const
{
    const bool is_success{pr_capture_index_keyword_()};
    if (is_success) {
        is_capture = true;
        return true;
    }
    return is_success;
}

// <node-name> ::= identifier
bool Generative_node_source::pr_node_name_() const
{
    const bool is_success = m_parser.pr_identifier_();
    return is_success;
}

// <opt-enum-bind> ::= <colon> <enum-name> | <null>
bool Generative_node_source::pr_opt_enum_bind_(std::string& enum_name) const
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{m_parser.pr_colon_() && pr_enum_name_(enum_name)};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = m_parser.pr_null_();

    return is_success;
}

// <opt-index-capture> ::= <colon> <index-capture> | <null>
bool Generative_node_source::pr_opt_index_capture_(bool& is_capture) const
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{m_parser.pr_colon_() && pr_index_capture_(is_capture)};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = m_parser.pr_null_();

    return is_success;
}

// <query-reader-keyword> ::= query_reader
bool Generative_node_source::pr_query_reader_keyword_(int& value) const
{
    const csp::Token& token{m_tokenizer.next()};
    const bool is_success{token.type == csp::Token_type::query_reader_keyword};
    if (is_success) {
        value = gsl::narrow<int>(m_parser.m_node_reader.get_undocumented_footer_bytes_count());
    }
    return is_success;
}

// <use-capture-node-reference> ::= <node-name> <open-square-bracket> <use-capture-keyword> <close-square-bracket>
bool Generative_node_source::pr_use_capture_node_reference_(int& value) const
{
    // We need to construct a well-formed node reference using the captured index when
    // we encounter [use_capture].  Then we need to use the ptree to look up the value
    // of the referenced node and set the function parameter accordingly.  We support
    // uniquely-named references only; thus, we search for any child of the root
    // with the give node_name.
    bool is_success{pr_node_name_()};
    if (!is_success) {
        return false;
    }
    const csp::Token& node_name{m_tokenizer.previous()};
    is_success = m_parser.pr_open_square_bracket_() && pr_use_capture_keyword_() && m_parser.pr_close_square_bracket_();
    if (!is_success) {
        return false;
    }

    auto it{m_parser.m_ptree_parent->find(node_name.value)};
    if (it == m_parser.m_ptree_parent->not_found()) {
        return false;
    }
    bpt::ptree* node{&it->second};

    std::string path_to_ref{"[" + std::to_string(m_captured_index) + "]"};
    const std::string path_to_ref_type{path_to_ref + "." + nn_attributes + "." + nn_type};
    const std::string path_to_ref_data{path_to_ref + "." + nn_attributes + "." + nn_data};

    // Verify that the referenced node is of type int.
    const Node_type type{node->get_child(path_to_ref_type).get_value<Node_type>()};
    if (type != Node_type::int_type) {
        throw make_ex<Parser_error>(fmt::referenced_node_not_int, node_name.loc, path_to_ref);
    }

    // Read the node data into value.
    value = node->get_child(path_to_ref_data).get_value<int>();

    return true;
}

// <use-capture-keyword> := use_capture
bool Generative_node_source::pr_use_capture_keyword_() const
{
    const csp::Token& token{m_tokenizer.next()};
    const bool is_success{token.type == csp::Token_type::use_capture_keyword};
    return is_success;
}

} // namespace c4lib::property_tree
