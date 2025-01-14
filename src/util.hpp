// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/14/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <c4lib-version.hpp>
#include <format>
#include <iostream>
#include <lib/util/timer.hpp>
#include <src/text.hpp>
#include <string>
#include <unordered_map>

namespace c4edit {

struct Write_option_info {
    std::string option;

    void (*func)(const boost::property_tree::ptree&, const std::string&, std::unordered_map<std::string, std::string>&);

    std::string progress_message;
};

inline std::string banner()
{
    return std::format("{} {} {}", text::exe_name, text::version, c4lib::constants::c4lib_version);
}

inline void display_help(const std::string& help)
{
    std::cout << std::format("{}: {} [{}]\n", text::usage_capitalized, text::exe_name, text::options);
    std::cout << text::options_capitalized << '\n';
    std::cout << help;
}

inline void process_write_option(const Write_option_info& write_option,
    const boost::property_tree::ptree& ptree,
    std::unordered_map<std::string, std::string>& exe_options,
    std::unordered_map<std::string, std::string>& lib_options)
{
    if (exe_options.contains(write_option.option)) {
        c4lib::Timer timer;
        timer.start();
        const std::string out_path{exe_options[write_option.option]};
        // Note: Output to cout is intentionally flushed because when running under the CLion IDE,
        // output that is not flushed does not appear until the program exits.
        std::cout << write_option.progress_message << ' ' << out_path << "... " << std::flush;
        (*write_option.func)(ptree, out_path, lib_options);
        std::cout << text::finished_in << ' ' << timer.to_string() << '\n' << std::flush;
    }
}

} // namespace c4edit
