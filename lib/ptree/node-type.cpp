// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/2/2024.

#include <array>
#include <cstddef>
#include <include/node-type.hpp>
#include <string>
#include <unordered_map>
#include <utility>

namespace bpt = boost::property_tree;

namespace c4lib::property_tree {

// Note: The lookup table below using name-value pairs allows for bidirectional
// lookup, that is enumerator-to-string as well as string-to-enumerator.  The order
// of pairs in the table must exactly match the enumerator order in the Node_type
// enumeration.
const std::array node_type_names{
    std::pair<Node_type, const std::string>{Node_type::invalid, "invalid"},

    // Integer types
    std::pair<Node_type, const std::string>{Node_type::bool_type, "bool_type"},
    std::pair<Node_type, const std::string>{Node_type::hex_type, "hex_type"},
    std::pair<Node_type, const std::string>{Node_type::int_type, "int_type"},
    std::pair<Node_type, const std::string>{Node_type::uint_type, "uint_type"},
    std::pair<Node_type, const std::string>{Node_type::enum_type, "enum_type"},

    // String types
    std::pair<Node_type, const std::string>{Node_type::string_type, "string_type"},
    std::pair<Node_type, const std::string>{Node_type::u16string_type, "wstring_type"},
    std::pair<Node_type, const std::string>{Node_type::md5_type, "md5_type"},

    // Compound types
    std::pair<Node_type, const std::string>{Node_type::struct_type, "struct_type"},
    std::pair<Node_type, const std::string>{Node_type::template_type, "template_type"},

    // Convenience types
    std::pair<Node_type, const std::string>{Node_type::array_type, "array_type"},
    std::pair<Node_type, const std::string>{Node_type::subscript_type, "subscript_type"},
};

static const std::unordered_map<std::string, Node_type> node_type_enum_lookup{[] {
    std::unordered_map<std::string, Node_type> enum_lookup;
    enum_lookup.reserve(node_type_names.size());
    for (const auto& p : node_type_names) {
        enum_lookup[p.second] = p.first;
    }
    return enum_lookup;
}()};

std::string node_type_as_string(const Node_type type)
{
    return bpt::translator_between<std::string, c4lib::property_tree::Node_type>::put_value(type).value();
}

Node_type to_node_type(const char* enumerator)
{
    return node_type_enum_lookup.at(enumerator);
}

const std::string& to_string(Node_type type)
{
    const size_t index{static_cast<size_t>(type)};
    return node_type_names.at(index).second;
}

} // namespace c4lib::property_tree
