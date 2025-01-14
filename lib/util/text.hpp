// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/1/2024.

#pragma once

#include <lib/util/file-location.hpp>
#include <string>

namespace c4lib::text {

void add_location_to_message(std::string& message, const File_location& loc);

// Returns the end column for text by expanding tabs using tab_width.  The returned value includes the start column.
int get_end_column(const std::string& text, int start_column, int tab_width);

// Returns SCREAMING_SNAKE_CASE for name which may be in camelCase or PascalCase.
std::string screaming_snake_case(const std::string& name);

std::u16string string_to_u16string(const std::string& string);

std::string u16string_to_string(const std::u16string& u16string);

} // namespace c4lib::text
