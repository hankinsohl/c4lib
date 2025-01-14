// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/5/2024.

#pragma once

#include <ranges>
#include <type_traits>

namespace c4lib {
constexpr auto enum_range(auto first, auto last)
{
    auto enum_range{std::views::iota(static_cast<std::underlying_type_t<decltype(first)>>(first),
                        static_cast<std::underlying_type_t<decltype(last)>>(last) + 1)
                    | std::views::transform([](auto enum_val) { return static_cast<decltype(first)>(enum_val); })};

    return enum_range;
};

} // namespace c4lib
