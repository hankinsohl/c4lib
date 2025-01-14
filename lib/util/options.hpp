// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/28/2024.

#pragma once

namespace c4lib::options {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REQUIRED WHEN LOADING A BTS SAVE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Required: Name of the schema file.  Defaults to BTS.Schema.
inline constexpr const char* schema{"SCHEMA"};

// Required: Path to the root of the BTS installation.
inline constexpr const char* bts_install_dir{"BTS_INSTALL_DIR"};

// Required: Path to the BTS custom assets directory.
inline constexpr const char* custom_assets_dir{"CUSTOM_ASSETS_DIR"};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OPTIONAL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Optional: Set to "1" to omit ASCII column in translations.
inline constexpr const char* omit_ascii_column{"OMIT_ASCII_COLUMN"};

// Optional: Set to "1" to omit hex column in translations.
inline constexpr const char* omit_hex_column{"OMIT_HEX_COLUMN"};

// Optional: Set to "1" to omit offset column in translations.
inline constexpr const char* omit_offset_column{"OMIT_OFFSET_COLUMN"};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Optional: Name of directory into which files are written.  If not specified, the current directory is used.
inline constexpr const char* debug_output_dir{"DEBUG_OUTPUT_DIR"};

// Optional: Set to "1" to write miscellaneous binaries associated with zlib inflation/deflation.
inline constexpr const char* debug_write_binaries{"DEBUG_WRITE_BINARIES"};

// Optional: Set to "1" to write imported enums and constants.
inline constexpr const char* debug_write_imports{"DEBUG_WRITE_IMPORTS"};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OPTIONAL - USED WHEN LOADING A BTS SAVE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Optional: Name of mod associated with the save.  Leave blank if the save isn't for a mod.
inline constexpr const char* mod_name{"MOD_NAME"};

// Optional: Set to "1" if modular loading should be used.  Leave blank to use normal loading.
inline constexpr const char* use_modular_loading{"USE_MODULAR_LOADING"};
} // namespace c4lib::options
