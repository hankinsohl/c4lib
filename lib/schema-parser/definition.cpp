// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/5/2024.

#include <cstddef>
#include <include/exceptions.hpp>
#include <lib/schema-parser/def-mem-type.hpp>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/definition.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/file-location.hpp>
#include <string>
#include <utility>
#include <vector>

namespace c4lib::schema_parser {

Definition::Definition(std::string name, Def_type type, File_location loc)
    : m_def_type(type), m_loc(std::move(loc)), m_name(std::move(name))
{}

// Adds a member to the definition.  If member is inconsistent with the definition, an exception is thrown.
void Definition::add_member(Def_mem& member, bool allow_duplicates, bool is_modular)
{
    check_member_type_(member);
    if (m_members_hash_map.contains(member.name) && !allow_duplicates) {
        if (!is_modular || m_members.at(m_members_hash_map[member.name]).type != member.type
            || (member.type != Def_mem_type::const_type && member.type != Def_mem_type::enum_type)) {
            throw make_ex<Importer_error>(fmt::duplicated_name, member.loc, member.name);
        }
        m_members.at(m_members_hash_map[member.name]) = member;
    }
    else {
        const size_t index{m_members.size()};
        m_members_hash_map[member.name] = index;
        m_members.push_back(member);
    }
}

const File_location& Definition::get_file_location() const
{
    return m_loc;
}

const std::vector<Def_mem>& Definition::get_members() const
{
    return m_members;
}

std::vector<Def_mem>& Definition::get_members()
{
    return m_members;
}

const std::string& Definition::get_name() const
{
    return m_name;
}

Def_type Definition::get_type() const
{
    return m_def_type;
}

void Definition::check_member_type_(const Def_mem& member) const
{
    bool is_compatible{false};
    switch (m_def_type) {
    case Def_type::alias_type:
        if (member.type == Def_mem_type::alias_type) {
            is_compatible = true;
        }
        break;

    case Def_type::const_type:
        if (member.type == Def_mem_type::const_type) {
            is_compatible = true;
        }
        break;

    case Def_type::enum_type:
        if (member.type == Def_mem_type::enum_type) {
            is_compatible = true;
        }
        break;

    case Def_type::struct_type:
        if (member.type == Def_mem_type::struct_type) {
            is_compatible = true;
        }
        break;

    case Def_type::template_type:
        if (member.type == Def_mem_type::template_type) {
            is_compatible = true;
        }
        break;

    default:
        throw make_ex<Parser_error>(fmt::unexpected_definition_type, member.loc, to_string(m_def_type));
    }

    if (!is_compatible) {
        throw make_ex<Parser_error>(
            fmt::incompatible_definition_member_type, member.loc, to_string(member.type), to_string(m_def_type));
    }
}

} // namespace c4lib::schema_parser
