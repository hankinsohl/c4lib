// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/1/2024.

#pragma once

#include <ios>
#include <iosfwd>
#include <lib/md5/md5.hpp>
#include <string>

namespace c4lib::md5 {

class Md5_digest {
public:
    void add(std::istream& in, std::streampos start, std::streamsize count);

    void add(std::istream& in, std::streamsize count);

    std::string get_hash();

private:
    MD5 m_md5;
};

} // namespace c4lib::md5
