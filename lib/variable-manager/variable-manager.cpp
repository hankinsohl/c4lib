// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/13/2024.

#include <array>
#include <boost/property_tree/ptree.hpp>
#include <cstddef>
#include <format>
#include <include/exceptions.hpp>
#include <include/node-attributes.hpp>
#include <include/node-type.hpp>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <string>
#include <vector>

namespace bpt = boost::property_tree;
namespace cpt = c4lib::property_tree;
namespace csp = c4lib::schema_parser;

namespace c4lib {

void Variable_manager::add(const std::string& variable, const int value)
{
    if (variable.find_first_of('.') != std::string::npos) {
        throw Variable_manager_error(std::format(fmt::variable_name_contains_dot, variable));
    }
    if (m_lookup.contains(variable)) {
        throw Variable_manager_error(std::format(fmt::add_variable_error, variable));
    }
    m_lookup[variable] = value;
    m_scopes.back().emplace_back(variable);
}

int Variable_manager::get(const std::string& variable)
{
    // Attempt to resolve the variable reference.  Resolution checks three sources:
    //      1. The lookup - resolves references to local variables, and;
    //      2. The ptrees - resolves references to ptree nodes
    //      3. The definition table - resolves references to global constants and enumerators;

    // We begin by checking variable for the presence of "::" - if found, it's a reference to an
    // enumerator, and we need only check the definition table.
    if (const size_t pos_sr_op{variable.find("::")}; pos_sr_op != std::string::npos) {
        if (pos_sr_op + 2 > variable.size()) {
            throw Variable_manager_error(std::format(fmt::malformed_enumerator_reference, variable));
        }
        const std::string enum_name{variable.substr(0, pos_sr_op)};
        const std::string enumerator{variable.substr(pos_sr_op + 2)};
        const csp::Def_mem& enumerator_def{m_definition_table->get_enumerator(enum_name, enumerator)};
        return enumerator_def.value;
    }

    // 1.  Check the lookup table.
    if (m_lookup.contains(variable)) {
        // Resolution succeeded.
        return m_lookup[variable];
    }

    // 2.  Check the ptrees.
    const std::string path{variable + "." + cpt::nn_attributes};
    // The variable reference might be relative to the root or to the parent.  Check both possibilities
    const std::array ptrees{*m_parent_ptree, m_root_ptree};
    for (bpt::ptree* ptree : ptrees) {
        if (const boost::optional<bpt::ptree&> node{ptree->get_child_optional(path)}) {
            // Resolution succeeded.

            // Check the node type.  We support lookup of integer values only.
            const cpt::Node_type type{node->get<cpt::Node_type>(cpt::nn_type)};
            if (type < cpt::Node_type::first_integer_type || type > cpt::Node_type::last_integer_type) {
                throw Variable_manager_error(std::format(fmt::variable_not_an_integer_type, variable));
            }
            return node->get<int>(cpt::nn_data);
        }
    }

    // 3. Check the definition table.  If lookup fails the definition table throws an exception.  We
    // check the definition table last to facilitate debugging with break on exception enabled.  Checking
    // the definition table earlier would result in spurious exceptions.
    const int value{m_definition_table->get_const_value(variable)};

    // Resolution succeeded.
    return value;
}

void Variable_manager::init(bpt::ptree* ptree, bpt::ptree** parent_ptree, csp::Def_tbl* definition_table)
{
    m_root_ptree = ptree;
    m_parent_ptree = parent_ptree;
    m_definition_table = definition_table;
}

void Variable_manager::pop()
{
    const std::vector<std::string>& last_scope{m_scopes.back()};
    for (const auto& name : last_scope) {
        m_lookup.erase(name);
    }
    m_scopes.pop_back();
}

// Pushes a new scope.
void Variable_manager::push()
{
    m_scopes.emplace_back();
}

void Variable_manager::set(const std::string& variable, const int value)
{
    if (variable.find_first_of('.') != std::string::npos) {
        throw Variable_manager_error(std::format(fmt::variable_name_contains_dot, variable));
    }
    if (!m_lookup.contains(variable)) {
        throw Variable_manager_error(std::format(fmt::variable_does_not_exist, variable));
    }
    m_lookup[variable] = value;
}

} // namespace c4lib
