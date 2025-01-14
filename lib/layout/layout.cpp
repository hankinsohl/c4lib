// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/8/2024.

#include <array>
#include <cstdint>
#include <cstring>
#include <ios>
#include <iosfwd>
#include <istream>
#include <lib/io/io.hpp>
#include <lib/layout/layout.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/narrow.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::streampos seek_past_bytes_(std::istream& in, std::streampos num_bytes);

std::streampos seek_past_dwords_(std::istream& in, std::streampos num_dwords);

template<typename C> std::streampos seek_past_strings_(std::istream& in, std::streampos num_strings)
{
    for (std::streamoff i{0}; i < num_strings; ++i) {
        uint32_t length{0};
        c4lib::io::read_int(in, length);
        seek_past_bytes_(in, length * sizeof(C));
    }
    if (!in) {
        throw std::runtime_error(c4lib::fmt::runtime_error_seek);
    }
    return in.tellg();
}

std::streampos seek_to_admin_password_hash_(std::istream& in);

std::streampos seek_to_checksum_byte_(std::istream& in);

std::streampos seek_to_checksum_dword_(std::istream& in);

std::streampos seek_to_first_player_password_hash_(
    std::istream& in, int max_players, int num_game_option_types, int num_multiplayer_option_types);

std::streampos seek_to_game_data_element_(std::istream& in);

std::streampos seek_to_game_version_(std::istream& in);

std::streampos seek_to_game_password_hash_(std::istream& in);

std::streampos seek_to_lma_string_(std::istream& in);

std::streampos seek_to_offset_(
    std::istream& in, std::streampos offset, std::ios_base::seekdir dir = std::ios_base::beg);

std::streampos seek_to_required_mod_field_(std::istream& in);
} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace c4lib::layout {
std::u16string get_admin_password_hash(std::istream& in)
{
    seek_to_admin_password_hash_(in);
    std::u16string adminPasswordHash;
    io::read_string(in, adminPasswordHash);
    return adminPasswordHash;
}

uint8_t get_checksum_byte(std::istream& in)
{
    seek_to_checksum_byte_(in);
    uint8_t byte{0};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    io::read_bytes(in, reinterpret_cast<char*>(&byte), sizeof(byte));
    return byte;
}

uint32_t get_checksum_dword(std::istream& in)
{
    seek_to_checksum_dword_(in);
    uint32_t dword{0};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    io::read_bytes(in, reinterpret_cast<char*>(&dword), sizeof(dword));
    return dword;
}

std::streamoff get_civ4_compressed_data_offset(std::istream& in, bool confirm_zlib_magic)
{
    // Read the compressed data offset field.  The value of this field is an offset relative to the game data element.
    seek_to_cv_init_core_md5_size_field(in);

    // std::streampos is an opaque object that we cannot safely read into.  Read into a 32-bit buffer first
    // and then construct a streampos using the buffer.
    uint32_t uint32_buffer{0};
    io::read_int(in, uint32_buffer);
    const std::streampos relative_offset_to_compressed_data{uint32_buffer};

    const std::streampos game_data_element_offset{seek_to_game_data_element_(in)};
    const std::streampos absolute_offset_to_compressed_data{
        game_data_element_offset + relative_offset_to_compressed_data};

    const std::streampos absolute_offset_to_zlib_magic{
        absolute_offset_to_compressed_data + gsl::narrow<std::streampos>(4LL)};
    seek_to_offset_(in, absolute_offset_to_zlib_magic);

    if (confirm_zlib_magic) {
        // Seek 4 bytes past the absolute compressed data field, read the WORD at this location, and confirm that
        // it's ZLIB_MAGIC (0x789c).  Note that Civ4 writes compressed data in chunks.  The size of the chunk is
        // written, followed by the compressed data for the chunk.  Each chunk is 64K bytes long except for the
        // last chunk, which is shorter.
        std::array<char, zlib_magic.size()> zlib_magic_buffer{0};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        io::read_bytes(in, zlib_magic_buffer.data(), zlib_magic_buffer.size());
        if (std::memcmp(zlib_magic_buffer.data(), zlib_magic.data(), zlib_magic_buffer.size()) != 0) {
            throw std::logic_error(fmt::zlib_error_bad_magic_value);
        }
    }

    return absolute_offset_to_compressed_data;
}

std::streamoff get_civ4_footer_offset(std::istream& in)
{
    const std::streamoff compressed_data_offset{get_civ4_compressed_data_offset(in, true)};
    in.seekg(compressed_data_offset);

    // Read the fist compressed data size
    uint32_t chunk_size{0};
    io::read_int(in, chunk_size);
    while (in && (chunk_size != 0U)) {
        io::read_int(in, chunk_size);
    }
    if (!in) {
        throw std::runtime_error{fmt::runtime_error_seek};
    }

    return in.tellg();
}

size_t get_cv_init_core_md5_data_size(std::istream& in)
{
    seek_to_cv_init_core_md5_size_field(in);
    uint32_t size{0};
    io::read_int(in, size);
    return size;
}

std::u16string get_game_password_hash(std::istream& in)
{
    std::u16string gamePasswordHash;
    seek_to_game_password_hash_(in);
    io::read_string(in, gamePasswordHash);
    return gamePasswordHash;
}

uint32_t get_game_version(std::istream& in)
{
    seek_to_game_version_(in);
    uint32_t gameVersion{0};
    io::read_int(in, gameVersion);
    return gameVersion;
}

void get_player_password_hashes(std::istream& in,
    std::vector<std::u16string>& player_password_hashes,
    int max_players,
    int num_game_option_types,
    int num_multiplayer_option_types)
{
    seek_to_first_player_password_hash_(in, max_players, num_game_option_types, num_multiplayer_option_types);
    player_password_hashes.clear();
    for (int i{0}; i < max_players; ++i) {
        std::u16string playerPasswordHash;
        io::read_string(in, playerPasswordHash);
        player_password_hashes.push_back(playerPasswordHash);
    }
}

void get_lma_strings(std::istream& in, std::vector<std::string>& lma_strings)
{
    seek_to_lma_string_(in);
    lma_strings.clear();
    for (size_t i{0}; i < num_lma_strings; ++i) {
        std::string lmaString;
        io::read_string(in, lmaString);
        lma_strings.push_back(lmaString);
    }
}

std::streampos seek_to_cv_init_core_md5_size_field(std::istream& in)
{
    seek_to_lma_string_(in);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    return seek_past_strings_<char>(in, 5);
}

} // namespace c4lib::layout

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace {
std::streampos seek_past_bytes_(std::istream& in, std::streampos num_bytes)
{
    in.seekg(num_bytes, std::ios_base::cur);
    if (!in) {
        throw std::runtime_error(c4lib::fmt::runtime_error_seek);
    }
    return in.tellg();
}

std::streampos seek_past_dwords_(std::istream& in, std::streampos num_dwords)
{
    return seek_past_bytes_(in, num_dwords * int{sizeof(uint32_t)});
}

std::streampos seek_to_admin_password_hash_(std::istream& in)
{
    seek_to_game_password_hash_(in);
    return seek_past_strings_<char16_t>(in, 1);
}

std::streampos seek_to_checksum_byte_(std::istream& in)
{
    return seek_to_offset_(in, -c4lib::layout::checksum_byte_offset, std::ios_base::end);
}

std::streampos seek_to_checksum_dword_(std::istream& in)
{
    seek_to_game_version_(in);
    return seek_past_dwords_(in, 1);
}

std::streampos seek_to_first_player_password_hash_(
    std::istream& in, int max_players, int num_game_option_types, int num_multiplayer_option_types)
{
    c4lib::layout::seek_to_cv_init_core_md5_size_field(in);
    // Seek past 1) header MD5 size; 2) uiSaveFlags;
    seek_past_dwords_(in, 2);
    // Seek past 1) CvInitCore::m_eType
    seek_past_dwords_(in, 1);
    // Seek past 1) CvInitCore::m_szGameName;
    //           2) CvInitCore::m_szGamePassword;
    //           3) CvInitCore::m_szAdminPassword;
    //           4) CvInitCore::m_szMapScriptName
    seek_past_strings_<char16_t>(in, 4);
    // Seek past 1) CvInitCore::m_bWBMapNoPlayers
    seek_past_bytes_(in, 1);
    // Seek past 1) CvInitCore::m_eWorldSize;
    //           2) CvInitCore::m_eClimate;
    //           3) CvInitCore::m_eSeaLevel;
    //           4) CvInitCore::m_eEra;
    //           5) CvInitCore::m_eGameSpeed;
    //           6) CvInitCore::m_eTurnTimer;
    //           7) CvInitCore::m_eCalendar
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    seek_past_dwords_(in, 7);
    uint32_t num_custom_map_options{0};
    c4lib::io::read_int(in, num_custom_map_options);
    // Seek past 1) CvInitCore::m_iNumHiddenCustomMapOptions
    seek_past_dwords_(in, 1);
    // Seek past 1)  CvInitCore::m_aeCustomMapOptions
    seek_past_dwords_(in, num_custom_map_options);
    uint32_t numVictories{0};
    c4lib::io::read_int(in, numVictories);
    // Seek past 1) CvInitCore::m_abVictories
    seek_past_bytes_(in, numVictories);
    // Seek past 1)  CvInitCore::m_abOptions
    seek_past_bytes_(in, num_game_option_types);
    // Seek past 1)  CvInitCore::m_abMPOptions
    seek_past_bytes_(in, num_multiplayer_option_types);
    // Seek past 1) CvInitCore::m_bStatReporting
    seek_past_bytes_(in, 1);
    // Seek past 1) CvInitCore::m_iGameTurn;
    //           2) CvInitCore::m_iMaxTurns;
    //           3) CvInitCore::m_iPitbossTurnTime;
    //           4) CvInitCore::m_iTargetScore;
    //           5) CvInitCore::m_iMaxCityElimination;
    //           6) CvInitCore::m_iNumAdvancedStartPoints
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    seek_past_dwords_(in, 6);
    // Seek past CvInitCore::m_aszLeaderName
    seek_past_strings_<char16_t>(in, max_players);
    // Seek past CvInitCore::m_aszCivDescription
    seek_past_strings_<char16_t>(in, max_players);
    // Seek past CvInitCore::m_aszCivShortDesc
    seek_past_strings_<char16_t>(in, max_players);
    // Seek past CvInitCore::m_aszCivAdjective
    return seek_past_strings_<char16_t>(in, max_players);
}

std::streampos seek_to_game_data_element_(std::istream& in)
{
    c4lib::layout::seek_to_cv_init_core_md5_size_field(in);
    // Seek past 1) relativeCompressedDataOffsetField; 2) uiSaveFlag
    return seek_past_dwords_(in, 2);
}

std::streampos seek_to_game_version_(std::istream& in)
{
    return seek_to_offset_(in, c4lib::layout::game_version_offset);
}

std::streampos seek_to_game_password_hash_(std::istream& in)
{
    c4lib::layout::seek_to_cv_init_core_md5_size_field(in);
    // Seek to game name wstring
    seek_past_dwords_(in, 3); // Seek past 1) relativeCompressedDataOffsetField; 2) uiSaveFlag; 3) CvInitCore::m_eType
    return seek_past_strings_<char16_t>(in, 1);
}

std::streampos seek_to_lma_string_(std::istream& in)
{
    seek_to_required_mod_field_(in);
    // Seek past RequiredMod and ModMd5 fields.
    seek_past_strings_<char>(in, 2);
    // Seek past ChecksumDWord.
    return seek_past_dwords_(in, 1);
}

std::streampos seek_to_offset_(std::istream& in, std::streampos offset, std::ios_base::seekdir dir)
{
    in.seekg(offset, dir);
    if (!in) {
        throw std::runtime_error(c4lib::fmt::runtime_error_seek);
    }
    return in.tellg();
}

std::streampos seek_to_required_mod_field_(std::istream& in)
{
    return seek_to_offset_(in, c4lib::layout::required_mod_offset);
}

} // namespace
