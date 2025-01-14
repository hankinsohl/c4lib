// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/28/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <lib/native/path.hpp>
#include <lib/ptree/node-reader.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <string>
#include <unordered_map>

namespace c4lib::property_tree {

// Provides functions common to all Node_reader implementations.
class Base_node_reader : public Node_reader {
public:
    Base_node_reader() = default;

    ~Base_node_reader() override = default;

    Base_node_reader(const Base_node_reader&) = delete;

    Base_node_reader& operator=(const Base_node_reader&) = delete;

    Base_node_reader(Base_node_reader&&) noexcept = delete;

    Base_node_reader& operator=(Base_node_reader&&) noexcept = delete;

    void init(const native::Path& filename,
        c4lib::schema_parser::Def_tbl* definition_table,
        std::unordered_map<std::string, std::string>& options) final;

    void read_node(boost::property_tree::ptree& node) final;

protected:
    // Node readers may optionally override init_impl_ to perform reader-specific initialization
    // not covered by Base_node_reader::init.  Within Base_node_reader's implementation of init,
    // m_filename and m_definition_table are set and then init_impl_ is called.  The default
    // implementation of init_impl_ does noting.
    virtual void init_impl_() {};

    [[nodiscard]] bool is_leader_array_member_(const boost::property_tree::ptree& node) const;

    // Base_node_reader calls read_node_impl_ from within its implementation of read_node.  When
    // read_node_impl_ returns, Base_node_reader performs processing common to all readers such as
    // creating the PlayerTypes enumeration based on values in the "Leader" array. Classes inheriting
    // from Base_node_reader must override read_node_impl_ and set the nodes size and data members
    // for non-aggregate types.
    virtual void read_node_impl_(boost::property_tree::ptree& node) = 0;

    // Base_node_reader sets m_array_name within read_node prior to calling read_node_impl_.  Child classes
    // may use m_array_name to determine whether a node is an array member.
    std::string m_array_name;

    // Base_node_reader sets m_definition_table within the call to init.
    c4lib::schema_parser::Def_tbl* m_definition_table{nullptr};

    // Base_node_reader sets m_filename within the call to init.
    native::Path m_filename;

    std::unordered_map<std::string, std::string>* m_options{nullptr};

private:
    void create_player_types_enumerator_definition_(const boost::property_tree::ptree& node) const;
};

} // namespace c4lib::property_tree
