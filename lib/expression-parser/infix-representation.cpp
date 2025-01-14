// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/22/2024.

#include <lib/expression-parser/infix-representation.hpp>
#include <string>

namespace c4lib::expression_parser {

std::string Infix_representation::pop()
{
    std::string e{m_stack.top()};
    m_stack.pop();
    return e;
}

void Infix_representation::push(const std::string& e)
{
    m_stack.push(e);
}

} // namespace c4lib::expression_parser
