// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/11/2024.

#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <format>
#include <include/exceptions.hpp>
#include <lib/importer/file-manager.hpp>
#include <lib/importer/importer.hpp>
#include <lib/schema-parser/def-mem-type.hpp>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/definition.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/file-location.hpp>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

namespace bpt = boost::property_tree;
namespace csp = c4lib::schema_parser;

namespace c4lib {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Importer::add_const(const csp::Token& const_name)
{
    if (m_const_import_table.contains(const_name.value)) {
        throw make_ex<Importer_error>(fmt::duplicated_name, const_name.loc, const_name.value);
    }
    m_const_import_table.try_emplace(const_name.value, const_name);
}

void Importer::add_enum(const csp::Token& enum_name, const csp::Token& xml_path, const csp::Token& search_path)
{
    if (m_enum_import_table.contains(enum_name.value)) {
        throw make_ex<Importer_error>(fmt::duplicated_name, enum_name.loc, enum_name.value);
    }
    m_enum_import_table.try_emplace(enum_name.value, enum_name, xml_path.value, native::Path{search_path.value});
}

void Importer::import_definitions(csp::Def_tbl& definition_table,
    const native::Path& install_root,
    const native::Path& custom_assets_path,
    const std::string& mod_name,
    bool use_modular_loading)
{
    m_definition_table = &definition_table;
    m_use_modular_loading = use_modular_loading;
    const File_manager fileManager{install_root, custom_assets_path, mod_name};

    import_consts_(fileManager);
    import_enums_(fileManager);
}

void Importer::reset()
{
    m_const_import_table.clear();
    m_definition_table = nullptr;
    m_enum_import_table.clear();
    m_use_modular_loading = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Importer::import_const_(
    const schema_parser::Token& const_name, const native::Path& file_path, bool is_modular /* = false */) const
{
    // When importing we want the file location to refer to the file path to the XML file.  We'd also like
    // to reference the line and column number; unfortunately these values cannot be obtained using the property tree.
    File_location xml_file_location;
    xml_file_location.filename = std::make_shared<std::string>(file_path);
    xml_file_location.line = std::make_shared<std::string>("");
    xml_file_location.line_number = 0;
    xml_file_location.character_number = 0;

    bpt::ptree tree;
    bpt::read_xml(file_path, tree);

    // Iterate over Civ4Defines child nodes.
    for (const auto& [key, value] : tree.get_child("Civ4Defines")) {
        // If the child node is a Define node, check the value of DefineName to see if it matches the const name.
        if (key == "Define") {
            if (auto define_name{value.get<std::string>("DefineName")}; define_name == const_name.value) {
                const int int_value{value.get<int>("iDefineIntVal")};
                bool was_created{false};
                csp::Definition& definition{m_definition_table->create_definition(
                    const_name.value, csp::Def_type::const_type, xml_file_location, was_created)};

                // If we're not using modular loading, the definition must not yet exist.  Check
                // was_created to verify this.
                if (!is_modular && !was_created) {
                    throw make_ex<Importer_error>(fmt::const_definition_exists, const_name.loc, const_name.value);
                }

                // Add a definition member to set the value of the const.
                csp::Def_mem const_member{
                    csp::Def_mem_type::const_type, const_name.value, int_value, xml_file_location};
                definition.add_member(const_member, false, is_modular);
                return true;
            }
        }
    }
    return false;
}

void Importer::import_consts_(const File_manager& file_manager)
{
    native::Path global_defines_alt_full_path;
    file_manager.get_full_path(native::Path{"GlobalDefinesAlt.xml"}, global_defines_alt_full_path);
    if (global_defines_alt_full_path.empty()) {
        throw Importer_error(std::format(fmt::missing_file, global_defines_alt_full_path));
    }

    native::Path global_defines_full_path;
    file_manager.get_full_path(native::Path{"GlobalDefines.xml"}, global_defines_full_path);
    if (global_defines_alt_full_path.empty()) {
        throw Importer_error(std::format(fmt::missing_file, global_defines_full_path));
    }

    for (const auto& token : m_const_import_table | std::views::values) {
        if (!import_const_(token, global_defines_alt_full_path) && !import_const_(token, global_defines_full_path)) {
            throw make_ex<Importer_error>(fmt::failure_importing_const, token.loc, token.value);
        }
    }

    if (m_use_modular_loading) {
        // Note: For const import, modular loading applies to GlobalDefines.xml but not to GlobalDefinesAlt.xml.
        std::vector<native::Path> global_defines_modular_paths;
        file_manager.get_full_paths_modular(native::Path{"GlobalDefines.xml"}, global_defines_modular_paths);
        for (const auto& full_path : global_defines_modular_paths) {
            for (const auto& token : m_const_import_table | std::views::values) {
                static_cast<void>(import_const_(token, full_path, true));
            }
        }
    }
}

bool Importer::import_enum_(const schema_parser::Token& enum_name,
    const std::string& xml_path,
    const native::Path& file_path,
    bool is_modular /* = false */) const
{
    bpt::ptree tree;
    bpt::read_xml(file_path, tree);

    // When importing an enum, we want the file location to refer to the file path to the XML file.  We'd also like
    // to reference the line and column number; unfortunately these values cannot be obtained using the property tree.
    File_location xml_file_location;
    xml_file_location.filename = std::make_shared<std::string>(file_path);
    xml_file_location.line = std::make_shared<std::string>("");
    xml_file_location.line_number = 0;
    xml_file_location.character_number = 0;

    // Break the xmlPath into parent and node components to facilitate search.  Also change path separator from
    // "/" to "." because BPT uses "."
    const std::string::size_type last_separator{xml_path.find_last_of('/')};
    if (last_separator == std::string::npos) {
        throw make_ex<Importer_error>(fmt::bad_search_path, enum_name.loc, xml_path);
    }
    std::string parent_xml_path{xml_path.substr(0, last_separator)};
    const std::string xml_node{xml_path.substr(last_separator + 1)};
    std::ranges::replace(parent_xml_path, '/', '.');

    // Get the definition for the enum.
    bool was_created{false};
    csp::Definition& definition{m_definition_table->create_definition(
        enum_name.value, csp::Def_type::enum_type, xml_file_location, was_created)};

    // If we're not using modular loading, the definition must not yet exist.  Check wasCreated to verify this.
    if (!is_modular && !was_created) {
        throw make_ex<Importer_error>(fmt::enum_definition_exists, enum_name.loc, enum_name.value);
    }

    // Enumerator values begin at 0 and increment by 1 each time an enumerator is added.
    int enumerator_value{0};

    for (auto& [node_name, child] : tree.get_child(parent_xml_path)) {
        // If the child node matches the search node, get the enumerator name from its <Type> value.
        if (node_name == xml_node) {
            auto enumerator_name{child.get<std::string>("Type")};

            // Add a definition member to set the value of the enumerator.
            csp::Def_mem enum_member{
                csp::Def_mem_type::enum_type, enumerator_name, enumerator_value++, xml_file_location};
            definition.add_member(enum_member, false, is_modular);
        }
    }

    return enumerator_value != 0;
}

void Importer::import_enums_(const File_manager& file_manager)
{
    for (const auto& enum_data : m_enum_import_table | std::views::values) {
        native::Path full_path;
        file_manager.get_full_path(enum_data.search_path, full_path);
        if (full_path.empty()) {
            throw make_ex<Importer_error>(fmt::search_error, enum_data.token.loc, enum_data.search_path);
        }
        if (!import_enum_(enum_data.token, enum_data.xml_path, full_path, false)) {
            throw make_ex<Importer_error>(fmt::failure_importing_enum, enum_data.token.loc, enum_data.token.value);
        }

        if (m_use_modular_loading) {
            std::vector<native::Path> modular_paths;
            file_manager.get_full_paths_modular(enum_data.search_path, modular_paths);
            for (const auto& modular_full_path : modular_paths) {
                static_cast<void>(import_enum_(enum_data.token, enum_data.xml_path, modular_full_path, true));
            }
        }
    }
}

} // namespace c4lib
