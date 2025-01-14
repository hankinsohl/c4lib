// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/31/2024.

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <format>
#include <include/exceptions.hpp>
#include <lib/native/path.hpp>
#include <lib/schema-parser/def-mem-type.hpp>
#include <lib/schema-parser/def-mem.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/definition.hpp>
#include <lib/schema-parser/parser-phase-one.hpp>
#include <lib/schema-parser/parser.hpp>
#include <lib/schema-parser/token-type.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/constants.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/text.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace c4lib::schema_parser {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Parser_phase_one::Parser_phase_one(native::Path schema,
    native::Path install_root,
    native::Path custom_assets_path,
    std::string mod_name,
    bool use_modular_loading,
    Tokenizer& tokenizer,
    Def_tbl& definition_table,
    size_t& root_name_index,
    Variable_manager& variable_manager)
    : m_custom_assets_path(std::move(custom_assets_path)),
      m_definition_table(definition_table),
      m_install_root(std::move(install_root)),
      m_mod_name(std::move(mod_name)),
      m_root_name_index(root_name_index),
      m_schema(std::move(schema)),
      m_tokenizer(tokenizer),
      m_use_modular_loading(use_modular_loading),
      m_variable_manager(variable_manager)
{}

void Parser_phase_one::parse()
{
    // Clear state of objects populated during phase one parsing.
    m_enum_name_token = nullptr;
    m_definition_table.reset();
    m_importer.reset();
    m_tokenizer.reset();
    m_tokenizer.run(m_schema);

    if (!pr_schema_()) {
        const Token& token{m_tokenizer.peek()};
        throw make_ex<Parser_error>(fmt::syntax_error, token.loc, to_string(token.type));
    }

    m_importer.import_definitions(
        m_definition_table, m_install_root, m_custom_assets_path, m_mod_name, m_use_modular_loading);
    tidy_definitions_();

    // Check that vector resizing is minimized
    assert(tune::schema_token_vector_reserve_size >= m_tokenizer.get_tokens().size());
    assert(tune::definition_reserve_size >= m_definition_table.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Parser_phase_one::add_alias_definition_(const Token& template_token, const Token& alias_token) const
{
    bool was_created{false};
    Definition& alias_definition{
        m_definition_table.create_definition(alias_token.value, Def_type::alias_type, alias_token.loc, was_created)};
    if (!was_created) {
        // If an alias_definition exists for the token name, it's an error.
        throw make_ex<Parser_error>(fmt::duplicated_name, alias_token.loc, alias_token.value);
    }

    // Get the corresponding template definition's first alias_member and the corresponding template index.
    // We store the template index in the alias definition to facilitate lookup of the template.
    const Def_mem& template_member{m_definition_table.get_first_member(template_token.value, Def_type::template_type)};
    const int template_index{template_member.value};
    Def_mem alias_member{Def_mem_type::alias_type, template_token.value, template_index, template_token.loc};
    alias_definition.add_member(alias_member, false, false);
}

void Parser_phase_one::add_const_definition_(const Token& token, int value) const
{
    // Note: was_created is unused.  If the definition already exists, the appropriate exception will be thrown when
    // we call add_member below.
    bool was_created{false};
    Definition& definition{
        m_definition_table.create_definition(token.value, Def_type::const_type, token.loc, was_created)};
    Def_mem member{Def_mem_type::const_type, token.value, value, token.loc};
    definition.add_member(member, false, false);
}

void Parser_phase_one::add_enumerator_definition_(const Token& token) const
{
    bool was_created{false};
    Definition& definition{
        m_definition_table.create_definition(m_enum_name_token->value, Def_type::enum_type, token.loc, was_created)};
    int value{0};
    if (!was_created) {
        // The value of the enumerator is one more than the value of the enumerator last added.
        const std::vector<Def_mem>& members{definition.get_members()};
        const size_t index{members.size() - 1};
        value = members.at(index).value + 1;
    }
    Def_mem member(Def_mem_type::enum_type, token.value, value, token.loc);
    definition.add_member(member, false, false);
}

void Parser_phase_one::add_enumerator_definition_(const Token& token, int value) const
{
    // Note: was_created is unused.  If the definition already exists, the appropriate exception will be thrown when
    // we call add_member below.
    bool was_created{false};
    Definition& definition{
        m_definition_table.create_definition(m_enum_name_token->value, Def_type::enum_type, token.loc, was_created)};
    Def_mem member{Def_mem_type::enum_type, token.value, value, token.loc};
    definition.add_member(member, false, false);
}

void Parser_phase_one::add_minus_one_enumerators_() const
{
    for (auto& def : m_definition_table.get_definitions() | std::views::values) {
        if (def.get_type() == Def_type::enum_type) {
            bool has_minus_one_enumerator{false};
            for (const auto& enumerator_def : def.get_members()) {
                if (enumerator_def.value == -1) {
                    has_minus_one_enumerator = true;
                    break;
                }
            }
            if (!has_minus_one_enumerator) {
                std::string enum_suffix{def.get_name()};
                const std::string::size_type pos_types{enum_suffix.find("Types")};
                enum_suffix = enum_suffix.substr(0, pos_types);
                std::ranges::transform(enum_suffix, enum_suffix.begin(),
                    [](const unsigned char c) { return gsl::narrow<char>(std::toupper(c)); });
                const std::string name{"NO_" + enum_suffix};
                const File_location loc;
                Def_mem member{Def_mem_type::enum_type, name, -1, loc};
                def.add_member(member, false, false);
            }
        }
    }
}

void Parser_phase_one::add_struct_definition_(const Token& token) const
{
    bool was_created{false};
    Definition& definition{
        m_definition_table.create_definition(token.value, Def_type::struct_type, token.loc, was_created)};
    if (!was_created) {
        // If a definition exists for the token name, it's an error.
        throw make_ex<Parser_error>(fmt::duplicated_name, token.loc, token.value);
    }
    // token.index corresponds to the name of the struct.  The definition begins one token later,
    // at the open-brace.  Add one to token.index to reflect this.
    Def_mem member{Def_mem_type::struct_type, constants::index_member, gsl::narrow<int>(token.index + 1), token.loc};
    definition.add_member(member, false, false);
}

void Parser_phase_one::add_template_definition_(const Token& token) const
{
    bool was_created{false};
    Definition& definition{
        m_definition_table.create_definition(token.value, Def_type::template_type, token.loc, was_created)};
    if (!was_created) {
        // If a definition exists for the token name, it's an error.
        throw make_ex<Parser_error>(fmt::duplicated_name, token.loc, token.value);
    }
    // token.index corresponds to the name of the template.  Add one to token.index
    // to skip past the name and point at the bracketed typename, <T>.
    Def_mem member{Def_mem_type::template_type, constants::index_member, gsl::narrow<int>(token.index + 1), token.loc};
    definition.add_member(member, false, false);
}

void Parser_phase_one::generate_enum_num_constants_() const
{
    for (auto& def : m_definition_table.get_definitions() | std::views::values) {
        if (def.get_type() == Def_type::enum_type) {
            const std::vector<Def_mem>& members{def.get_members()};

            // We'll generate a NUM_ constant for the enum if its first member has value -1
            // and each successive member has a value one higher than the previous member.
            bool generate_enum_num_constant{true};
            int requiredValue{-1};
            for (const auto& member : members) {
                if (member.value != requiredValue++) {
                    generate_enum_num_constant = false;
                    break;
                }
            }

            if (generate_enum_num_constant) {
                const std::string constant_name{"NUM_" + text::screaming_snake_case(def.get_name())};
                if (members.empty()) {
                    throw std::out_of_range{
                        std::format(fmt::out_of_range_error, "constant_value", "generate_enum_num_constants_")};
                }
                const int constant_value{gsl::narrow<int>(members.size()) - 1};
                File_location loc;
                loc.filename = std::make_shared<std::string>("auto-generated");
                // Note: was_created is unused.  If the constant somehow exists already, addMember will throw.
                bool was_created{false};
                Definition& definition{
                    m_definition_table.create_definition(constant_name, Def_type::const_type, loc, was_created)};
                Def_mem member{Def_mem_type::const_type, constant_name, constant_value, loc};
                definition.add_member(member, false, false);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION - PRODUCTION RULES
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Note: Production rules below are written in the form
//
//    bool is_success{pr_production_rule_()};
//    return is_success;
//
// This format allows breakpoints to be set following invocation of the production rule.
// Using the concise format
//
//    return pr_production_rule_();
//
// unfortunately makes it impossible to break right after the production rule
// returns.

// <alias-definition-statement> ::= <template-name> <alias-keyword> <alias-name>
bool Parser_phase_one::pr_alias_definition_statement_() const
{
    bool is_success{pr_template_name_()};
    if (!is_success) {
        return false;
    }
    const Token& template_name{m_tokenizer.previous()};

    is_success = pr_alias_keyword_();
    if (!is_success) {
        return false;
    }

    is_success = pr_alias_name_();
    if (!is_success) {
        return false;
    }
    const Token& alias_name{m_tokenizer.previous()};

    add_alias_definition_(template_name, alias_name);
    return true;
}

// <alias-keyword> ::= alias
bool Parser_phase_one::pr_alias_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::alias_keyword};
    return is_success;
}

// <alias-name> ::= <identifier>
bool Parser_phase_one::pr_alias_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <assignment-operator> ::= =
bool Parser_phase_one::pr_assignment_operator_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::equals};
    return is_success;
}

//<close-brace> ::= }
bool Parser_phase_one::pr_close_brace_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::close_brace};
    return is_success;
}

// <const-definition-statement> ::= <const-keyword> <const-name> <assignment-operator> <constant-expression>
bool Parser_phase_one::pr_const_definition_statement_()
{
    bool is_success{pr_const_keyword_()};
    if (!is_success) {
        return false;
    }

    is_success = pr_const_name_();
    if (!is_success) {
        return false;
    }
    const Token& const_name{m_tokenizer.previous()};

    is_success = pr_assignment_operator_();
    if (!is_success) {
        return false;
    }

    int value{limits::invalid_value};
    is_success = pr_constant_expression_(value);
    if (!is_success) {
        return false;
    }

    add_const_definition_(const_name, value);
    return true;
}

// <const-import-statement> ::= <const-keyword> <identifier>
bool Parser_phase_one::pr_const_import_statement_()
{
    if (const bool is_success{pr_const_keyword_() && pr_identifier_()}; !is_success) {
        return false;
    }

    const Token& token{m_tokenizer.previous()};
    m_importer.add_const(token);

    return true;
}

// <const-keyword> ::= const
bool Parser_phase_one::pr_const_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::const_keyword};
    return is_success;
}

// <const-name> ::= <identifier>
bool Parser_phase_one::pr_const_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <constant-expression> ::= <expression>
bool Parser_phase_one::pr_constant_expression_(int& value)
{
    const bool is_success{pr_expression_(value)};
    return is_success;
}

// <definition> ::= <alias-definition-statement> | <const-definition-statement> | <enum-definition-block> |
//     <struct-definition-block> | <template-definition-block>
bool Parser_phase_one::pr_definition_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_alias_definition_statement_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_const_definition_statement_();
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_enum_definition_block_();
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_struct_definition_block_();
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_template_definition_block_();

    return is_success;
}

// <definitions> ::= <definition> <opt-definitions>
bool Parser_phase_one::pr_definitions_()
{
    const bool is_success{pr_definition_() && pr_opt_definitions_()};
    return is_success;
}

// <enum-definition> ::= <open-brace> <enumerator-definition-statements> <close-brace>
bool Parser_phase_one::pr_enum_definition_()
{
    const bool is_success{pr_open_brace_() && pr_enumerator_definition_statements_() && pr_close_brace_()};
    return is_success;
}

// <enum-definition-block> ::= <enum-keyword> <enum-name> <enum-definition>
bool Parser_phase_one::pr_enum_definition_block_()
{
    bool is_success{pr_enum_keyword_()};
    if (!is_success) {
        return false;
    }

    is_success = pr_enum_name_();
    if (!is_success) {
        return false;
    }
    // Store the name of the enum.  The name is used during the definition of the enum, inclusive of its
    // enumerator constants which are parsed separately below.
    m_enum_name_token = &m_tokenizer.previous();

    is_success = pr_enum_definition_();
    return is_success;
}

// <enum-import-statement> ::= <enum-keyword> <identifier> <xml-path-keyword> <xml-path>
//     <exact-or-search-path-keyword> <file-path>
bool Parser_phase_one::pr_enum_import_statement_()
{
    bool is_success{pr_enum_keyword_() && pr_identifier_()};
    if (!is_success) {
        return false;
    }
    const Token& enum_name{m_tokenizer.previous()};

    is_success = pr_xml_path_keyword_() && pr_xml_path_();
    if (!is_success) {
        return false;
    }
    const Token& xml_path{m_tokenizer.previous()};

    is_success = pr_exact_or_search_path_keyword_() && pr_file_path_();
    if (!is_success) {
        return false;
    }
    const Token& file_path{m_tokenizer.previous()};

    m_importer.add_enum(enum_name, xml_path, file_path);

    return true;
}

// <enum-keyword> ::= enum
bool Parser_phase_one::pr_enum_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::enum_keyword};
    return is_success;
}

// <enum-name> ::= <identifier>
bool Parser_phase_one::pr_enum_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <enumerator> ::= <identifier>
bool Parser_phase_one::pr_enumerator_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <enumerator-definition-statement> ::= <enumerator> <assignment-operator> <constant-expression> |
//     <enumerator>
bool Parser_phase_one::pr_enumerator_definition_statement_()
{
    bool is_success{pr_enumerator_()};
    if (!is_success) {
        return false;
    }
    const Token& enumerator_name_token{m_tokenizer.previous()};

    // Peek at the current token.  If it's the assignment operator we proceed with the production
    //     <assignment-operator> <constant-expression>
    // which must succeed, otherwise there's a syntax error and we return false.  If the current token isn't
    // an assignment operator, we have the alternative production
    //     <enumerator>
    // which we've already successfully parsed.
    const Token& next_token{m_tokenizer.peek()};
    if (next_token.type == Token_type::equals) {
        // <assignment-operator> <constant-expression>
        int value{limits::invalid_value};
        is_success = pr_assignment_operator_() && pr_constant_expression_(value);
        if (!is_success) {
            return false;
        }
        // Add the enumerator definition along with its value
        add_enumerator_definition_(enumerator_name_token, value);
    }
    else {
        // Add the enumerator definition with no value specified.
        add_enumerator_definition_(enumerator_name_token);
    }

    return true;
}

// <enumerator-definition-statements> ::= <enumerator-definition-statement> <opt-enumerator-definition-statements>
bool Parser_phase_one::pr_enumerator_definition_statements_()
{
    const bool is_success{pr_enumerator_definition_statement_() && pr_opt_enumerator_definition_statements_()};
    return is_success;
}

// <exact-or-search-path-keyword> ::= <exact-path-keyword> | <search-path-keyword>
bool Parser_phase_one::pr_exact_or_search_path_keyword_() const
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_exact_path_keyword_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_search_path_keyword_();

    return is_success;
}

// <exact-path-keyword> ::= exact_path
bool Parser_phase_one::pr_exact_path_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::exact_path_keyword};
    return is_success;
}

// No production rule exists for expression.  Instead, expressions are parsed using the
// Pratt Parsing method within the expression parser class.
bool Parser_phase_one::pr_expression_(int& value)
{
    const bool is_success{Parser::parse_expression(m_expression_parser, m_tokenizer, m_variable_manager, value)};
    return is_success;
}

// <file-path> ::= <string-literal>
bool Parser_phase_one::pr_file_path_() const
{
    const bool is_success{pr_string_literal_()};
    return is_success;
}

// <identifier> ::= [a-zA-Z][_a-zA-Z0-9]{0,30}
bool Parser_phase_one::pr_identifier_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::identifier};
    return is_success;
}

// <import-block> ::= <import-keyword> <open-brace> <opt-import-statements> <close-brace>
bool Parser_phase_one::pr_import_block_()
{
    const bool is_success{pr_import_keyword_() && pr_open_brace_() && pr_opt_import_statements_() && pr_close_brace_()};
    return is_success;
}

// <import-keyword> ::= import
bool Parser_phase_one::pr_import_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::import_keyword};
    return is_success;
}

// <import-statement> ::= <enum-import-statement> | <const-import-statement>
bool Parser_phase_one::pr_import_statement_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_enum_import_statement_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_const_import_statement_();

    return is_success;
}

// <null> ::= NULL
bool Parser_phase_one::pr_null_()
{
    // NULL always succeeds and does not advance the token stream
    return true;
}

// <open-brace> ::= {
bool Parser_phase_one::pr_open_brace_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::open_brace};
    return is_success;
}

// <opt-definitions> ::= <definition> <opt-definitions> | <null>
bool Parser_phase_one::pr_opt_definitions_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_definition_() && pr_opt_definitions_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_null_();

    return is_success;
}

// <opt-enumerator-definition-statements> ::= <enumerator-definition-statement>
//     <opt-enumerator-definition-statements> | <null>
bool Parser_phase_one::pr_opt_enumerator_definition_statements_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_enumerator_definition_statement_() && pr_opt_enumerator_definition_statements_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_null_();

    return is_success;
}

// <opt-import-blocks> ::= <import-block> <opt-import-blocks> | <null>
bool Parser_phase_one::pr_opt_import_blocks_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_import_block_() && pr_opt_import_blocks_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_null_();

    return is_success;
}

// <opt-import-statements> ::= <import-statement> <opt-import-statements> | <null>
bool Parser_phase_one::pr_opt_import_statements_()
{
    const size_t index{m_tokenizer.get_index()};

    bool is_success{pr_import_statement_() && pr_opt_import_statements_()};
    if (is_success) {
        return true;
    }

    m_tokenizer.set_index(index);
    is_success = pr_null_();

    return is_success;
}

// <read-keyword> ::= read
bool Parser_phase_one::pr_read_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::read_keyword};
    return is_success;
}

// <read-statement> ::= <read-keyword> <struct-name>
bool Parser_phase_one::pr_read_statement_() const
{
    const bool is_success{pr_read_keyword_() && pr_struct_name_()};
    if (is_success) {
        m_root_name_index = m_tokenizer.previous().index;
    }
    return is_success;
}

// <schema> ::= <opt-import-blocks> <definitions> <read-statement>
bool Parser_phase_one::pr_schema_()
{
    const bool is_success{pr_opt_import_blocks_() && pr_definitions_() && pr_read_statement_()};
    return is_success;
}

// <search-path-keyword> ::= search_path
bool Parser_phase_one::pr_search_path_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::search_path_keyword};
    return is_success;
}

// <string-literal> ::= "([^"]){0,118}"
bool Parser_phase_one::pr_string_literal_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::string_literal};
    return is_success;
}

// <struct-definition-block> ::= <struct-keyword> <struct-name> <struct-definition>
bool Parser_phase_one::pr_struct_definition_block_() const
{
    bool is_success{pr_struct_keyword_()};
    if (!is_success) {
        return false;
    }

    is_success = pr_struct_name_();
    if (!is_success) {
        return false;
    }

    const Token& structNameToken{m_tokenizer.previous()};
    add_struct_definition_(structNameToken);

    // In phase one, struct and template definitions are not processed.
    Parser::skip_past_enclosed_tokens(m_tokenizer, Token_type::open_brace);
    return is_success;
}

// <struct-keyword> ::= struct
bool Parser_phase_one::pr_struct_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::struct_keyword};
    return is_success;
}

// <struct-name> ::= <identifier>
bool Parser_phase_one::pr_struct_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <template-definition-block> ::= <template-keyword> <template-name> <bracketed-typename> <template-definition>
bool Parser_phase_one::pr_template_definition_block_() const
{
    bool is_success{pr_template_keyword_()};
    if (!is_success) {
        return false;
    }

    is_success = pr_template_name_();
    if (!is_success) {
        return false;
    }

    const Token& template_name_token{m_tokenizer.previous()};
    add_template_definition_(template_name_token);

    // In phase one, struct and template definitions are not processed.
    Parser::skip_past_enclosed_tokens(m_tokenizer, Token_type::open_angle_bracket);
    Parser::skip_past_enclosed_tokens(m_tokenizer, Token_type::open_brace);
    return is_success;
}

// <template-keyword> ::= template
bool Parser_phase_one::pr_template_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::template_keyword};
    return is_success;
}

// <template-name> ::= <identifier>
bool Parser_phase_one::pr_template_name_() const
{
    const bool is_success{pr_identifier_()};
    return is_success;
}

// <xml-path> ::= <string-literal>
bool Parser_phase_one::pr_xml_path_() const
{
    const bool is_success{pr_string_literal_()};
    return is_success;
}

// <xml-path-keyword> ::= xml_path
bool Parser_phase_one::pr_xml_path_keyword_() const
{
    const Token& token{m_tokenizer.next()};
    const bool is_success{token.type == Token_type::xml_path_keyword};
    return is_success;
}

void Parser_phase_one::sort_enumerators_() const
{
    for (auto& def : m_definition_table.get_definitions() | std::views::values) {
        if (def.get_type() == Def_type::enum_type) {
            std::vector<Def_mem>& members{def.get_members()};
            std::sort(members.begin(), members.end());
        }
    }
}

void Parser_phase_one::tidy_definitions_() const
{
    tidy_enum_definitions_();
}

void Parser_phase_one::tidy_enum_definitions_() const
{
    // Add NO_XXX = -1 enumerators to enums that lack a -1 value.
    add_minus_one_enumerators_();

    // Sort enumerators by value, in ascending order.
    sort_enumerators_();

    // Add NUM_ constants for enums where a zero value exists
    // and a contiguous run of values is present up to the highest
    // value.  Such enums are likely to be used as array indices with
    // the array dimension equal to the size of the enum.
    generate_enum_num_constants_();
}

} // namespace c4lib::schema_parser
