// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/30/2024.

#pragma once

#include <cstddef>
#include <limits>

namespace c4edit::limits {

// Value used to indicate that a size type is in an invalid state.  Useful for debugging.
inline constexpr size_t invalid_size{std::numeric_limits<size_t>::max()};

// Value used to indicate that an integer type is in an invalid state.  Useful for debugging.
inline constexpr int invalid_value{-1};

} // namespace c4edit::limits
