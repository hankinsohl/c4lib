// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/12/2024.

#pragma once

#include <array>
#include <climits>
#include <cstdint>
#include <lib/options/options-manager.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace hankinsohl::options {

inline const char* const help_fmt{"{:<25}{:<10}{:<20}"};

inline const char* const help_message{
    R"(BOOL_OPTION_FALSE        [0|1]     Help for bool option false.
BOOL_OPTION_TRUE         [0|1]     Help for bool option true.
INT_OPTION_NEGATIVE      <int>     Help for int option negative.
INT_OPTION_POSITIVE      <int>     Help for int option positive.
INT_OPTION_ZERO          <int>     Help for int option zero.
TEXT_OPTION_1            <text>    Help for text option 1.
TEXT_OPTION_2            <text>    Help for text option 2.
)"};

inline constexpr std::array good_options_all_cli{
    "c4edit",

    "-BOOL_OPTION_TRUE=1",
    "-BOOL_OPTION_FALSE=0",

    "-INT_OPTION_NEGATIVE=-1",
    "-INT_OPTION_ZERO=0",
    "-INT_OPTION_POSITIVE=1",

    "-text_option_1=1",
    "-text_option_2=*& Hello !?_+",
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOLEAN OPTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const Option_info bool_option_true_info{.name = "BOOL_OPTION_TRUE",
    .help_type = "[0|1]",
    .help_meaning = "Help for bool option true.",
    .help_sort_order = 2,
    .type = Option_type::boolean,
    .default_value = "1",
    .required = false,
    .depends_on = {}};

inline const Option_info bool_option_false_info{.name = "BOOL_OPTION_FALSE",
    .help_type = "[0|1]",
    .help_meaning = "Help for bool option false.",
    .help_sort_order = 1,
    .type = Option_type::boolean,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

inline const std::unordered_map<std::string, Option_info> boolean_options_info_lookup{
    {bool_option_true_info.name, bool_option_true_info},
    {bool_option_false_info.name, bool_option_false_info},
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTEGER OPTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const Option_info int_option_negative_info{.name = "INT_OPTION_NEGATIVE",
    .help_type = "<int>",
    .help_meaning = "Help for int option negative.",
    .help_sort_order = 3,
    .type = Option_type::integer,
    .default_value = "-1",
    .required = false,
    .depends_on = {}};

inline const Option_info int_option_zero_info{.name = "INT_OPTION_ZERO",
    .help_type = "<int>",
    .help_meaning = "Help for int option zero.",
    .help_sort_order = 5,
    .type = Option_type::integer,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

inline const Option_info int_option_positive_info{.name = "INT_OPTION_POSITIVE",
    .help_type = "<int>",
    .help_meaning = "Help for int option positive.",
    .help_sort_order = 4,
    .type = Option_type::integer,
    .default_value = "1",
    .required = false,
    .depends_on = {}};

inline const std::unordered_map<std::string, Option_info> integer_options_info_lookup{
    {int_option_negative_info.name, int_option_negative_info},
    {int_option_zero_info.name, int_option_zero_info},
    {int_option_positive_info.name, int_option_positive_info},
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TEXT OPTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const Option_info text_option_1_info{.name = "TEXT_OPTION_1",
    .help_type = "<text>",
    .help_meaning = "Help for text option 1.",
    .help_sort_order = 6,
    .type = Option_type::text,
    .default_value = "text 1",
    .required = false,
    .depends_on = {}};

inline const Option_info text_option_2_info{.name = "TEXT_OPTION_2",
    .help_type = "<text>",
    .help_meaning = "Help for text option 2.",
    .help_sort_order = 7,
    .type = Option_type::text,
    .default_value = "text 2",
    .required = false,
    .depends_on = {}};

inline const std::unordered_map<std::string, Option_info> text_options_info_lookup{
    {text_option_1_info.name, text_option_1_info},
    {text_option_2_info.name, text_option_2_info},
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ALL OPTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const std::unordered_map<std::string, Option_info> all_options_info_lookup{
    {bool_option_true_info.name, bool_option_true_info},
    {bool_option_false_info.name, bool_option_false_info},

    {int_option_negative_info.name, int_option_negative_info},
    {int_option_zero_info.name, int_option_zero_info},
    {int_option_positive_info.name, int_option_positive_info},

    {text_option_1_info.name, text_option_1_info},
    {text_option_2_info.name, text_option_2_info},
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GOOD AND BAD OPTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const std::unordered_map<std::string, std::string> good_option_help{
    {"HELP", ""},
};

inline const std::unordered_map<std::string, std::string> bad_boolean_option_out_of_range{
    {"BOOL_OPTION_TRUE", "2"},
};

inline const std::unordered_map<std::string, std::string> bad_boolean_option_as_text{
    {"BOOL_OPTION_TRUE", "true"},
};

inline const std::unordered_map<std::string, std::string> bad_integer_option_out_of_range_overflow{
    {"INT_OPTION_POSITIVE", std::to_string(int64_t{INT_MAX} + 1)},
};

inline const std::unordered_map<std::string, std::string> bad_integer_option_out_of_range_underflow{
    {"INT_OPTION_NEGATIVE", std::to_string(int64_t{INT_MIN} - 1)},
};

inline const std::unordered_map<std::string, std::string> bad_integer_option_as_text{
    {"INT_OPTION_POSITIVE", "one"},
};

inline const std::unordered_map<std::string, std::string> good_boolean_options{
    {"BOOL_OPTION_TRUE", "1"},
    {"BOOL_OPTION_FALSE", "0"},
};

inline const std::unordered_map<std::string, std::string> good_integer_options{
    {"INT_OPTION_NEGATIVE", "-1"},
    {"INT_OPTION_ZERO", "0"},
    {"INT_OPTION_POSITIVE", "1"},
};

inline const std::unordered_map<std::string, std::string> good_text_options{
    {"TEXT_OPTION_1", "1"},
    {"TEXT_OPTION_2", "*& Hello !?_+"},
};

inline const std::unordered_map<std::string, std::string> all_good_options{
    {"BOOL_OPTION_TRUE", "1"},
    {"BOOL_OPTION_FALSE", "0"},
    {"INT_OPTION_NEGATIVE", "-1"},
    {"INT_OPTION_ZERO", "0"},
    {"INT_OPTION_POSITIVE", "1"},
    {"TEXT_OPTION_1", "1"},
    {"TEXT_OPTION_2", "*& Hello !?_+"},
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEPENDANT OPTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const Option_info dependant_text_option_info{.name = "DEPENDANT_TEXT_OPTION",
    .help_type = "<text>",
    .help_meaning = "help_for_dependant_text_option",
    .help_sort_order = 1,
    .type = Option_type::text,
    .default_value = "needy",
    .required = false,
    .depends_on = {"PREREQUISITE_1", "PREREQUISITE_2", "PREREQUISITE_3"}};

inline const Option_info prerequisite_1_option_info{.name = "PREREQUISITE_1",
    .help_type = "[0|1]",
    .help_meaning = "help_for_prerequisite_1",
    .help_sort_order = 2,
    .type = Option_type::boolean,
    .default_value = "0",
    .required = false,
    .depends_on = {}};

inline const Option_info prerequisite_2_option_info{.name = "PREREQUISITE_2",
    .help_type = "<int>",
    .help_meaning = "help_for_prerequisite_2",
    .help_sort_order = 3,
    .type = Option_type::integer,
    .default_value = "3",
    .required = false,
    .depends_on = {}};

inline const Option_info prerequisite_3_option_info{.name = "PREREQUISITE_3",
    .help_type = "<text>",
    .help_meaning = "help_for_prerequisite_3",
    .help_sort_order = 4,
    .type = Option_type::text,
    .default_value = "prerequisite 3",
    .required = false,
    .depends_on = {}};

inline const std::unordered_map<std::string, Option_info> options_dependant_on_3_info_lookup{
    {dependant_text_option_info.name, dependant_text_option_info},
    {prerequisite_1_option_info.name, prerequisite_1_option_info},
    {prerequisite_2_option_info.name, prerequisite_2_option_info},
    {prerequisite_3_option_info.name, prerequisite_3_option_info},
};

inline const std::unordered_map<std::string, std::string> good_dependant_option{
    {"DEPENDANT_TEXT_OPTION", "needy"},
};

inline const std::unordered_map<std::string, std::string> good_prerequisite_1_option{
    {"PREREQUISITE_1", "0"},
};

inline const std::unordered_map<std::string, std::string> good_prerequisite_2_option{
    {"PREREQUISITE_2", "3"},
};

inline const std::unordered_map<std::string, std::string> good_prerequisite_3_option{
    {"PREREQUISITE_3", "prerequisite 3"},
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AGGREGATE OPTION INFO
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline const std::vector<std::string> requires_one_of{
    "ONE_OF_THREE_BOOL",
    "TWO_OF_THREE_INT",
    "THREE_OF_THREE_TEXT",
};

inline const std::vector<std::string> incompatible_options{
    "ONE_OF_THREE_BOOL",
    "TWO_OF_THREE_INT",
    "THREE_OF_THREE_TEXT",
};

inline const Option_info one_of_three_bool_option_info{.name = "ONE_OF_THREE_BOOL",
    .help_type = "[0|1]",
    .help_meaning = "help_for_one_of_three_bool",
    .help_sort_order = 1,
    .type = Option_type::boolean,
    .default_value = "1",
    .required = false,
    .depends_on = {}};

inline const Option_info two_of_three_int_option_info{.name = "TWO_OF_THREE_INT",
    .help_type = "<int>",
    .help_meaning = "help_for_two_of_three_int",
    .help_sort_order = 2,
    .type = Option_type::integer,
    .default_value = "1",
    .required = false,
    .depends_on = {}};

inline const Option_info three_of_three_text_option_info{.name = "THREE_OF_THREE_TEXT",
    .help_type = "<text>",
    .help_meaning = "help_for_three_of_three_text",
    .help_sort_order = 3,
    .type = Option_type::text,
    .default_value = "1",
    .required = false,
    .depends_on = {}};

inline const std::unordered_map<std::string, Option_info> options_info_for_aggregate{
    {one_of_three_bool_option_info.name, one_of_three_bool_option_info},
    {two_of_three_int_option_info.name, two_of_three_int_option_info},
    {three_of_three_text_option_info.name, three_of_three_text_option_info},
};

inline const std::unordered_map<std::string, std::string> good_option_one_of_three_bool{
    {"ONE_OF_THREE_BOOL", "1"},
};

inline const std::unordered_map<std::string, std::string> good_option_two_of_three_int{
    {"TWO_OF_THREE_INT", "1"},
};

inline const std::unordered_map<std::string, std::string> good_option_three_of_three_text{
    {"THREE_OF_THREE_TEXT", "1"},
};

} // namespace hankinsohl::options
