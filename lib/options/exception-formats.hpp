// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/11/2024.

#pragma once

namespace hankinsohl::options::format {

inline constexpr const char* bad_boolean_value{"Option '{}' must be '0' or '1', but was '{}' instead."};
inline constexpr const char* bad_format{"Bad format: '{}'."};
inline constexpr const char* bad_integer_value{"Option '{}' must be an integer, but was '{}' instead."};
inline constexpr const char* bad_integer_value_range{"Integer option '{}' value out of range.  Value was '{}'."};
inline constexpr const char* cli_option_missing_dash{"Command line option does not begin with '-': '{}'."};
inline constexpr const char* dependency_missing{"Option '{0}' depends on option '{1}', but '{1}' is missing."};
inline constexpr const char* incompatible_options{"Incompatible options specified: '{}'"};
inline constexpr const char* none_from_required_set{"Required option from set not specified: '{}'."};
inline constexpr const char* required_option_missing{"Required option '{}' is missing."};
inline constexpr const char* too_many_from_required_set{"More than one option from set specified: '{}'."};
inline constexpr const char* unknown_option{"Unknown option: '{}'."};
inline constexpr const char* xml_config_element_missing{"Expected config element.  Read '{}' element instead."};
} // namespace hankinsohl::options::format
