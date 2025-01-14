// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/30/2024.

#pragma once

namespace c4lib {

template<typename T> class auto_pop {
public:
    explicit auto_pop(T& t)
        : m_t(t)
    {}

    ~auto_pop()
    {
        m_t.pop();
    }

    auto_pop(const auto_pop&) = delete;

    auto_pop& operator=(const auto_pop&) = delete;

    auto_pop(auto_pop&&) noexcept = delete;

    auto_pop& operator=(auto_pop&&) noexcept = delete;

private:
    T& m_t;
};

} // namespace c4lib
