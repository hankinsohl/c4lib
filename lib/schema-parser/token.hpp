// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/28/2024.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <lib/schema-parser/token-type.hpp>
#include <lib/util/file-location.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/text.hpp>
#include <string>
#include <utility>

namespace c4lib::schema_parser {

struct Token {
public:
    Token() = default;

    Token(Token_type token_type_, std::string value_)
        : index(limits::invalid_size), type(token_type_), value(std::move(value_))
    {}

    Token(Token_type token_type_, std::string value_, File_location loc_, size_t index_)
        : index(index_), loc(std::move(loc_)), type(token_type_), value(std::move(value_))
    {}

    // Index within the token vector at which this token is found
    size_t index{limits::invalid_size};
    // Location at which the token was found.  Useful for debugging.
    File_location loc;
    Token_type type{Token_type::invalid};
    std::string value;
};

inline std::ostream& operator<<(std::ostream& out, const Token& token)
{
    std::string message{"Token: " + to_string(token.type) + "; Value: " + token.value};
    text::add_location_to_message(message, token.loc);
    return out << message;
}

} // namespace c4lib::schema_parser
