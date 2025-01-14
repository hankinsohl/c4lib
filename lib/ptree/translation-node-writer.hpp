// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/9/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <lib/ptree/node-writer.hpp>
#include <lib/util/limits.hpp>
#include <span>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace c4lib::property_tree {

class Translation_node_writer : public Node_writer {
public:
    Translation_node_writer() = default;

    ~Translation_node_writer() override = default;

    Translation_node_writer(const Translation_node_writer&) = delete;

    Translation_node_writer& operator=(const Translation_node_writer&) = delete;

    Translation_node_writer(Translation_node_writer&&) noexcept = delete;

    Translation_node_writer& operator=(Translation_node_writer&&) noexcept = delete;

    void finish() override;

    void init(const boost::property_tree::ptree& root,
        std::ostream& out,
        std::unordered_map<std::string, std::string>& options) override;

    void write_node(std::pair<int, const boost::property_tree::ptree&> depth_node_pair) override;

private:
    std::string get_consolidated_translation_value_equals_prefix_() const;

    void get_node_data_and_translation_(const std::pair<int, const boost::property_tree::ptree&>& pr,
    std::vector<uint8_t>& data,
        std::string& translation,
        bool& is_empty_aggregate);

    void gndt_consolidated_(const std::pair<int, const boost::property_tree::ptree&>& pr,
        std::vector<uint8_t>& data,
        std::string& translation);

    void gndt_not_consolidated_(const std::pair<int, const boost::property_tree::ptree&>& pr,
        std::vector<uint8_t>& data,
        std::string& translation,
        bool& is_empty_aggregate);

    void print_ascii_(const std::span<uint8_t>& sp) const;

    void print_ascii_column_title_() const;

    void print_column_header_() const;

    void print_column_title_(const std::string& title, size_t width, bool enabled) const;

    void print_end_translations_(int depth);

    void print_hex_(const std::span<uint8_t>& sp) const;

    void print_hex_column_title_() const;

    void print_offset_();

    void print_offset_column_title_() const;

    void print_origin_info_(const boost::property_tree::ptree& root) const;

    void print_translation_(const std::string& translation) const;

    void print_translation_column_title_() const;

    std::stack<std::string> m_aggregate_name_stack;
    bool m_ascii_column_enabled{true};

    std::vector<uint8_t> m_consolidated_data;
    size_t m_consolidated_data_size{limits::invalid_size};
    std::stringstream m_consolidated_translation;
    size_t m_count_consolidated{limits::invalid_size};
    int m_depth{0};
    bool m_hex_column_enabled{true};
    bool m_is_consolidated_output_ready{false};
    bool m_is_output_consolidating{false};
    std::streamoff m_offset{0};
    bool m_offset_column_enabled{true};
    std::ostream* m_out{nullptr};
};

} // namespace c4lib::property_tree
