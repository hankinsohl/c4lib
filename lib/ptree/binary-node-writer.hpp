// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/30/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <iostream>
#include <lib/ptree/node-writer.hpp>
#include <string>
#include <unordered_map>
#include <utility>

namespace c4lib::property_tree {

class Binary_node_writer : public Node_writer {
public:
    Binary_node_writer() = default;
    
    ~Binary_node_writer() override = default;

    Binary_node_writer(const Binary_node_writer&) = delete;   
	
    Binary_node_writer& operator=(const Binary_node_writer&) = delete;  
	
    Binary_node_writer(Binary_node_writer&&) noexcept = delete;   
	
    Binary_node_writer& operator=(Binary_node_writer&&) noexcept = delete;    

    void finish() override;

    void init(const boost::property_tree::ptree& root,
        std::ostream& out,
        std::unordered_map<std::string, std::string>& options) override;

    void write_node(std::pair<int, const boost::property_tree::ptree&> depth_node_pair) override;

private:
    std::ostream* m_out{nullptr};
};

} // namespace c4lib::property_tree
