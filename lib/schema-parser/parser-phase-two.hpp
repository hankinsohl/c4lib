// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/31/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <cstddef>
#include <lib/expression-parser/parser.hpp>
#include <lib/ptree/node-reader.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/limits.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <stack>
#include <string>
#include <unordered_map>

namespace c4lib::property_tree {
class Generative_node_source;
}

namespace c4lib::schema_parser {

class Parser_phase_two {
    friend class auto_parent;

    friend class c4lib::property_tree::Generative_node_source;

public:
    Parser_phase_two(Tokenizer& tokenizer,
        Def_tbl& def_tbl,
        size_t root_name_index,
        Variable_manager& variable_manager,
        boost::property_tree::ptree& ptree_root,
        c4lib::property_tree::Node_reader& node_reader,
        std::unordered_map<std::string, std::string>& options);

    ~Parser_phase_two() = default;

    Parser_phase_two(const Parser_phase_two&) = delete;

    Parser_phase_two& operator=(const Parser_phase_two&) = delete;

    Parser_phase_two(Parser_phase_two&&) noexcept = delete;

    Parser_phase_two& operator=(Parser_phase_two&&) noexcept = delete;

    // The phase two parser is responsible for generating a property tree reflective of the Beyond the Sword
    // save.  During phase two the save is read and decompressed into memory.  The parser then processes
    // the schema, starting at the root structure to generate nodes for the property tree.
    void parse();

private:
    class auto_parent {
    public:
        explicit auto_parent(Parser_phase_two* parser, boost::property_tree::ptree* ptree_new_parent)
            : m_old_parent(parser->m_ptree_parent), m_parser(parser)
        {
            parser->m_ptree_parent = ptree_new_parent;
        }

        ~auto_parent()
        {
            m_parser->m_ptree_parent = m_old_parent;
        }

        auto_parent(const auto_parent&) = delete;

        auto_parent& operator=(const auto_parent&) = delete;

        auto_parent(auto_parent&&) noexcept = delete;

        auto_parent& operator=(auto_parent&&) noexcept = delete;

    private:
        boost::property_tree::ptree* m_old_parent{nullptr};
        Parser_phase_two* m_parser{nullptr};
    };

    struct If_context {
        // True if a conditional block for the current if-elif-else construct has
        // been processed, otherwise false.
        bool fulfilled{false};
    };

    struct Template_context {
        // Pointer to the type-name token for the template
        const Token* type_name{nullptr};
        // Pointer to the token for the instantiating type for the template
        const Token* instantiating_type{nullptr};
    };

    bool emit_nodes_(const Token& type, const Token& identifier);

    bool pr_array_suffix_() const;

    bool pr_assert_keyword_() const;

    bool pr_assert_statement_();

    bool pr_assignment_operator_() const;

    bool pr_block_or_statement_();

    bool pr_blocks_or_statements_();

    bool pr_bool_type_() const;

    bool pr_bracketed_type_();

    bool pr_bracketed_typename_();

    bool pr_close_angle_bracket_() const;

    bool pr_close_brace_() const;

    bool pr_close_parenthesis_() const;

    bool pr_close_square_bracket_() const;

    bool pr_colon_() const;

    bool pr_complex_enum_type_();

    bool pr_complex_integer_type_();

    bool pr_complex_string_like_type_();

    bool pr_complex_struct_type_();

    bool pr_complex_template_type_();

    bool pr_complex_typename_type_();

    bool pr_conditional_block_(int condition);

    bool pr_control_block_();

    bool pr_definition_statement_();

    bool pr_elif_block_();

    bool pr_elif_keyword_() const;

    bool pr_else_block_();

    bool pr_else_keyword_() const;

    bool pr_enum_type_() const;

    bool pr_enum_variable_name_() const;

    bool pr_expression_(int& value);

    bool pr_for_assignment_();

    bool pr_for_continuation_(bool& continuation_condition);

    bool pr_for_keyword_() const;

    bool pr_for_loop_block_();

    bool pr_for_update_();

    bool pr_hex_type_() const;

    bool pr_identifier_() const;

    bool pr_if_block_();

    bool pr_if_elif_else_block_();

    bool pr_if_expression_(int& value);

    bool pr_if_keyword_() const;

    bool pr_instantiating_type_();

    bool pr_int_type_() const;

    bool pr_integer_type_() const;

    bool pr_integer_variable_name_() const;

    bool pr_md5_type_() const;

    static bool pr_null_();

    bool pr_open_angle_bracket_() const;

    bool pr_open_brace_() const;

    bool pr_open_parenthesis_() const;

    bool pr_open_square_bracket_() const;

    bool pr_opt_array_suffix_();

    bool pr_opt_blocks_or_statements_();

    bool pr_opt_elif_blocks_();

    bool pr_opt_else_block_();

    bool pr_opt_template_blocks_or_statements_();

    bool pr_semicolon_() const;

    bool pr_string_like_type_() const;

    bool pr_string_like_variable_name_() const;

    bool pr_string_type_() const;

    bool pr_struct_definition_();

    bool pr_struct_type_() const;

    bool pr_struct_variable_name_() const;

    bool pr_template_block_or_statement_();

    bool pr_template_blocks_or_statements_();

    bool pr_template_definition_();

    bool pr_template_definition_statement_();

    bool pr_template_type_() const;

    bool pr_template_variable_name_() const;

    bool pr_typename_();

    bool pr_typename_type_();

    bool pr_typename_variable_name_() const;

    bool pr_uint_type_() const;

    bool pr_wstring_type_() const;

    Def_tbl& m_definition_table;
    c4lib::expression_parser::Parser m_expression_parser;
    std::stack<If_context> m_if_context_stack;
    c4lib::property_tree::Node_reader& m_node_reader;
    std::unordered_map<std::string, std::string>& m_options;
    boost::property_tree::ptree* m_ptree_parent{nullptr};
    boost::property_tree::ptree& m_ptree_root;
    size_t m_root_name_index{limits::invalid_size};
    std::stack<Template_context> m_template_context_stack;
    Tokenizer& m_tokenizer;
    Variable_manager& m_variable_manager;
};

} // namespace c4lib::schema_parser
