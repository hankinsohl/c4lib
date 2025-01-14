// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/27/2024.

#pragma once

#include <string>
#include <utility>

namespace c4lib::schema_parser {

// NOLINTNEXTLINE(readability-enum-initial-value)
enum class Token_type {
    // N.B.: Token_type is iterable.  For this to work, the first enumerator must have value 0,
    // each succeeding enumerator must increment the previous value by 1, and the helper enumerators
    // "count", "begin" and "end" must not be removed.

    // Invalid value indicative of error.
    invalid = 0,

    // Token types that may appear as part of an expression are listed first.  This facilitates use of the
    // enumerator constant as a lookup value into a table of token interpretations.

    // *** BEGIN EXPRESSION TOKENS ************************************************************************************

    // Numeric literal
    numeric_literal,

    equals,

    // Arithmetic operators
    minus, // Subtraction or unary minus
    plus, // Addition or unary plus
    asterisk, // Multiplication
    slash, // Division
    percent, // Modulo operator

    // Logical operators
    double_ampersand, // Logical and
    double_bar, // Logical or
    bang, // Logical not

    // Comparison operators
    open_angle_bracket, // Less than operator or start template typename specification
    open_angle_equals, // Less than or equal to
    double_equals, // Equal to
    bang_equals, // Not equal to
    close_angle_equals, // Greater than or equal to
    close_angle_bracket, // Greater than operator or end template typename specification

    // Miscellaneous operators
    double_colon, // Scope resolution

    // Function call/grouping
    open_parenthesis, // Start group or function call
    close_parenthesis, // End group or function call

    // Identifier-related token types
    function_name,
    identifier,
    dot, // Structure/template member resolution operator

    // Meta token denoting end-of-stream for expressions
    meta_expression_eos,

    // *** END EXPRESSION TOKENS *************************************************************************************

    // Keywords
    alias_keyword,

    assert_keyword,

    import_keyword,

    const_keyword,
    enum_keyword,

    struct_keyword,
    template_keyword,

    if_keyword,
    elif_keyword,
    else_keyword,

    for_keyword,

    read_keyword,

    capture_index_keyword,
    use_capture_keyword,

    exact_path_keyword,
    search_path_keyword,
    xml_path_keyword,

    query_reader_keyword,

    // Integer types
    bool_type,
    hex_type,
    int_type,
    uint_type,
    enum_type,

    // String types
    string_type,
    u16string_type,
    md5_type,

    // Compound types
    struct_type,
    template_type,

    string_literal, // String literals are enclosed in double quotes.  The string literal token
    // excludes the enclosing quotation marks

    double_slash, // Comment

    // Miscellaneous punctuation
    colon, // Array index element separator
    semicolon, // For-loop element separator

    open_square_bracket, // Begin array dimension
    close_square_bracket, // End array dimension

    open_brace, // Begin block
    close_brace, // End block

    // Meta-token used to indicate the end of the token stream.
    meta_eos,

    // Helper enumerators used for iteration - do not remove
    count,
    begin = 0,
    end = count - 1,
};

std::string enum_name_from_type(const std::string& type);

const std::pair<Token_type, Token_type>& get_punc_pair(Token_type punc);

const std::string& to_string(Token_type type);

std::string token_type_to_node_type_as_string(Token_type type);

} // namespace c4lib::schema_parser
