// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/11/2024.

#pragma once

#include <cstddef>
#include <lib/schema-parser/tokenizer.hpp>

namespace c4lib::schema_parser {

class auto_index {
public:
    explicit auto_index(Tokenizer& t)
        : m_i(t.get_index()), m_t(t)
    {
    }

    explicit auto_index(Tokenizer& t, size_t i)
        : m_i(t.get_index()), m_t(t)
    {
        m_t.set_index(i);
    }

    ~auto_index()
    {
        m_t.set_index_noexcept(m_i);
    }

    auto_index(const auto_index&) = delete;   
	
    auto_index& operator=(const auto_index&) = delete;  
	
    auto_index(auto_index&&) noexcept = delete;   
	
    auto_index& operator=(auto_index&&) noexcept = delete;    

private:
    size_t m_i;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    Tokenizer& m_t;
};

} // namespace c4lib::schema_parser
