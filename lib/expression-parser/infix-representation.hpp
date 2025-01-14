// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/22/2024.

#pragma once

#include <stack>
#include <string>

namespace c4lib::expression_parser {

// Infix_representation is used to record an infix representation of the parse.  Infix_representation
// is intended primarily to help debug and test the expression parser.
class Infix_representation {
public:
    // Use pop to obtain the infix representation.  Note that pop removes the
    // string and thus should be called a single time once the expression parser's
    // parse method returns.
    std::string pop();

    void push(const std::string& e);

private:
    std::stack<std::string> m_stack;
};

} // namespace c4lib::expression_parser
