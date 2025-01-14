// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/3/2024.

#pragma once

#include <string>

namespace c4lib::schema_parser {

// NOLINTNEXTLINE(readability-enum-initial-value)
enum class Def_mem_type {
    // N.B.: Def_mem_type is iterable.  For this to work, the first enumerator must have value 0,
    // each succeeding enumerator must increment the previous value by 1, and the helper enumerators
    // "count", "begin" and "end" must not be removed.

    // Invalid value indicative of error.
    invalid = 0,

    alias_type,
    const_type,
    enum_type,
    struct_type,
    template_type,

    // Helper enumerators used for iteration - do not remove
    count,
    begin = 0,
    end = count - 1,
};

const std::string& to_string(Def_mem_type type);

} // namespace c4lib::schema_parser
