// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/19/2024.

#pragma once

#include <cstddef>
#include <lib/native/path.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <string>
#include <unordered_map>
#include <boost/property_tree/ptree_fwd.hpp>

namespace c4lib::property_tree {

class Node_reader {
public:
    Node_reader() = default;
    
    virtual ~Node_reader() = default;

    Node_reader(const Node_reader&) = delete;   
	
    Node_reader& operator=(const Node_reader&) = delete;  
	
    Node_reader(Node_reader&&) noexcept = delete;   
	
    Node_reader& operator=(Node_reader&&) noexcept = delete;     

    // Readers must implement get_undocumented_footer_bytes_count to let the schema parser know how many bytes
    // are in the UndocumentedFooterBytes array.
    virtual size_t get_undocumented_footer_bytes_count() = 0;

    // Initializes the Node_reader.
    // filename is the name of the binary or text savegame file to read, depending on the Node_reader
    // implementation.
    // definition_table is used to set the definition table for the reader.  The reader must use the table
    // to store the definition of the PlayerTypes enumeration based on values in the "Civ" array.
    virtual void init(const native::Path& filename,
        schema_parser::Def_tbl* definition_table,
        std::unordered_map<std::string, std::string>& options)
        = 0;

    // read_node is responsible for setting the values of a node's size and data members for non-aggregate types.
    // It is also responsible for generating the PlayerTypes enumeration based on values in the "Leader" array.
    virtual void read_node(boost::property_tree::ptree& node) = 0;
};

} // namespace c4lib::property_tree
