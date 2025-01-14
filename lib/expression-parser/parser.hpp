// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/18/2024.

#pragma once

#include <array>
#include <cstddef>
#include <lib/expression-parser/infix-representation.hpp>
#include <lib/schema-parser/token-type.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/limits.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <stack>
#include <string>

namespace c4lib::expression_parser {

// Parses expressions using Pratt parsing.
class Parser {
public:
    Parser() = default;

    ~Parser() = default;

    Parser(const Parser&) = delete;   
	
    Parser& operator=(const Parser&) = delete;  
	
    Parser(Parser&&) noexcept = delete;   
	
    Parser& operator=(Parser&&) noexcept = delete;    

    // Processes the expression obtained from the tokenizer and returns the result of evaluation.
    // Throws an exception on error.
    int parse(schema_parser::Tokenizer& tokenizer,
        Variable_manager& variable_manager,
        Infix_representation* infix_representation = nullptr);

private:
    using Denotation_func = void (Parser::*)();

    struct Token_info {
        schema_parser::Token_type type{schema_parser::Token_type::invalid};
        int lbp{limits::invalid_value};
        int rbp{limits::invalid_value};
        Denotation_func nud{nullptr};
        Denotation_func led{nullptr};
    };

    void expect_(schema_parser::Token_type token_type) const;

    void expr_(int rbp);

    static Token_info get_token_info_(const schema_parser::Token& token);

    void led_();

    void led_binary_op_();

    void nud_();

    void nud_grouping_();

    void nud_number_();

    void nud_unary_op_();

    void nud_var_or_ref_();

    int pop_()
    {
        const int value{m_stack.top()};
        m_stack.pop();
        return value;
    }

    bool pr_array_node_name_(std::string& path);

    bool pr_array_node_name_or_node_name_(std::string& path);

    bool pr_close_square_bracket_(std::string& path) const;

    bool pr_dot_(std::string& path) const;

    bool pr_enum_name_(std::string& enumerator_reference) const;

    bool pr_enumerator_(std::string& enumerator_reference) const;

    bool pr_enumerator_reference_(std::string& enumerator_reference) const;

    bool pr_expression_(std::string& path);

    bool pr_identifier_(std::string& path) const;

    bool pr_node_name_(std::string& path) const;

    bool pr_node_reference_(std::string& path);

    static bool pr_null_();

    bool pr_open_square_bracket_(std::string& path) const;

    bool pr_opt_path_(std::string& path);

    bool pr_path_separator_(std::string& path) const;

    bool pr_scope_resolution_operator_(std::string& enumerator_reference) const;

    void push_(int value)
    {
        m_stack.push(value);
    }

    Infix_representation* m_infix_representation{nullptr};
    std::stack<int> m_stack;
    schema_parser::Tokenizer* m_tokenizer{nullptr};
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    static std::array<Token_info, 24> token_info_table;
    // Verify that we've set the array size correctly.
    static_assert(token_info_table.size() == static_cast<size_t>(schema_parser::Token_type::meta_expression_eos) + 1);
    Variable_manager* m_variable_manager{nullptr};
};

} // namespace c4lib::expression_parser
