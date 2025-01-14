// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/3/2024.

#pragma once

#include <cstddef>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/util/file-location.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace c4lib::schema_parser {

// Defines certain compound types (structures and templates) as well as enumerations.
class Definition {
public:
    Definition(std::string name, Def_type type, File_location loc);

    ~Definition() = default;

    Definition(const Definition&) = delete;

    Definition& operator=(const Definition&) = delete;

    Definition(Definition&&) noexcept = delete;

    Definition& operator=(Definition&&) noexcept = delete;

    // Adds a member to the definition.  If member type is inconsistent with the definition, an exception is thrown.
    // is_modular applies to const and enum definitions.  If is_modular is true, an existing member definition of the
    // same name is overwritten.  If is_modular is false or does not apply, the member name must not yet exist,
    // otherwise an exception is thrown.
    void add_member(Def_mem& member, bool allow_duplicates, bool is_modular);

    // Returns the location at which the definition was found (useful for debugging).
    [[nodiscard]] const File_location& get_file_location() const;

    [[nodiscard]] const std::vector<Def_mem>& get_members() const;

    [[nodiscard]] std::vector<Def_mem>& get_members();

    // Returns the name of the definition.
    [[nodiscard]] const std::string& get_name() const;

    // Returns the type of the definition.
    [[nodiscard]] Def_type get_type() const;

private:
    void check_member_type_(const Def_mem& member) const;

    Def_type m_def_type;
    const File_location m_loc;
    std::vector<Def_mem> m_members;
    std::unordered_map<std::string, size_t> m_members_hash_map;
    std::string m_name;
};

} // namespace c4lib::schema_parser
