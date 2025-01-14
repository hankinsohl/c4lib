// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/27/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <cstddef>
#include <optional>
#include <string>

namespace c4lib::property_tree {

int get_array_dimension(const boost::property_tree::ptree& pt, const std::string& path);

size_t get_footer_size(const boost::property_tree::ptree& pt);

int get_max_players(const boost::property_tree::ptree& pt);

int get_num_game_option_types(const boost::property_tree::ptree& pt);

int get_num_multiplayer_option_types(const boost::property_tree::ptree& pt);

} // namespace c4lib::property_tree
