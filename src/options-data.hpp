// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/13/2024.

#pragma once

#include <lib/options/options-manager.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace hopts = hankinsohl::options;

namespace c4edit::options {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USE CONFIG FILE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const hopts::Option_info config_file_option_info{.name = "CONFIG_FILE",
    .help_type = "<filename>",
    .help_meaning = "Name of a config file from which to read options.",
    .help_sort_order = 100,
    .type = hopts::Option_type::text,
    .default_value = "",
    .required = false,
    .depends_on = {}};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILE TO LOAD - ONE REQUIRED
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const hopts::Option_info load_save_option_info{.name = "LOAD_SAVE",
    .help_type = "<filename>",
    .help_meaning = "Name of a .CivBeyondSwordSave to load.  Either a BTS save or an info file must be loaded.",
    .help_sort_order = 200,
    .type = hopts::Option_type::text,
    .default_value = "",
    .required = false,
    .depends_on = {"BTS_INSTALL_DIR", "CUSTOM_ASSETS_DIR", "SCHEMA"}};

inline const hopts::Option_info load_info_option_info{.name = "LOAD_INFO",
    .help_type = "<filename>",
    .help_meaning = "Name of an info file to load.  Either a BTS save or an info file must be loaded.",
    .help_sort_order = 210,
    .type = hopts::Option_type::text,
    .default_value = "",
    .required = false,
    .depends_on = {}};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILE(S) TO SAVE - AT LEAST ONE REQUIRED
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const hopts::Option_info write_translation_option_info{.name = "WRITE_TRANSLATION",
    .help_type = "<filename>",
    .help_meaning = "Write a text file translation of the save to filename.",
    .help_sort_order = 400,
    .type = hopts::Option_type::text,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

inline const hopts::Option_info write_info_option_info{.name = "WRITE_INFO",
    .help_type = "<filename>",
    .help_meaning = "Write an info file for the save to filename.  Info files can be edited to change a save.",
    .help_sort_order = 410,
    .type = hopts::Option_type::text,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

inline const hopts::Option_info write_save_option_info{.name = "WRITE_SAVE",
    .help_type = "<filename>",
    .help_meaning = "Write a BTS save to filename.  Use this option to convert an info file to a BTS save.",
    .help_sort_order = 420,
    .type = hopts::Option_type::text,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LOGGING
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const hopts::Option_info log_info{.name = "LOG",
    .help_type = "[0|1]",
    .help_meaning = "Set to 1 to log diagnostic messages to the log file.",
    .help_sort_order = 700,
    .type = hopts::Option_type::boolean,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EXE OPTIONS INFO
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const std::unordered_map<std::string, hopts::Option_info> exe_options_info_lookup{
    {config_file_option_info.name, config_file_option_info},

    {load_save_option_info.name, load_save_option_info},
    {load_info_option_info.name, load_info_option_info},

    {write_translation_option_info.name, write_translation_option_info},
    {write_info_option_info.name, write_info_option_info},
    {write_save_option_info.name, write_save_option_info},

    {log_info.name, log_info},
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EXE OPTIONS AGGREGATE INFO
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const std::vector<std::string> requires_one_load_option{
    "LOAD_SAVE",
    "LOAD_INFO",
};

inline const std::vector<std::string> requires_one_write_option{
    "WRITE_TRANSLATION",
    "WRITE_INFO",
    "WRITE_SAVE",
};

inline const std::vector<std::string> multiple_load_options_are_incompatible{
    "LOAD_SAVE",
    "LOAD_INFO",
};

} // namespace c4edit::options
