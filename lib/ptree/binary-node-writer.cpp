// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/30/2024.

#include <boost/property_tree/ptree.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <include/node-attributes.hpp>
#include <include/node-type.hpp>
#include <iosfwd>
#include <lib/io/io.hpp>
#include <lib/ptree/binary-node-writer.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/text.hpp>
#include <string>
#include <unordered_map>
#include <utility>

using namespace std::string_literals;
namespace bpt = boost::property_tree;

namespace c4lib::property_tree {

void Binary_node_writer::finish() {}

void Binary_node_writer::init(const bpt::ptree&, std::ostream& out, std::unordered_map<std::string, std::string>&)
{
    m_out = &out;
}

void Binary_node_writer::write_node(std::pair<int, const bpt::ptree&> depth_node_pair)
{
    const bpt::ptree& node{depth_node_pair.second};
    std::ostream& out{*m_out};

    const bpt::ptree& attributes_node{node.get_child(nn_attributes)};

    switch (const Node_type type{node.get<Node_type>(nn_attributes + "."s + nn_type)}) {
    case Node_type::bool_type:
    case Node_type::hex_type:
    case Node_type::int_type:
    case Node_type::uint_type:
    case Node_type::enum_type: {
        const bpt::ptree& data_node{attributes_node.get_child(nn_data)};
        const size_t size{node.get<size_t>(nn_attributes + "."s + nn_size)};
        assert(size == 1 || size == 2 || size == 4);

        // Get the integer from the ptree and write it to the output stream.
        // Signed integer types
        if (type == Node_type::int_type || type == Node_type::enum_type) {
            if (size == 1) {
                int8_t raw_value{gsl::narrow<int8_t>(data_node.get_value<int>())};
                io::write_int(out, raw_value);
            }
            else if (size == 2) {
                int16_t raw_value{gsl::narrow<int16_t>(data_node.get_value<int>())};
                io::write_int(out, raw_value);
            }
            else {
                int32_t raw_value{data_node.get_value<int>()};
                io::write_int(out, raw_value);
            }
        }
        // Unsigned integer types
        else {
            if (size == 1) {
                uint8_t raw_value{gsl::narrow<uint8_t>(data_node.get_value<uint32_t>())};
                io::write_int(out, raw_value);
            }
            else if (size == 2) {
                uint16_t raw_value{gsl::narrow<uint16_t>(data_node.get_value<uint32_t>())};
                io::write_int(out, raw_value);
            }
            else {
                uint32_t raw_value{data_node.get_value<uint32_t>()};
                io::write_int(out, raw_value);
            }
        }
    } break;

    case Node_type::u16string_type:
    case Node_type::string_type:
    case Node_type::md5_type: {
        const bpt::ptree& data_node{attributes_node.get_child(nn_data)};
        if (type == Node_type::u16string_type) {
            const std::string utf8_string{data_node.get_value<std::string>()};
            // Convert UTF-8 to UTF-16
            const std::u16string utf16_string{text::string_to_u16string(utf8_string)};
            io::write_string(out, utf16_string);
        }
        else {
            const std::string value{data_node.get_value<std::string>()};
            if (type == Node_type::md5_type) {
                if (const size_t md5_length{value.length()}; md5_length != limits::md5_length && md5_length != 0) {
                    throw Parser_error{std::format(fmt::invalid_md5_length, value.length(), limits::md5_length)};
                }
            }
            io::write_string(out, value);
        }
    } break;

    case Node_type::struct_type:
    case Node_type::template_type:
    case Node_type::array_type:
        // Aggregate types lack size and data attributes.
        break;

    default:
        throw Parser_error(std::format(fmt::bad_type_enumeration, to_string(type)));
    }
}

} // namespace c4lib::property_tree
