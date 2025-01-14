// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/27/2024.

#pragma once

#include <iosfwd>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace hankinsohl::options {

// Special, reserved option.  If present when check_options is called, causes the Option_manager to
// throw Display_help_error.  Callers can wrap check_options in a try-catch block and if
// Display_help_error is caught, write_help_message can be called to compose a message to display
// to the user.
const char* const option_help{"HELP"};

// Default format for the help message.  The format can be changed using
// set_help_format.
inline const char* const default_help_fmt{"-{:<25}{:<20}{:<20}"};

enum class Option_type { invalid, boolean, integer, text };

struct Option_info {
    std::string name;
    std::string help_type;
    std::string help_meaning;
    int help_sort_order{0};
    Option_type type{Option_type::invalid};
    std::string default_value;
    bool required{false};
    // Vector of options which this option depends on
    std::vector<std::string> depends_on;
};

// Check that no more than one of incompatible are present.  At most one from the set is allowed.
void check_compatibility(
    const std::unordered_map<std::string, std::string>& options, const std::vector<std::string>& incompatible);

// Check that at least one of requires_one_of is present.
void check_requires_at_least_one_of(
    const std::unordered_map<std::string, std::string>& options, const std::vector<std::string>& requires_one_of);

using aggregate_check_func
    = void (*)(const std::unordered_map<std::string, std::string>&, const std::vector<std::string>&);

// Options_manager provides methods to read, check, and compose a help message for options.
// Options are specified using an unordered map with option name as the key and option value
// as the value.  Both option name and option value are strings.
// To use Option_manager, callers should first call add_info to set the information about each
// option.
// Next, one of the methods to add options should be called:
//    add_options_from_config_file or;
//    add_options_from_command_line or;
//    add_options
// Once options are set, call check_options to verify that options meet requirements specified in
// the option info and to set default values for options that are present but whose value is empty.
// If a user help message is needed, call write_help_message to get a help message describing the
// options.  The help message is composed using information specified in the options info.
// Once options have been checked, call get_options to obtain a copy of verified options with default
// values set for options whose value was originally empty.
class Options_manager {
public:
    Options_manager() = default;

    ~Options_manager() = default;

    Options_manager(const Options_manager&) = delete;   
	
    Options_manager& operator=(const Options_manager&) = delete;  
	
    Options_manager(Options_manager&&) noexcept = delete;   
	
    Options_manager& operator=(Options_manager&&) noexcept = delete;
    
    // Adds aggregate option checks, augmenting existing checks.
    void add_aggregate_checks(
        const std::vector<std::pair<std::vector<std::string>, aggregate_check_func>>& aggregate_checks);

    // Adds information for each option, augmenting existing information.  Options_manager uses this
    // information to check options and to compose the help message.
    void add_info(const std::unordered_map<std::string, Option_info>& info);

    // Adds options, augmenting exising options.  If an existing option is passed, the new option replaces
    // the existing one.
    void add_options(const std::unordered_map<std::string, std::string>& options);

    // Reads options from the command line and add them to existing options.
    void add_options_from_command_line(std::span<const char* const> args);

    // Reads options from an XML config file and add them to existing options.
    void add_options_from_config_file(const std::string& config_file);

    // Returns a copy of the options.
    [[nodiscard]] std::unordered_map<std::string, std::string> get_options() const;

    // Returns a copy of the options, excluding options_to_exclude.  This makes it easy to
    // combine options and requirements from two sources, set defaults for and check all options
    // at once, and then to retrieve options from each source separately.  For example, an
    // application may wish to combine its options with that of a library for purposes of checking,
    // and then to separately retrieve application-specific and library-specific options
    // separately once checking is done.
    [[nodiscard]] std::unordered_map<std::string, std::string> get_options_exclusive_of(
        const std::unordered_map<std::string, Option_info>& options_to_exclude) const;

    // Clears all options and option info, resetting the manager to its initial state.
    void reset();

    // If an option is present, but its value is empty, sets the option to its default value.  Then
    // checks options against requirements specified in the option info. Throws an exception if no
    // option info exists for an option, or if a requirement is violated, or if the special help option
    // is present.
    void set_defaults_then_check_options();

    // Sets the format for help messages.  Use to change the format in case the default isn't desirable.
    void set_help_format(const std::string& help_format);

    // Uses the option info to write a help message to out.  Throws an exception if option info hasn't
    // been set.
    void write_help_message(std::ostream& out) const;

private:
    void check_aggregate_requirements_();

    void check_for_dependency_violations_();

    void check_for_missing_required_options_();

    void check_options_();

    void check_option_(const std::string& option_name, const std::string& option_value);

    void set_defaults_();

    std::vector<std::pair<std::vector<std::string>, aggregate_check_func>> m_aggregate_checks;
    std::string m_help_format{default_help_fmt};
    std::unordered_map<std::string, Option_info> m_info;
    std::unordered_map<std::string, std::string> m_options;
};

} // namespace hankinsohl::options
