// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/30/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <iosfwd>
#include <string>
#include <unordered_map>

namespace c4lib {

void write_composite(
    const boost::property_tree::ptree& pt, std::ostream& out, std::unordered_map<std::string, std::string>& options);

} // namespace c4lib
