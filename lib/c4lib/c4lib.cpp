// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/25/2024.

#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <c4lib-version.hpp>
#include <chrono>
#include <cstddef>
#include <exception>
#include <format>
#include <fstream>
#include <include/c4lib.hpp>
#include <include/logger.hpp>
#include <include/node-attributes.hpp>
#include <ios>
#include <iosfwd>
#include <lib/c4lib/c4lib-internal.hpp>
#include <lib/io/io.hpp>
#include <lib/logger/log-formats.hpp>
#include <lib/md5/checksum.hpp>
#include <lib/native/path.hpp>
#include <lib/ptree/binary-node-reader.hpp>
#include <lib/ptree/binary-node-writer.hpp>
#include <lib/ptree/recursive-node-source.hpp>
#include <lib/ptree/translation-node-writer.hpp>
#include <lib/ptree/util.hpp>
#include <lib/schema-parser/parser.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/options.hpp>
#include <lib/util/timer.hpp>
#include <lib/zlib/zlib-engine.hpp>
#include <string>
#include <unordered_map>

namespace bpt = boost::property_tree;
namespace cpt = c4lib::property_tree;
namespace csp = c4lib::schema_parser;
namespace czlib = c4lib::zlib;

namespace {
template<typename F, typename N, typename... Args> void dispatch_(F func, const N& name, Args&... args)
{
    try {
        c4lib::Logger::info(std::format(c4lib::fmt::calling, name));
        c4lib::Timer timer;
        timer.start();
        func(std::forward<Args&>(args)...);
        c4lib::Logger::info(std::format(c4lib::fmt::finished_in, name, timer.to_string()));
    }
    catch (const std::exception& ex) {
        c4lib::Logger::error(std::format(c4lib::fmt::caught_std_exception, ex.what()));
        throw;
    }
    catch (...) {
        c4lib::Logger::error(c4lib::fmt::caught_unknown_exception);
        throw;
    }
}

void read_info_dispatch_(bpt::ptree& pt, const std::string& filename)
{
    bpt::read_info(c4lib::native::Path{filename}, pt);
}

void read_save_dispatch_(
    bpt::ptree& pt, const std::string& filename, std::unordered_map<std::string, std::string>& options)
{
    // Clear the ptree and add an origin node.
    pt.clear();
    bpt::ptree& origin{pt.put_child(cpt::nn_origin, bpt::ptree{cpt::nv_meta})};
    const c4lib::native::Path filename_path{filename};
    const c4lib::native::Path schema_path{options[c4lib::options::schema]};
    origin.add(cpt::nn_savegame, filename_path.str());
    origin.add(cpt::nn_schema, schema_path.str());
    // Note: std::chrono::current_zone() is not yet fully implemented by most compilers; therefore
    // we'll use UTC instead of local time.
    const auto now{std::chrono::system_clock::now()};
    origin.add(cpt::nn_date, std::format("{:%m-%d-%Y %H:%M:%OS} UTC", now));
    origin.add(cpt::nn_c4lib_version, c4lib::constants::c4lib_version);

    const c4lib::native::Path custom_assets_path{options[c4lib::options::custom_assets_dir]};
    const c4lib::native::Path install_path{options[c4lib::options::bts_install_dir]};
    const std::string mod_name{options[c4lib::options::mod_name]};
    const bool use_modular_loading{options[c4lib::options::use_modular_loading] == "1"};

    cpt::Binary_node_reader binary_node_reader;
    csp::Parser parser;
    parser.parse(schema_path, install_path, custom_assets_path, mod_name, use_modular_loading, pt, filename_path,
        binary_node_reader, options);
}

void write_composite_dispatch_(
    const bpt::ptree& pt, std::ostream& out, std::unordered_map<std::string, std::string>& options)
{
    cpt::Binary_node_writer writer;
    writer.init(pt, out, options);
    for (const cpt::Recursive_node_source node_source{&pt, cpt::skip_meta_nodes}; const auto& pr : node_source) {
        writer.write_node(pr);
    }
}

void write_info_dispatch_(const bpt::ptree& pt, const std::string& filename)
{
    bpt::write_info(c4lib::native::Path{filename}, pt);
}

void write_save_dispatch_(
    const bpt::ptree& pt, const std::string& filename, std::unordered_map<std::string, std::string>& options)
{
    const c4lib::native::Path filename_path{filename};

    // Generate a composite savegame stream as input to deflate.
    std::stringstream composite;
    composite.unsetf(std::ios::skipws);
    c4lib::write_composite(pt, composite, options);

    // Deflate the composite savegame stream.
    czlib::ZLib_engine engine;
    std::stringstream binary_savegame;
    binary_savegame.unsetf(std::ios::skipws);
    const size_t count_footer{cpt::get_footer_size(pt)};
    size_t count_header{c4lib::limits::invalid_size};
    size_t count_compressed{c4lib::limits::invalid_size};
    size_t count_decompressed{c4lib::limits::invalid_size};
    size_t count_total{c4lib::limits::invalid_size};

    engine.deflate(filename_path, composite, binary_savegame, count_footer, count_header, count_compressed,
        count_decompressed, count_total, options);

    // Calculate the checksum for the savegame.
    const int max_players{cpt::get_max_players(pt)};
    const int num_game_option_types{cpt::get_num_game_option_types(pt)};
    const int num_multiplayer_option_types{cpt::get_num_multiplayer_option_types(pt)};
    c4lib::md5::Checksum checksum(binary_savegame, max_players, num_game_option_types, num_multiplayer_option_types);
    const std::string md5{checksum.get_hash()};

    // Position savegame to checksum location.  The checksum is the final field written to the savegame and is
    // written as a civ4 string (4 byte length followed by characters in string).
    const std::streamoff checksum_offset{4 + gsl::narrow<std::streamoff>(md5.length())};
    binary_savegame.seekp(-checksum_offset, std::ios_base::end);

    // Write the checksum.  write_string also writes the string length.
    c4lib::io::write_string(binary_savegame, md5);

    // Write the savegame to the destination file.
    c4lib::io::write_binary_stream_to_file(binary_savegame, 0, 0, filename_path);
}

void write_translation_dispatch_(
    const bpt::ptree& pt, const std::string& filename, std::unordered_map<std::string, std::string>& options)
{
    c4lib::native::Path filename_path{filename};
    std::ofstream out{filename_path, std::ios_base::out};
    if (!out.is_open() || out.bad()) {
        throw std::runtime_error{std::format(c4lib::fmt::runtime_error_opening_file, filename_path)};
    }

    cpt::Translation_node_writer writer;
    writer.init(pt, out, options);
    for (const cpt::Recursive_node_source node_source{&pt, cpt::skip_meta_nodes}; const auto& pr : node_source) {
        writer.write_node(pr);
    }
    writer.finish();
}

} // namespace

namespace c4lib {

void read_info(bpt::ptree& pt, const std::string& filename, std::unordered_map<std::string, std::string>&)
{
    dispatch_(read_info_dispatch_, "read_info", pt, filename);
}

void read_save(bpt::ptree& pt, const std::string& filename, std::unordered_map<std::string, std::string>& options)
{
    dispatch_(read_save_dispatch_, "read_save", pt, filename, options);
}

void write_composite(const bpt::ptree& pt, std::ostream& out, std::unordered_map<std::string, std::string>& options)
{
    dispatch_(write_composite_dispatch_, "write_composite", pt, out, options);
}

void write_info(const bpt::ptree& pt, const std::string& filename, std::unordered_map<std::string, std::string>&)
{
    dispatch_(write_info_dispatch_, "write_info", pt, filename);
}

void write_save(
    const bpt::ptree& pt, const std::string& filename, std::unordered_map<std::string, std::string>& options)
{
    dispatch_(write_save_dispatch_, "write_save", pt, filename, options);
}

void write_translation(
    const bpt::ptree& pt, const std::string& filename, std::unordered_map<std::string, std::string>& options)
{
    dispatch_(write_translation_dispatch_, "write_translation", pt, filename, options);
}

} // namespace c4lib
