// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/31/2024.

#include <boost/property_tree/ptree.hpp>
#include <cstddef>
#include <include/exceptions.hpp>
#include <include/node-attributes.hpp>
#include <include/node-type.hpp>
#include <lib/ptree/generative-node-source.hpp>
#include <lib/ptree/node-reader.hpp>
#include <lib/schema-parser/auto-index.hpp>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/parser-phase-two.hpp>
#include <lib/schema-parser/parser.hpp>
#include <lib/schema-parser/token-type.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/auto-pop.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/file-location.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/schema.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>

using namespace std::string_literals;
namespace bpt = boost::property_tree;
namespace cpt = c4lib::property_tree;

namespace c4lib::schema_parser {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Parser_phase_two::Parser_phase_two(Tokenizer& tokenizer,
    Def_tbl& def_tbl,
    size_t root_name_index,
    Variable_manager& variable_manager,
    boost::property_tree::ptree& ptree_root,
    c4lib::property_tree::Node_reader& node_reader,
    std::unordered_map<std::string, std::string>& options)
    : m_definition_table(def_tbl),
      m_node_reader(node_reader),
      m_options(options),
      m_ptree_root(ptree_root),
      m_root_name_index(root_name_index),
      m_tokenizer(tokenizer),
      m_variable_manager(variable_manager)
{
    m_variable_manager.init(&m_ptree_root, &m_ptree_parent, &m_definition_table);
}

void Parser_phase_two::parse()
{
    // Clear state of objects populated during phase two parsing.
    while (!m_if_context_stack.empty()) {
        m_if_context_stack.pop();
    }
    while (!m_template_context_stack.empty()) {
        m_template_context_stack.pop();
    }
    m_ptree_parent = &m_ptree_root;

    // Start phase 2 parsing by calling emit_nodes_, passing a type token and an
    // identifier token corresponding to the root structure.  This sequence of tokens
    // does not exist in the tokenizer so we'll need to create them here.  The
    // tokens we're creating need to reflect the result of parsing the following
    // statement:
    //        struct_<root-name> <root-name>
    // where:
    //        struct_<root-name> corresponds to the type token
    //        <root-name> corresponds to the name referenced by m_root_name_index
    const std::string root_name{m_tokenizer.at(m_root_name_index).value};
    const std::string struct_root_name{"struct_" + root_name};
    // When creating our tokens, we need to set the token indices such that the
    // index of the identifier token is one more than the index of the type token
    // because emit_nodes_ relies on this.
    // Choose a fabricated index value far beyond the value of the last extant index
    // for debugging purposes.
    constexpr size_t fabricated_root_struct_index_offset{50'000};
    size_t index{fabricated_root_struct_index_offset + m_tokenizer.count()};
    const File_location loc{std::make_shared<std::string>("internally generated tokens"),
        std::make_shared<std::string>(struct_root_name + " " + root_name), 1, 1};
    const Token type_token{Token_type::struct_type, struct_root_name, loc, index++};
    if (const Token identifier_token{Token_type::identifier, root_name, loc, index++};
        !emit_nodes_(type_token, identifier_token)) {
        const Token& t{m_tokenizer.peek()};
        throw make_ex<Parser_error>(fmt::syntax_error, t.loc, to_string(t.type));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Parser_phase_two::emit_nodes_(const Token& type, const Token& identifier)
{
    // Get each ptree node associated with identifier.  In the case of arrays, several nodes will
    // be generated.  Then, use the node reader to read the node's data and size attributes.
    // Finally, in case identifier refers to an aggregate type (struct or template), we look up
    // the definition of the aggregate, set the tokenizer to point to the definition and then
    // call the appropriate production to finish processing the aggregate.
    for (cpt::Generative_node_source node_source(*this, type, identifier); bpt::ptree & node : node_source) {
        m_node_reader.read_node(node);

        const bpt::ptree& attributes_node{node.get_child(cpt::nn_attributes)};
        const bpt::ptree& type_node{attributes_node.get_child(cpt::nn_type)};
        const bpt::ptree& type_name_node{attributes_node.get_child(cpt::nn_typename)};
        const cpt::Node_type node_type{type_node.get_value<cpt::Node_type>()};

        if (node_type == cpt::Node_type::struct_type) {
            const std::string struct_name{identifier_from_type(type_name_node.get_value<std::string>())};
            const Def_mem& struct_def{m_definition_table.get_first_member(struct_name, Def_type::struct_type)};
            const size_t struct_index{gsl::narrow<size_t>(struct_def.value)};
            const auto_index as{m_tokenizer, struct_index};
            const auto_parent ap{this, &node};
            if (!pr_struct_definition_()) {
                const Token& t{m_tokenizer.peek()};
                throw make_ex<Parser_error>(fmt::syntax_error, t.loc, to_string(t.type));
            }
        }
        else if (node_type == cpt::Node_type::template_type) {
            // The type is either template or alias (toa)
            const std::string toa_name{identifier_from_type(type_name_node.get_value<std::string>())};
            const Def_type toa_type{m_definition_table.get_type(toa_name)};
            const Def_mem& template_def{m_definition_table.get_first_member(toa_name, toa_type)};
            const size_t template_index{gsl::narrow<size_t>(template_def.value)};
            const auto_index as{m_tokenizer, template_index};
            const auto_parent ap{this, &node};

            // The template_index points to the bracketed typename.  We first run the bracketed typename
            // production and if that succeeds we then run the template definition production.
            if (!pr_bracketed_typename_() || !pr_template_definition_()) {
                const Token& t{m_tokenizer.peek()};
                throw make_ex<Parser_error>(fmt::syntax_error, t.loc, to_string(t.type));
            }
        }
        else if (node_type == cpt::Node_type::enum_type) {
            // Check that the enumerator enumerator_value is valid
            const bpt::ptree& enum_node{attributes_node.get_child(cpt::nn_enum)};
            const std::string enum_name{enum_node.get_value<std::string>()};
            const bpt::ptree& data_node{attributes_node.get_child(cpt::nn_data)};
            const int enumerator_value{data_node.get_value<int>()};
            // get_enumerator will throw an exception if the enumerator value is not valid.
            static_cast<void>(m_definition_table.get_enumerator(enum_name, enumerator_value));
        }
        else if (node_type == cpt::Node_type::bool_type) {
            // Check that the value is either 0 or 1
            const bpt::ptree& data_node{attributes_node.get_child(cpt::nn_data)};
            int value{data_node.get_value<int>()};
            if (value != 0 && value != 1) {
                throw make_ex<Parser_error>(fmt::illegal_boolean_value, identifier.loc, value);
            }
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION - PRODUCTION RULES
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Note: Production rules below are written in the form
//
//    bool is_success{pr_production_rule_()};
//    return is_success;
//
// This format allows breakpoints to be set following invocation of the production rule.
// Using the concise format
//
//    return pr_production_rule_();
//
// unfortunately makes it impossible to break right after the production rule
// returns.

// <array-suffix> ::= <open-square-bracket> <expression> <opt-enum-bind> <opt-index-capture> <close-square-bracket>
bool Parser_phase_two::pr_array_suffix_() const
{
    const bool is_success{pr_open_square_bracket_()};
    if (is_success) {
        // Parsing of the array-suffix production occurs within the Generative_node_source class.  Within the
        // Parser_phase_two class we simply want to skip over any tokens enclosed by square brackets.
        Parser::skip_past_enclosed_tokens(m_tokenizer, Token_type::close_square_bracket);
    }
    return is_success;
}

// <assert-keyword> ::= assert
bool Parser_phase_two::pr_assert_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::assert_keyword};
    return is_success;
}

// <assert-statement> ::= <assert-keyword> <open-parenthesis> <expression> <close-parenthesis>
bool Parser_phase_two::pr_assert_statement_()
{
    int result{limits::invalid_value};
    const Token& begin_assert = m_tokenizer.peek();
    const bool is_success{
        pr_assert_keyword_() && pr_open_parenthesis_() && pr_expression_(result) && pr_close_parenthesis_()};
    if (is_success && !result) {
        throw make_ex<Parser_error>(fmt::assertion_failed, begin_assert.loc);
    }
    return is_success;
}

// <assignment-operator> ::= =
bool Parser_phase_two::pr_assignment_operator_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::equals};
    return is_success;
}

// <block-or-statement> ::= <definition-statement> | <control-block> | <assert-statement>
bool Parser_phase_two::pr_block_or_statement_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_definition_statement_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_control_block_();
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_assert_statement_();

    return is_success;
}

// <blocks-or-statements> ::= <block-or-statement> <opt-blocks-or-statements>
bool Parser_phase_two::pr_blocks_or_statements_()
{
    const bool is_success{pr_block_or_statement_() && pr_opt_blocks_or_statements_()};
    return is_success;
}

// <bool-type> ::= bool(8|16|32)
bool Parser_phase_two::pr_bool_type_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::bool_type};
    return is_success;
}

// <bracketed-type> ::= <open-angle-bracket> <instantiating-type> <close-angle-bracket>
bool Parser_phase_two::pr_bracketed_type_()
{
    const bool is_success{pr_open_angle_bracket_() && pr_instantiating_type_() && pr_close_angle_bracket_()};
    return is_success;
}

// <bracketed-typename> ::= <open-angle-bracket> <typename> <close-angle-bracket>
bool Parser_phase_two::pr_bracketed_typename_()
{
    const bool is_success{pr_open_angle_bracket_() && pr_typename_() && pr_close_angle_bracket_()};
    return is_success;
}

// <close-angle-bracket> ::= ">"
bool Parser_phase_two::pr_close_angle_bracket_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::close_angle_bracket};
    return is_success;
}

//<close-brace> ::= }
bool Parser_phase_two::pr_close_brace_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::close_brace};
    return is_success;
}

// <close-parenthesis> ::= )
bool Parser_phase_two::pr_close_parenthesis_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::close_parenthesis};
    return is_success;
}

// <close-square-bracket> ::= ]
bool Parser_phase_two::pr_close_square_bracket_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::close_square_bracket};
    return is_success;
}

// <colon> ::= :
bool Parser_phase_two::pr_colon_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::colon};
    return is_success;
}

// <complex-enum-type> ::= <enum-type> <opt-array-suffix>
bool Parser_phase_two::pr_complex_enum_type_()
{
    const bool is_success{pr_enum_type_() && pr_opt_array_suffix_()};
    return is_success;
}

// <complex-integer-type> ::= <integer-type> <opt-array-suffix>
bool Parser_phase_two::pr_complex_integer_type_()
{
    const bool is_success{pr_integer_type_() && pr_opt_array_suffix_()};
    return is_success;
}

// <complex-string-like-type> ::= <string-like-type> <opt-array-suffix>
bool Parser_phase_two::pr_complex_string_like_type_()
{
    const bool is_success{pr_string_like_type_() && pr_opt_array_suffix_()};
    return is_success;
}

// <complex-struct-type> ::= <struct-type> <opt-array-suffix>
bool Parser_phase_two::pr_complex_struct_type_()
{
    const bool is_success{pr_struct_type_() && pr_opt_array_suffix_()};
    return is_success;
}

// <complex-template-type> ::= <template-type> <bracketed-type> <opt-array-suffix>
bool Parser_phase_two::pr_complex_template_type_()
{
    const bool is_success{pr_template_type_() && pr_bracketed_type_() && pr_opt_array_suffix_()};
    return is_success;
}

// <complex-typename-type> ::= <typename-type> <opt-array-suffix>
bool Parser_phase_two::pr_complex_typename_type_()
{
    const bool is_success{pr_typename_type_() && pr_opt_array_suffix_()};
    return is_success;
}

// <conditional_block> ::= <open-brace> <opt-blocks-or-statements> <close-brace>
bool Parser_phase_two::pr_conditional_block_(int condition)
{
    If_context& context{m_if_context_stack.top()};
    if (!condition || context.fulfilled) {
        Parser::skip_past_enclosed_tokens(m_tokenizer, Token_type::open_brace);
        return true;
    }

    context.fulfilled = true;
    const bool is_success{pr_open_brace_() && pr_opt_blocks_or_statements_() && pr_close_brace_()};
    return is_success;
}

// <control-block> ::= <if-elif-else-block> | <for-loop-block>
bool Parser_phase_two::pr_control_block_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_if_elif_else_block_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_for_loop_block_();
    if (is_success) {
        return true;
    }

    return is_success;
}

// <definition-statement> ::= <complex-integer-type> <integer-variable-name> |
//                            <complex-enum-type> <enum-variable-name> |
//                            <complex-string_type-like-type> <string_type-like-variable-name> |
//                            <complex-struct-type> <struct-variable-name> |
//                            <complex-template-type> <template-variable-name>
bool Parser_phase_two::pr_definition_statement_()
{
    const Token& type{m_tokenizer.peek()};

    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_complex_integer_type_() && pr_integer_variable_name_()};
    if (is_success) {
        is_success = emit_nodes_(type, m_tokenizer.previous());
        return is_success;
    }

    m_tokenizer.set_index(index);
    is_success = pr_complex_enum_type_() && pr_enum_variable_name_();
    if (is_success) {
        is_success = emit_nodes_(type, m_tokenizer.previous());
        return is_success;
    }

    m_tokenizer.set_index(index);
    is_success = pr_complex_string_like_type_() && pr_string_like_variable_name_();
    if (is_success) {
        is_success = emit_nodes_(type, m_tokenizer.previous());
        return is_success;
    }

    m_tokenizer.set_index(index);
    is_success = pr_complex_struct_type_() && pr_struct_variable_name_();
    if (is_success) {
        is_success = emit_nodes_(type, m_tokenizer.previous());
        return is_success;
    }

    m_tokenizer.set_index(index);
    {
        m_template_context_stack.emplace();
        const auto_pop<std::stack<Template_context>> ap{m_template_context_stack};

        is_success = pr_complex_template_type_() && pr_template_variable_name_();
        if (is_success) {
            is_success = emit_nodes_(type, m_tokenizer.previous());
            return is_success;
        }
    }

    return false;
}

// <elif-block> ::= <elif-keyword> <open-parenthesis> <if-expression> <close-parenthesis>
//                    <conditional_block>
bool Parser_phase_two::pr_elif_block_()
{
    int condition{limits::invalid_value};
    const bool is_success{pr_elif_keyword_() && pr_open_parenthesis_() && pr_if_expression_(condition)
                          && pr_close_parenthesis_() && pr_conditional_block_(condition)};
    return is_success;
}

// <elif-keyword> ::= elif
bool Parser_phase_two::pr_elif_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::elif_keyword};
    return is_success;
}

// <else-block> ::= <else-keyword> <conditional-block>
bool Parser_phase_two::pr_else_block_()
{
    const bool is_success{pr_else_keyword_() && pr_conditional_block_(1)};
    return is_success;
}

// <else-keyword> ::= else
bool Parser_phase_two::pr_else_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::else_keyword};
    return is_success;
}

// <enum-type> ::= enum(8|16|32)_[a-zA-Z][_a-zA-Z0-9]{0,120}
bool Parser_phase_two::pr_enum_type_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::enum_type};
    return is_success;
}

// <enum-variable-name> ::= <identifier>
bool Parser_phase_two::pr_enum_variable_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// No production rule exists for expression.  Instead, expressions are parsed using the
// Pratt Parsing method within the expression parser class.
bool Parser_phase_two::pr_expression_(int& value)
{
    const bool is_success{Parser::parse_expression(m_expression_parser, m_tokenizer, m_variable_manager, value)};
    return is_success;
}

// <for-assignment> ::= <identifier> <assignment-operator> <expression>
bool Parser_phase_two::pr_for_assignment_()
{
    bool is_success{pr_identifier_()};
    if (!is_success) {
        return false;
    }

    const Token& identifier{m_tokenizer.previous()};
    int value{limits::invalid_value};
    is_success = pr_assignment_operator_() && pr_expression_(value);

    if (is_success) {
        m_variable_manager.add(identifier.value, value);
    }
    return is_success;
}

// <for-continuation> ::= <expression>
bool Parser_phase_two::pr_for_continuation_(bool& continuation_condition)
{
    int result{limits::invalid_value};
    const bool is_success{pr_expression_(result)};
    continuation_condition = result;
    return is_success;
}

// <for-keyword> ::= for
bool Parser_phase_two::pr_for_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::for_keyword};
    return is_success;
}

// <for-loop-block> ::= <for-keyword> <open-parenthesis> <for-assignment> <semicolon>
//                         <for-continuation> <semicolon>
//                         <for-update> <close-parenthesis>
//			   <open-brace> <opt_blocks-or-statements> <close-brace>
bool Parser_phase_two::pr_for_loop_block_()
{
    // Create a variable scope and automatically remove the scope once this
    // production is finished.
    m_variable_manager.push();
    const auto_pop pop_scope{m_variable_manager};

    bool is_success{pr_for_keyword_() && pr_open_parenthesis_() && pr_for_assignment_() && pr_semicolon_()};
    if (!is_success) {
        return false;
    }

    // Remember the for-loop continuation state so that we can return to this state each time we need to
    // evaluate the condition.
    const size_t for_loop_continuation{m_tokenizer.get_index()};

    // Evaluate the condition associated with the for-loop continuation.
    bool continuation_condition{false};
    is_success = pr_for_continuation_(continuation_condition) && pr_semicolon_();
    if (!is_success) {
        return false;
    }

    // Remember the for-loop update state so that we can return to this state each time we need to
    // run the update production.
    const size_t for_loop_update{m_tokenizer.get_index()};

    while (continuation_condition) {
        // The for-loop condition is true, so we want to skip ahead to the for-block and execute it.
        Parser::skip_past_enclosed_tokens(m_tokenizer, Token_type::close_parenthesis);
        is_success = pr_open_brace_() && pr_opt_blocks_or_statements_() && pr_close_brace_();
        if (!is_success) {
            return false;
        }

        // Now that we've run the for-block productions, we need to return to the for-update state
        // and run the update productions.
        m_tokenizer.set_index(for_loop_update);
        is_success = pr_for_update_() && pr_close_parenthesis_();
        if (!is_success) {
            return false;
        }

        // Finally, we return to the for-loop continuation and re-evaluate the continuation condition.
        m_tokenizer.set_index(for_loop_continuation);
        is_success = pr_for_continuation_(continuation_condition) && pr_semicolon_();
        if (!is_success) {
            return false;
        }
    }

    // We're now done with the for-loop, the continuation_condition having failed.  We need to skip past the
    // brace-enclosed for-block to resume parsing.
    Parser::skip_past_enclosed_tokens(m_tokenizer, Token_type::open_brace);
    return true;
}

// <for-update> ::= <identifier> <assignment-operator> <expression>
bool Parser_phase_two::pr_for_update_()
{
    bool is_success{pr_identifier_()};
    if (!is_success) {
        return false;
    }

    const Token& identifier{m_tokenizer.previous()};
    int value{limits::invalid_value};
    is_success = pr_assignment_operator_() && pr_expression_(value);

    if (is_success) {
        // Note: unlike pr_for_assignment_, pr_for_update_ cannot introduce new variables,
        m_variable_manager.set(identifier.value, value);
    }
    return is_success;
}

// <hex-type> ::= hex(8|16|32)
bool Parser_phase_two::pr_hex_type_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::hex_type};
    return is_success;
}

// <identifier> ::= [a-zA-Z][_a-zA-Z0-9]{0,30}
bool Parser_phase_two::pr_identifier_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::identifier};
    return is_success;
}

// <if-block> ::= <if-keyword> <open-parenthesis> <if-expression> <close-parenthesis>
//                    <conditional_block>
bool Parser_phase_two::pr_if_block_()
{
    int condition{limits::invalid_value};
    const bool is_success{pr_if_keyword_() && pr_open_parenthesis_() && pr_if_expression_(condition)
                          && pr_close_parenthesis_() && pr_conditional_block_(condition)};
    return is_success;
}

// if-elif-else-block> ::= <if-block> <opt-elif-blocks> <opt-else-block>
bool Parser_phase_two::pr_if_elif_else_block_()
{
    m_if_context_stack.emplace();
    const auto_pop<std::stack<If_context>> ap{m_if_context_stack};
    const bool is_success{pr_if_block_() && pr_opt_elif_blocks_() && pr_opt_else_block_()};
    return is_success;
}

// <if-expression> ::= <expression>
bool Parser_phase_two::pr_if_expression_(int& value)
{
    // If-expressions should run only if the if-context has not been fulfilled.
    const If_context& context{m_if_context_stack.top()};
    if (context.fulfilled) {
        Parser::skip_past_enclosed_tokens(m_tokenizer, Token_type::close_parenthesis);
        return true;
    }
    const bool is_success{pr_expression_(value)};
    return is_success;
}

// <if-keyword> ::= if
bool Parser_phase_two::pr_if_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::if_keyword};
    return is_success;
}

// <instantiating-type> ::= <integer-type> | <enum-type> | <string_type-like-type> | <struct-type>
bool Parser_phase_two::pr_instantiating_type_()
{
    bool is_success{false};
    do {
        const size_t index{m_tokenizer.get_index()};

        is_success = pr_integer_type_();
        if (is_success) {
            break;
        }

        m_tokenizer.set_index(index);
        is_success = pr_enum_type_();
        if (is_success) {
            break;
        }

        m_tokenizer.set_index(index);
        is_success = pr_string_like_type_();
        if (is_success) {
            break;
        }

        m_tokenizer.set_index(index);
        is_success = pr_struct_type_();
    }
    while (false);

    if (is_success) {
        Template_context& tc{m_template_context_stack.top()};
        tc.instantiating_type = &m_tokenizer.previous();
    }

    return is_success;
}

// <int-type> ::= int(8|16|32)
bool Parser_phase_two::pr_int_type_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::int_type};
    return is_success;
}

// <integer-type> ::= <bool-type> | <hex-type> | <int-type> | <uint-type>
bool Parser_phase_two::pr_integer_type_() const
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_bool_type_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_hex_type_();
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_int_type_();
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_uint_type_();

    return is_success;
}

// <integer-variable-name> ::= <identifier>
bool Parser_phase_two::pr_integer_variable_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <md5-type> ::= md5
bool Parser_phase_two::pr_md5_type_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::md5_type};
    return is_success;
}

// <null> ::= NULL
bool Parser_phase_two::pr_null_()
{
    // NULL always succeeds and does not advance the token stream
    return true;
}

// <open-angle-bracket> ::= "<"
bool Parser_phase_two::pr_open_angle_bracket_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::open_angle_bracket};
    return is_success;
}

// <open-brace> ::= {
bool Parser_phase_two::pr_open_brace_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::open_brace};
    return is_success;
}

// <open-parenthesis> ::= (
bool Parser_phase_two::pr_open_parenthesis_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::open_parenthesis};
    return is_success;
}

// <open-square-bracket> ::= [
bool Parser_phase_two::pr_open_square_bracket_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::open_square_bracket};
    return is_success;
}

// <opt-array-suffix> ::= <array-suffix><opt-array-suffix> | <null>
bool Parser_phase_two::pr_opt_array_suffix_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_array_suffix_() && pr_opt_array_suffix_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_null_();

    return is_success;
}

// <opt-blocks-or-statements> ::= <block-or-statement> <opt-blocks-or-statements> | <null>
bool Parser_phase_two::pr_opt_blocks_or_statements_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_block_or_statement_() && pr_opt_blocks_or_statements_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_null_();

    return is_success;
}

// <opt-elif-blocks> ::= <elif-block> <opt-elif-blocks> | <null>
bool Parser_phase_two::pr_opt_elif_blocks_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_elif_block_() && pr_opt_elif_blocks_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_null_();

    return is_success;
}

// <opt-else-block> ::= <else-block> | <null>
bool Parser_phase_two::pr_opt_else_block_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_else_block_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_null_();

    return is_success;
}

// <opt-template-blocks-or-statements> ::= <template-block-or-statement> <opt-template-blocks-or-statements> | <null>
bool Parser_phase_two::pr_opt_template_blocks_or_statements_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_template_block_or_statement_() && pr_opt_template_blocks_or_statements_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_null_();

    return is_success;
}

// <semicolon> ::= ;
bool Parser_phase_two::pr_semicolon_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::semicolon};
    return is_success;
}

// <string-like-type> ::= <string-type> | <wstring-type> | <md5-type>
bool Parser_phase_two::pr_string_like_type_() const
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_string_type_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_wstring_type_();
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_md5_type_();

    return is_success;
}

// <string-like-variable-name> ::= <identifier>
bool Parser_phase_two::pr_string_like_variable_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <string-type> ::= string
bool Parser_phase_two::pr_string_type_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::string_type};
    return is_success;
}

// <struct-definition> ::= <open-brace> <blocks-or-statements> <close-brace>
bool Parser_phase_two::pr_struct_definition_()
{
    const bool is_success{pr_open_brace_() && pr_blocks_or_statements_() && pr_close_brace_()};
    return is_success;
}

// <struct-type> ::= struct_[a-zA-Z][_a-zA-Z0-9]{0,120}
bool Parser_phase_two::pr_struct_type_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::struct_type};
    return is_success;
}

// <struct-variable-name> ::= <identifier>
bool Parser_phase_two::pr_struct_variable_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <template-block-or-statement> ::= <template-definition-statement> | <control-block> | <assert-statement>
bool Parser_phase_two::pr_template_block_or_statement_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_template_definition_statement_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_control_block_();
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_assert_statement_();

    return is_success;
}

// <template-blocks-or-statements> ::= <template-block-or-statement> <opt-template-blocks-or-statements>
bool Parser_phase_two::pr_template_blocks_or_statements_()
{
    const bool is_success{pr_template_block_or_statement_() && pr_opt_template_blocks_or_statements_()};
    return is_success;
}

// <template-definition> ::= <open-brace> <template-blocks-or-statements> <close-brace>
bool Parser_phase_two::pr_template_definition_()
{
    const bool is_success{pr_open_brace_() && pr_template_blocks_or_statements_() && pr_close_brace_()};
    return is_success;
}

// <template-definition-statement> ::= <complex-typename-type> <typename-variable-name> | <definition-statement>
bool Parser_phase_two::pr_template_definition_statement_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_complex_typename_type_() && pr_typename_variable_name_()};
    if (is_success) {
        // If <complex-typename-type> <typename-variable-name> succeeds it means that we need to
        // process a definition-statement production as if the first token parsed is the
        // instantiating type.  To accomplish this, we restore state, replace the current token
        // with the instantiating type and run the <definition-statement> production.  Note that the
        // replacement remains in force until Generative_node_source finishes its initialization  Generative_node_source
        // uses the replacement to determine what type should be emitted.  At the end of the initialization,
        // Generative_node_source restores the replaced token since it is finished processing the instantiated
        // template.
        m_tokenizer.set_index(index);
        // replace_type_name_token replaces the current token, which must be of type identifier
        // with *m_template_context_stack.top().instantiating_type.  Only one replacement may
        // be in force at a time - a restriction designed to prevent bugs - and, because
        // the processing of an instantiated template does not nest and requires just one
        // replacement.
        m_tokenizer.replace_type_name_token(*m_template_context_stack.top().instantiating_type);
        is_success = pr_definition_statement_();
        return is_success;
    }

    m_tokenizer.set_index(index);
    is_success = pr_definition_statement_();

    return is_success;
}

// <template-type> ::= template_[a-zA-Z][_a-zA-Z0-9]{0,120}
bool Parser_phase_two::pr_template_type_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::template_type};
    return is_success;
}

// <template-variable-name> ::= <identifier>
bool Parser_phase_two::pr_template_variable_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <typename> ::= <identifier>
bool Parser_phase_two::pr_typename_()
{
    const bool is_success{pr_identifier_()};
    if (is_success) {
        Template_context& tc{m_template_context_stack.top()};
        const Token& type_name{m_tokenizer.previous()};
        tc.type_name = &type_name;
    }
    return is_success;
}

// <typename-type> ::= <identifier>
bool Parser_phase_two::pr_typename_type_()
{
    const bool is_success{pr_identifier_()};
    if (is_success) {
        // Verify that the typename matches that from the template context.
        const Token& identifier{m_tokenizer.previous()};
        const Template_context& tc{m_template_context_stack.top()};
        if (tc.type_name->value != identifier.value) {
            throw make_ex<Parser_error>(
                fmt::mismatched_type_names, identifier.loc, tc.type_name->value, identifier.value);
        }
    }
    return is_success;
}

// <typename-variable-name> ::= <identifier>
bool Parser_phase_two::pr_typename_variable_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <uint-type> ::= uint(8|16|32)
bool Parser_phase_two::pr_uint_type_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::uint_type};
    return is_success;
}

// <wstring-type> ::= wstring
bool Parser_phase_two::pr_wstring_type_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::u16string_type};
    return is_success;
}

} // namespace c4lib::schema_parser
