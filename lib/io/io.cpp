// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/29/2024.

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <iterator>
#include <lib/io/io.hpp>
#include <lib/util/exception-formats.hpp>
#include <string>

namespace c4lib::io {

std::string make_path(const std::string& output_dir, const std::string& filename, const std::string& extension)
{
    std::filesystem::path const fs_dir{output_dir};
    std::filesystem::path fs_filename{filename};
    fs_filename = fs_filename.filename();
    std::string well_formed_ext{extension};
    if (!well_formed_ext.empty() && well_formed_ext[0] == '.') {
        well_formed_ext = well_formed_ext.substr(1);
    }

    return ((fs_dir / fs_filename).string() + "." + well_formed_ext);
}

void read_binary_file_to_stream(const std::string& filename, std::streampos offset, size_t size, std::ostream& out)
{
    std::ifstream file{filename, std::ios_base::in | std::ios_base::binary};
    file.unsetf(std::ios::skipws);
    if (!file.is_open() || file.bad()) {
        throw std::runtime_error{std::format(fmt::runtime_error_opening_file, filename)};
    }

    file.seekg(offset);
    if (size == 0) {
        std::copy(
            std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::ostream_iterator<char>(out));
    }
    else {
        std::copy_n(std::istreambuf_iterator<char>(file), size, std::ostream_iterator<char>(out));
    }
    if (!file || !out) {
        throw std::runtime_error{std::format(fmt::runtime_error_reading_from_file, filename)};
    }
}

void read_bytes(std::istream& in, char* out, std::streamsize size)
{
    in.read(out, size);
    if (!in) {
        throw std::runtime_error(fmt::runtime_error_read);
    }
}

void write_binary_stream_to_file(std::istream& source, std::streampos offset, size_t size, const std::string& filename)
{
    std::ofstream file{filename, std::ios_base::out | std::ios_base::binary};
    file.unsetf(std::ios::skipws);
    if (!file.is_open() || file.bad()) {
        throw std::runtime_error{std::format(fmt::runtime_error_opening_file, filename)};
    }
    source.seekg(offset);

    if (size == 0) {
        std::copy(std::istreambuf_iterator<char>(source), std::istreambuf_iterator<char>(),
            std::ostream_iterator<char>(file));
    }
    else {
        std::copy_n(std::istreambuf_iterator<char>(source), size, std::ostream_iterator<char>(file));
    }
    if (!source || !file) {
        throw std::runtime_error{std::format(fmt::runtime_error_writing_to_file, filename)};
    }
}

void write_bytes(std::ostream& out, const char* in, std::streamsize size)
{
    out.write(in, size);
    if (!out) {
        throw std::runtime_error(fmt::runtime_error_write);
    }
}

} // namespace c4lib::io
