// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/11/2024.

#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace hankinsohl::options {

// Display_help_error is not an exception per se.  It is thrown to jump to the appropriate point of
// execution where a help message can be displayed.
class Display_help_error : public std::exception {
public:
    explicit Display_help_error(std::string help)
        : m_help(std::move(help))
    {}

    [[nodiscard]] const char* what() const noexcept override
    {
        return m_help.c_str();
    }

private:
    std::string m_help;
};

class Options_error : public std::logic_error {
public:
    explicit Options_error(const std::string& what)
        : logic_error(what)
    {}
};

class Xml_error : public std::logic_error {
public:
    explicit Xml_error(const std::string& what)
        : logic_error(what)
    {}
};

} // namespace hankinsohl::options
