// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/23/2024.

#include <lib/schema-parser/def-type.hpp>
#include <lib/util/exception-formats.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cassert>
#include <cstddef>
#include <format>
#include <include/exceptions.hpp>
#include <include/node-attributes.hpp>
#include <include/node-type.hpp>
#include <lib/ptree/null-node-reader.hpp>
#include <lib/util/constants.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/schema.hpp>
#include <string>

using namespace std::string_literals;
namespace bpt = boost::property_tree;
namespace csp = c4lib::schema_parser;

namespace c4lib::property_tree {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SHARED IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t Null_node_reader::get_undocumented_footer_bytes_count()
{
    constexpr size_t fabricated_byte_count{10};
    return fabricated_byte_count;
}

void Null_node_reader::read_node_impl_(bpt::ptree& node)
{
    bpt::ptree& attributes_node{node.get_child(nn_attributes)};

    bpt::ptree const& type_node{attributes_node.get_child(nn_type)};
    bpt::ptree& type_name_node{attributes_node.get_child(nn_typename)};
    bpt::ptree const& name_node{attributes_node.get_child(nn_name)};
    std::string const node_name{name_node.get_value<std::string>()};

    switch (Node_type const type{type_node.get_value<Node_type>()}) {
    case Node_type::bool_type:
    case Node_type::hex_type:
    case Node_type::int_type:
    case Node_type::uint_type:
    case Node_type::enum_type: {
        std::string const size{size_from_type(type_name_node.data())};
        attributes_node.add(nn_size, size);

        if (node_name == constants::game_version) {
            // Set the value of the GameVersion node to pass the assert statement in the schema:
            //     assert(GameVersion >= 100 && GameVersion < 400)
            attributes_node.add(nn_data, "302");
        }
        else if (node_name == constants::revealed_route_type_count) {
            // RevealedRouteTypeCount must be [0, NUM_ROUTE_TYPES) and NUM_ROUTE_TYPES is 2.
            attributes_node.add(nn_data, "2");
        }
        else if (type == Node_type::bool_type) {
            // We'll set the data for bools to 1 indicating truth.
            attributes_node.add(nn_data, "1");
        }
        else if (type == Node_type::enum_type) {
            // Check to see if this is a "Leader" array member.  If so, we need to create the
            // data for the PlayerTypes enumerator.
            if (is_leader_array_member_(node)) {
                std::string const data{create_player_types_enumerator_data_(node)};
                attributes_node.add(nn_data, data);
            }
            else {
                bpt::ptree const& enum_node{attributes_node.get_child(nn_enum)};
                std::string const enum_name{enum_node.get_value<std::string>()};
                if (enum_name == constants::chat_target_types || enum_name == constants::player_vote_types) {
                    attributes_node.add(nn_data, "-1");
                }
                else {
                    // We'll set the data for enums to 1 since 1 is probably going to be valid.  If
                    // this causes errors later on we'll need to do something a bit more sophisticated.
                    attributes_node.add(nn_data, "1");
                }
            }
        }
        else {
            // We'll set the data for hex, int, and uint all simple types to 4 since 4 is probably
            // going to be valid, and since some types are used to determine array size, using 4
            // will cause array generation and enhance testing of arrays.
            attributes_node.add(nn_data, "4");
        }
    } break;

    case Node_type::u16string_type: {
        std::string const wchar_string{"wstring"};
        attributes_node.add(nn_size, 4 + (2 * wchar_string.length()));
        attributes_node.add(nn_data, wchar_string);
    } break;

    case Node_type::string_type: {
        std::string const char_string{"string"};
        attributes_node.add(nn_size, 4 + char_string.length());
        attributes_node.add(nn_data, char_string);
    } break;

    case Node_type::md5_type: {
        // md5 of the text "frog" - used here as valid filler data
        std::string const md5_string{"938c2cc0dcc05f2b68c4287040cfcf71"};
        attributes_node.add(nn_size, 4 + md5_string.length());
        attributes_node.add(nn_data, md5_string);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string Null_node_reader::create_player_types_enumerator_data_(const bpt::ptree& node) const
{
    // Set the player types enumerator value using the Civ array subscript obtained from the Civ array node name.
    const bpt::ptree& attributes_node{node.get_child(nn_attributes)};
    const bpt::ptree& civ_array_name_node{attributes_node.get_child(nn_name)};
    std::string const civ_array_node_name{civ_array_name_node.get_value<std::string>()};
    assert(civ_array_node_name[0] == '[');
    int const player_types_enumerator_value{std::stoi(civ_array_node_name.substr(1))};

    // Fabricate the civ enumerator value which will be used to set the data for this node.
    int civ_enumerator_value{player_types_enumerator_value + 10};
    if (int const max_players{m_definition_table->get_const_value(constants::max_players)};
        player_types_enumerator_value + 1 == max_players) {
        // The last PlayerTypes enumerator should be assigned CIVILIZATION_BARBARIAN.  CIVILIZATION_BARBARIAN is
        // the last entry in the CivilizationTypes enumeration.  We subtract 2 from the vector size because
        // enumerator values begin at -1 (-1 is NO_CIVILIZATION), not 1.
        auto& ct_enum_def{m_definition_table->get_definition(constants::leader_head_types, csp::Def_type::enum_type)};
        civ_enumerator_value = gsl::narrow<int>(ct_enum_def.get_members().size() - 2);
    }

    return std::to_string(civ_enumerator_value);
}

} // namespace c4lib::property_tree
