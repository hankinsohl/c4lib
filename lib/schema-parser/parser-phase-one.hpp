// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/31/2024.

#pragma once

#include <cstddef>
#include <lib/expression-parser/parser.hpp>
#include <lib/importer/importer.hpp>
#include <lib/native/path.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <string>

namespace c4lib::schema_parser {

class Parser_phase_one {
public:
    Parser_phase_one(native::Path schema,
        native::Path install_root,
        native::Path custom_assets_path,
        std::string mod_name,
        bool use_modular_loading,
        Tokenizer& tokenizer,
        Def_tbl& definition_table,
        size_t& root_name_index,
        Variable_manager& variable_manager);

    ~Parser_phase_one() = default;

    Parser_phase_one(const Parser_phase_one&) = delete;

    Parser_phase_one& operator=(const Parser_phase_one&) = delete;

    Parser_phase_one(Parser_phase_one&&) noexcept = delete;

    Parser_phase_one& operator=(Parser_phase_one&&) noexcept = delete;

    // The phase one parser does the following:
    //      * Builds the definition tables
    //      * Imports enums and consts
    //      * Adds enums and consts from the schema to the definition tables
    //      * Adds token stream location entries to the definition tables for structs and templates
    //      * When the read statement is parsed, stores the index of the root structure in rootNameIndex.
    void parse();

private:
    void add_alias_definition_(const Token& template_token, const Token& alias_token) const;

    void add_const_definition_(const Token& token, int value) const;

    void add_enumerator_definition_(const Token& token) const;

    void add_enumerator_definition_(const Token& token, int value) const;

    void add_minus_one_enumerators_() const;

    void add_struct_definition_(const Token& token) const;

    void add_template_definition_(const Token& token) const;

    void generate_enum_num_constants_() const;

    bool pr_alias_definition_statement_() const;

    bool pr_alias_keyword_() const;

    bool pr_alias_name_() const;

    bool pr_assignment_operator_() const;

    bool pr_close_brace_() const;

    bool pr_const_definition_statement_();

    bool pr_const_import_statement_();

    bool pr_const_keyword_() const;

    bool pr_const_name_() const;

    bool pr_constant_expression_(int& value);

    bool pr_definition_();

    bool pr_definitions_();

    bool pr_enum_definition_();

    bool pr_enum_definition_block_();

    bool pr_enum_import_statement_();

    bool pr_enum_keyword_() const;

    bool pr_enum_name_() const;

    bool pr_enumerator_() const;

    bool pr_enumerator_definition_statement_();

    bool pr_enumerator_definition_statements_();

    bool pr_exact_or_search_path_keyword_() const;

    bool pr_exact_path_keyword_() const;

    bool pr_expression_(int& value);

    bool pr_file_path_() const;

    bool pr_identifier_() const;

    bool pr_import_block_();

    bool pr_import_keyword_() const;

    bool pr_import_statement_();

    static bool pr_null_();

    bool pr_open_brace_() const;

    bool pr_opt_definitions_();

    bool pr_opt_enumerator_definition_statements_();

    bool pr_opt_import_blocks_();

    bool pr_opt_import_statements_();

    bool pr_read_keyword_() const;

    bool pr_read_statement_() const;

    bool pr_schema_();

    bool pr_search_path_keyword_() const;

    bool pr_string_literal_() const;

    bool pr_struct_definition_block_() const;

    bool pr_struct_keyword_() const;

    bool pr_struct_name_() const;

    bool pr_template_definition_block_() const;

    bool pr_template_keyword_() const;

    bool pr_template_name_() const;

    bool pr_xml_path_() const;

    bool pr_xml_path_keyword_() const;

    void sort_enumerators_() const;

    void tidy_definitions_() const;

    void tidy_enum_definitions_() const;

    const native::Path m_custom_assets_path;
    Def_tbl& m_definition_table;
    const Token* m_enum_name_token{nullptr};
    expression_parser::Parser m_expression_parser;
    Importer m_importer;
    const native::Path m_install_root;
    const std::string m_mod_name;
    [[maybe_unused]] size_t& m_root_name_index;
    const native::Path m_schema;
    Tokenizer& m_tokenizer;
    bool m_use_modular_loading{false};
    Variable_manager& m_variable_manager;
};

} // namespace c4lib::schema_parser
