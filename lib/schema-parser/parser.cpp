// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/4/2024.

#include <boost/property_tree/ptree_fwd.hpp>
#include <format>
#include <include/exceptions.hpp>
#include <include/logger.hpp>
#include <ios>
#include <iosfwd>
#include <lib/expression-parser/parser.hpp>
#include <lib/io/io.hpp>
#include <lib/logger/log-formats.hpp>
#include <lib/native/path.hpp>
#include <lib/ptree/debug.hpp>
#include <lib/ptree/node-reader.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/parser-phase-one.hpp>
#include <lib/schema-parser/parser-phase-two.hpp>
#include <lib/schema-parser/parser.hpp>
#include <lib/schema-parser/token-type.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/util/constants.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/options.hpp>
#include <lib/util/timer.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace bpt = boost::property_tree;
namespace cpt = c4lib::property_tree;
namespace esp = c4lib::expression_parser;

namespace c4lib::schema_parser {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Parser::Parser()
{
    reset();
}

void Parser::parse(const native::Path& schema,
    const native::Path& install_root,
    const native::Path& custom_assets_path,
    const std::string& mod_name,
    bool use_modular_loading,
    bpt::ptree& ptree_root,
    const native::Path& filename,
    cpt::Node_reader& node_reader,
    std::unordered_map<std::string, std::string>& options)
{
    reset();

    m_schema = schema;
    m_install_root = install_root;
    m_custom_assets_path = custom_assets_path;
    m_mod_name = mod_name;
    m_use_modular_loading = use_modular_loading;
    m_options = &options;
    m_ptree_root = &ptree_root;
    m_node_reader = &node_reader;
    m_node_reader->init(filename, &m_definition_table, options);

    Parser_phase_one p1_parser(m_schema, m_install_root, m_custom_assets_path, m_mod_name, m_use_modular_loading,
        m_tokenizer, m_definition_table, m_root_name_index, m_variable_manager);
    Logger::info(std::format(c4lib::fmt::calling, "Parser_phase_one::parse"));
    Timer timer;
    timer.start();
    ;
    p1_parser.parse();
    Logger::info(std::format(fmt::finished_in, "Parser_phase_one::parse", timer.to_string()));

    if (options[options::debug_write_imports] == "1") {
        const native::Path const_definitions_filename{io::make_path(options[options::debug_output_dir],
            constants::const_definitions_filename, constants::definitions_extension)};
        export_definitions_(Def_type::const_type, const_definitions_filename);

        const native::Path enum_definitions_filename{io::make_path(options[options::debug_output_dir],
            constants::enum_definitions_filename, constants::definitions_extension)};
        export_definitions_(Def_type::enum_type, enum_definitions_filename);
    }

    Parser_phase_two p2_parser(m_tokenizer, m_definition_table, m_root_name_index, m_variable_manager, *m_ptree_root,
        *m_node_reader, *m_options);
    try {
        Logger::info(std::format(c4lib::fmt::calling, "Parser_phase_two::parse"));
        timer.start();
        ;
        p2_parser.parse();
        Logger::info(std::format(fmt::finished_in, "Parser_phase_two::parse", timer.to_string()));
    }
    catch (...) {
        const std::string path{
            io::make_path(options[options::debug_output_dir], filename, constants::crash_dump_extension)};
        cpt::dump_ptree(path, *m_ptree_root);
        throw;
    }
}

void Parser::reset()
{
    m_custom_assets_path.clear();
    m_definition_table.reset();
    m_install_root.clear();
    m_mod_name = "";
    m_ptree_root = nullptr;
    m_node_reader = nullptr;
    m_root_name_index = limits::invalid_size;
    m_schema.clear();
    m_tokenizer.reset();
    m_use_modular_loading = false;
}

bool Parser::parse_expression(esp::Parser& parser, Tokenizer& tokenizer, Variable_manager& variable_manager, int& value)
{
    bool is_success{true};
    try {
        value = parser.parse(tokenizer, variable_manager);
    }
    catch (const Expression_parser_error& ex) {
        Logger::warn(std::format(fmt::caught_expression_parser_error, ex.what()));
        value = limits::invalid_value;
        is_success = false;
    }
    return is_success;
}

void Parser::skip_past_enclosed_tokens(Tokenizer& tokenizer, Token_type punc)
{
    auto pr{get_punc_pair(punc)};

    const Token_type open_punctuation{pr.first};
    const Token_type close_punctuation{pr.second};
    int current_nest{punc == pr.first ? 0 : 1};

    // If current_nest is 0, we're outside the enclosed tokens we wish to skip.  Begin by finding the open token.
    while (current_nest == 0) {
        const Token& t{tokenizer.next()};
        if (t.type == close_punctuation || t.type == Token_type::meta_eos) {
            throw make_ex<Parser_error>(fmt::parser_skip_error, t.loc, to_string(open_punctuation), to_string(t.type));
        }
        else if (t.type == open_punctuation) {
            current_nest = 1;
        }
    }

    while (current_nest > 0) {
        const Token& t{tokenizer.next()};
        if (t.type == close_punctuation) {
            --current_nest;
        }
        else if (t.type == open_punctuation) {
            ++current_nest;
        }
        else if (t.type == Token_type::meta_eos) {
            throw make_ex<Parser_error>(fmt::parser_skip_error, t.loc, to_string(open_punctuation), to_string(t.type));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Parser::export_definitions_(Def_type type, const native::Path& filename) const
{
    std::ofstream out(filename, std::ios_base::out);
    if (!out.is_open() || out.bad()) {
        throw std::runtime_error(std::format(fmt::runtime_error_opening_file, filename));
    }
    m_definition_table.export_definitions(type, out);
}

} // namespace c4lib::schema_parser
