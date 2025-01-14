// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/28/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <cstddef>
#include <lib/ptree/base-node-reader.hpp>
#include <lib/util/limits.hpp>
#include <sstream>

namespace c4lib::property_tree {

// Binary_node_reader reads the binary savegame file and uses the data read to populate the ptree data
// and size members.
class Binary_node_reader : public Base_node_reader {
public:
    Binary_node_reader() = default;
    
    ~Binary_node_reader() override = default;

    Binary_node_reader(const Binary_node_reader&) = delete;   
	
    Binary_node_reader& operator=(const Binary_node_reader&) = delete;  
	
    Binary_node_reader(Binary_node_reader&&) noexcept = delete;   
	
    Binary_node_reader& operator=(Binary_node_reader&&) noexcept = delete;    

protected:
    size_t get_undocumented_footer_bytes_count() override;

    void init_impl_() override;

    void read_node_impl_(boost::property_tree::ptree& node) override;

private:
    std::stringstream m_save;
    size_t m_undocumented_footer_bytes_count{limits::invalid_size};
};

} // namespace c4lib::property_tree
