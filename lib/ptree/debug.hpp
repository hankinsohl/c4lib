// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/25/2024.

#pragma once

#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <string>
#include <iosfwd>

namespace c4lib::property_tree {

inline void dump_ptree(const std::string& filename, const boost::property_tree::ptree& ptree)
{
    boost::property_tree::write_info(filename, ptree);
}

inline void dump_ptree(std::ostream& out, const boost::property_tree::ptree& ptree)
{
    boost::property_tree::write_info(out, ptree);
}

} // namespace c4lib::property_tree
