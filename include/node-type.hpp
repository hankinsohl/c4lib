// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/25/2024.

#pragma once

#include <boost/none.hpp>
#include <boost/optional/optional.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <string>

namespace c4lib::property_tree {

// NOLINTNEXTLINE(readability-enum-initial-value)
enum class Node_type {
    // N.B.: Node_type is iterable.  For this to work, the first enumerator must have value 0,
    // each succeeding enumerator must increment the previous value by 1, and the helper enumerators
    // "count", "begin" and "end" must not be removed.

    invalid = 0,

    // Integer types.  Do not reorder or add to integer types without also updating the helper
    // enumerators first_integer_type and last_integer_type.
    bool_type,
    hex_type,
    int_type,
    uint_type,
    enum_type,

    // String types
    string_type,
    u16string_type,
    md5_type,

    // Compound types
    struct_type,
    template_type,

    // Convenience types
    array_type,
    subscript_type,

    // Helper enumerators used for iteration - do not remove
    count,
    begin = 0,
    end = count - 1,

    // Helper enumerators to quickly determine if an enumerator is an integer type.
    first_integer_type = bool_type,
    last_integer_type = enum_type,
};

std::string node_type_as_string(Node_type type);

Node_type to_node_type(const char* enumerator);

const std::string& to_string(Node_type type);

} // namespace c4lib::property_tree

template<> struct boost::property_tree::translator_between<std::string, c4lib::property_tree::Node_type> {
    using internal_type = std::string;
    using external_type = c4lib::property_tree::Node_type;
    using type = translator_between<std::string, c4lib::property_tree::Node_type>;

    // Converts a string to Node_type
    static boost::optional<external_type> get_value(const internal_type& str)
    {
        if (str.empty()) {
            return boost::optional<external_type>{boost::none};
        }
        else {
            return boost::optional<external_type>{c4lib::property_tree::to_node_type(str.c_str())};
        }
    }

    // Converts a Node_type to int string
    static boost::optional<internal_type> put_value(const external_type& nt)
    {
        return boost::optional<internal_type>{c4lib::property_tree::to_string(nt)};
    }
};
