// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/2/2024.

#include <array>
#include <cstddef>
#include <lib/schema-parser/def-mem-type.hpp>
#include <string>
#include <utility>

namespace csp = c4lib::schema_parser;

namespace {

// Note: The lookup table below using name-value pairs allows for bidirectional
// lookup, that is enumerator-to-string as well as string-to-enumerator.
const std::array def_mem_type_names{
    std::pair<csp::Def_mem_type, const std::string>{csp::Def_mem_type::invalid, "invalid"},

    std::pair<csp::Def_mem_type, const std::string>{csp::Def_mem_type::alias_type, "alias_type"},
    std::pair<csp::Def_mem_type, const std::string>{csp::Def_mem_type::const_type, "const_type"},
    std::pair<csp::Def_mem_type, const std::string>{csp::Def_mem_type::enum_type, "enum_type"},
    std::pair<csp::Def_mem_type, const std::string>{csp::Def_mem_type::struct_type, "struct_type"},
    std::pair<csp::Def_mem_type, const std::string>{csp::Def_mem_type::template_type, "template_type"},
};

} // namespace

namespace c4lib::schema_parser {

const std::string& to_string(Def_mem_type type)
{
    return def_mem_type_names.at(static_cast<size_t>(type)).second;
}

} // namespace c4lib::schema_parser
