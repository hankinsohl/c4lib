// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/11/2024.

#pragma once

#include <lib/importer/file-manager.hpp>
#include <lib/native/path.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/token.hpp>
#include <string>
#include <unordered_map>
#include <utility>

namespace c4lib {

class Importer {
public:
    Importer() = default;

    ~Importer() = default;

    Importer(const Importer&) = delete;   
	
    Importer& operator=(const Importer&) = delete;  
	
    Importer(Importer&&) noexcept = delete;   
	
    Importer& operator=(Importer&&) noexcept = delete;    

    // Adds the const name to the const import table.
    void add_const(const schema_parser::Token& const_name);

    // Adds the enum name to the enum import table.
    void add_enum(const schema_parser::Token& enum_name,
        const schema_parser::Token& xml_path,
        const schema_parser::Token& search_path);

    void import_definitions(schema_parser::Def_tbl& definition_table,
        const native::Path& install_root,
        const native::Path& custom_assets_path,
        const std::string& mod_name,
        bool use_modular_loading);

    // Resets the definition importer returning it to the state of a newly created importer.
    void reset();

private:
    struct Enum_data {
        Enum_data(const schema_parser::Token& token_, std::string xml_path_, native::Path search_path_)
            : token(token_), xml_path(std::move(xml_path_)), search_path(std::move(search_path_))
        {}

        // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
        const schema_parser::Token& token;
        std::string xml_path;
        native::Path search_path;
    };

    bool import_const_(const schema_parser::Token& const_name, const native::Path& file_path, bool is_modular = false) const;

    void import_consts_(const File_manager& file_manager);

    bool import_enum_(const schema_parser::Token& enum_name,
        const std::string& xml_path,
        const native::Path& file_path,
        bool is_modular = false) const;

    void import_enums_(const File_manager& file_manager);

    std::unordered_map<std::string, schema_parser::Token> m_const_import_table;
    schema_parser::Def_tbl* m_definition_table{nullptr};
    std::unordered_map<std::string, Enum_data> m_enum_import_table;
    bool m_use_modular_loading{false};
};

} // namespace c4lib
