// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/3/2024.
// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.

#pragma once

#include <lib/schema-parser/def-mem-type.hpp>
#include <lib/util/file-location.hpp>
#include <lib/util/limits.hpp>
#include <string>
#include <utility>

namespace c4lib::schema_parser {

struct Def_mem {
    Def_mem(Def_mem_type type_, std::string name_, int value_, File_location loc_)
        : loc(std::move(loc_)), name(std::move(name_)), type(type_), value(value_)
    {}

    // Used to sort definitions in ascending order based on value.
    bool operator<(const Def_mem& rhs) const
    {
        return value < rhs.value;
    }

    // Location at which the definition was found.  Useful for debugging and error messages.
    File_location loc;
    std::string name;
    Def_mem_type type{Def_mem_type::invalid};
    // For consts and enumerators, value is the value of the type.  For structs and templates,
    // value is an index into the token vector where the definition is found.
    int value{limits::invalid_value};
};

} // namespace c4lib::schema_parser
