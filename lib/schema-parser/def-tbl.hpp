// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/4/2024.

#pragma once

#include <lib/util/file-location.hpp>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/definition.hpp>
#include <lib/util/tune.hpp>
#include <map>
#include <string>
#include <unordered_map>
#include <iosfwd>
#include <cstddef>

namespace c4lib::schema_parser {

class Def_tbl {
public:
    Def_tbl() = default;
	
    ~Def_tbl() = default;
    
    Def_tbl(const Def_tbl&) = delete;   
	
    Def_tbl& operator=(const Def_tbl&) = delete;  
	
    Def_tbl(Def_tbl&&) noexcept = delete;   
	
    Def_tbl& operator=(Def_tbl&&) noexcept = delete;
    
    // Returns a reference to the specified definition.  If the definition does not exist it is created and
    // was_created is set to true.  Throws an exception if the definition exists but the type passed does not match the
    // existing type.
    Definition& create_definition(const std::string& name, Def_type type, const File_location& loc, bool& was_created);

    // Prints the definitions of the specified type to the output stream.  Throws an exception on error.
    void export_definitions(Def_type type, std::ostream& out) const;

    // Returns the value of constant const_name.  Throws an exception if const_name does not exist.
    [[nodiscard]] int get_const_value(const std::string& const_name) const;

    // Returns a reference to the existing named definition.  Throws an exception if the definition does not
    // exist or is of unexpected type.
    [[nodiscard]] const Definition& get_definition(const std::string& name, Def_type type) const;

    // Returns a reference to the existing named definition.  Throws an exception if the definition does not
    // exist or is of unexpected type.
    Definition& get_definition(const std::string& name, Def_type type);

    // Returns a reference to the definition table.
    std::unordered_map<std::string, Definition>& get_definitions();

    // Returns a reference to the definition member for the specified enumerator.  Throws an exception if the
    // definition member does not exist.
    [[nodiscard]] const Def_mem& get_enumerator(const std::string& enum_name, int enumerator_value) const;

    // Returns a reference to the definition member for the specified enumerator.  Throws an exception if the
    // definition member does not exist.
    [[nodiscard]] const Def_mem& get_enumerator(const std::string& enum_name, const std::string& enumerator_name) const;

    // Returns a reference to the first member of the existing named definition.  Throws an exception if the
    // definition does not or if no first member exists or if the definition is of unexpected type.
    [[nodiscard]] const Def_mem& get_first_member(const std::string& name, Def_type type) const;

    // Returns a reference to the first member of the existing named definition.  Throws an exception if the
    // definition does not exist or if no first member exists or if the definition is of unexpected type.
    Def_mem& get_first_member(const std::string& name, Def_type type);

    // Returns the type for name.  Throws an exception if name does not exist.
    [[nodiscard]] Def_type get_type(const std::string& name) const;

    // Resets the definition table returning it to the state of a newly created table.
    void reset();

    // Returns the number of definitions in the table.
    [[nodiscard]] size_t size() const;

private:
    void export_const_definitions_(std::ostream& out) const;

    void export_enum_definitions_(std::ostream& out) const;

    void make_map_(std::map<std::string, const Definition*>& def_map, Def_type type) const;

    std::unordered_map<std::string, Definition> m_definition_table{tune::definition_reserve_size};
};

} // namespace c4lib::schema_parser
