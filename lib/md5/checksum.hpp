// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/1/2024.

#pragma once

#include <iosfwd>
#include <lib/util/limits.hpp>
#include <sstream>
#include <string>

namespace c4lib::md5 {

// Checksum is used to calculate the md5 digest which appears at the end of a civ4 savegame.
class Checksum {
public:
    Checksum(std::istream& civ4_savegame, int max_players, int num_game_option_types, int num_multiplayer_option_types);

    ~Checksum() = default;

    Checksum(const Checksum&) = delete;   
	
    Checksum& operator=(const Checksum&) = delete;  
	
    Checksum(Checksum&&) noexcept = delete;   
	
    Checksum& operator=(Checksum&&) noexcept = delete;    

    // Calculates and returns the rollup md5 checksum for the savegame.
    std::string get_hash();

    // Returns a reference to the data stream used to calculate the checksum.  Primarily intended for debugging.
    const std::ostream& get_hash_data();

private:
    void calculate_rollup_md5_();

    void get_compressed_data_md5_();

    void get_cv_init_core_md5_();

    void get_rollup_md5_();

    std::string m_compressed_data_md5;
    std::streampos m_compressed_data_offset{limits::invalid_off};
    std::string m_cv_init_core_md5;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    std::istream& m_in;
    int m_max_players{limits::invalid_value};
    int m_num_game_option_types{limits::invalid_value};
    int m_num_multiplayer_option_types{limits::invalid_value};
    std::string m_rollup_md5;
    std::stringstream m_rollup_md5_buffer;
};

} // namespace c4lib::md5
