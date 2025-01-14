// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/29/2024.

#pragma once

#include <stdexcept>
#include <string>

namespace c4lib {
class Checksum_error : public std::logic_error {
public:
    explicit Checksum_error(const std::string& what)
        : logic_error(what)
    {}
};

class Expression_parser_error : public std::logic_error {
public:
    explicit Expression_parser_error(const std::string& what)
        : logic_error(what)
    {}
};

class Importer_error : public std::logic_error {
public:
    explicit Importer_error(const std::string& what)
        : logic_error(what)
    {}
};

class Iterator_error : public std::logic_error {
public:
    explicit Iterator_error(const std::string& what)
        : logic_error(what)
    {}
};

// Note: IO_error is a logic error not a runtime error.  IO_errors are thrown
// when some sort of logical invariant is violated.
class IO_error : public std::logic_error {
public:
    explicit IO_error(const std::string& what)
        : logic_error(what)
    {}
};

class Node_source_error : public std::logic_error {
public:
    explicit Node_source_error(const std::string& what)
        : logic_error(what)
    {}
};

class Parser_error : public std::logic_error {
public:
    explicit Parser_error(const std::string& what)
        : logic_error(what)
    {}
};

class Ptree_error : public std::logic_error {
public:
    explicit Ptree_error(const std::string& what)
        : logic_error(what)
    {}
};

class Tokenizer_error : public std::logic_error {
public:
    explicit Tokenizer_error(const std::string& what)
        : logic_error(what)
    {}
};

class Variable_manager_error : public std::logic_error {
public:
    explicit Variable_manager_error(const std::string& what)
        : logic_error(what)
    {}
};

class ZLib_error : public std::logic_error {
public:
    explicit ZLib_error(const std::string& what)
        : logic_error(what)
    {}
};

} // namespace c4lib
