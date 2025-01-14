// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/28/2024.

#include <array>
#include <cstddef>
#include <format>
#include <fstream>
#include <include/exceptions.hpp>
#include <ios>
#include <iosfwd>
#include <lib/schema-parser/token-type.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/schema-parser/tokenizer-constants.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/file-location.hpp>
#include <lib/util/limits.hpp>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace c4lib::schema_parser {

using namespace std::string_literals;

constexpr const char* whitespace{" \t\r\n"};

const std::unordered_map<std::string, Token_type> Tokenizer::m_ambiguous_hash_map{[] {
    std::unordered_map<std::string, Token_type> ambiguous_hash_map;
    for (const auto& [value, type] : ambiguous_tokens) {
        ambiguous_hash_map[value] = type;
    }
    return ambiguous_hash_map;
}()};

const std::unordered_map<std::string, Token_type> Tokenizer::m_keyword_hash_map{[] {
    std::unordered_map<std::string, Token_type> keyword_hash_map;
    for (const auto& [value, type] : keywords) {
        keyword_hash_map[value] = type;
    }
    return keyword_hash_map;
}()};

const std::unordered_map<std::string, Token_type> Tokenizer::m_op_hash_map{[] {
    std::unordered_map<std::string, Token_type> operator_hash_map;
    for (const auto& [value, type] : operators) {
        operator_hash_map[value] = type;
    }
    return operator_hash_map;
}()};

const std::unordered_map<std::string, Token_type> Tokenizer::m_punc_hash_map{[] {
    std::unordered_map<std::string, Token_type> punctuation_hash_map;
    for (const auto& [value, type] : punctuation) {
        punctuation_hash_map[value] = type;
    }
    return punctuation_hash_map;
}()};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Tokenizer::run(const std::string& filename)
{
    check_bad_();
    set_filename(filename);
    std::ifstream in(filename, std::ios_base::in);
    if (!in) {
        throw std::runtime_error{std::format(fmt::runtime_error_opening_file, filename)};
    }
    run(in);
}

void Tokenizer::run(std::istream& in)
{
    check_bad_();
    reset();
    auto filename{std::make_shared<std::string>(get_filename())};
    size_t lineNumber{0};
    while (in) {
        auto line{std::make_shared<std::string>()};
        std::getline(in, *line);
        lineNumber++;
        if (line->length() > limits::max_schema_line_length) {
            const File_location loc{filename, line, lineNumber, 1};
            throw make_ex<Tokenizer_error>(fmt::line_exceeds_maximum_length, loc, limits::max_schema_line_length);
        }

        size_t start{0};
        while (skip_whitespace_(*line, start)) {
            const File_location loc{filename, line, lineNumber, start + 1};
            m_stream.emplace_back(Token_type::invalid, "", loc, m_stream.size());
            Token& token{m_stream.back()};
            get_token_(*line, start, token);

            // Comments are not stored in the token stream.
            if (token.type == Token_type::double_slash) {
                m_stream.pop_back();
            }
        }
    }
    if (in.bad()) {
        throw std::runtime_error{std::format(fmt::runtime_error_reading_from_file, get_filename())};
    }

    // Mark the end of the token stream with the end-of-stream meta token.
    const File_location end_of_file{filename, std::make_shared<std::string>(""), lineNumber + 1, 1};
    m_stream.emplace_back(Token_type::meta_eos, "$", end_of_file, m_stream.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Tokenizer::check_number_(Token& token)
{
    // Check length of number.  Note that this length check is somewhat redundant because we'll check to see if the
    // number is within range during later parse phases; nonetheless we perform the length check here as a safety
    // precaution.
    if (token.value.length() > limits::max_number_length) {
        throw make_ex<Tokenizer_error>(
            fmt::number_exceeds_maximum_length, token.loc, token.value, limits::max_number_length);
    }

    token.type = Token_type::numeric_literal;
}

void Tokenizer::disambiguate_name_(Token& token)
{
    // Check length of identifier
    if (token.value.length() > limits::max_identifier_length) {
        throw make_ex<Tokenizer_error>(
            fmt::identifier_exceeds_maximum_length, token.loc, token.value, limits::max_identifier_length);
    }

    // Check for function name
    if (match_function_name_(token)) {
        token.type = Token_type::function_name;
        return;
    }

    // Check for keyword
    auto it{m_keyword_hash_map.find(token.value)};
    if (it != m_keyword_hash_map.end()) {
        token.type = it->second;
        return;
    }

    // Check for type
    if (match_type_(token)) {
        return;
    }

    // If it's not a keyword, and it's not a type, it's an identifier
    token.type = Token_type::identifier;
}

void Tokenizer::disambiguate_punc_or_op_(Token& token)
{
    // N.B.: m_ambiguous_hash_map must appear first in the hash_maps array b/c we want to classify ambiguous tokens
    // first so that tokenization is consistent.
    const std::array hash_maps{m_ambiguous_hash_map, m_op_hash_map, m_punc_hash_map};
    for (auto hmap : hash_maps) {
        auto it{hmap.find(token.value)};
        if (it != hmap.end()) {
            token.type = it->second;
            return;
        }
    }

    // If we reach this point there's an error in the code.  The token should have matched an operator or punctuation.
    throw std::logic_error(std::format(fmt::internal_bug_in_function, "disambiguate_punc_or_op_"));
}

void Tokenizer::get_token_(const std::string& line, size_t& start, Token& token)
{
    // Note: The parse order below should not be changed to prevent mis-identification of tokens.

    // The current character is either
    //     1) the start of a comment; or
    //     2) the start of a string literal; or
    //     3) the start of a numeric literal in either decimal or hexadecimal format, possibly preceded by + or -; or
    //     4) punctuation allowed by the grammar; or
    //     5) the start of a name (i.e., an identifier, function name, keyword, or type); or
    //     6) is invalid
    if (match_comment_(line, start, token) || match_string_literal_(line, start, token)
        || match_number_(line, start, token) || match_punc_or_op_(line, start, token)
        || match_name_(line, start, token)) {
        start += token.value.length();
        if (token.type == Token_type::string_literal) {
            // Creation of TokenType::StringLiteral consumed the enclosing quotation marks, but they
            // are not part of the token value.  Add 2 to start to account for the quotation marks.
            start += 2;
        }
        return;
    }
    throw make_ex<Tokenizer_error>(fmt::invalid_token, token.loc, line[start]);
}

bool Tokenizer::match_comment_(const std::string& line, size_t start, Token& token)
{
    if (line.find("//", start) == start) {
        // N.B.: We choose to define the comment token value to consist of "//" up until the end
        // of the line.  This way we retain the comment itself.
        token.value = line.substr(start);
        token.type = Token_type::double_slash;
        return true;
    }
    else {
        return false;
    }
}

bool Tokenizer::match_function_name_(Token& token)
{
    if (token.value.starts_with("func_")) {
        token.type = Token_type::function_name;
        return true;
    }
    else {
        return false;
    }
}

bool Tokenizer::match_name_(const std::string& line, size_t start, Token& token)
{
    // Leading underscores are allowed so that we match built-in function names.
    const std::string name_regex{"^[_a-zA-Z][_a-zA-Z0-9]*"};
    return match_using_regex_(line, start, token, name_regex, disambiguate_name_);
}

bool Tokenizer::match_number_(const std::string& line, size_t start, Token& token)
{
    const std::string hex_number_regex{"^[+\\-]?(0x|0X)[0-9a-fA-F]+"};
    const std::string decimal_number_regex{"^[+\\-]?[0-9]+"};
    return match_using_regex_(line, start, token, hex_number_regex, check_number_)
           || match_using_regex_(line, start, token, decimal_number_regex, check_number_);
}

bool Tokenizer::match_punc_or_op_(const std::string& line, size_t start, Token& token)
{
    // Check for double-character punctuation first because the first character of some double-character punctuation
    // is an allowed single-character punctuation.  Checking for single-character punctuation first would incorrectly
    // tokenize these double-character tokens.

    const std::string double_char_punc_regex{"^(::|<=|==|>=|!=|&&|\\|\\|)"};
    const std::string single_char_punc_regex{R"(^[\.:;<>{}[\]()=+\-*/%!])"};
    return match_using_regex_(line, start, token, double_char_punc_regex, disambiguate_punc_or_op_)
           || match_using_regex_(line, start, token, single_char_punc_regex, disambiguate_punc_or_op_);
}

bool Tokenizer::match_string_literal_(const std::string& line, size_t start, Token& token)
{
    if (line.find('"', start) == start) {
        std::string::size_type end{line.find('"', start + 1)};
        if (end != std::string::npos) {
            // Note: The value excludes surrounding quotation marks.
            start += 1; // Skip over opening quotation mark
            end -= 1; // Stop before closing quotation mark
            token.value = line.substr(start, end - start + 1);
            token.type = Token_type::string_literal;

            // Check length of string literal.
            if (token.value.length() > limits::max_string_literal_length) {
                throw make_ex<Tokenizer_error>(fmt::string_literal_exceeds_maximum_length, token.loc, token.value,
                    limits::max_string_literal_length);
            }

            return true;
        }
    }
    return false;
}

bool Tokenizer::match_type_(Token& token)
{
    for (const auto& [regex_str, type] : base_types) {
        if (std::smatch matches; std::regex_search(token.value, matches, std::regex(regex_str))) {
            token.type = type;
            return true;
        }
    }

    return false;
}

bool Tokenizer::match_using_regex_(
    const std::string& line, size_t start, Token& token, const std::string& regex, void (*disambiguate)(Token&))
{
    const std::string word{line.substr(start)};
    if (std::smatch matches; std::regex_search(word, matches, std::regex(regex))) {
        token.value = std::string(matches[0]);
        disambiguate(token);
        return true;
    }

    return false;
}

bool Tokenizer::skip_whitespace_(const std::string& line, size_t& start)
{
    start = line.find_first_not_of(whitespace, start);
    return start != std::string::npos;
}

} // namespace c4lib::schema_parser
