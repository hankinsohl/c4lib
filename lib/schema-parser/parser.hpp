// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/4/2024.

#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <cstddef>
#include <lib/expression-parser/parser.hpp>
#include <lib/native/path.hpp>
#include <lib/ptree/node-reader.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/token-type.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/limits.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <string>
#include <unordered_map>

namespace c4lib::schema_parser {

class Parser {
public:
    Parser();

    ~Parser() = default;

    Parser(const Parser&) = delete;

    Parser& operator=(const Parser&) = delete;

    Parser(Parser&&) noexcept = delete;

    Parser& operator=(Parser&&) noexcept = delete;

    // Parses the token vector, consuming import blocks, constant definitions, enumeration definitions, structure
    // definitions and template definitions.  Consumed tokens are marked as such but are left in the token vector.
    // Creates an entry in the definition table for each constant, enumeration, structure and template.  When an error
    // is detected, throws ParserException.
    //
    // When importing enums and consts, uses install_root, custom_assets_dir and mod_name to locate the XML files
    // for import.
    void parse(const native::Path& schema,
        const native::Path& install_root,
        const native::Path& custom_assets_path,
        const std::string& mod_name,
        bool use_modular_loading,
        boost::property_tree::ptree& ptree_root,
        const native::Path& filename,
        c4lib::property_tree::Node_reader& node_reader,
        std::unordered_map<std::string, std::string>& options);

    static bool parse_expression(
        c4lib::expression_parser::Parser& parser, Tokenizer& tokenizer, Variable_manager& variable_manager, int& value);

    // Resets the parser returning it to the state of a newly created parser.
    void reset();

    static void skip_past_enclosed_tokens(Tokenizer& tokenizer, Token_type punc);

private:
    void export_definitions_(Def_type type, const native::Path& filename) const;

    native::Path m_custom_assets_path;
    Def_tbl m_definition_table;
    native::Path m_install_root;
    std::string m_mod_name;
    c4lib::property_tree::Node_reader* m_node_reader{nullptr};
    std::unordered_map<std::string, std::string>* m_options{nullptr};
    boost::property_tree::ptree* m_ptree_root{nullptr};
    size_t m_root_name_index{limits::invalid_size};
    native::Path m_schema;
    Tokenizer m_tokenizer;
    bool m_use_modular_loading{false};
    Variable_manager m_variable_manager;
};

} // namespace c4lib::schema_parser
