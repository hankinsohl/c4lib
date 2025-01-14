// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/2/2024.

#include <cstddef>
#include <format>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <lib/io/io.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/util.hpp>
#include <sstream>
#include <string>
#include <test/util/util.hpp>
#include <vector>

namespace {
void remove_trailing_carriage_return_(std::string& s)
{
    if (!s.empty() && s.back() == '\r') {
        s.pop_back();
    }
}

} // namespace

namespace c4lib::test {

bool compare_binary_files(const std::string& f1, const std::string& f2, std::stringstream& errors)
{
    std::ifstream fs1{f1, std::ios_base::in | std::ios_base::binary};
    if (!fs1.is_open() || fs1.bad()) {
        throw std::runtime_error{std::format(fmt::runtime_error_opening_file, f1)};
    }
    fs1.unsetf(std::ios::skipws);

    std::ifstream fs2{f2, std::ios_base::in | std::ios_base::binary};
    if (!fs2.is_open() || fs2.bad()) {
        throw std::runtime_error{std::format(fmt::runtime_error_opening_file, f2)};
    }
    fs2.unsetf(std::ios::skipws);

    return compare_binary_streams(fs1, fs2, errors);
}

bool compare_binary_streams(std::istream& s1, std::istream& s2, std::stringstream& errors)
{
    bool is_same{true};

    std::stringstream().swap(errors);

    s1.clear();
    s2.clear();

    const std::streampos s1_size{io::stream_size(s1)};
    const std::streampos s2_size{io::stream_size(s2)};

    if (s1_size != s2_size) {
        is_same = false;
        static const char* const fmt{"Stream 1 size '{}' != stream 2 size '{}'\n"};
        errors << std::vformat(
            fmt, std::make_format_args(unmove(static_cast<int>(s1_size)), unmove(static_cast<int>(s2_size))));
    }

    s1.seekg(0, std::ios::beg);
    char c1{0};
    s2.seekg(0, std::ios::beg);
    char c2{0};

    std::streampos diff_pos{0};

    s1.read(&c1, sizeof(c1));
    s2.read(&c2, sizeof(c2));
    while (s1 && s2) {
        if (c1 != c2) {
            is_same = false;
            // Subtract 1 b/c tellg reports the position of the next character, one
            // in advance of where the difference occurred.
            diff_pos = s1.tellg() - static_cast<std::streamoff>(1);
            break;
        }
        s1.read(&c1, sizeof(c1));
        s2.read(&c2, sizeof(c2));
    }

    if (!is_same) {
        static const char* const fmt{"Stream 1 first differs from stream 2 at position {0} 0x{0:X})\n"};
        errors << std::vformat(fmt, std::make_format_args(unmove(static_cast<int>(diff_pos))));
    }

    return !is_same;
}

bool compare_text_files(const std::string& f1, const std::string& f2, size_t max_diffs, std::stringstream& errors)
{
    return compare_text_files(f1, f2, max_diffs, false, {}, errors);
}

bool compare_text_files(const std::string& f1,
    const std::string& f2,
    size_t max_diffs,
    bool ignore_file_sizes,
    const std::vector<std::string>& filter,
    std::stringstream& errors)
{
    std::ifstream fs1{f1, std::ios_base::in};
    if (!fs1.is_open() || fs1.bad()) {
        throw std::runtime_error{std::format(fmt::runtime_error_opening_file, f1)};
    }

    std::ifstream fs2{f2, std::ios_base::in};
    if (!fs2.is_open() || fs2.bad()) {
        throw std::runtime_error{std::format(fmt::runtime_error_opening_file, f2)};
    }

    return compare_text_streams(fs1, fs2, max_diffs, ignore_file_sizes, filter, errors);
}

bool compare_text_streams(std::istream& s1, std::istream& s2, size_t max_diffs, std::stringstream& errors)
{
    return compare_text_streams(s1, s2, max_diffs, false, {}, errors);
}

bool compare_text_streams(std::istream& s1,
    std::istream& s2,
    size_t max_diffs,
    bool ignore_file_sizes,
    const std::vector<std::string>& filter,
    std::stringstream& errors)
{
    bool is_same{true};

    std::stringstream().swap(errors);

    s1.clear();
    s2.clear();

    if (!ignore_file_sizes) {
        const std::streampos s1_size{io::stream_size(s1)};
        const std::streampos s2_size{io::stream_size(s2)};

        if (s1_size != s2_size) {
            is_same = false;
            errors << "Stream 1 size " << s1_size << " != stream 2 size " << s2_size << "\n";
        }
    }

    s1.seekg(0, std::ios::beg);
    std::string l1;
    std::getline(s1, l1);
    s2.seekg(0, std::ios::beg);
    std::string l2;
    std::getline(s2, l2);

    size_t line{1};
    size_t diff_count{0};

    while (s1 && s2) {
        remove_trailing_carriage_return_(l1);
        remove_trailing_carriage_return_(l2);
        if (l1 != l2) {
            bool ignore_mismatch{false};
            for (const auto& ignore : filter) {
                if (l1.find(ignore) != std::string::npos && l2.find(ignore) != std::string::npos) {
                    ignore_mismatch = true;
                    break;
                }
            }
            if (!ignore_mismatch) {
                is_same = false;
                errors << "Line " << line << ":\n";
                errors << "    Stream 1: '" << l1 << "'\n";
                errors << "    Stream 2: '" << l2 << "'\n\n";
                if (++diff_count == max_diffs) {
                    break;
                }
            }
        }
        std::getline(s1, l1);
        std::getline(s2, l2);
        ++line;
    }

    return !is_same;
}

} // namespace c4lib::test
