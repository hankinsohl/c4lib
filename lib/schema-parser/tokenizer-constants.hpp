// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/2/2024.

#pragma once

#include <array>
#include <lib/schema-parser/token-type.hpp>
#include <lib/util/limits.hpp>
#include <string>
#include <utility>

// Constants below are used by both the Tokenizer class and various test classes.
namespace c4lib::schema_parser {

const std::string identifier_regex{"[_a-zA-Z][_a-zA-Z0-9]{0," + std::to_string(limits::max_identifier_length) + "}"};
const std::string length_suffix_regex{"(8|16|32)"};

// Base types are identified using regular expressions.  A simple hash table lookup
// cannot be used to tokenize these token types.
const std::array base_types{
    std::pair<const std::string, Token_type>{"^bool" + length_suffix_regex, Token_type::bool_type},
    std::pair<const std::string, Token_type>{"^hex" + length_suffix_regex, Token_type::hex_type},
    std::pair<const std::string, Token_type>{"^int" + length_suffix_regex, Token_type::int_type},
    std::pair<const std::string, Token_type>{"^uint" + length_suffix_regex, Token_type::uint_type},

    std::pair<const std::string, Token_type>{"^string", Token_type::string_type},
    std::pair<const std::string, Token_type>{"^wstring", Token_type::u16string_type},

    std::pair<const std::string, Token_type>{"^md5", Token_type::md5_type},

    std::pair<const std::string, Token_type>{
        "^enum" + length_suffix_regex + "_" + identifier_regex, Token_type::enum_type},
    std::pair<const std::string, Token_type>{"^struct_" + identifier_regex, Token_type::struct_type},
    std::pair<const std::string, Token_type>{"^template_" + identifier_regex, Token_type::template_type},
};

const std::array keywords{
    std::pair<const std::string, Token_type>{"alias", Token_type::alias_keyword},
    std::pair<const std::string, Token_type>{"assert", Token_type::assert_keyword},
    std::pair<const std::string, Token_type>{"capture_index", Token_type::capture_index_keyword},
    std::pair<const std::string, Token_type>{"const", Token_type::const_keyword},
    std::pair<const std::string, Token_type>{"elif", Token_type::elif_keyword},
    std::pair<const std::string, Token_type>{"else", Token_type::else_keyword},
    std::pair<const std::string, Token_type>{"enum", Token_type::enum_keyword},
    std::pair<const std::string, Token_type>{"exact_path", Token_type::exact_path_keyword},
    std::pair<const std::string, Token_type>{"for", Token_type::for_keyword},
    std::pair<const std::string, Token_type>{"if", Token_type::if_keyword},
    std::pair<const std::string, Token_type>{"import", Token_type::import_keyword},
    std::pair<const std::string, Token_type>{"read", Token_type::read_keyword},
    std::pair<const std::string, Token_type>{"search_path", Token_type::search_path_keyword},
    std::pair<const std::string, Token_type>{"struct", Token_type::struct_keyword},
    std::pair<const std::string, Token_type>{"template", Token_type::template_keyword},
    std::pair<const std::string, Token_type>{"use_capture", Token_type::use_capture_keyword},
    std::pair<const std::string, Token_type>{"xml_path", Token_type::xml_path_keyword},
    std::pair<const std::string, Token_type>{"query_reader", Token_type::query_reader_keyword},
};

const std::array ambiguous_tokens{
    std::pair<const std::string, Token_type>{"<", Token_type::open_angle_bracket},
    std::pair<const std::string, Token_type>{">", Token_type::close_angle_bracket},
    std::pair<const std::string, Token_type>{"-", Token_type::minus},
    std::pair<const std::string, Token_type>{"+", Token_type::plus},
};

const std::array operators{
    std::pair<const std::string, Token_type>{"=", Token_type::equals},

    std::pair<const std::string, Token_type>{"+", Token_type::plus}, // Unary plus or addition
    std::pair<const std::string, Token_type>{"-", Token_type::minus}, // Unary minus or subtraction

    std::pair<const std::string, Token_type>{"*", Token_type::asterisk},
    std::pair<const std::string, Token_type>{"/", Token_type::slash},
    std::pair<const std::string, Token_type>{"%", Token_type::percent},

    std::pair<const std::string, Token_type>{"&&", Token_type::double_ampersand},
    std::pair<const std::string, Token_type>{"||", Token_type::double_bar},
    std::pair<const std::string, Token_type>{"!", Token_type::bang},

    std::pair<const std::string, Token_type>{"<", Token_type::open_angle_bracket}, // < or start typename specification
    std::pair<const std::string, Token_type>{"<=", Token_type::open_angle_equals},
    std::pair<const std::string, Token_type>{"==", Token_type::double_equals},
    std::pair<const std::string, Token_type>{">=", Token_type::close_angle_equals},
    std::pair<const std::string, Token_type>{">", Token_type::close_angle_bracket}, // > or end typename specification
    std::pair<const std::string, Token_type>{"!=", Token_type::bang_equals},

    std::pair<const std::string, Token_type>{"::", Token_type::double_colon},
};

const std::array punctuation{
    std::pair<const std::string, Token_type>{".", Token_type::dot},

    std::pair<const std::string, Token_type>{":", Token_type::colon},
    std::pair<const std::string, Token_type>{";", Token_type::semicolon},

    std::pair<const std::string, Token_type>{"<", Token_type::open_angle_bracket},
    std::pair<const std::string, Token_type>{">", Token_type::close_angle_bracket},

    std::pair<const std::string, Token_type>{"{", Token_type::open_brace},
    std::pair<const std::string, Token_type>{"}", Token_type::close_brace},

    std::pair<const std::string, Token_type>{"[", Token_type::open_square_bracket},
    std::pair<const std::string, Token_type>{"]", Token_type::close_square_bracket},

    std::pair<const std::string, Token_type>{"(", Token_type::open_parenthesis},
    std::pair<const std::string, Token_type>{")", Token_type::close_parenthesis},
};

} // namespace c4lib::schema_parser
