// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/2/2024.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <string>
#include <vector>

// Constants and functions related to the layout of a Beyond the Sword save.
namespace c4lib::layout {

// Offset to the checksum byte from the end of the file.  This byte is located immediately  behind the
// save MD5 (32 bytes), the MD5 string length field (4), plus 1 for the byte itself.
inline constexpr std::streamoff checksum_byte_offset{32 + 4 + 1};

// Magic constant used by civ4 as part of its rollup md5 calculation
inline constexpr std::array<uint8_t, 4> civ4_md5_magic{0x4D, 0xE6, 0x40, 0xBB};

// Offset to the Civ4 game version field.
inline constexpr std::streamoff game_version_offset{0};

inline constexpr std::size_t num_lma_strings{5};

// Offset to the required mod string.
inline constexpr std::streamoff required_mod_offset{4};

// Magic constant found at the beginning of compressed data.
inline constexpr std::array<uint8_t, 2> zlib_magic{0x78, 0x9c};

std::u16string get_admin_password_hash(std::istream& in);

uint8_t get_checksum_byte(std::istream& in);

uint32_t get_checksum_dword(std::istream& in);

std::streamoff get_civ4_compressed_data_offset(std::istream& in, bool confirm_zlib_magic);

std::streamoff get_civ4_footer_offset(std::istream& in);

size_t get_cv_init_core_md5_data_size(std::istream& in);

std::u16string get_game_password_hash(std::istream& in);

uint32_t get_game_version(std::istream& in);

void get_player_password_hashes(std::istream& in,
    std::vector<std::u16string>& player_password_hashes,
    int max_players,
    int num_game_option_types,
    int num_multiplayer_option_types);

void get_lma_strings(std::istream& in, std::vector<std::string>& lma_strings);

std::streampos seek_to_cv_init_core_md5_size_field(std::istream& in);
} // namespace c4lib::layout
