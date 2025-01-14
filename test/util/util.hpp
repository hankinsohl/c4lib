// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/2/2024.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>

namespace c4lib::test {

// Opens f1 and f2 for binary input and then returns the result of compare_binary_streams when called
// with the corresponding file streams.
bool compare_binary_files(const std::string& f1, const std::string& f2, std::stringstream& errors);

// Compares s1 and s2.  Returns false if the streams are equal, true otherwise.  Writes error messages to
// errors if s1 is not equal to s2.
bool compare_binary_streams(std::istream& s1, std::istream& s2, std::stringstream& errors);

// Returns the result of compare_text_files(f1, f2, max_diffs, false, {}, errors).
bool compare_text_files(const std::string& f1, const std::string& f2, size_t max_diffs, std::stringstream& errors);

// Opens f1 and f2 for text input and then returns the result of compare_text_streams when called
// with the corresponding file streams.
bool compare_text_files(const std::string& f1,
    const std::string& f2,
    size_t max_diffs,
    bool ignore_file_sizes,
    const std::vector<std::string>& filter,
    std::stringstream& errors);

// Returns the value of compare_text_streams(s1, s2, max_diffs, false, {}, errors).
bool compare_text_streams(std::istream& s1, std::istream& s2, size_t max_diffs, std::stringstream& errors);

// Compares s1 and s2.  Returns false if the streams are equal, true otherwise.  Writes error messages to
// errors if s1 is not equal to s2.  If ignore_file_sizes is true, differences in file sizes are ignored.
// errors contains a vector of strings against which differences are compared.  If the differing lines
// both contain one or more of the filter strings, the difference is ignored.
bool compare_text_streams(std::istream& s1,
    std::istream& s2,
    size_t max_diffs,
    bool ignore_file_sizes,
    const std::vector<std::string>& filter,
    std::stringstream& errors);
} // namespace c4lib::test
