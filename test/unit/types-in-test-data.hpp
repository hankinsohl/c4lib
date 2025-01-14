// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/6/2024.

#pragma once

#include <array>
#include <lib/schema-parser/token-type.hpp>
#include <string>
#include <utility>

namespace c4lib::schema_parser {
const std::array types_in_test{
    std::pair<const std::string, Token_type>{"bool8", Token_type::bool_type},
    std::pair<const std::string, Token_type>{"bool16", Token_type::bool_type},
    std::pair<const std::string, Token_type>{"bool32", Token_type::bool_type},

    std::pair<const std::string, Token_type>{"hex8", Token_type::hex_type},
    std::pair<const std::string, Token_type>{"hex16", Token_type::hex_type},
    std::pair<const std::string, Token_type>{"hex32", Token_type::hex_type},

    std::pair<const std::string, Token_type>{"int8", Token_type::int_type},
    std::pair<const std::string, Token_type>{"int16", Token_type::int_type},
    std::pair<const std::string, Token_type>{"int32", Token_type::int_type},

    std::pair<const std::string, Token_type>{"uint8", Token_type::uint_type},
    std::pair<const std::string, Token_type>{"uint16", Token_type::uint_type},
    std::pair<const std::string, Token_type>{"uint32", Token_type::uint_type},

    std::pair<const std::string, Token_type>{"string", Token_type::string_type},
    std::pair<const std::string, Token_type>{"wstring", Token_type::u16string_type},

    std::pair<const std::string, Token_type>{"md5", Token_type::md5_type},

    std::pair<const std::string, Token_type>{"enum8_Enumeration", Token_type::enum_type},
    std::pair<const std::string, Token_type>{"enum16_Enumeration", Token_type::enum_type},
    std::pair<const std::string, Token_type>{"enum32_Enumeration", Token_type::enum_type},

    std::pair<const std::string, Token_type>{"struct_Structure", Token_type::struct_type},
    std::pair<const std::string, Token_type>{"template_Template", Token_type::template_type},
};

}
