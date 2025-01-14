// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/1/2024.

#include <algorithm>
#include <cstddef>
#include <ios>
#include <iosfwd>
#include <istream>
#include <lib/md5/md5-digest.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/tune.hpp>
#include <string>
#include <vector>

namespace c4lib::md5 {

void Md5_digest::add(std::istream& in, std::streamsize count)
{
    std::vector<char> buffer(tune::md5_buffer_size);
    // Read file and update hash
    std::streamsize bytes_remaining{count};
    while (in && (bytes_remaining != 0)) {
        const std::streamsize bytes_to_read{std::min(gsl::narrow<std::streamsize>(buffer.size()), bytes_remaining)};
        in.read(buffer.data(), bytes_to_read);
        const std::streamsize bytes_read{in.gcount()};
        bytes_remaining -= bytes_read;
        m_md5.add(buffer.data(), gsl::narrow<size_t>(bytes_read));
    }
}

void Md5_digest::add(std::istream& in, std::streampos start, std::streamsize count)
{
    in.seekg(start);
    add(in, count);
}

std::string Md5_digest::get_hash()
{
    return m_md5.getHash();
}

} // namespace c4lib::md5
