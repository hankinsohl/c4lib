// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/2/2024.

#pragma once

#include <cstddef>

// The following constants are used to tune the performance of the library.
namespace c4lib::tune {

// Number of definitions originally allocated.  DEFINITION_RESERVE_SIZE should be the least power of 2 greater
// than the size of the definition table once all variables have been defined.  In our case, the BTS schema had about
// 200 definitions.  We'll reserve 512 which should prevent rehashes.
inline constexpr size_t definition_reserve_size{512};

// 64K Buffer for MD5 data.  64K chosen because this is the size of a civ4 compressed data chuck.
inline constexpr size_t md5_buffer_size{0x10000};

// When the schema-processor parses the BTS schema, somewhat over 4000 tokens are generated.  Reserve space for 8192
// tokens to avoid token vector resizing.
inline constexpr size_t schema_token_vector_reserve_size{8192};

} // namespace c4lib::tune
