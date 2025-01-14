// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/23/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <cstddef>
#include <lib/ptree/base-node-reader.hpp>
#include <string>

namespace c4lib::property_tree {

// Binary_node_reader does not read from any source.  Instead, it initializes nodes with fixed values.
// Binary_node_reader is intended for debugging the schema parser and for testing as it decouples parser errors from
// those that might have been caused by reading from an actual source (e.g., binary save file or c4 text representation
// of a save).
class Null_node_reader : public Base_node_reader {
public:
    Null_node_reader() = default;

    ~Null_node_reader() override = default;

    Null_node_reader(const Null_node_reader&) = delete;

    Null_node_reader& operator=(const Null_node_reader&) = delete;

    Null_node_reader(Null_node_reader&&) noexcept = delete;

    Null_node_reader& operator=(Null_node_reader&&) noexcept = delete;

protected:
    size_t get_undocumented_footer_bytes_count() override;

    void read_node_impl_(boost::property_tree::ptree& node) override;

private:
    [[nodiscard]] std::string create_player_types_enumerator_data_(const boost::property_tree::ptree& node) const;
};

} // namespace c4lib::property_tree
