// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/4/2024.

#include <cstddef>
#include <format>
#include <include/exceptions.hpp>
#include <iosfwd>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/definition.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/file-location.hpp>
#include <lib/util/text.hpp>
#include <map>
#include <ostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace c4lib::schema_parser {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Definition& Def_tbl::create_definition(
    const std::string& name, Def_type type, const File_location& loc, bool& was_created)
{
    if (!m_definition_table.contains(name)) {
        m_definition_table.try_emplace(name, name, type, loc);
        was_created = true;
    }
    else {
        was_created = false;
    }
    Definition& def{m_definition_table.at(name)};

    if (def.get_type() != type) {
        throw make_ex<Parser_error>(
            fmt::type_mismatch_in_definition, loc, name, to_string(type), to_string(def.get_type()));
    }

    return def;
}

void Def_tbl::export_definitions(Def_type type, std::ostream& out) const
{
    switch (type) {
    case Def_type::const_type:
        export_const_definitions_(out);
        break;

    case Def_type::enum_type:
        export_enum_definitions_(out);
        break;

    default:
        throw std::logic_error(std::format(fmt::export_of_type_not_supported, to_string(type)));
    }
}

int Def_tbl::get_const_value(const std::string& const_name) const
{
    const Def_mem& member{get_first_member(const_name, Def_type::const_type)};
    return member.value;
}

const Definition& Def_tbl::get_definition(const std::string& name, Def_type type) const
{
    try {
        const Definition& def{m_definition_table.at(name)};
        if (def.get_type() != type) {
            throw make_ex<Parser_error>(fmt::type_mismatch_in_definition, def.get_file_location(), name,
                to_string(type), to_string(def.get_type()));
        }

        return def;
    }
    catch (std::out_of_range&) {
        throw Parser_error(std::format(fmt::definition_does_not_exist, name));
    }
}

Definition& Def_tbl::get_definition(const std::string& name, Def_type type)
{
    try {
        Definition& def{m_definition_table.at(name)};
        if (def.get_type() != type) {
            throw make_ex<Parser_error>(fmt::type_mismatch_in_definition, def.get_file_location(), name,
                to_string(type), to_string(def.get_type()));
        }

        return def;
    }
    catch (std::out_of_range&) {
        throw Parser_error(std::format(fmt::definition_does_not_exist, name));
    }
}

const Def_mem& Def_tbl::get_enumerator(const std::string& enum_name, int enumerator_value) const
{
    const Definition& def{get_definition(enum_name, Def_type::enum_type)};
    for (const auto& def_mem : def.get_members()) {
        if (def_mem.value == enumerator_value) {
            return def_mem;
        }
    }
    throw make_ex<Parser_error>(fmt::enumerator_not_found, def.get_file_location(), enum_name, enumerator_value);
}

const Def_mem& Def_tbl::get_enumerator(const std::string& enum_name, const std::string& enumerator_name) const
{
    const Definition& def{get_definition(enum_name, Def_type::enum_type)};
    for (const auto& def_mem : def.get_members()) {
        if (def_mem.name == enumerator_name) {
            return def_mem;
        }
    }
    throw make_ex<Parser_error>(fmt::enumerator_not_found, def.get_file_location(), enum_name, enumerator_name);
}

const Def_mem& Def_tbl::get_first_member(const std::string& name, Def_type type) const
{
    const Definition& def{get_definition(name, type)};
    const std::vector<Def_mem>& members{def.get_members()};
    return members.at(0);
}

Def_mem& Def_tbl::get_first_member(const std::string& name, Def_type type)
{
    Definition& def{get_definition(name, type)};
    std::vector<Def_mem>& members{def.get_members()};
    return members.at(0);
}

std::unordered_map<std::string, Definition>& Def_tbl::get_definitions()
{
    return m_definition_table;
}

Def_type Def_tbl::get_type(const std::string& name) const
{
    try {
        const Definition& def{m_definition_table.at(name)};
        return def.get_type();
    }
    catch (std::out_of_range&) {
        throw Parser_error(std::format(fmt::definition_does_not_exist, name));
    }
}

void Def_tbl::reset()
{
    m_definition_table.clear();
}

// Returns the number of definitions in the table.
size_t Def_tbl::size() const
{
    return m_definition_table.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Def_tbl::export_const_definitions_(std::ostream& out) const
{
    std::map<std::string, const Definition*> def_map;
    make_map_(def_map, Def_type::const_type);
    for (const auto& def : def_map | std::views::values) {
        out << "const " << def->get_name() << " = " << def->get_members().at(0).value
            << " // from:" << to_string(def->get_file_location()) << '\n';
    }
}

void Def_tbl::export_enum_definitions_(std::ostream& out) const
{
    std::map<std::string, const Definition*> def_map;
    make_map_(def_map, Def_type::enum_type);
    for (const auto& def : def_map | std::views::values) {
        out << "enum " << def->get_name() << " // from:" << to_string(def->get_file_location()) << '\n';
        out << "{" << '\n';
        for (const auto& member : def->get_members()) {
            out << "    " << member.name << " = " << member.value << '\n';
        }
        out << "}" << '\n';

        // export the NUM_ constant for the enum, should one exist.
        const std::string const_name = "NUM_" + text::screaming_snake_case(def->get_name());
        if (m_definition_table.contains(const_name)) {
            const Definition& const_def{get_definition(const_name, Def_type::const_type)};
            const std::vector<Def_mem>& members{const_def.get_members()};
            const int const_value{members.at(0).value};
            out << const_name << " = " << const_value << '\n';
        }

        out << '\n';
    }
}

// Creates a map so that definitions appear in lexicographical sort order.
void Def_tbl::make_map_(std::map<std::string, const Definition*>& def_map, Def_type type) const
{
    for (const auto& def : m_definition_table | std::views::values) {
        if (def.get_type() == type) {
            def_map[def.get_name()] = &def;
        }
    }
}

} // namespace c4lib::schema_parser
