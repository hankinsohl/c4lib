// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/27/2024.

#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <iosfwd>
#include <iterator>
#include <lib/options/exception-formats.hpp>
#include <lib/options/exceptions.hpp>
#include <lib/options/options-manager.hpp>
#include <lib/util/narrow.hpp>
#include <map>
#include <ranges>
#include <span>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std::string_literals;
namespace bpt = boost::property_tree;
namespace fmt = hankinsohl::options::format;

namespace hankinsohl::options {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void check_compatibility(
    const std::unordered_map<std::string, std::string>& options, const std::vector<std::string>& incompatible)
{
    std::stringstream incompatible_set;
    size_t count_options_specified{0};
    std::ranges::for_each(
        std::as_const(incompatible), [&count_options_specified, &incompatible_set, &options](const auto& option_name) {
            if (options.contains(option_name)) {
                if (count_options_specified) {
                    incompatible_set << ", ";
                }
                ++count_options_specified;
                incompatible_set << option_name;
            }
        });

    if (count_options_specified > 1) {
        throw Options_error{std::format(fmt::incompatible_options, incompatible_set.str())};
    }
}

void check_requires_at_least_one_of(
    const std::unordered_map<std::string, std::string>& options, const std::vector<std::string>& requires_one_of)
{
    const bool option_specified{std::ranges::any_of(
        std::as_const(requires_one_of), [&options](const auto& option_name) { return options.contains(option_name); })};
    if (!option_specified) {
        bool first{true};
        std::stringstream required_set;
        std::ranges::for_each(std::as_const(requires_one_of), [&required_set, &first](const auto& option_name) {
            if (!first) {
                required_set << ", ";
            }
            first = false;
            required_set << option_name;
        });
        throw Options_error{std::format(fmt::none_from_required_set, required_set.str())};
    }
}

void Options_manager::add_aggregate_checks(
    const std::vector<std::pair<std::vector<std::string>, aggregate_check_func>>& ac)
{
    std::ranges::copy(ac.begin(), ac.end(), std::insert_iterator{m_aggregate_checks, m_aggregate_checks.end()});
}

void Options_manager::add_info(const std::unordered_map<std::string, Option_info>& info)
{
    for (const auto& option_info : info | std::views::values) {
        // Convert option name in info to uppercase to facilitate lookup.
        Option_info info_copy{option_info};
        std::ranges::transform(
            info_copy.name, info_copy.name.begin(), [](unsigned char c) { return gsl::narrow<char>(std::toupper(c)); });
        m_info[info_copy.name] = info_copy;
    }
}

void Options_manager::add_options(const std::unordered_map<std::string, std::string>& options)
{
    for (const auto& [key, value] : options) {
        // Convert option name to uppercase to facilitate lookup.
        std::string name{key};
        std::ranges::transform(name, name.begin(), [](unsigned char c) { return gsl::narrow<char>(std::toupper(c)); });
        m_options[name] = value;
    }
}

void Options_manager::add_options_from_command_line(std::span<const char* const> args)
{
    for (size_t i{1}; i < args.size(); ++i) {
        std::string option{args[i]};

        // All options begin with dash.  Check for dash and then remove it if found.
        if (option[0] != '-') {
            throw Options_error{std::format(fmt::cli_option_missing_dash, option)};
        }
        const std::string trimmed_option{option.substr(1)};

        // Separate the option name from its value.  If the option has a value, the option name is
        // separated from its value by '='
        const std::string::size_type pos{trimmed_option.find('=')};
        std::string option_name;
        std::string option_value;
        if (pos != std::string::npos) {
            if (pos == 0 || pos == option.length() - 1) {
                throw Options_error{std::format(fmt::bad_format, option)};
            }
            option_name = trimmed_option.substr(0, pos);
            option_value = trimmed_option.substr(pos + 1);
        }
        else {
            option_name = trimmed_option;
            option_value = "";
        }

        // Convert option name to uppercase to facilitate lookup.
        std::string uc{option_name};
        std::ranges::transform(uc, uc.begin(), [](unsigned char c) { return gsl::narrow<char>(std::toupper(c)); });

        m_options[uc] = option_value;
    }
}

void Options_manager::add_options_from_config_file(const std::string& config_file)
{
    // Parse the XML into the property tree.
    bpt::ptree tree;
    bpt::read_xml(config_file, tree);

    for (const bpt::ptree& config_element{tree.get_child("config")}; const auto& option_element : config_element) {
        if (option_element.first != "option") {
            throw Xml_error{std::format(fmt::xml_config_element_missing, option_element.first)};
        }
        const std::string option_name{option_element.second.get("<xmlattr>.name", "")};
        const std::string option_value{option_element.second.get("<xmlattr>.value", "")};

        // Convert option name to uppercase to facilitate lookup.
        std::string uc{option_name};
        std::ranges::transform(uc, uc.begin(), [](unsigned char c) { return gsl::narrow<char>(std::toupper(c)); });
        m_options[uc] = option_value;
    }
}

std::unordered_map<std::string, std::string> Options_manager::get_options() const
{
    return m_options;
}

std::unordered_map<std::string, std::string> Options_manager::get_options_exclusive_of(
    const std::unordered_map<std::string, Option_info>& options_to_exclude) const
{
    std::unordered_map<std::string, std::string> filtered_options;
    std::ranges::copy_if(m_options, std::inserter(filtered_options, filtered_options.end()),
        [&options_to_exclude](const auto& pr) { return !options_to_exclude.contains(pr.first); });

    return filtered_options;
}

void Options_manager::reset()
{
    m_info.clear();
    m_options.clear();
}

void Options_manager::set_defaults_then_check_options()
{
    set_defaults_();
    check_options_();
}

void Options_manager::write_help_message(std::ostream& out) const
{
    // Create a map of option-help message pairs in order to sort help messages.
    std::map<int, std::string> sorted_help_messages;
    std::ranges::transform(m_info, std::inserter(sorted_help_messages, sorted_help_messages.end()),
        [this](const auto& info) -> std::pair<int, std::string> {
            std::string help_message{std::vformat(m_help_format,
                std::make_format_args(info.second.name, info.second.help_type, info.second.help_meaning))};
            return std::make_pair(info.second.help_sort_order, help_message);
        });

    std::ranges::for_each(std::as_const(sorted_help_messages), [&out](const auto& pr) { out << pr.second << '\n'; });
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Options_manager::check_aggregate_requirements_()
{
    for (const auto& [members, check_func] : m_aggregate_checks) {
        check_func(m_options, members);
    }
}

void Options_manager::check_for_dependency_violations_()
{
    std::ranges::for_each(std::as_const(m_info), [this](const auto& pr) {
        if (m_options.contains(pr.first)) {
            std::ranges::for_each(std::as_const(pr.second.depends_on), [this, pr](const auto& dependency) {
                if (!m_options.contains(dependency)) {
                    throw Options_error{std::format(fmt::dependency_missing, pr.first, dependency)};
                }
            });
        }
    });
}

void Options_manager::check_for_missing_required_options_()
{
    std::ranges::for_each(std::as_const(m_info), [this](const auto& pr) {
        if (pr.second.required && !m_options.contains(pr.first)) {
            throw Options_error{std::format(fmt::required_option_missing, pr.first)};
        }
    });
}

void Options_manager::check_options_()
{
    std::ranges::for_each(std::as_const(m_options), [this](const auto& pr) { check_option_(pr.first, pr.second); });
    check_for_missing_required_options_();
    check_for_dependency_violations_();
    check_aggregate_requirements_();
}

void Options_manager::check_option_(const std::string& option_name, const std::string& option_value)
{
    // Check for option_help.  If set, throw Display_help_error so that the caller can display a help message.
    if (option_name == option_help) {
        std::stringstream help;
        write_help_message(help);
        throw Display_help_error{help.str()};
    }

    if (!m_info.contains(option_name)) {
        throw Options_error{std::format(fmt::unknown_option, option_name)};
    }
    const Option_info& info = m_info[option_name];

    // At this point, options that take values should have had their value set.  Check
    // that this is so.
    if (option_name != "HELP" && !info.default_value.empty()) {
        assert(!option_value.empty());
    }

    // If the option is of type boolean, check that the value is either 0 or 1.
    if (info.type == Option_type::boolean) {
        if (option_value != "0" && option_value != "1") {
            throw Options_error{std::format(fmt::bad_boolean_value, option_name, option_value)};
        }
    }

    // Check format of integer options
    if (info.type == Option_type::integer) {
        bool ok{true};
        // Call strtol and check for errors.  The option value is invalid if an error occurs.
        errno = 0;
        char* end{nullptr};
        const long value{std::strtol(option_value.c_str(), &end, 10)};
        if (errno != 0 || end == option_value.c_str() || value < INT_MIN || value > INT_MAX) {
            ok = false;
        }
        if (!ok) {
            if (errno == ERANGE || errno == EOVERFLOW || value < INT_MIN || value > INT_MAX) {
                throw Options_error{std::format(fmt::bad_integer_value_range, option_name, option_value)};
            }
            else {
                throw Options_error{std::format(fmt::bad_integer_value, option_name, option_value)};
            }
        }
    }
}

void Options_manager::set_defaults_()
{
    for (const auto& [key, value] : m_options) {
        if (value.empty()) {
            if (!m_info.contains(key)) {
                if (key == option_help) {
                    std::stringstream help;
                    write_help_message(help);
                    throw Display_help_error{help.str()};
                }
                else {
                    throw Options_error{std::format(fmt::unknown_option, key)};
                }
            }
            const Option_info& info = m_info[key];
            m_options[key] = info.default_value;
        }
    }
}

void Options_manager::set_help_format(const std::string& help_format)
{
    m_help_format = help_format;
}

} // namespace hankinsohl::options
