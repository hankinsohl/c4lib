// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/25/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <string>
#include <unordered_map>

// @formatter:off
/**
 *  The main c4lib API.\n
 *  Each API function takes a name-value pair of options stored in an unordered_map.  Options\n
 *  are not required except when calling read_save.  Options are as follows:\n
 *  <pre>
 *    SCHEMA               <filename>          Name of the schema file.  Defaults to
 *                                             BTS.Schema.  Required for read_save.
 *    BTS_INSTALL_DIR      <directory>         Name of root BTS install directory.
 *                                             Required for read_save.
 *    CUSTOM_ASSETS_DIR    <directory>         Name of BTS custom assets directory.
 *                                             Required for read_save.
 *    MOD_NAME             <name>              If the BTS save is for a mod, the mod name.
 *                                             Do not use unless the BTS save is for a mod.
 *    USE_MODULAR_LOADING  [0|1]               Set to 1 if modular loading is used.
 *                                             Do not use unless the save uses modular loading.
 *    OMIT_OFFSET_COLUMN   [0|1]               Set to 1 to omit the offset column when
 *                                             generating translation files.
 *    OMIT_HEX_COLUMN      [0|1]               Set to 1 to omit the hex column when
 *                                             generating translation files.
 *    OMIT_ASCII_COLUMN    [0|1]               Set to 1 to omit the ASCII column when
 *                                             generating translation files.
 *    LOG                  [0|1]               Set to 1 to log diagnostic messages to
 *                                             the log file.
 *    DEBUG_OUTPUT_DIR     <directory>         Name of directory into which debug files
 *                                             are written.  If not specified, the current
 *                                             directory is used.
 *    DEBUG_WRITE_BINARIES [0|1]               Write various binary files generated internally
 *                                             by the library.
 *    DEBUG_WRITE_IMPORTS  [0|1]               Write imported enums and constants.</pre>
 */
// @formatter:on
namespace c4lib {
/**
 * Reads a .info-format file.
 * @param pt output property tree.  pt will contain a representation of the .info file upon return.
 * @param filename path to the .info-format file.
 * @param options options to use.  No options are currently supported.
 */
void read_info(boost::property_tree::ptree& pt,
    const std::string& filename,
    std::unordered_map<std::string, std::string>& options);

/**
 * Reads a .CivBeyondSwordSave save.
 * @param pt output property tree.  pt will contain a representation of the save upon return.
 * @param filename path to the save.
 * @param options options to use.  SCHEMA, BTS_INSTALL_DIR and CUSTOM_ASSETS_DIR are required.
 */
void read_save(boost::property_tree::ptree& pt,
    const std::string& filename,
    std::unordered_map<std::string, std::string>& options);

/**
 * Writes a .info-format file.
 * @param pt property tree to save in .info-file format.
 * @param filename path to .info-format file to create.  An existing file is overwritten.
 * @param options options to use.  No options are currently supported.
 */
void write_info(const boost::property_tree::ptree& pt,
    const std::string& filename,
    std::unordered_map<std::string, std::string>& options);

/**
 * Writes a .CivBeyondSwordSave save.
 * @param pt property tree to save as a .CivBeyondSwordSave file.
 * @param filename path to save file to create.  An existing file is overwritten.
 * @param options options to use.
 */
void write_save(const boost::property_tree::ptree& pt,
    const std::string& filename,
    std::unordered_map<std::string, std::string>& options);

/**
 * Writes a translation.  A translation is a human-readable text file representing a save.
 * @param pt property tree to save as translation.
 * @param filename path to translation file to create.  An existing file is overwritten.
 * @param options options to use.
 */
void write_translation(const boost::property_tree::ptree& pt,
    const std::string& filename,
    std::unordered_map<std::string, std::string>& options);
} // namespace c4lib
