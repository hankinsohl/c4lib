// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/6/2024.

#pragma once

#include <cstdint>

namespace c4lib::zlib::constants {

// Constant used if z_stream.msg is null.
inline constexpr const char* const null_message{""};

// 128K
inline constexpr int buffer_size{1UL << 17UL};

// 64K
inline constexpr uint32_t max_chunk_size{1UL << 16UL};

// Suffixes used for binary files created by deflate/inflate
inline constexpr const char* const deflate_binaries_suffix{"_deflate"};
inline constexpr const char* const inflate_binaries_suffix{"_inflate"};

// Extensions used for binary files created by deflate/inflate
inline constexpr const char* const composite_ext{".composite.bin"};
inline constexpr const char* const compressed_ext{".compressed.bin"};
inline constexpr const char* const decompressed_ext{".decompressed.bin"};
inline constexpr const char* const footer_ext{".footer.bin"};
inline constexpr const char* const header_ext{".header.bin"};
inline constexpr const char* const original_ext{".original.CivBeyondSwordSave"};
} // namespace c4lib::zlib::constants
