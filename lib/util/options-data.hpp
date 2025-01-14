// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/13/2024.

#pragma once

#include <lib/options/options-manager.hpp>
#include <string>
#include <unordered_map>

namespace hopts = hankinsohl::options;

namespace c4lib::options {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USED WHEN LOADING CIV4 SAVE - REQUIRED
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const hopts::Option_info schema_option_info{.name = "SCHEMA",
    .help_type = "<filename>",
    .help_meaning = "Name of the schema file.  Defaults to BTS.Schema.  Required to load a BTS save.",
    .help_sort_order = 320,
    .type = hopts::Option_type::text,
    .default_value = "BTS.Schema",
    .required = false,
    .depends_on = {}};

inline const hopts::Option_info bts_install_dir_option_info{.name = "BTS_INSTALL_DIR",
    .help_type = "<directory>",
    .help_meaning = "Name of root BTS install directory.  Required to load a BTS save.",
    .help_sort_order = 330,
    .type = hopts::Option_type::text,
    .default_value = "",
    .required = false,
    .depends_on = {}};

inline const hopts::Option_info custom_assets_dir_option_info{.name = "CUSTOM_ASSETS_DIR",
    .help_type = "<directory>",
    .help_meaning = "Name of BTS custom assets directory.  Required to load a BTS save.",
    .help_sort_order = 340,
    .type = hopts::Option_type::text,
    .default_value = "",
    .required = false,
    .depends_on = {}};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USED WHEN LOADING CIV4 SAVE - OPTIONAL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const hopts::Option_info mod_name_option_info{.name = "MOD_NAME",
    .help_type = "<name>",
    .help_meaning = "If the BTS save is for a mod, the mod name.  Do not use unless the BTS save is for a mod.",
    .help_sort_order = 350,
    .type = hopts::Option_type::text,
    .default_value = "",
    .required = false,
    .depends_on = {}};

inline const hopts::Option_info use_modular_loading_option_info{.name = "USE_MODULAR_LOADING",
    .help_type = "[0|1]",
    .help_meaning = "Set to 1 if modular loading is used.  Do not use unless the save uses modular loading.",
    .help_sort_order = 360,
    .type = hopts::Option_type::boolean,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TRANSLATION - OPTIONAL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const hopts::Option_info omit_offset_column_option_info{.name = "OMIT_OFFSET_COLUMN",
    .help_type = "[0|1]",
    .help_meaning = "Set to 1 to omit the offset column when generating translation files.",
    .help_sort_order = 620,
    .type = hopts::Option_type::boolean,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

inline const hopts::Option_info omit_hex_column_option_info{.name = "OMIT_HEX_COLUMN",
    .help_type = "[0|1]",
    .help_meaning = "Set to 1 to omit the hex column when generating translation files.",
    .help_sort_order = 630,
    .type = hopts::Option_type::boolean,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

inline const hopts::Option_info omit_ascii_column_option_info{.name = "OMIT_ASCII_COLUMN",
    .help_type = "[0|1]",
    .help_meaning = "Set to 1 to omit the ASCII column when generating translation files.",
    .help_sort_order = 640,
    .type = hopts::Option_type::boolean,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG - OPTIONAL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const hopts::Option_info debug_output_dir_option_info{.name = "DEBUG_OUTPUT_DIR",
    .help_type = "<directory>",
    .help_meaning
    = "Name of directory into which debug files are written.  If not specified, the current directory is used.",
    .help_sort_order = 720,
    .type = hopts::Option_type::text,
    .default_value = "",
    .required = false,
    .depends_on = {}};

inline const hopts::Option_info debug_write_binaries_option_info{.name = "DEBUG_WRITE_BINARIES",
    .help_type = "[0|1]",
    .help_meaning = "Write various binary files generated internally by the library.",
    .help_sort_order = 730,
    .type = hopts::Option_type::boolean,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

inline const hopts::Option_info debug_write_imports_option_info{.name = "DEBUG_WRITE_IMPORTS",
    .help_type = "[0|1]",
    .help_meaning = "Write imported enums and constants.",
    .help_sort_order = 740,
    .type = hopts::Option_type::boolean,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LIB OPTIONS INFO
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const std::unordered_map<std::string, hopts::Option_info> lib_options_info_lookup{
    {bts_install_dir_option_info.name, bts_install_dir_option_info},
    {custom_assets_dir_option_info.name, custom_assets_dir_option_info},

    {schema_option_info.name, schema_option_info},
    {mod_name_option_info.name, mod_name_option_info},
    {use_modular_loading_option_info.name, use_modular_loading_option_info},

    {omit_offset_column_option_info.name, omit_offset_column_option_info},
    {omit_hex_column_option_info.name, omit_hex_column_option_info},
    {omit_ascii_column_option_info.name, omit_ascii_column_option_info},

    {debug_output_dir_option_info.name, debug_output_dir_option_info},
    {debug_write_binaries_option_info.name, debug_write_binaries_option_info},
    {debug_write_imports_option_info.name, debug_write_imports_option_info},
};

} // namespace c4lib::options
