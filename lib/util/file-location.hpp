// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/28/2024.

#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

namespace c4lib {

struct File_location {
public:
    File_location() = default;

    File_location(std::shared_ptr<const std::string> filename_,
        std::shared_ptr<const std::string> line_,
        size_t line_number_,
        size_t column_number_)
        : filename(std::move(filename_)),
          line(std::move(line_)),
          line_number(line_number_),
          character_number(column_number_)
    {}

    // Files and lines are stored using shared pointers to allow sharing of file and line values.
    std::shared_ptr<const std::string> filename{std::make_shared<std::string>("")};
    std::shared_ptr<const std::string> line{std::make_shared<std::string>("")};
    size_t line_number{1};
    // N.B.: FileLocation tracks the character number within a line, with the first character number = 1.  To convert
    // this to a column number, it is necessary to add number of tabs to left * (tab-width - 1).
    size_t character_number{1};
};

inline std::string to_string(const File_location& loc)
{
    return std::string{
        *loc.filename + ":" + std::to_string(loc.line_number) + ":" + std::to_string(loc.character_number)};
}

} // namespace c4lib
