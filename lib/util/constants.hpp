// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/2/2024.

#pragma once

#include <cstddef>

namespace c4lib::constants {

// Constants used to format error messages containing file location
inline constexpr int tab_width{4};
inline constexpr const char* message_indent{"    "};

// Constants used in translations
inline constexpr int translation_indent_width{2};
inline constexpr int translation_max_bytes_per_line{16};

// Array names
inline constexpr const char* leader_array{"Leader"};

// Constant names
inline constexpr const char* max_players{"MAX_PLAYERS"};
inline constexpr const char* num_game_option_types{"NUM_GAME_OPTION_TYPES"};
inline constexpr const char* num_multiplayer_option_types{"NUM_MULTIPLAYER_OPTION_TYPES"};

// Enum names
inline constexpr const char* chat_target_types{"ChatTargetTypes"};
inline constexpr const char* leader_head_types{"LeaderHeadTypes"};
inline constexpr const char* player_types{"PlayerTypes"};
inline constexpr const char* player_vote_types{"PlayerVoteTypes"};

// Enumerator names
inline constexpr const char* no_leader_head{"NO_LEADERHEAD"};
inline constexpr const char* no_player{"NO_PLAYER"};

// Field names from the schema
inline constexpr const char* game_footer{"GameFooter"};
inline constexpr const char* game_version{"GameVersion"};
inline constexpr const char* revealed_route_type_count{"RevealedRouteTypeCount"};
inline constexpr const char* savegame{"Savegame"};
inline constexpr const char* undocumented_footer_bytes{"UndocumentedFooterBytes"};

// Paths from the schema
inline constexpr const char* leader_name_path{"Savegame.CvInitCore.LeaderName"};
inline constexpr const char* multiplayer_options_path{"Savegame.CvInitCore.MPOptions"};
inline constexpr const char* options_path{"Savegame.CvInitCore.Options"};
inline constexpr const char* undocumented_footer_bytes_path{"Savegame.GameFooter.UndocumentedFooterBytes"};

// Filenames
inline constexpr const char* const_definitions_filename{"ConstDefinitions"};
inline constexpr const char* enum_definitions_filename{"EnumDefinitions"};

// File extensions
inline constexpr const char* definitions_extension{".txt"};
inline constexpr const char* crash_dump_extension{".crash-dump.info"};
inline constexpr const char* info_extension{".info"};
inline constexpr const char* translation_extension{".txt"};

// Lengths
inline constexpr size_t checksum_length{32};

// Special member name for struct and template definitions.  The value of the index member is the index into the token
// stream at which the definition for the composite type is found.
inline constexpr const char* index_member{"__Index__"};

} // namespace c4lib::constants
