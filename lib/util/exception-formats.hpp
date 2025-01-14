// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/6/2024.

#pragma once

#include <format>
#include <lib/util/file-location.hpp>
#include <lib/util/text.hpp>
#include <string>

namespace c4lib::fmt {

// Exception message formats are listed below.  Formats are standardized to ensure that messages are consistent
// to facilitate testing.  Exception messages should use one of these formats.

inline constexpr const char* add_variable_error{"Add error - variable {} already exists."};
inline constexpr const char* array_dimension_out_of_range{"Array dimension {} out of range."};
inline constexpr const char* assertion_failed{"Assertion failed."};
inline constexpr const char* bad_enumerator_reference{"Bad enumerator reference."};
inline constexpr const char* bad_file_offset{"Bad file offset."};
inline constexpr const char* bad_node_reference{"Bad ptree node reference."};
inline constexpr const char* bad_search_path{"Bad XML path '{}'."};
inline constexpr const char* bad_state{"Bad state."};
inline constexpr const char* bad_subscripts_format{"Bad subscripts format."};
inline constexpr const char* bad_type_enum_format{"Bad type.  Bad enum format: '{}'."};
inline constexpr const char* bad_type_enumeration{"Bad type enumeration: '{}'."};
inline constexpr const char* bad_type_invalid_size{"Bad type.  Invalid size: '{}'."};
inline constexpr const char* bad_type_size_missing{"Bad type.  Size missing: '{}'."};
inline constexpr const char* bad_type_underscore_missing{"Bad type.  Underscore missing: '{}'."};
inline constexpr const char* const_definition_exists{"Duplicated const name '{}' during import."};
inline constexpr const char* definition_does_not_exist{"Definition for '{}' does not exist."};
inline constexpr const char* dereference_of_iterator_at_end{"Error - deference of iterator at end."};
inline constexpr const char* duplicated_name{"Duplicated name '{}' during import."};
inline constexpr const char* enum_definition_exists{"Duplicated enum name '{}' during import."};
inline constexpr const char* enumerator_not_found{"Enumerator {}.{} not found."};
inline constexpr const char* export_of_type_not_supported{"Export of definition type {} not supported."};
inline constexpr const char* failure_importing_const{"Failure importing const '{}'."};
inline constexpr const char* failure_importing_enum{"Failure importing enum '{}'."};
inline constexpr const char* identifier_exceeds_maximum_length{"Identifier '{}' exceeds maximum length {}."};
inline constexpr const char* illegal_boolean_value{"Illegal boolean value '{}'.  Booleans must be zero or one."};
inline constexpr const char* incompatible_definition_member_type{
    "Definition member type {} incompatible with definition type {}"};
inline constexpr const char* index_out_of_range{"Index '{}' out of range."};
inline constexpr const char* internal_bug_in_function{"Internal tokenizer bug in function {}."};
inline constexpr const char* invalid_chunk_size{"Compressed data size exceeds buffer size."};
inline constexpr const char* invalid_md5_length{"Invalid md5 length '{}': length must be {}."};
inline constexpr const char* invalid_token{"Invalid token starting with character '{}'."};
inline constexpr const char* line_exceeds_maximum_length{"Line exceeds maximum length {}."};
inline constexpr const char* malformed_enumerator_reference{"Malformed enumerator reference: '{}'."};
inline constexpr const char* mismatched_type_names{
    "Typename from template definition '{}' does not match typename from statement '{}'"};
inline constexpr const char* missing_file{"Cannot find '{}'."};
inline constexpr const char* narrowing_error{"Narrowing error."};
inline constexpr const char* no_led{"No left denotation for token '{}'."};
inline constexpr const char* no_nud{"No null denotation for token '{}'."};
inline constexpr const char* node_not_found{"Node '{}' not found."};
inline constexpr const char* node_source_error{"Error parsing token {}."};
inline constexpr const char* null_pointer_error{"Null pointer error."};
inline constexpr const char* number_exceeds_maximum_length{"Number '{}' exceeds maximum length {}."};
inline constexpr const char* out_of_range_error{"{} out of range in {}."};
inline constexpr const char* parser_skip_error{"Error skipping past tokens.  Expected {}: actual {}."};
inline constexpr const char* replace_typename_error{"Error replacing typename."};
inline constexpr const char* referenced_node_not_int{"Referenced node '{}' is not of type int."};
inline constexpr const char* runtime_error_io{"I/O error in '{}'."};
inline constexpr const char* runtime_error_opening_file{"Error opening file '{}'."};
inline constexpr const char* runtime_error_read{"Read error."};
inline constexpr const char* runtime_error_reading_from_file{"Error reading form file '{}'."};
inline constexpr const char* runtime_error_seek{"Seek error."};
inline constexpr const char* runtime_error_write{"Write error."};
inline constexpr const char* runtime_error_writing_to_file{"Error writing to file '{}'."};
inline constexpr const char* search_error{"Cannot find file using search path '{}'."};
inline constexpr const char* string_length_exceeds_maximum{"String length {} exceeds maximum {}."};
inline constexpr const char* string_literal_exceeds_maximum_length{"String literal '{}' exceeds maximum length {}."};
inline constexpr const char* syntax_error{"Syntax error parsing token {}."};
inline constexpr const char* type_mismatch_in_definition{
    "Type mismatch in definition for '{}'.  Expected: {}; actual: {}."};
inline constexpr const char* unexpected_definition_type{"Unexpected definition type {}."};
inline constexpr const char* unexpected_token_type{"Unexpected token type.  Expected {}; actual {}."};
inline constexpr const char* variable_does_not_exist{"Variable '{}' does not exist."};
inline constexpr const char* variable_name_contains_dot{"Illegal variable name '{}': name contains '.'"};
inline constexpr const char* variable_not_an_integer_type{"Variable '{}' is not an integer type."};
inline constexpr const char* zlib_error_bad_magic_value{"Bad zlib magic value."};
inline constexpr const char* zlib_error_deflate_init{"Error: deflateInit returned {}({}): {}."};
inline constexpr const char* zlib_error_inflate{"Error: inflate returned {}({}): {}."};
inline constexpr const char* zlib_error_inflate_init{"Error: inflateInit returned {}({}): {}."};
inline constexpr const char* zlib_initialization_error{"Error initializing ZStream: {}."};

} // namespace c4lib::fmt

namespace c4lib {
template<typename Ex, typename... Args>
Ex make_ex(const std::format_string<Args...> fmt, const File_location& loc, Args&&... args)
{
    std::string error{std::vformat(fmt.get(), std::make_format_args(args...))};
    text::add_location_to_message(error, loc);
    return Ex{error};
}

} // namespace c4lib
