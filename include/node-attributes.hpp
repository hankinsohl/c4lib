// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/10/2024.

#pragma once

#include <cstddef>
#include <include/node-type.hpp>
#include <string>

namespace c4lib::property_tree {

// Attributes for main-path ptree nodes.  Each node on the main path
// has a child attributes node, which in turn has one child for
// each attribute.
struct Attributes_node {
    // The name of the node.
    std::string name;
    // Enumerator for the type.  The enumerator is used to determine how
    // the data should be formatted and presented to the user - for example
    // as an MD5 value, or a string, or a 32-bit integer, etc.
    Node_type type{Node_type::invalid};
    // The name of the type.
    std::string type_name;
    // Sequence of hex bytes for the type in little-endian format, exactly as the
    // type appears on disk.  If the type has no on-disk representation, data is the
    // empty string.  The format of the string is hexadecimal with a 0x prefix, e.g.,
    // 0x78653412, for a 4-byte integer.
    std::string data;
    // Size of the type in bytes.  Although size can be calculated from
    // the data member, it is provided here for convenience.
    size_t size;
    // If the node is an array member, the name of the array, otherwise an empty string.
    std::string array_name;
    // Formatted array subscripts for the type.  Empty string if the type is not part
    // of an array.  The subscripts string does not control the on-disk value of
    // the type but is instead used to better format the type for text output.
    // Example subscripts strings are:
    //    [5]
    //    [4:LEADER_SALADIN]
    //    [2:LEADER_ZARA_YAQOB][41]
    std::string subscripts;
    // If the type is an enumerator constant, enum_name is the name of the
    // associated enumeration.  If the type is not an enumerator constant,
    // enum_name is the empty string.  The enum_name field is used to format
    // the type when presented in text format.
    std::string enum_name;
};

// The first node in a savegame ptree is an origin node.  The origin node documents the origin of the ptree.
struct Origin_node {
    // The name of the Beyond the Sword savegame used to generate the ptree.
    std::string savegame;

    // The name of the schema used to generate the ptree.
    std::string schema;

    // The date of creation of the ptree.
    std::string date;

    // The version of the c4 library used to generate the ptree in Major.Minor.Bug-fix format where
    //    * Changes to Major denote format changes incompatible with previous versions
    //    * Changes to Minor denote new features
    //    * Changes to Bug-fix denotes changes to fix bugs or refactorings
    std::string c4lib_version;
};

// Value of meta nodes (used to skip meta nodes during recursive tree traversal).
inline constexpr const char* nv_meta{"*"};

// Name of the attributes node.
inline constexpr const char* nn_attributes{"__Attributes__"};

// Names for the attribute child nodes.
inline constexpr const char* nn_name{"__Name__"};
inline constexpr const char* nn_type{"__Type__"};
inline constexpr const char* nn_typename{"__Typename__"};
inline constexpr const char* nn_data{"__Data__"};
inline constexpr const char* nn_formatted_data{"__FormattedData__"};
inline constexpr const char* nn_size{"__Size__"};
inline constexpr const char* nn_array_name{"__ArrayName__"};
inline constexpr const char* nn_subscripts{"__Subscripts__"};
inline constexpr const char* nn_enum{"__Enum__"};

// Name of the origin node.
inline constexpr const char* nn_origin{"__Origin__"};

// Names for the origin child nodes.
inline constexpr const char* nn_savegame{"__Savegame__"};
inline constexpr const char* nn_schema{"__Schema__"};
inline constexpr const char* nn_date{"__Date__"};
inline constexpr const char* nn_c4lib_version{"__C4Lib_Version__"};

} // namespace c4lib::property_tree
