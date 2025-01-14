// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/28/2024.

#include <boost/property_tree/ptree.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <include/exceptions.hpp>
#include <include/node-attributes.hpp>
#include <include/node-type.hpp>
#include <iosfwd>
#include <lib/io/io.hpp>
#include <lib/ptree/binary-node-reader.hpp>
#include <lib/ptree/internationalization-text.hpp>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/util/constants.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/schema.hpp>
#include <lib/util/text.hpp>
#include <lib/util/util.hpp>
#include <lib/zlib/zlib-engine.hpp>
#include <string>

namespace bpt = boost::property_tree;
namespace cpt = c4lib::property_tree;
namespace csp = c4lib::schema_parser;
namespace czlib = c4lib::zlib;

namespace {
template<typename T> void add_data(
    std::istream& in, bpt::ptree& attributes_node, cpt::Node_type type, size_t size, csp::Def_tbl& definition_table)
{
    T value;
    c4lib::io::read_int(in, value);
    if (std::is_signed_v<T>) {
        // Cast to int32_t to avoid to_string(char)
        attributes_node.add(cpt::nn_data, std::to_string(static_cast<int32_t>(value)));
    }
    else {
        attributes_node.add(cpt::nn_data, std::to_string(value));
    }

    switch (type) {
    case cpt::Node_type::bool_type:
        attributes_node.add(cpt::nn_formatted_data, value ? cpt::text_true : cpt::text_false);
        break;

    case cpt::Node_type::hex_type: {
        static const char* const fmt_for_size{"0x{{:0{}x}}"};
        const std::string fmt{std::vformat(fmt_for_size, std::make_format_args(c4lib::unmove(size * 2)))};
        const std::string formatted_data{std::vformat(fmt, std::make_format_args(c4lib::unmove(value)))};
        attributes_node.add(cpt::nn_formatted_data, formatted_data);
    } break;

    case cpt::Node_type::int_type:
    case cpt::Node_type::uint_type:
        attributes_node.add(cpt::nn_formatted_data, std::to_string(value));
        break;

    case cpt::Node_type::enum_type: {
        const std::string enum_name{attributes_node.get<std::string>(cpt::nn_enum)};
        const csp::Def_mem& enum_def{definition_table.get_enumerator(enum_name, gsl::narrow<int>(value))};
        attributes_node.add(cpt::nn_formatted_data, enum_def.name);
    } break;

    default:
        throw c4lib::Parser_error(std::format(c4lib::fmt::bad_type_enumeration, to_string(type)));
    }
}

} // namespace

namespace c4lib::property_tree {

size_t Binary_node_reader::get_undocumented_footer_bytes_count()
{
    return m_undocumented_footer_bytes_count;
}

void Binary_node_reader::init_impl_()
{
    // Prepare the stream for binary input
    m_save.unsetf(std::ios::skipws);

    // Read the savegame into m_save.
    czlib::ZLib_engine zlib;
    size_t count_header{limits::invalid_size};
    size_t count_compressed{limits::invalid_size};
    size_t count_decompressed{limits::invalid_size};
    size_t count_footer{limits::invalid_size};
    size_t count_total{limits::invalid_size};
    zlib.inflate(
        m_filename, m_save, count_header, count_compressed, count_decompressed, count_footer, count_total, *m_options);
    m_save.seekg(0);

    // The number of bytes in the undocumented footer equals the total savegame file size minus the
    // header size - 4 pad bytes - size of decompressed data minus the checksum byte minus the savegame
    // md5 checksum characters plus preceding length.
    m_undocumented_footer_bytes_count
        = count_total - count_header - 4 - count_decompressed - 1 - (4 + constants::checksum_length);
}

void Binary_node_reader::read_node_impl_(bpt::ptree& node)
{
    bpt::ptree& attributes_node{node.get_child(nn_attributes)};

    const bpt::ptree& type_node{attributes_node.get_child(nn_type)};
    bpt::ptree& type_name_node{attributes_node.get_child(nn_typename)};

    switch (const Node_type type{type_node.get_value<Node_type>()}) {
    case Node_type::bool_type:
    case Node_type::hex_type:
    case Node_type::int_type:
    case Node_type::uint_type:
    case Node_type::enum_type: {
        const std::string size_string{size_from_type(type_name_node.data())};
        attributes_node.add(nn_size, size_string);

        // Note: size_from_type checks that size is 1, 2 or 4.
        // We use an assert to ensure that this is so.
        const size_t size{std::stoul(size_string)};
        assert(size == 1 || size == 2 || size == 4);

        // Read the integer and set the node data.
        // Signed integer types
        if (type == Node_type::int_type || type == Node_type::enum_type) {
            if (size == 1) {
                add_data<int8_t>(m_save, attributes_node, type, size, *m_definition_table);
            }
            else if (size == 2) {
                add_data<int16_t>(m_save, attributes_node, type, size, *m_definition_table);
            }
            else {
                add_data<int32_t>(m_save, attributes_node, type, size, *m_definition_table);
            }
        }
        // Unsigned integer types
        else {
            if (size == 1) {
                add_data<uint8_t>(m_save, attributes_node, type, size, *m_definition_table);
            }
            else if (size == 2) {
                add_data<uint16_t>(m_save, attributes_node, type, size, *m_definition_table);
            }
            else {
                add_data<uint32_t>(m_save, attributes_node, type, size, *m_definition_table);
            }
        }
    } break;

    case Node_type::u16string_type:
    case Node_type::string_type:
    case Node_type::md5_type: {
        if (type == Node_type::u16string_type) {
            std::u16string wide_string;
            io::read_string(m_save, wide_string);
            // Convert the UTF-16 value to UTF-8.
            std::string utf8_string{text::u16string_to_string(wide_string)};
            attributes_node.add(nn_data, utf8_string);
            utf8_string = "\"" + utf8_string + "\"";
            attributes_node.add(nn_formatted_data, utf8_string);
        }
        else {
            std::string char_string;
            io::read_string(m_save, char_string);
            if (type == Node_type::md5_type) {
                const size_t md5_length{char_string.length()};
                if (md5_length != limits::md5_length && md5_length != 0) {
                    throw Parser_error(std::format(fmt::invalid_md5_length, char_string.length(), limits::md5_length));
                }
            }
            attributes_node.add(nn_data, char_string);
            char_string = "\"" + char_string + "\"";
            attributes_node.add(nn_formatted_data, char_string);
        }
    } break;

    case Node_type::struct_type:
    case Node_type::template_type:
        // Aggregate types lack size and data attributes.
        break;

        // If the node type isn't one of the types previously processed, an error has occurred.
        // By design, the Generative_node_source should not emit array_type nodes, so if we're passed one
        // it's an error.
    case Node_type::array_type:
    default:
        throw Parser_error(std::format(fmt::bad_type_enumeration, to_string(type)));
    }
}

} // namespace c4lib::property_tree
