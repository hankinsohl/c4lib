// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/13/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace c4lib {

class Variable_manager {
public:
    Variable_manager() = default;

    ~Variable_manager() = default;

    Variable_manager(const Variable_manager&) = delete;

    Variable_manager& operator=(const Variable_manager&) = delete;

    Variable_manager(Variable_manager&&) noexcept = delete;

    Variable_manager& operator=(Variable_manager&&) noexcept = delete;

    // Adds variable to the current scope and sets its value.  If the variable currently exists,
    // an exception is thrown.  The variable name must not contain "." otherwise an exception is
    // thrown.  The dot syntax is used to refer to ptree variables.
    void add(const std::string& variable, int value);

    // Looks up variable and returns its value.  Throws an exception if variable does not exist.
    // Variable may refer to a scoped variable or to a ptree variable.
    int get(const std::string& variable);

    // Initializes property trees enabling resolution of ptree node references.  Also initializes the
    // definition table, used to resolve references to consts. The variable manager can be used prior to calling
    // init if ptree reference resolution and const-name lookup are not required (e.g., in  phase 1 parsing).
    void init(boost::property_tree::ptree* ptree,
        boost::property_tree::ptree** parent_ptree,
        schema_parser::Def_tbl* definition_table);

    // Pops the current scope, removing all variables defined in the scope.
    void pop();

    // Pushes a new scope.
    void push();

    // Looks up variable and sets its value.  Throws an exception if the variable does
    // not exist, or if the variable name refers to a ptree variable.
    void set(const std::string& variable, int value);

private:
    schema_parser::Def_tbl* m_definition_table{nullptr};
    std::unordered_map<std::string, int> m_lookup;
    boost::property_tree::ptree** m_parent_ptree{nullptr};
    boost::property_tree::ptree* m_root_ptree{nullptr};
    std::vector<std::vector<std::string>> m_scopes;
};

} // namespace c4lib
