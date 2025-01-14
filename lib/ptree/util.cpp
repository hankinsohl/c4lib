// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/27/2024.

#include <boost/property_tree/ptree.hpp>
#include <cstddef>
#include <format>
#include <include/exceptions.hpp>
#include <include/node-attributes.hpp>
#include <lib/ptree/util.hpp>
#include <lib/util/constants.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/narrow.hpp>
#include <optional>
#include <ranges>
#include <string>

namespace bpt = boost::property_tree;

namespace c4lib::property_tree {

int get_array_dimension(const bpt::ptree& pt, const std::string& path)
{
    const std::string path_to_subscript{path + "." + nn_attributes + "." + nn_subscripts};
    const boost::optional<const bpt::ptree&> subscripts_node{pt.get_child_optional(path_to_subscript)};
    if (!subscripts_node) {
        throw Ptree_error{std::format(fmt::node_not_found, path)};
    }

    const std::string subscripts{subscripts_node->data()};
    if (subscripts.empty()) {
        throw Ptree_error{std::format(fmt::bad_subscripts_format, nn_subscripts)};
    }
    int array_dimension{std::stoi(subscripts.substr(1))};
    if (array_dimension < 0 || gsl::narrow<size_t>(array_dimension) > limits::max_array_dimension) {
        throw Ptree_error{std::format(fmt::array_dimension_out_of_range, array_dimension)};
    }
    return array_dimension;
}

size_t get_footer_size(const bpt::ptree& pt)
{
    // Get the UndocumentedFooterBytes node from pt.
    const int undocumented_footer_byte_count{get_array_dimension(pt, constants::undocumented_footer_bytes_path)};

    // Compute the footer size.  The footer size = undocumented footer byte count + 1 (checksum byte) +
    // 4 (md5 length field) + 32 (characters in md5).
    const int footer_size{undocumented_footer_byte_count + 1 + 4 + 32};

    return gsl::narrow<size_t>(footer_size);
}

int get_max_players(const bpt::ptree& pt)
{
    // From the schema we have:
    //     wstring_type[MAX_PLAYERS] LeaderName
    // With the path to LeaderName as follows:
    // Savegame.CvInitCore.LeaderName
    // Obtain MAX_PLAYERS from LeaderName.
    return get_array_dimension(pt, constants::leader_name_path);
}

int get_num_game_option_types(const bpt::ptree& pt)
{
    // From the schema we have:
    //     bool8[NUM_GAME_OPTION_TYPES:GameOptionTypes] Options
    // With the path to Options as follows:
    // Savegame.CvInitCore.Options
    // Obtain NUM_GAME_OPTION_TYPES from Options.
    return get_array_dimension(pt, constants::options_path);
}

int get_num_multiplayer_option_types(const bpt::ptree& pt)
{
    // From the schema we have:
    //     bool8[NUM_MULTIPLAYER_OPTION_TYPES:MultiplayerOptionTypes] MPOptions
    // With the path to MPOptions as follows:
    // Savegame.CvInitCore.MPOptions
    // Obtain NUM_MULTIPLAYER_OPTION_TYPES from MPOptions.
    return get_array_dimension(pt, constants::multiplayer_options_path);
}

} // namespace c4lib::property_tree
