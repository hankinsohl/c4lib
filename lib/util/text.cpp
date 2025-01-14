// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Passenger on 6/27/2024.

#include <cctype>
#include <lib/util/constants.hpp>
#include <lib/util/file-location.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/text.hpp>
#include <string>
#include <utf8.h>

namespace c4lib::text {

void add_location_to_message(std::string& message, const File_location& loc)
{
    const int endColumn{get_end_column(loc.line->substr(0, loc.character_number - 1), 1, constants::tab_width)};
    message = *loc.filename + ":" + std::to_string(loc.line_number) + ":" + std::to_string(endColumn) + ": " + message
              + "\n";
    message += constants::message_indent + *loc.line + "\n";
    message += constants::message_indent + std::string(gsl::narrow<size_t>(endColumn - 1), ' ') + "^~~~~~~";
}

int get_end_column(const std::string& text, int start_column, int tab_width)
{
    int end_column{start_column};
    for (auto c : text) {
        if (c == '\t') {
            end_column += (tab_width - (end_column - 1) % tab_width);
        }
        else {
            end_column += 1;
        }
    }

    return end_column;
}

std::string screaming_snake_case(const std::string& name)
{
    std::string s;
    bool is_prev_lower(false);
    for (const char c : name) {
        if (std::islower(gsl::narrow<unsigned char>(c))) {
            is_prev_lower = true;
        }
        else if (std::isupper(gsl::narrow<unsigned char>(c))) {
            if (is_prev_lower) {
                is_prev_lower = false;
                s.push_back('_');
            }
        }
        s.push_back(gsl::narrow<char>(std::toupper(gsl::narrow<unsigned char>(c))));
    }

    return s;
}

std::u16string string_to_u16string(const std::string& string)
{
    return utf8::utf8to16(string);
}

std::string u16string_to_string(const std::u16string& u16string)
{
    return utf8::utf16to8(u16string);
}

} // namespace c4lib::text
