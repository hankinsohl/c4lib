// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/2/2024.

#pragma once

#include <cstddef>
#include <ios>
#include <lib/native/compiler-support.hpp>
#include <lib/util/narrow.hpp>
#include <limits>

namespace c4lib::limits {

// Values used to indicate that a type is likely invalid.  Useful for debugging.
inline constexpr std::streamoff invalid_off{std::numeric_limits<std::streamoff>::max()};
// Use std::numeric_limits<ssize_t>::max() to permit conversion of size_t to ssize_t.
inline constexpr size_t invalid_size{gsl::narrow<size_t>(std::numeric_limits<ssize_t>::max())};
inline constexpr ssize_t invalid_ssize{-1};
inline constexpr int invalid_value{-1};

// Maximum array dimension.  Used by the phase 2 parser to ensure that data read is reasonable and in
// sync with the parser.  50,000 is chosen to accommodate plots for the maximum map size, 144 x 96 = 13,824
// with plenty of extra room.
inline constexpr size_t max_array_dimension = 50'000;

inline constexpr size_t max_identifier_length{120};

inline constexpr size_t max_number_length{10};

inline constexpr size_t max_schema_line_length{1024};

// Maximum string length.  Used by the phase 2 parser to ensure that data read is reasonable and in sync with
// the parser.  10,000 is chosen as a reasonable-seeming upper bound given that strings are used for map scripts
// and presumably the script length could be fairly large.
inline constexpr size_t max_string_length = 10'000;

inline constexpr size_t max_string_literal_length{120};

inline constexpr size_t md5_length{32};

} // namespace c4lib::limits
