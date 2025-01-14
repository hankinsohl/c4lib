// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/28/2024.

#include <boost/property_tree/ptree.hpp>
#include <cassert>
#include <cstddef>
#include <include/node-attributes.hpp>
#include <include/node-type.hpp>
#include <lib/native/path.hpp>
#include <lib/ptree/base-node-reader.hpp>
#include <lib/schema-parser/def-mem-type.hpp>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/definition.hpp>
#include <lib/util/constants.hpp>
#include <lib/util/file-location.hpp>
#include <lib/util/narrow.hpp>
#include <memory>
#include <string>
#include <unordered_map>

using namespace std::string_literals;
namespace bpt = boost::property_tree;
namespace csp = c4lib::schema_parser;

namespace c4lib::property_tree {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Base_node_reader::init(const native::Path& filename,
    schema_parser::Def_tbl* definition_table,
    std::unordered_map<std::string, std::string>& options)
{
    m_filename = filename;
    m_definition_table = definition_table;
    m_options = &options;
    init_impl_();
}

void Base_node_reader::read_node(boost::property_tree::ptree& node)
{
    const bpt::ptree& attributes_node{node.get_child(nn_attributes)};
    const bpt::ptree empty_ptree{};
    const bpt::ptree& array_name_node{attributes_node.get_child(nn_array_name, empty_ptree)};
    m_array_name = array_name_node.get_value<std::string>("");
    read_node_impl_(node);

    const bpt::ptree& type_node{attributes_node.get_child(nn_type)};
    if (const Node_type type{type_node.get_value<Node_type>()}; type == Node_type::enum_type) {
        // Check to see if this is a "Leader" array member.  If so, we need to create the
        // definition for the corresponding PlayerTypes enumeration.
        if (is_leader_array_member_(node)) {
            create_player_types_enumerator_definition_(node);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SHARED IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Base_node_reader::is_leader_array_member_(const bpt::ptree& node) const
{
    if (m_array_name != constants::leader_array) {
        return false;
    }

    const bpt::ptree& attributes_node{node.get_child(nn_attributes)};
    const bpt::ptree& enum_name_node{attributes_node.get_child(nn_enum)};
    const std::string enum_name{enum_name_node.get_value<std::string>()};
    return enum_name == constants::leader_head_types;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Base_node_reader::create_player_types_enumerator_definition_(const bpt::ptree& node) const
{
    // Set the player types enumerator value using the Leader array subscript obtained from the Leader array node name.
    const bpt::ptree& attributes_node{node.get_child(nn_attributes)};
    const bpt::ptree& leader_array_name_node{attributes_node.get_child(nn_name)};
    const std::string leader_array_node_name{leader_array_name_node.get_value<std::string>()};
    assert(leader_array_node_name[0] == '[');
    const size_t player_types_enumerator_value{std::stoul(leader_array_node_name.substr(1))};
    const std::string player_types_enumerator_value_string{std::to_string(player_types_enumerator_value)};

    // Get the leader head enumerator value from the value of the node and then
    // get the leader head types enumerator definition from the definition table.
    const bpt::ptree& data_node{attributes_node.get_child(nn_data)};
    const int leader_head_types_enumerator_value{data_node.get_value<int>()};
    const csp::Def_mem& leader_head_types_enumerator_def{
        m_definition_table->get_enumerator(constants::leader_head_types, leader_head_types_enumerator_value)};

    // Get/create the player types enum definition from the definition table.
    bool was_created{false};
    const File_location enum_loc{std::make_shared<std::string>("internally generated PlayerTypes definition"),
        std::make_shared<std::string>(
            leader_head_types_enumerator_def.name + " = " + player_types_enumerator_value_string),
        1, 1};
    csp::Definition& player_types_enum_definition{m_definition_table->create_definition(
        constants::player_types, csp::Def_type::enum_type, enum_loc, was_created)};
    if (was_created) {
        // If we created the enum definition, add the NO_PLAYER = -1 enumerator.
        const File_location loc{std::make_shared<std::string>("internally generated PlayerTypes enumerator"),
            std::make_shared<std::string>(constants::no_player + " = -1"s), 1, 1};
        csp::Def_mem enum_member{csp::Def_mem_type::enum_type, constants::no_player, -1, loc};
        player_types_enum_definition.add_member(enum_member, false, false);
    }

    // If the leader head enumerator name is "NO_LEADERHEAD", use "NO_PLAYER" instead.
    std::string player_name;
    if (leader_head_types_enumerator_def.name == constants::no_leader_head) {
        player_name = constants::no_player;
    }
    else {
        player_name = leader_head_types_enumerator_def.name;
    }

    // Create the definition for the player types enumerator and add it to the definition table.
    const File_location loc{std::make_shared<std::string>("internally generated PlayerTypes enumerator"),
        std::make_shared<std::string>(
            leader_head_types_enumerator_def.name + " = " + player_types_enumerator_value_string),
        1, 1};
    csp::Def_mem player_types_enum_member_def{
        csp::Def_mem_type::enum_type, player_name, gsl::narrow<int>(player_types_enumerator_value), loc};
    // Pass true for allow_duplicates if the enumerator name is NO_PLAYER b/c NO_PLAYER is used for any player slot
    // that's not actually playing the game and there may be multiple of such.
    const bool allow_duplicates{leader_head_types_enumerator_def.name == constants::no_leader_head};
    player_types_enum_definition.add_member(player_types_enum_member_def, allow_duplicates, false);
}

} // namespace c4lib::property_tree
