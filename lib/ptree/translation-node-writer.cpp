// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/9/2024.

#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <format>
#include <include/exceptions.hpp>
#include <include/node-attributes.hpp>
#include <include/node-type.hpp>
#include <ios>
#include <iosfwd>
#include <lib/io/io.hpp>
#include <lib/ptree/internationalization-text.hpp>
#include <lib/ptree/translation-node-writer.hpp>
#include <lib/util/constants.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/options.hpp>
#include <lib/util/text.hpp>
#include <lib/util/util.hpp>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std::string_literals;
namespace bpt = boost::property_tree;

namespace c4lib::property_tree {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Translation_node_writer::finish()
{
    print_end_translations_(0);
}

void Translation_node_writer::init(
    const bpt::ptree& root, std::ostream& out, std::unordered_map<std::string, std::string>& options)
{
    m_out = &out;

    m_ascii_column_enabled = (options[options::omit_ascii_column] != "1");
    m_hex_column_enabled = (options[options::omit_hex_column] != "1");
    m_offset_column_enabled = (options[options::omit_offset_column] != "1");

    m_consolidated_data.reserve(constants::translation_max_bytes_per_line);

    print_origin_info_(root);
    print_column_header_();
}

void Translation_node_writer::write_node(std::pair<int, const boost::property_tree::ptree&> depth_node_pair)
{
    const int depth{depth_node_pair.first};
    if (depth < m_depth) {
        print_end_translations_(depth);
    }
    m_depth = depth;

    std::vector<uint8_t> data;
    constexpr size_t max_expected_node_data{128};
    data.reserve(max_expected_node_data);
    std::string translation;
    bool is_empty_aggregate{false};
    get_node_data_and_translation_(depth_node_pair, data, translation, is_empty_aggregate);
    if (is_empty_aggregate || (m_is_output_consolidating && !m_is_consolidated_output_ready)) {
        return;
    }

    size_t span_begin{0};
    size_t span_length{std::min(gsl::narrow<size_t>(constants::translation_max_bytes_per_line), data.size())};
    const std::span<uint8_t> full_span{data};
    do {
        const std::span<uint8_t> sp{full_span.subspan(span_begin, span_length)};
        print_offset_();
        print_hex_(sp);
        print_ascii_(sp);
        print_translation_(translation);
        m_offset += gsl::narrow<std::streamoff>(span_length);

        // If data exceeds 16 characters, additional output lines will be generated.
        span_begin += span_length;
        span_length
            = std::min(gsl::narrow<std::size_t>(constants::translation_max_bytes_per_line), data.size() - span_begin);
        translation = "...";
    }
    while (span_length);

    if (m_is_output_consolidating) {
        m_is_consolidated_output_ready = false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string Translation_node_writer::get_consolidated_translation_value_equals_prefix_() const
{
    size_t start{m_count_consolidated};
    size_t end{
        std::min(m_count_consolidated + constants::translation_max_bytes_per_line - 1, m_consolidated_data_size - 1)};
    return std::format("[{}-{}]=", start, end);
}

void Translation_node_writer::get_node_data_and_translation_(const std::pair<int, const bpt::ptree&>& pr,
    std::vector<uint8_t>& data,
    std::string& translation,
    bool& is_empty_aggregate)
{
    is_empty_aggregate = false;
    if (m_is_output_consolidating) {
        gndt_consolidated_(pr, data, translation);
    }
    else {
        gndt_not_consolidated_(pr, data, translation, is_empty_aggregate);
    }
}

void Translation_node_writer::gndt_consolidated_(
    const std::pair<int, const bpt::ptree&>& pr, std::vector<uint8_t>& data, std::string& translation)
{
    const bpt::ptree& node{pr.second};

    // Add this node's content to the pending data and translation.
    const uint8_t raw_value{node.get<uint8_t>(nn_attributes + "."s + nn_data)};
    m_consolidated_data.emplace_back(raw_value);
    m_consolidated_translation << node.get<std::string>(nn_attributes + "."s + nn_formatted_data) << ' ';

    // Update the number of bytes we've consolidated
    ++m_count_consolidated;

    // Check to see if we should output the consolidated data.  Output occurs every 16th byte
    // or when all data has been processed.
    if (m_consolidated_data.size() == constants::translation_max_bytes_per_line
        || m_count_consolidated == m_consolidated_data_size) {
        data = m_consolidated_data;
        m_consolidated_data.clear();
        translation = m_consolidated_translation.str();
        std::stringstream{}.swap(m_consolidated_translation);
        m_consolidated_translation << get_consolidated_translation_value_equals_prefix_();
        m_is_consolidated_output_ready = true;
    }

    // If we've processed all data, disable consolidated output
    if (m_count_consolidated == m_consolidated_data_size) {
        m_is_output_consolidating = false;
    }
}

// TODO: Consider refactoring the info representation of byte arrays, up to 16 bytes per node
// and with subscripts reflecting the [start-end] format currently used.  This refactor has
// the following advantages:
//     1) It eliminates the need for consolidating output, thus simplifying the translation node writer;
//     2) It reduces the number of nodes in the info output, which should slightly improve processing
//        speed.
void Translation_node_writer::gndt_not_consolidated_(const std::pair<int, const bpt::ptree&>& pr,
    std::vector<uint8_t>& data,
    std::string& translation,
    bool& is_empty_aggregate)
{
    const bpt::ptree& node{pr.second};

    auto type{node.get<Node_type>(nn_attributes + "."s + nn_type)};
    const boost::optional<const bpt::ptree&> subscripts_node_optional{
        node.get_child_optional(nn_attributes + "."s + nn_subscripts)};

    // Process aggregate types.  Note that aggregate types have no data.
    const boost::optional<const bpt::ptree&> data_node_optional{
        node.get_child_optional(nn_attributes + "."s + nn_data)};
    if (!data_node_optional) {
        // If this is an array with no members, set the is_empty_aggregate flag so that a Begin Array
        // line is not generated.
        size_t array_dimension{limits::invalid_size};
        if (type == Node_type::array_type) {
            array_dimension = gsl::narrow<size_t>(std::stoi(subscripts_node_optional->data().substr(1)));
            if (array_dimension == 0) {
                is_empty_aggregate = true;
                return;
            }
        }

        const std::string aggregate_name{node.get<std::string>(nn_attributes + "."s + nn_name)};
        const std::string subscripts{node.get<std::string>(nn_attributes + "."s + nn_subscripts, "")};
        std::string full_name;
        if (type == Node_type::array_type) {
            full_name = aggregate_name + subscripts;
        }
        else if (!subscripts.empty()) {
            full_name = subscripts;
        }
        else {
            full_name = aggregate_name;
        }

        m_aggregate_name_stack.push(full_name);
        translation = text_begin + " "s + full_name;

        // If this is an array of bytes, consolidate the output, printing 16 bytes per line of translation.
        if (type == Node_type::array_type && node.get<std::string>(nn_attributes + "."s + nn_typename) == "hex8") {
            m_is_output_consolidating = true;
            m_consolidated_data_size = array_dimension;
            m_count_consolidated = 0;
            // Start by setting m_is_consolidated_output_ready true in order to print the Begin XXX line.
            m_is_consolidated_output_ready = true;
            std::stringstream{}.swap(m_consolidated_translation);
            m_consolidated_translation << get_consolidated_translation_value_equals_prefix_();
            m_consolidated_data.clear();
        }

        return;
    }

    std::string translation_name;
    if (subscripts_node_optional) {
        translation_name = subscripts_node_optional->data();
    }
    else {
        translation_name = node.get<std::string>(nn_attributes + "."s + nn_name);
    }
    const std::string value{node.get<std::string>(nn_attributes + "."s + nn_formatted_data, "")};
    translation = translation_name + "=" + value;

    const bpt::ptree& data_node{*data_node_optional};
    switch (type) {
    case Node_type::bool_type:
    case Node_type::hex_type:
    case Node_type::int_type:
    case Node_type::uint_type:
    case Node_type::enum_type: {
        const size_t size{node.get<size_t>(nn_attributes + "."s + nn_size)};
        assert(size == 1 || size == 2 || size == 4);
        data.resize(size);

        // Get the integer from the node and write it to the vector.
        // Signed integer types
        if (type == Node_type::int_type || type == Node_type::enum_type) {
            if (size == 1) {
                int8_t raw_value{data_node.get_value<int8_t>()};
                io::write_int(data.data(), raw_value);
            }
            else if (size == 2) {
                int16_t raw_value{data_node.get_value<int16_t>()};
                io::write_int(data.data(), raw_value);
            }
            else {
                int32_t raw_value{data_node.get_value<int32_t>()};
                io::write_int(data.data(), raw_value);
            }
        }
        // Unsigned integer types
        else {
            if (size == 1) {
                uint8_t raw_value{data_node.get_value<uint8_t>()};
                io::write_int(data.data(), raw_value);
            }
            else if (size == 2) {
                uint16_t raw_value{data_node.get_value<uint16_t>()};
                io::write_int(data.data(), raw_value);
            }
            else {
                uint32_t raw_value{data_node.get_value<uint32_t>()};
                io::write_int(data.data(), raw_value);
            }
        }
    } break;

    case Node_type::u16string_type:
    case Node_type::string_type:
    case Node_type::md5_type: {
        if (type == Node_type::u16string_type) {
            const std::string utf8_string{data_node.get_value<std::string>()};
            // Convert UTF-8 to UTF-16
            const std::u16string utf16_string{text::string_to_u16string(utf8_string)};
            data.resize(4 + (utf16_string.length() * sizeof(utf16_string[0])));
            io::write_string(data.data(), utf16_string);
        }
        else {
            const std::string char_string{data_node.get_value<std::string>()};
            if (type == Node_type::md5_type) {
                const size_t md5_length{char_string.length()};
                if (md5_length != limits::md5_length && md5_length != 0) {
                    throw Parser_error(std::format(fmt::invalid_md5_length, char_string.length(), limits::md5_length));
                }
            }
            data.resize(4 + (char_string.length() * sizeof(char_string[0])));
            io::write_string(data.data(), char_string);
        }
    } break;

    default:
        break;
    }
}

void Translation_node_writer::print_ascii_(const std::span<uint8_t>& sp) const
{
    if (!m_ascii_column_enabled) {
        return;
    }

    std::ostream& out{*m_out};
    for (const uint8_t byte : sp) {
        if (std::isprint(byte)) {
            out << static_cast<char>(byte);
        }
        else {
            out << ".";
        }
    }
    for (size_t written{sp.size()}; written < constants::translation_max_bytes_per_line; ++written) {
        out << "-";
    }
    out << " | ";
}

void Translation_node_writer::print_ascii_column_title_() const
{
    static constexpr size_t ascii_column_width{19};
    print_column_title_(text_ascii, ascii_column_width, m_ascii_column_enabled);
}

void Translation_node_writer::print_column_header_() const
{
    print_offset_column_title_();
    print_hex_column_title_();
    print_ascii_column_title_();
    print_translation_column_title_();
    *m_out << '\n';
}

void Translation_node_writer::print_column_title_(const std::string& title, size_t width, bool enabled) const
{
    if (!enabled) {
        return;
    }

    std::ostream& out{*m_out};
    static const char* const fmt_for_size{"{{:<{}}}"};
    const std::string fmt{std::vformat(fmt_for_size, std::make_format_args(unmove(width)))};
    out << std::vformat(fmt, std::format_args(std::make_format_args(unmove(title))));
}

void Translation_node_writer::print_end_translations_(int depth)
{
    while (m_depth != depth) {
        --m_depth;
        const std::string name{m_aggregate_name_stack.top()};
        m_aggregate_name_stack.pop();
        const std::string translation{text_end + " "s + name};
        const std::span<uint8_t, 0> sp;
        print_offset_();
        print_hex_(sp);
        print_ascii_(sp);
        print_translation_(translation);
    }
}

void Translation_node_writer::print_hex_(const std::span<uint8_t>& sp) const
{
    if (!m_hex_column_enabled) {
        return;
    }

    std::ostream& out{*m_out};
    int written{0};
    for (uint8_t byte : sp) {
        if (written && !(written % 4)) {
            out << " | ";
        }
        out << std::format("{:02x} ", byte);
        ++written;
    }
    while (written < constants::translation_max_bytes_per_line) {
        if (written && !(written % 4)) {
            out << " | ";
        }
        out << "-- ";
        ++written;
    }
    out << " | ";
}

void Translation_node_writer::print_hex_column_title_() const
{
    static constexpr size_t hex_column_width{60};
    print_column_title_(text_hex, hex_column_width, m_hex_column_enabled);
}

void Translation_node_writer::print_offset_()
{
    if (!m_offset_column_enabled) {
        return;
    }

    std::ostream& out{*m_out};
    out << std::format("0x{:08x} | ", m_offset);
}

void Translation_node_writer::print_offset_column_title_() const
{
    static constexpr size_t offset_column_width{13};
    print_column_title_(text_offset, offset_column_width, m_offset_column_enabled);
}

void Translation_node_writer::print_origin_info_(const bpt::ptree& root) const
{
    // Print origin info in the following format:
    // Savegame: "data\\Brennus BC-4000.CivBeyondSwordSave"
    // Schema: "..\\..\\..\\doc\\BTS.schema"
    // Date: 12-09-2024 16:59:28 UTC
    // c4lib version: 01.00.00
    std::ostream& out{*m_out};

    const boost::optional<const bpt::ptree&> origin_node{root.get_child_optional(nn_origin)};
    if (!origin_node) {
        throw Ptree_error{std::format(fmt::node_not_found, nn_origin)};
    }

    out << text_savegame << ": " << origin_node->get<std::string>(nn_savegame) << '\n';
    out << text_schema << ": " << origin_node->get<std::string>(nn_schema) << '\n';
    out << text_date << ": " << origin_node->get<std::string>(nn_date) << '\n';
    out << text_c4lib_version << ": " << origin_node->get<std::string>(nn_c4lib_version) << '\n';
    out << '\n';
}

void Translation_node_writer::print_translation_(const std::string& translation) const
{
    std::ostream& out{*m_out};

    const std::string indent(gsl::narrow<size_t>(m_depth * constants::translation_indent_width), ' ');
    out << indent << translation << '\n';
}

void Translation_node_writer::print_translation_column_title_() const
{
    static constexpr size_t translation_column_width{50};
    print_column_title_(text_translation, translation_column_width, true);
}

} // namespace c4lib::property_tree
