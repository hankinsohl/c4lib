// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/2/2024.

#include <array>
#include <cstddef>
#include <include/node-type.hpp>
#include <lib/schema-parser/token-type.hpp>
#include <string>
#include <unordered_map>
#include <utility>

namespace cpt = c4lib::property_tree;
namespace csp = c4lib::schema_parser;

namespace {
const std::unordered_map<csp::Token_type, std::pair<csp::Token_type, csp::Token_type>> punctuation_pairs{
    {csp::Token_type::open_angle_bracket,
        {
            csp::Token_type::open_angle_bracket,
            csp::Token_type::close_angle_bracket,
        }},
    {csp::Token_type::close_angle_bracket,
        {
            csp::Token_type::open_angle_bracket,
            csp::Token_type::close_angle_bracket,
        }},

    {csp::Token_type::open_brace,
        {
            csp::Token_type::open_brace,
            csp::Token_type::close_brace,
        }},
    {csp::Token_type::close_brace,
        {
            csp::Token_type::open_brace,
            csp::Token_type::close_brace,
        }},

    {csp::Token_type::open_parenthesis,
        {
            csp::Token_type::open_parenthesis,
            csp::Token_type::close_parenthesis,
        }},
    {csp::Token_type::close_parenthesis,
        {
            csp::Token_type::open_parenthesis,
            csp::Token_type::close_parenthesis,
        }},

    {csp::Token_type::open_square_bracket,
        {
            csp::Token_type::open_square_bracket,
            csp::Token_type::close_square_bracket,
        }},
    {csp::Token_type::close_square_bracket,
        {
            csp::Token_type::open_square_bracket,
            csp::Token_type::close_square_bracket,
        }},
};

// Note: The lookup table below using name-value pairs allows for bidirectional
// lookup, that is enumerator-to-string as well as string-to-enumerator.
const std::array token_type_names{
    std::pair<csp::Token_type, std::string>{csp::Token_type::invalid, "invalid"},

    // *** BEGIN EXPRESSION TOKENS ***
    // Numeric literal
    std::pair<csp::Token_type, std::string>{csp::Token_type::numeric_literal, "numeric_literal"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::equals, "equals"},

    // Arithmetic operators
    std::pair<csp::Token_type, std::string>{csp::Token_type::minus, "-"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::plus, "+"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::asterisk, "*"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::slash, "/"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::percent, "%"},

    // Logical operators
    std::pair<csp::Token_type, std::string>{csp::Token_type::double_ampersand, "&&"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::double_bar, "||"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::bang, "!"},

    // Comparison operators
    std::pair<csp::Token_type, std::string>{csp::Token_type::open_angle_bracket, "<"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::open_angle_equals, "<="},
    std::pair<csp::Token_type, std::string>{csp::Token_type::double_equals, "=="},
    std::pair<csp::Token_type, std::string>{csp::Token_type::bang_equals, "!="},
    std::pair<csp::Token_type, std::string>{csp::Token_type::close_angle_equals, ">="},
    std::pair<csp::Token_type, std::string>{csp::Token_type::close_angle_bracket, ">"},

    // Miscellaneous operators
    std::pair<csp::Token_type, std::string>{csp::Token_type::double_colon, "::"},

    // Function call/grouping
    std::pair<csp::Token_type, std::string>{csp::Token_type::open_parenthesis, "("},
    std::pair<csp::Token_type, std::string>{csp::Token_type::close_parenthesis, ")"},

    // Identifier-related token types
    std::pair<csp::Token_type, std::string>{csp::Token_type::function_name, "function_name"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::identifier, "identifier"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::dot, "."},

    std::pair<csp::Token_type, std::string>{csp::Token_type::meta_expression_eos, "meta_expression_eos"},

    // *** END EXPRESSION TOKENS ***

    // Keywords
    std::pair<csp::Token_type, std::string>{csp::Token_type::alias_keyword, "alias_keyword"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::assert_keyword, "assert_keyword"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::import_keyword, "import_keyword"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::const_keyword, "const_keyword"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::enum_keyword, "enum_keyword"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::struct_keyword, "struct_keyword"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::template_keyword, "template_keyword"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::if_keyword, "if_keyword"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::elif_keyword, "elif_keyword"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::else_keyword, "else_keyword"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::for_keyword, "for_keyword"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::read_keyword, "read_keyword"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::capture_index_keyword, "capture_index_keyword"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::use_capture_keyword, "use_capture_keyword"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::exact_path_keyword, "exact_path_keyword"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::search_path_keyword, "search_path_keyword"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::xml_path_keyword, "xml_path_keyword"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::query_reader_keyword, "query_reader_keyword"},

    // Integer types
    std::pair<csp::Token_type, std::string>{csp::Token_type::bool_type, "bool_type"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::hex_type, "hex_type"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::int_type, "int_type"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::uint_type, "uint_type"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::enum_type, "enum_type"},

    // String types
    std::pair<csp::Token_type, std::string>{csp::Token_type::string_type, "string_type"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::u16string_type, "u16string_type"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::md5_type, "md5_type"},

    // Compound types
    std::pair<csp::Token_type, std::string>{csp::Token_type::struct_type, "struct_type"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::template_type, "template_type"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::string_literal, "string_literal"},

    // Comment
    std::pair<csp::Token_type, std::string>{csp::Token_type::double_slash, "//"},

    // Miscellaneous punctuation
    std::pair<csp::Token_type, std::string>{csp::Token_type::colon, ":"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::semicolon, ";"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::open_square_bracket, "["},
    std::pair<csp::Token_type, std::string>{csp::Token_type::close_square_bracket, "]"},

    std::pair<csp::Token_type, std::string>{csp::Token_type::open_brace, "{"},
    std::pair<csp::Token_type, std::string>{csp::Token_type::close_brace, "}"},

    // Meta-token used to indicate the end of the token stream.
    std::pair<csp::Token_type, std::string>{csp::Token_type::meta_eos, "meta_eos"},
};
} // namespace

namespace c4lib::schema_parser {

const std::pair<Token_type, Token_type>& get_punc_pair(Token_type punc)
{
    return punctuation_pairs.at(punc);
}

const std::string& to_string(Token_type type)
{
    const size_t index{static_cast<size_t>(type)};
    return token_type_names.at(index).second;
}

std::string token_type_to_node_type_as_string(Token_type type)
{
    // Ensure that order of Token_type enumerators matches that of Node_type.
    constexpr int offset{static_cast<int>(Token_type::bool_type) - static_cast<int>(cpt::Node_type::bool_type)};

    static_assert(static_cast<int>(Token_type::bool_type) == static_cast<int>(cpt::Node_type::bool_type) + offset);
    static_assert(static_cast<int>(Token_type::hex_type) == static_cast<int>(cpt::Node_type::hex_type) + offset);
    static_assert(static_cast<int>(Token_type::int_type) == static_cast<int>(cpt::Node_type::int_type) + offset);
    static_assert(static_cast<int>(Token_type::uint_type) == static_cast<int>(cpt::Node_type::uint_type) + offset);
    static_assert(static_cast<int>(Token_type::enum_type) == static_cast<int>(cpt::Node_type::enum_type) + offset);

    static_assert(static_cast<int>(Token_type::string_type) == static_cast<int>(cpt::Node_type::string_type) + offset);
    static_assert(
        static_cast<int>(Token_type::u16string_type) == static_cast<int>(cpt::Node_type::u16string_type) + offset);
    static_assert(static_cast<int>(Token_type::md5_type) == static_cast<int>(cpt::Node_type::md5_type) + offset);

    static_assert(static_cast<int>(Token_type::struct_type) == static_cast<int>(cpt::Node_type::struct_type) + offset);
    static_assert(
        static_cast<int>(Token_type::template_type) == static_cast<int>(cpt::Node_type::template_type) + offset);

    // The above assertions ensure that we can use simple arithmetic to convert a Token_type to Node_type.
    const cpt::Node_type node_type{static_cast<int>(type) - offset};
    return cpt::node_type_as_string(node_type);
}
} // namespace c4lib::schema_parser
