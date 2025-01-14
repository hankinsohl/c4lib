// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/18/2024.

#pragma once

#include <chrono>
#include <sstream>
#include <string>

namespace c4lib {
class Timer {
public:
    [[nodiscard]] std::chrono::milliseconds elapsed_time() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - m_start);
    }

    void start()
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    [[nodiscard]] std::string to_string() const
    {
        std::stringstream ss;
        ss << elapsed_time();
        return ss.str();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start{std::chrono::high_resolution_clock::now()};
};

} // namespace c4lib
