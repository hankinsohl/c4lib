// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/29/2024.

#pragma once

#include <lib/util/exception-formats.hpp>
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <format>
#include <include/exceptions.hpp>
#include <ios>
#include <iosfwd>
#include <lib/util/limits.hpp>
#include <lib/util/narrow.hpp>
#include <string>

namespace c4lib::io {

inline void make_little_endian(char* out, std::streamsize size)
{
    if constexpr (std::endian::native == std::endian::big) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::reverse(out, out + size);
    }
}

template<typename I> void make_little_endian(I& value)
{
    make_little_endian(&value, sizeof(value));
}

// Uses std::filesystem to compose a well-formed path given an output directory, a filename (which may
// be a path to a filename) and an extension.  Path information is stripped from filename to obtain just
// the filename plus existing extension if any, and then the extension is added.  Finally output_dir
// is prepended to obtain a well-formed path which is then returned.
std::string make_path(const std::string& output_dir, const std::string& filename, const std::string& extension);

void read_binary_file_to_stream(const std::string& filename, std::streampos offset, size_t size, std::ostream& out);

void read_bytes(std::istream& in, char* out, std::streamsize size);

// Reads binary integer values from the input stream.  The input stream is assumed to be in little endian byte
// order.  If the native system is big endian, byte order will be reversed in order to preserve the intended
// integer enumeratorValue.
template<typename I> void read_int(std::istream& in, I& out)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    read_bytes(in, reinterpret_cast<char*>(&out), sizeof(I));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    make_little_endian(reinterpret_cast<char*>(&out), gsl::narrow<std::streamsize>(sizeof(I)));
}

template<typename S> void read_string(std::istream& in, S& str)
{
    uint32_t length{0};
    read_int(in, length);
    if (length > limits::max_string_length) {
        throw IO_error{std::format(fmt::string_length_exceeds_maximum, length, limits::max_string_length)};
    }
    str.resize(length);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    read_bytes(in, reinterpret_cast<char*>(str.data()), length * sizeof(typename S::value_type));
}

inline std::streampos stream_size(std::istream& s)
{
    std::streampos const cur_pos{s.tellg()};
    s.seekg(0, std::ios::end);
    std::streampos const size{s.tellg()};
    s.seekg(cur_pos, std::ios::beg);
    return size;
}

void write_binary_stream_to_file(std::istream& source, std::streampos offset, size_t size, const std::string& filename);

void write_bytes(std::ostream& out, const char* in, std::streamsize size);

// Writes binary integer values to the output stream.  The stream is assumed to be in little endian byte order.
// If the native system is big endian, a copy of the input integer will be created and the copy's bytes will
// be reversed.  The copy will then be written to the output stream in order to write in little endian byte
// order.
template<typename I> void write_int(std::ostream& out, I& value)
{
    if constexpr (std::endian::native == std::endian::big) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        make_little_endian(reinterpret_cast<char*>(&value), gsl::narrow<std::streamsize>(sizeof(I)));
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    write_bytes(out, reinterpret_cast<const char*>(&value), sizeof(value));
}

template<typename I> void write_int(void* out, I& value)
{
    if constexpr (std::endian::native == std::endian::big) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        make_little_endian(reinterpret_cast<char*>(&value), gsl::narrow<std::streamsize>(sizeof(I)));
    }
    memcpy(out, &value, sizeof(value));
}

template<typename S> void write_string(std::ostream& out, const S& str)
{
    // Write the length of the string (in characters)
    uint32_t length{gsl::narrow<uint32_t>(str.length())};
    write_int(out, length);
    if (length > 0) {
        // Write the string itself
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        write_bytes(out, reinterpret_cast<const char*>(str.c_str()), gsl::narrow<std::streamsize>(str.length() * sizeof(str[0])));
    }
}

template<typename S> void write_string(void* out, const S& str)
{
    // Write the length of the string (in characters)
    uint32_t length{gsl::narrow<uint32_t>(str.length())};
    write_int(out, length);
    if (length > 0) {
        // Write the string itself
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        memcpy(static_cast<char*>(out) + sizeof(length), str.c_str(), str.length() * sizeof(str[0]));
    }
}

} // namespace c4lib::io
