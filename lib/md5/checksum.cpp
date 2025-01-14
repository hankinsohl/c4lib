// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/5/2024.

#include <array>
#include <cstdint>
#include <cstring>
#include <format>
#include <include/exceptions.hpp>
#include <include/logger.hpp>
#include <ios>
#include <istream>
#include <lib/io/io.hpp>
#include <lib/layout/layout.hpp>
#include <lib/logger/log-formats.hpp>
#include <lib/md5/checksum.hpp>
#include <lib/md5/md5-digest.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/tune.hpp>
#include <ostream>
#include <string>
#include <vector>

namespace c4lib::md5 {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Checksum::Checksum(
    std::istream& civ4_savegame, int max_players, int num_game_option_types, int num_multiplayer_option_types)
    : m_in(civ4_savegame),
      m_max_players(max_players),
      m_num_game_option_types(num_game_option_types),
      m_num_multiplayer_option_types(num_multiplayer_option_types)
{}

std::string Checksum::get_hash()
{
    if (m_rollup_md5.empty()) {
        calculate_rollup_md5_();
    }
    return m_rollup_md5;
}

const std::ostream& Checksum::get_hash_data()
{
    if (m_rollup_md5.empty()) {
        calculate_rollup_md5_();
    }
    m_rollup_md5_buffer.seekg(0);
    return m_rollup_md5_buffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Checksum::calculate_rollup_md5_()
{
    m_compressed_data_offset = layout::get_civ4_compressed_data_offset(m_in, true);
    get_cv_init_core_md5_();
    get_compressed_data_md5_();
    get_rollup_md5_();
}

void Checksum::get_compressed_data_md5_()
{
    Md5_digest digest;
    m_in.seekg(m_compressed_data_offset);
    uint32_t chunk_size{0};
    io::read_int(m_in, chunk_size);
    while (chunk_size > 0) {
        if (chunk_size > tune::md5_buffer_size) {
            throw Checksum_error(fmt::invalid_chunk_size);
        }
        digest.add(m_in, chunk_size);
        io::read_int(m_in, chunk_size);
    }
    m_compressed_data_md5 = digest.get_hash();
    Logger::info(std::format(fmt::compressed_data_md5, m_compressed_data_md5));
}

void Checksum::get_cv_init_core_md5_()
{
    Md5_digest digest;
    const std::streampos cv_init_core_md5_size_field_offset{layout::seek_to_cv_init_core_md5_size_field(m_in)};
    const std::streamsize cv_init_core_md5_data_size{
        gsl::narrow<std::streamsize>(layout::get_cv_init_core_md5_data_size(m_in))};
    // N.B.: The header MD5 excludes the data size field (add 4 to offset).
    digest.add(m_in, cv_init_core_md5_size_field_offset + gsl::narrow<std::streampos>(4LL), cv_init_core_md5_data_size);
    m_cv_init_core_md5 = digest.get_hash();
    Logger::info(std::format(fmt::cv_init_core_md5, m_cv_init_core_md5));
}

void Checksum::get_rollup_md5_()
{
    // Write the checksum DWORD to the rollup buffer.
    uint32_t checksum_dword{layout::get_checksum_dword(m_in)};
    io::write_int(m_rollup_md5_buffer, checksum_dword);

    // Write the game version to the rollup buffer.
    uint32_t game_version{layout::get_game_version(m_in)};
    io::write_int(m_rollup_md5_buffer, game_version);

    // Write the checksum byte to the rollup buffer.
    uint8_t checksum_byte{layout::get_checksum_byte(m_in)};
    io::write_int(m_rollup_md5_buffer, checksum_byte);

    // Write the Lock Modified Assets strings to the rollup buffer.
    std::vector<std::string> lmaStrings;
    layout::get_lma_strings(m_in, lmaStrings);
    for (const auto& lmaString : lmaStrings) {
        io::write_string(m_rollup_md5_buffer, lmaString);
    }

    // Write CvInitCore.m_szAdminPassword (CvWString) to the rollup buffer.
    const std::u16string admin_password_hash{layout::get_admin_password_hash(m_in)};
    io::write_string(m_rollup_md5_buffer, admin_password_hash);

    // Write CvInitCore.m_szGamePassword (CvWString) to the rollup buffer.
    const std::u16string game_password_hash{layout::get_game_password_hash(m_in)};
    io::write_string(m_rollup_md5_buffer, game_password_hash);

    // Write each player's password hash (CvWString) to the rollup buffer.
    std::vector<std::u16string> playerPasswordHashes;
    layout::get_player_password_hashes(
        m_in, playerPasswordHashes, m_max_players, m_num_game_option_types, m_num_multiplayer_option_types);
    for (const auto& player_password_hash : playerPasswordHashes) {
        io::write_string(m_rollup_md5_buffer, player_password_hash);
    }

    // Write the compressed data MD5 to the rollup MD5 buffer
    io::write_string(m_rollup_md5_buffer, m_compressed_data_md5);

    // Write the header MD5 to the rollup MD5 buffer
    io::write_string(m_rollup_md5_buffer, m_cv_init_core_md5);

    // Write hard-coded magic value to end of buffer.
    std::array<char, 4> civ4_md5_magic{0};
    memcpy(civ4_md5_magic.data(), layout::civ4_md5_magic.data(), civ4_md5_magic.size());
    io::write_bytes(m_rollup_md5_buffer, civ4_md5_magic.data(), civ4_md5_magic.size());
    const std::streampos rollup_md5_data_length{m_rollup_md5_buffer.tellp()};

    Md5_digest digest;
    m_rollup_md5_buffer.seekg(0);
    digest.add(m_rollup_md5_buffer, 0, rollup_md5_data_length);
    m_rollup_md5 = digest.get_hash();
    Logger::info(std::format(fmt::rollup_md5, m_rollup_md5));
}

} // namespace c4lib::md5
