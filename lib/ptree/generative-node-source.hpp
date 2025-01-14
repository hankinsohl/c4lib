// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/10/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <cstddef>
#include <include/exceptions.hpp>
#include <lib/schema-parser/parser-phase-two.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <memory>
#include <string>
#include <vector>

namespace c4lib::property_tree {

// Generative_node_source encapsulates the information required to emit a property tree node.
class Generative_node_source {
public:
    friend class iterator;

    Generative_node_source(schema_parser::Parser_phase_two& parser,
        const schema_parser::Token& type,
        const schema_parser::Token& identifier);

    ~Generative_node_source() = default;

    Generative_node_source(const Generative_node_source&) = delete;

    Generative_node_source& operator=(const Generative_node_source&) = delete;

    Generative_node_source(Generative_node_source&&) noexcept = delete;

    Generative_node_source& operator=(Generative_node_source&&) noexcept = delete;

    class iterator {
    public:
        iterator() = default;

        explicit iterator(Generative_node_source* ns)
            : m_ns(ns), m_ptree(ns == nullptr ? nullptr : ns->m_parser.m_ptree_parent)
        {
            if (ns == nullptr) {
                throw std::invalid_argument{fmt::null_pointer_error};
            }

            // Call next_ to initialize the tree.
            if (!m_ns->next_(m_ptree)) {
                throw make_ex<Node_source_error>(
                    fmt::node_source_error, m_ns->m_identifier.loc, m_ns->m_identifier.value);
            }
        }

        iterator& operator++()
        {
            if (!m_ns->next_(m_ptree)) {
                throw make_ex<Node_source_error>(
                    fmt::node_source_error, m_ns->m_identifier.loc, m_ns->m_identifier.value);
            }
            return *this;
        }

        bool operator!=(const iterator& rhs) const
        {
            return m_ptree != rhs.m_ptree;
        }

        boost::property_tree::ptree& operator*() const
        {
            return *m_ptree;
        }

    private:
        Generative_node_source* m_ns{nullptr};
        boost::property_tree::ptree* m_ptree{nullptr};
    };

    iterator begin()
    {
        return iterator{this};
    }

    static iterator end()
    {
        return iterator{};
    }

private:
    // Dimension_nodes form a linked tree structure representing both simple types and
    // arrays of a type.  Branch nodes correspond to array dimensions whose size
    // is given by the size of the associated member nodes vector.  Leaf nodes correspond
    // to instances of the type associated with the array.
    struct Dimension_node {
        // Pointer to parent of this node.  Nullptr if this node is the root.
        Dimension_node* parent{nullptr};
        // ptree associated with this node.
        boost::property_tree::ptree* ptree{nullptr};
        // Index into the nodes vector used to iterate over the dimension
        size_t index{limits::invalid_size};
        // If the nodes vector is size 0, the node is a leaf and represents a type.  Otherwise,
        // the node represents an array dimension whose size is the size of the nodes vector.
        // Each entry in the nodes vector is a child node for the corresponding dimension
        // index.
        std::vector<Dimension_node> nodes;
    };

    // Initializes the tree associated with m_type.
    bool init_();

    bool init_node_(Dimension_node* node,
        Dimension_node* parent,
        boost::property_tree::ptree* ptree_parent,
        size_t bracket_token_index,
        size_t array_subscript,
        const std::string& array_name,
        const std::string& cumulative_subscript_string);

    [[nodiscard]] bool is_query_reader_production_(size_t bracket_token_index) const;

    [[nodiscard]] bool is_use_capture_production_(size_t bracket_token_index) const;

    // The next_ function is used to support ranged-for iteration over the node source.  The function
    // is private because it is an implementation detail of iteration.
    //
    // Gets the next ptree node for the type passed to the ctor, and stores a pointer to the node
    // in ptree.  If no additional nodes exist for the type, nullptr is returned.  If an error occurs,
    // false is returned.
    //
    // In the case of arrays, next iterates over each array member in ascending subscript order.  It also
    // generates  and links fully-formed array nodes so that array navigation can take the form
    // ".identifier_name.[1].[3]" for example (for the [1][3] member of the 2-dimensional array).  Finally,
    // it creates a child __Attributes__ node to serve as the root for attributes of the type.
    bool next_(boost::property_tree::ptree*& ptree);

    static boost::property_tree::ptree* next_(Dimension_node& node, bool& increment_caller_index);

    bool parse_dimension_info_(size_t bracket_token_index,
        size_t& next_bracket_token_index,
        bool& is_array,
        size_t& dimension_size,
        std::string& enum_name,
        bool& is_capture) const;

    bool pr_array_suffix_(size_t bracket_token_index,
        size_t& next_bracket_token_index,
        size_t& dimension_size,
        std::string& enum_name,
        bool& is_capture) const;

    bool pr_array_suffix_standard_(size_t bracket_token_index,
        size_t& next_bracket_token_index,
        size_t& dimension_size,
        std::string& enum_name,
        bool& is_capture) const;

    bool pr_array_suffix_with_query_reader_(
        size_t bracket_token_index, size_t& next_bracket_token_index, size_t& dimension_size) const;

    bool pr_array_suffix_with_use_capture_(
        size_t bracket_token_index, size_t& next_bracket_token_index, size_t& dimension_size, std::string& enum_name) const;

    [[nodiscard]] bool pr_capture_index_keyword_() const;

    bool pr_enum_name_(std::string& enum_name) const;

    [[nodiscard]] bool pr_node_name_() const;

    bool pr_index_capture_(bool& is_capture) const;

    bool pr_opt_enum_bind_(std::string& enum_name) const;

    bool pr_opt_index_capture_(bool& is_capture) const;

    bool pr_query_reader_keyword_(int& value) const;

    [[nodiscard]] bool pr_use_capture_keyword_() const;

    bool pr_use_capture_node_reference_(int& value) const;

    size_t m_captured_index{limits::invalid_size};
    const schema_parser::Token& m_identifier;
    schema_parser::Parser_phase_two& m_parser;
    std::unique_ptr<Dimension_node> m_root;
    schema_parser::Tokenizer& m_tokenizer;
    const schema_parser::Token& m_type;
};

} // namespace c4lib::property_tree
