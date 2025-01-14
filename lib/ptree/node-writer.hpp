// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/19/2024.

#pragma once

#include <string>
#include <unordered_map>
#include <iosfwd>
#include <boost/property_tree/ptree_fwd.hpp>
#include <utility>

namespace c4lib::property_tree {

class Node_writer {
public:
    Node_writer() = default;
    
    virtual ~Node_writer() = default;

    Node_writer(const Node_writer&) = delete;   
	
    Node_writer& operator=(const Node_writer&) = delete;  
	
    Node_writer(Node_writer&&) noexcept = delete;   
	
    Node_writer& operator=(Node_writer&&) noexcept = delete;
    
    virtual void finish() = 0;

    virtual void init(const boost::property_tree::ptree& root,
        std::ostream& out,
        std::unordered_map<std::string, std::string>& options)
        = 0;

    virtual void write_node(std::pair<int, const boost::property_tree::ptree&> depth_node_pair) = 0;
};

} // namespace c4lib::property_tree
