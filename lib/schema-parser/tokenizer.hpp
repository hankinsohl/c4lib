// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/28/2024.

#pragma once

#include <cstddef>
#include <format>
#include <include/exceptions.hpp>
#include <iostream>
#include <lib/schema-parser/token-type.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/tune.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace c4lib::schema_parser {

class Tokenizer {
public:
    friend class Tokenizer_test;

    Tokenizer()
    {
        rewind();
        m_stream.reserve(tune::schema_token_vector_reserve_size);
    }

    ~Tokenizer() = default;

    Tokenizer(const Tokenizer&) = delete;

    Tokenizer& operator=(const Tokenizer&) = delete;

    Tokenizer(Tokenizer&&) noexcept = delete;

    Tokenizer& operator=(Tokenizer&&) noexcept = delete;

    // Returns the token at index.  Throws an exception if no such token exists.
    [[nodiscard]] const Token& at(size_t index) const
    {
        check_bad_();
        return m_stream.at(index);
    }

    // Returns the current token and backs up one position in the stream.  Throws an exception if seeking beyond the
    // beginning of the stream.
    const Token& back()
    {
        check_bad_();
        return at(m_index--);
    }

    [[nodiscard]] size_t count() const
    {
        return m_stream.size();
    }

    [[nodiscard]] const std::string& get_filename() const
    {
        check_bad_();
        return m_filename;
    }

    [[nodiscard]] size_t get_index() const
    {
        check_bad_();
        return m_index;
    }

    // Provides raw access to the token stream; intended primarily for debugging.  Callers should generally use other
    // access methods instead.
    [[nodiscard]] const std::vector<Token>& get_tokens() const
    {
        check_bad_();
        return m_stream;
    }

    // Returns the current token and advances the stream one position.  Throws an exception if seeking beyond the end
    // of the stream.
    const Token& next()
    {
        check_bad_();
        return at(m_index++);
    }

    // Returns the current token but does not advance the stream position.  Throws an exception if peeking past the
    // end of the stream.
    [[nodiscard]] const Token& peek() const
    {
        check_bad_();
        return peek_ahead(0);
    }

    // Returns the token at the current position + offset but does not advance the stream position.  Throws an
    // exception if peeking past the end of the stream.
    [[nodiscard]] const Token& peek_ahead(size_t offset) const
    {
        check_bad_();
        return at(m_index + offset);
    }

    // Returns the previous token.  Throws an exception if at start of stream.
    [[nodiscard]] const Token& previous() const
    {
        check_bad_();
        return at(m_index - 1);
    }

    // Prints the token stack to out.
    void print_tokens(std::ostream& out) const
    {
        check_bad_();
        for (const auto& token : m_stream) {
            out << token << "\n";
        }
    }

    // Reset the state of the tokenizer.  After reset is called, the tokenizer is in the same state as
    // it was just after construction.
    void reset()
    {
        m_bad = false;
        m_stream.clear();
        rewind();
    }

    // Replaces the current token, which must be of type identifier with type.  Throws an exception if the
    // current token is not of type identifier or if a replacement is already in force.  Only one replacement
    // is allowed at a time.  The replacement can be restored using restore_type_name_token.  The pair of
    // functions - replace_type_name_token and restore_type_name_token - are used by the phase 2 parser to
    // instantiate templates
    void replace_type_name_token(const Token& type)
    {
        check_bad_();
        if (m_replaced_type_name.type != Token_type::invalid || peek().type != Token_type::identifier) {
            throw make_ex<Tokenizer_error>(fmt::replace_typename_error, type.loc);
        }
        m_replaced_type_name = peek();
        m_stream.at(m_index) = type;
        m_stream.at(m_index).index = m_index;
    }

    // Sets the current index into the token vector.  If index is out-of-range, sets the bad boolean.
    void set_index_noexcept(size_t index) noexcept
    {
        if (index >= m_stream.size()) {
            m_bad = true;
        }
        else {
            m_index = index;
        }
    }

    // Restores the previously replaced type_name token.  Takes no action if a replacement has not been made.
    void restore_type_name_token()
    {
        check_bad_();
        if (m_replaced_type_name.type != Token_type::invalid) {
            m_stream.at(m_replaced_type_name.index) = m_replaced_type_name;
            m_replaced_type_name = Token();
        }
    }

    // Rewinds the token stream.  Following rewind, next returns the first token in the stream.
    void rewind()
    {
        check_bad_();
        m_index = 0;
    }

    // Calls setFile(filename), opens filename and creates an ifstream.  Then calls run(ifstream).
    void run(const std::string& filename);

    // Reads lines from in, performs tokenization and generates the token stack.
    void run(std::istream& in);

    void set_filename(const std::string& filename)
    {
        check_bad_();
        m_filename = filename;
    }

    // Sets the current index.
    void set_index(size_t index)
    {
        check_bad_();
        if (index >= m_stream.size()) {
            throw Tokenizer_error{std::format(fmt::index_out_of_range, index)};
        }
        m_index = index;
    }

private:
    void check_bad_() const
    {
        if (m_bad) {
            throw Tokenizer_error{std::format(fmt::bad_state)};
        }
    }

    static void check_number_(Token& token);

    static void disambiguate_name_(Token& token);

    static void disambiguate_punc_or_op_(Token& token);

    // Gets the next token value from word where word is a contiguous sequence of non-whitespace
    // characters.  The token will be one of:
    //   1) A comment; or
    //   2) A string literal; or
    //   3) A numeric literal in either decimal or hexadecimal format, possibly preceded by + or -; or
    //   4) A single punctuation character taken from the set ": ; < > { } [ ] ( )"; or
    //   5) An operator, taken from the set "+ - * / % && || ! <= == >= !="; or
    //   6) A keyword; or
    //   7) A type; or
    //   8) An identifier
    // If the current character sequence cannot be classified into one of the categories above,
    // throws std::logic_error.  If end-of-word is reached, sets start to std::string::npos,
    // token to "" and returns false.  Otherwise, sets start to the start character of the token,
    // token to the token value and returns true.
    static void get_token_(const std::string& line, size_t& start, Token& token);

    static bool match_comment_(const std::string& line, size_t start, Token& token);

    static bool match_function_name_(Token& token);

    static bool match_name_(const std::string& line, size_t start, Token& token);

    static bool match_number_(const std::string& line, size_t start, Token& token);

    static bool match_punc_or_op_(const std::string& line, size_t start, Token& token);

    static bool match_string_literal_(const std::string& line, size_t start, Token& token);

    static bool match_type_(Token& token);

    static bool match_using_regex_(
        const std::string& line, size_t start, Token& token, const std::string& regex, void (*disambiguate)(Token&));

    // Beginning at start, searches for the first non-whitespace character.  If found, updates start to this
    // location and returns true.  If not found, sets start to std::string::npos and returns false.
    static bool skip_whitespace_(const std::string& line, size_t& start);

    static const std::unordered_map<std::string, Token_type> m_ambiguous_hash_map;
    bool m_bad{false};
    std::string m_filename;
    size_t m_index{limits::invalid_size};
    static const std::unordered_map<std::string, Token_type> m_keyword_hash_map;
    static const std::unordered_map<std::string, Token_type> m_op_hash_map;
    static const std::unordered_map<std::string, Token_type> m_punc_hash_map;
    Token m_replaced_type_name;
    std::vector<Token> m_stream;
};

} // namespace c4lib::schema_parser
