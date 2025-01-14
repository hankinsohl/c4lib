// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/18/2024.

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <functional>
#include <include/exceptions.hpp>
#include <lib/expression-parser/infix-representation.hpp>
#include <lib/expression-parser/parser.hpp>
#include <lib/schema-parser/token-type.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <stdexcept>
#include <string>

namespace csp = c4lib::schema_parser;

namespace c4lib::expression_parser {

// Token information table.  Assigns a left-binding-power, right-binding-power, null denotation and
// left denotation to each expression token.
//
// Note: The order of entries in this table must match the order of enumerator definition in the
// TokenTypes enumeration because the enumerator value is used as an index into this table.
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
std::array<Parser::Token_info, 24> Parser::token_info_table{{
    {.type = csp::Token_type::invalid, .lbp = 0, .rbp = 0, .nud = nullptr, .led = nullptr},

    // Numeric literal
    {.type = csp::Token_type::numeric_literal, .lbp = 0, .rbp = 0, .nud = &Parser::nud_number_, .led = nullptr},

    {.type = csp::Token_type::equals, .lbp = 0, .rbp = 0, .nud = nullptr, .led = nullptr},

    // Arithmetic operators
    {.type = csp::Token_type::minus,
        .lbp = 70,
        .rbp = 70,
        .nud = &Parser::nud_unary_op_,
        .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::plus,
        .lbp = 70,
        .rbp = 70,
        .nud = &Parser::nud_unary_op_,
        .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::asterisk, .lbp = 80, .rbp = 80, .nud = nullptr, .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::slash, .lbp = 80, .rbp = 80, .nud = nullptr, .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::percent, .lbp = 80, .rbp = 80, .nud = nullptr, .led = &Parser::led_binary_op_},

    // Logical operators
    {.type = csp::Token_type::double_ampersand, .lbp = 40, .rbp = 40, .nud = nullptr, .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::double_bar, .lbp = 30, .rbp = 30, .nud = nullptr, .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::bang, .lbp = 0, .rbp = 0, .nud = &Parser::nud_unary_op_, .led = nullptr},

    // Comparison operators
    {.type = csp::Token_type::open_angle_bracket, .lbp = 60, .rbp = 60, .nud = nullptr, .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::open_angle_equals, .lbp = 60, .rbp = 60, .nud = nullptr, .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::double_equals, .lbp = 50, .rbp = 50, .nud = nullptr, .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::bang_equals, .lbp = 50, .rbp = 50, .nud = nullptr, .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::close_angle_equals, .lbp = 60, .rbp = 60, .nud = nullptr, .led = &Parser::led_binary_op_},
    {.type = csp::Token_type::close_angle_bracket,
        .lbp = 60,
        .rbp = 60,
        .nud = nullptr,
        .led = &Parser::led_binary_op_},

    // Miscellaneous operators
    {.type = csp::Token_type::double_colon, .lbp = 0, .rbp = 0, .nud = nullptr, .led = nullptr},

    // Function call/grouping
    {.type = csp::Token_type::open_parenthesis, .lbp = 0, .rbp = 0, .nud = &Parser::nud_grouping_, .led = nullptr},
    {.type = csp::Token_type::close_parenthesis, .lbp = 0, .rbp = 0, .nud = nullptr, .led = nullptr},

    // Identifier-related token types
    {.type = csp::Token_type::function_name, .lbp = 0, .rbp = 0, .nud = nullptr, .led = nullptr},
    {.type = csp::Token_type::identifier, .lbp = 0, .rbp = 0, .nud = &Parser::nud_var_or_ref_, .led = nullptr},
    {.type = csp::Token_type::dot, .lbp = 0, .rbp = 0, .nud = nullptr, .led = nullptr},

    // Meta token denoting end-of-stream
    {.type = csp::Token_type::meta_expression_eos, .lbp = -1, .rbp = -1, .nud = nullptr, .led = nullptr},
}};

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Parser::parse(
    csp::Tokenizer& tokenizer, c4lib::Variable_manager& variable_manager, Infix_representation* infix_representation)
{
    while (!m_stack.empty()) {
        m_stack.pop();
    }
    m_tokenizer = &tokenizer;
    m_variable_manager = &variable_manager;
    m_infix_representation = infix_representation;
    expr_(0);
    const int value{pop_()};
    return value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Parser::expect_(csp::Token_type token_type) const
{
    if (const csp::Token & current{m_tokenizer->next()}; current.type != token_type) {
        throw make_ex<Expression_parser_error>(
            fmt::unexpected_token_type, current.loc, to_string(current.type), to_string(token_type));
    }
}

void Parser::expr_(int rbp)
{
    nud_();
    while (rbp < get_token_info_(m_tokenizer->peek()).lbp) {
        led_();
    }
}

Parser::Token_info Parser::get_token_info_(const csp::Token& token)
{
    size_t token_type{static_cast<size_t>(token.type)};
    token_type = std::min(token_type, static_cast<size_t>(csp::Token_type::meta_expression_eos));
    return token_info_table.at(token_type);
}

void Parser::led_()
{
    const csp::Token& token{m_tokenizer->next()};
    const Token_info ti = get_token_info_(token); // = used for initialization to avoid spurious warning
    if (ti.led == nullptr) {
        throw make_ex<Expression_parser_error>(fmt::no_led, token.loc, to_string(token.type));
    }
    std::invoke(ti.led, this);
}

void Parser::led_binary_op_()
{
    const csp::Token& token{m_tokenizer->previous()};
    const Token_info ti = get_token_info_(token); // = used for initialization to avoid spurious warning
    expr_(ti.rbp); // Note: rbp passed to expr to accommodate right-associative operators
    const int right{pop_()};
    const int left{pop_()};
    int value{limits::invalid_value};
    switch (token.type) {
    case csp::Token_type::minus:
        value = (left - right);
        break;

    case csp::Token_type::plus:
        value = (left + right);
        break;

    case csp::Token_type::asterisk:
        value = (left * right);
        break;

    case csp::Token_type::slash:
        value = (left / right);
        break;

    case csp::Token_type::percent:
        value = (left % right);
        break;

        // NOLINTBEGIN(readability-implicit-bool-conversion)
    case csp::Token_type::double_ampersand:
        value = (left && right);
        break;

    case csp::Token_type::double_bar:
        value = (left || right);
        break;

    case csp::Token_type::open_angle_bracket:
        value = (left < right);
        break;

    case csp::Token_type::open_angle_equals:
        value = (left <= right);
        break;

    case csp::Token_type::double_equals:
        value = (left == right);
        break;

    case csp::Token_type::bang_equals:
        value = (left != right);
        break;

    case csp::Token_type::close_angle_equals:
        value = (left >= right);
        break;

    case csp::Token_type::close_angle_bracket:
        value = (left > right);
        break;
        // NOLINTEND(readability-implicit-bool-conversion)
    default:
        throw std::logic_error{std::format(fmt::internal_bug_in_function, "Parser::led_binary_op_")};
    }
    push_(value);
    if (m_infix_representation != nullptr) {
        const std::string r{m_infix_representation->pop()};
        const std::string l{m_infix_representation->pop()};
        const std::string e{"(" + l + " " + token.value + " " + r + ")"};
        m_infix_representation->push(e);
    }
}

void Parser::nud_()
{
    const csp::Token& token{m_tokenizer->next()};
    const Token_info ti = get_token_info_(token); // = used for initialization to avoid spurious warning
    if (ti.nud == nullptr) {
        throw make_ex<Expression_parser_error>(fmt::no_nud, token.loc, to_string(token.type));
    }
    std::invoke(ti.nud, this);
}

void Parser::nud_grouping_()
{
    expr_(0);
    expect_(csp::Token_type::close_parenthesis);
}

void Parser::nud_number_()
{
    const csp::Token& token{m_tokenizer->previous()};
    const int value{std::stoi(token.value, nullptr, 0)};
    push_(value);
    if (m_infix_representation != nullptr) {
        m_infix_representation->push(token.value);
    }
}

void Parser::nud_unary_op_()
{
    const csp::Token& token{m_tokenizer->previous()};
    const Token_info ti = get_token_info_(token); // = used for initialization to avoid spurious warning
    expr_(ti.rbp);
    const int right{pop_()};
    int value{limits::invalid_value};
    switch (token.type) {
    case csp::Token_type::minus:
        value = (-right);
        break;

    case csp::Token_type::plus:
        value = (+right);
        break;

    case csp::Token_type::bang:
        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        value = (!right);
        break;

    default:
        throw std::logic_error(std::format(fmt::internal_bug_in_function, "Parser::nud_unary_op_"));
    }
    push_(value);
    if (m_infix_representation != nullptr) {
        const std::string r{m_infix_representation->pop()};
        const std::string e{"(" + token.value + r + ")"};
        m_infix_representation->push(e);
    }
}

void Parser::nud_var_or_ref_()
{
    // nud_var_or_ref_ is the null derivation for variables, node references and enumerator references.  Parsing
    // for variables is similar to that for numeric literals except that the node value is obtained from the
    // variable manager.  Parsing for node references and enumerator references requires that a sequence of tokens
    // be processed and for that reason involves the use of several production rules.
    // Variables, node references and enumerator references all begin with an identifier token.  To distinguish
    // between them, we first peek at the current token.  If it's an open square bracket or a dot, we need to process
    // a node reference; if it's a scope resolution operator,  we need to process an enumerator reference; otherwise
    // we process a variable.  Note that a simple node reference without a path will be interpreted as a variable.
    // This is OK because the variable manager handles such a case.  The main point of the algorithm below is to
    // process the correct number of tokens associated with the variable or reference and then piece the tokens
    // together into a string which is passed to the variable manager for resolution.
    if (const csp::Token & cur_tok{m_tokenizer->peek()};
        cur_tok.type == csp::Token_type::open_square_bracket || cur_tok.type == csp::Token_type::dot) {
        std::string path;
        // We've already consumed the identifier token which the node reference production starts with.
        // Back up one token to sync with the production rule.
        m_tokenizer->back();
        const bool is_success{pr_node_reference_(path)};
        if (!is_success) {
            throw make_ex<Expression_parser_error>(fmt::bad_node_reference, cur_tok.loc);
        }
        const int value{m_variable_manager->get(path)};
        push_(value);
        if (m_infix_representation != nullptr) {
            m_infix_representation->push(path);
        }
    }
    else if (cur_tok.type == csp::Token_type::double_colon) {
        std::string enumerator_reference;
        // We've already consumed the identifier token which the enumerator reference production starts with.
        // Back up one token to sync with the production rule.
        m_tokenizer->back();
        const bool is_success{pr_enumerator_reference_(enumerator_reference)};
        if (!is_success) {
            throw make_ex<Expression_parser_error>(fmt::bad_enumerator_reference, cur_tok.loc);
        }
        const int value{m_variable_manager->get(enumerator_reference)};
        push_(value);
        if (m_infix_representation != nullptr) {
            m_infix_representation->push(enumerator_reference);
        }
    }
    else {
        const csp::Token& prev_tok{m_tokenizer->previous()};
        const int value{m_variable_manager->get(prev_tok.value)};
        push_(value);
        if (m_infix_representation != nullptr) {
            m_infix_representation->push(prev_tok.value);
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

// <array-node-name> ::= <open-square-bracket> <expression> <close-square-bracket>
bool Parser::pr_array_node_name_(std::string& path)
{
    const bool is_success = pr_open_square_bracket_(path) && pr_expression_(path) && pr_close_square_bracket_(path);
    return is_success;
}

// <array-node-name-or-node-name> ::= <array-node-name> | <node-name>
bool Parser::pr_array_node_name_or_node_name_(std::string& path)
{
    const size_t index{m_tokenizer->get_index()};

    bool is_success = pr_array_node_name_(path);
    if (is_success) {
        return true;
    }

    m_tokenizer->set_index(index);
    is_success = pr_node_name_(path);

    return is_success;
}

// <close-square-bracket> ::= ]
bool Parser::pr_close_square_bracket_(std::string& path) const
{
    const csp::Token& token{m_tokenizer->next()};
    const bool is_success{token.type == csp::Token_type::close_square_bracket};
    if (is_success) {
        path += token.value;
    }
    return is_success;
}

// <enum-name> ::= identifier
bool Parser::pr_enum_name_(std::string& enumerator_reference) const
{
    const bool is_success{pr_identifier_(enumerator_reference)};
    return is_success;
}

// <enumerator> ::= identifier
bool Parser::pr_enumerator_(std::string& enumerator_reference) const
{
    const bool is_success{pr_identifier_(enumerator_reference)};
    return is_success;
}

// <dot> ::= .
bool Parser::pr_dot_(std::string& path) const
{
    const csp::Token& token{m_tokenizer->next()};
    const bool is_success{token.type == csp::Token_type::dot};
    if (is_success) {
        path += token.value;
    }
    return is_success;
}

// <enumerator-reference> ::= <enum-name><scope-resolution-operator><enumerator>
bool Parser::pr_enumerator_reference_(std::string& enumerator_reference) const
{
    const bool is_success{pr_enum_name_(enumerator_reference) && pr_scope_resolution_operator_(enumerator_reference)
                          && pr_enumerator_(enumerator_reference)};
    return is_success;
}

bool Parser::pr_expression_(std::string& path)
{
    const int value{parse(*m_tokenizer, *m_variable_manager, m_infix_representation)};
    path += std::to_string(value);
    return true;
}

// <identifier> ::= [a-zA-Z][_a-zA-Z0-9]{0,30}
bool Parser::pr_identifier_(std::string& path) const
{
    const csp::Token& token{m_tokenizer->next()};
    const bool is_success{token.type == csp::Token_type::identifier};
    if (is_success) {
        path += token.value;
    }
    return is_success;
}

// <node-name> ::= identifier
bool Parser::pr_node_name_(std::string& path) const
{
    const bool is_success = pr_identifier_(path);
    return is_success;
}

// <node-reference> ::= <node-name> <opt-node-path>
bool Parser::pr_node_reference_(std::string& path)
{
    const bool is_success{pr_node_name_(path) && pr_opt_path_(path)};
    return is_success;
}

// <null> ::= NULL
bool Parser::pr_null_()
{
    // NULL always succeeds and does not advance the token stream
    return true;
}

// <open-square-bracket> ::= [
bool Parser::pr_open_square_bracket_(std::string& path) const
{
    const csp::Token& token{m_tokenizer->next()};
    const bool is_success{token.type == csp::Token_type::open_square_bracket};
    if (is_success) {
        path += token.value;
    }
    return is_success;
}

// <opt-node-path> ::= <path-separator> <array-node-name-or-node-name> <opt-node-path> | <null>
bool Parser::pr_opt_path_(std::string& path)
{
    const size_t index{m_tokenizer->get_index()};

    bool is_success = pr_path_separator_(path) && pr_array_node_name_or_node_name_(path) && pr_opt_path_(path);
    if (is_success) {
        return true;
    }

    m_tokenizer->set_index(index);
    is_success = pr_null_();
    return is_success;
}

// <path-separator> ::= <dot>
bool Parser::pr_path_separator_(std::string& path) const
{
    const bool is_success{pr_dot_(path)};
    return is_success;
}

// <scope-resolution-operator> := ::
bool Parser::pr_scope_resolution_operator_(std::string& enumerator_reference) const
{
    const csp::Token& token{m_tokenizer->next()};
    const bool is_success{token.type == csp::Token_type::double_colon};
    if (is_success) {
        enumerator_reference += token.value;
    }
    return is_success;
}

} // namespace c4lib::expression_parser
