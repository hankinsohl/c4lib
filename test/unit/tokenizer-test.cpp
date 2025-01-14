// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/29/2024.

#include <algorithm>
#include <cstddef>
#include <format>
#include <gtest/gtest.h>
#include <include/exceptions.hpp>
#include <iostream>
#include <iterator>
#include <lib/native/path.hpp>
#include <lib/schema-parser/token-type.hpp>
#include <lib/schema-parser/token.hpp>
#include <lib/schema-parser/tokenizer-constants.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <list>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <test/unit/types-in-test-data.hpp>
#include <test/util/constants.hpp>
#include <test/util/macros.hpp>
#include <utility>
#include <vector>

using namespace std::string_literals;
namespace ctc = c4lib::test::constants;

namespace {
const c4lib::native::Path schema_ambiguous_tokens{"schema-ambiguous-tokens.txt"};
const c4lib::native::Path schema_keywords{"schema-keywords.txt"};
const c4lib::native::Path schema_operators{"schema-operators.txt"};
const c4lib::native::Path schema_punctuation{"schema-punctuation.txt"};
const c4lib::native::Path schema_types{"schema-types.txt"};
} // namespace

namespace c4lib::schema_parser {

class Tokenizer_test : public testing::Test {
public:
    Tokenizer_test() = default;

    ~Tokenizer_test() override = default;

    Tokenizer_test(const Tokenizer_test&) = delete;

    Tokenizer_test& operator=(const Tokenizer_test&) = delete;

    Tokenizer_test(Tokenizer_test&&) noexcept = delete;

    Tokenizer_test& operator=(Tokenizer_test&&) noexcept = delete;

    static void prepare_expected_token_list(
        std::list<Token>& token_list, std::span<const std::pair<const std::string, Token_type>> tokens)
    {
        for (const auto& pr : tokens) {
            token_list.emplace_back(pr.second, pr.first);
        }
    }

    static void run_token_test(const std::string& test_tokens_file,
        std::span<const std::pair<const std::string, Token_type>> expected_tokens,
        bool fixup_ambiguous_tokens)
    {
        std::list<Token> expected_list;
        prepare_expected_token_list(expected_list, expected_tokens);

        Tokenizer tokenizer;
        tokenizer.run(test_tokens_file);

        std::list<Token> actual_list;
        const std::vector<Token>& actual_vector{tokenizer.get_tokens()};
        std::ranges::copy(actual_vector, std::back_inserter(actual_list));

        for (auto it_expected{expected_list.begin()}, it_actual{actual_list.begin()};
            it_expected != expected_list.end() && it_actual != actual_list.end(); ++it_expected, ++it_actual) {
            Token& expected_token{*it_expected};
            const Token& actual_token{*it_actual};

            // Fixup expected token in case it's ambiguous
            if (fixup_ambiguous_tokens) {
                auto it = Tokenizer::m_ambiguous_hash_map.find(expected_token.value);
                if (it != Tokenizer::m_ambiguous_hash_map.end()) {
                    expected_token.type = it->second;
                }
            }

            EXPECT_EQ(actual_token.type, expected_token.type);
            EXPECT_STREQ(to_string(actual_token.type).c_str(), to_string(expected_token.type).c_str());
            EXPECT_STREQ(actual_token.value.c_str(), expected_token.value.c_str());
        }
    }

protected:
    void SetUp() override
    {
        m_tokenizer.reset();
    }

    void TearDown() override
    {
        // Add common teardown to run after each test
    }

    Tokenizer m_tokenizer;
};

TEST_F(Tokenizer_test, DISABLED_unit_test_print_tokens)
{
    Tokenizer tokenizer;
    tokenizer.run("TokenizerTest_SchemaKeywords.txt");
    tokenizer.print_tokens(std::cout);
}

TEST_F(Tokenizer_test, unit_test_function_names)
{
    const std::string function_names{std::string("func_made_up_func\nfunc_is_ever_alive")};
    std::stringstream str;
    str << function_names;
    EXPECT_NO_THROW(m_tokenizer.run(str));

    const std::vector<Token>& tokens{m_tokenizer.get_tokens()};
    Token token{tokens[0]};
    EXPECT_EQ(token.type, Token_type::function_name);
    EXPECT_STREQ(token.value.c_str(), "func_made_up_func");
    token = tokens[1];
    EXPECT_EQ(token.type, Token_type::function_name);
    EXPECT_STREQ(token.value.c_str(), "func_is_ever_alive");
}

TEST_F(Tokenizer_test, unit_test_types)
{
    run_token_test(ctc::data_misc_dir / schema_types, std::span(types_in_test), false);
}

TEST_F(Tokenizer_test, unit_test_ambiguous_tokens)
{
    run_token_test(ctc::data_misc_dir / schema_ambiguous_tokens, std::span(ambiguous_tokens), false);
}

TEST_F(Tokenizer_test, unit_test_keywords)
{
    run_token_test(ctc::data_misc_dir / schema_keywords, std::span(keywords), true);
}

TEST_F(Tokenizer_test, unit_test_operators)
{
    run_token_test(ctc::data_misc_dir / schema_operators, std::span(operators), true);
}

TEST_F(Tokenizer_test, unit_test_punctuation)
{
    run_token_test(ctc::data_misc_dir / schema_punctuation, std::span(punctuation), true);
}

TEST_F(Tokenizer_test, unit_test_illegal_character)
{
    std::stringstream str;
    str << "~";
    EXPECT_THROW_CONTAINS_MSG(m_tokenizer.run(str), Tokenizer_error, std::format(fmt::invalid_token, '~').c_str());
}

TEST_F(Tokenizer_test, unit_test_line_too_long)
{
    const std::string long_line{std::string(limits::max_schema_line_length, '/')};
    std::stringstream str;
    str << long_line;
    EXPECT_NO_THROW(m_tokenizer.run(str));

    str.clear();
    str << long_line << "/";
    EXPECT_THROW_CONTAINS_MSG(m_tokenizer.run(str), Tokenizer_error,
        std::format(fmt::line_exceeds_maximum_length, limits::max_schema_line_length).c_str());
}

TEST_F(Tokenizer_test, unit_test_identifier_too_long)
{
    std::string long_identifier = std::string(limits::max_identifier_length, 'a');
    std::stringstream str;
    str << long_identifier;
    EXPECT_NO_THROW(m_tokenizer.run(str));

    str.clear();
    long_identifier += "a";
    str << long_identifier;
    EXPECT_THROW_CONTAINS_MSG(m_tokenizer.run(str), Tokenizer_error,
        std::format(fmt::identifier_exceeds_maximum_length, long_identifier, limits::max_identifier_length).c_str());
}

TEST_F(Tokenizer_test, unit_test_number_too_long)
{
    std::string long_number{std::string(limits::max_number_length, '1')};
    std::stringstream str;
    str << long_number;
    EXPECT_NO_THROW(m_tokenizer.run(str));

    str.clear();
    long_number += "1";
    str << long_number;
    EXPECT_THROW_CONTAINS_MSG(m_tokenizer.run(str), Tokenizer_error,
        std::format(fmt::number_exceeds_maximum_length, long_number, limits::max_number_length).c_str());
}

TEST_F(Tokenizer_test, unit_test_string_literal_too_long)
{
    const std::string string_literal{std::string("\"This is a string literal\"")};
    std::stringstream str;
    str << string_literal;
    EXPECT_NO_THROW(m_tokenizer.run(str));
    const Token string_literal_token{m_tokenizer.get_tokens()[0]};
    EXPECT_EQ(string_literal_token.type, Token_type::string_literal);
    EXPECT_STREQ(string_literal_token.value.c_str(), "This is a string literal");

    std::string too_long_string_literal{std::string(limits::max_string_literal_length + 1, '1')};
    const std::string quoted_too_long_string_literal{"\"" + too_long_string_literal + "\""};
    str.clear();
    str << quoted_too_long_string_literal;
    EXPECT_THROW_CONTAINS_MSG(m_tokenizer.run(str), Tokenizer_error,
        std::format(
            fmt::string_literal_exceeds_maximum_length, too_long_string_literal, limits::max_string_literal_length)
            .c_str());
}

TEST_F(Tokenizer_test, unit_test_at)
{
#if defined(__clang__)
    const std::string expected_out_of_range_error_message{"vector"};
#elif defined(_MSC_VER)
    const std::string expected_out_of_range_error_message{"invalid vector subscript"};
#else
    const std::string expected_out_of_range_error_message{">= this->size() (which is 19)"};
#endif

    Tokenizer tokenizer;
    tokenizer.run(ctc::data_misc_dir / native::Path{"schema-keywords.txt"});

    EXPECT_EQ(tokenizer.at(0).type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.at(1).type, Token_type::assert_keyword);
    EXPECT_EQ(tokenizer.at(9).type, Token_type::if_keyword);
    EXPECT_EQ(tokenizer.at(18).type, Token_type::meta_eos);
    EXPECT_THROW_CONTAINS_MSG(
        static_cast<void>(tokenizer.at(19).type), std::out_of_range, expected_out_of_range_error_message.c_str());
    EXPECT_THROW_CONTAINS_MSG(static_cast<void>(tokenizer.at(static_cast<size_t>(-3)).type), std::out_of_range,
        expected_out_of_range_error_message.c_str());
}

TEST_F(Tokenizer_test, unit_test_previous)
{
#if defined(__clang__)
    const std::string expected_out_of_range_error_message{"vector"};
#elif defined(_MSC_VER)
    const std::string expected_out_of_range_error_message{"invalid vector subscript"};
#else
    const std::string expected_out_of_range_error_message{">= this->size() (which is 19)"};
#endif

    Tokenizer tokenizer;
    tokenizer.run(ctc::data_misc_dir / native::Path{"schema-keywords.txt"});

    EXPECT_EQ(tokenizer.next().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.previous().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::assert_keyword);
    EXPECT_EQ(tokenizer.previous().type, Token_type::assert_keyword);

    tokenizer.rewind();
    EXPECT_THROW_CONTAINS_MSG(
        static_cast<void>(tokenizer.previous().type), std::out_of_range, expected_out_of_range_error_message.c_str());
}

TEST_F(Tokenizer_test, unit_test_save_and_restore_index)
{
    Tokenizer tokenizer;
    tokenizer.run(ctc::data_misc_dir / schema_keywords);

    size_t index{m_tokenizer.get_index()};
    EXPECT_EQ(index, 0);
    EXPECT_EQ(tokenizer.next().type, Token_type::alias_keyword);
    index = tokenizer.get_index();
    EXPECT_EQ(index, 1);
    EXPECT_EQ(tokenizer.next().type, Token_type::assert_keyword);
    index = tokenizer.get_index();
    EXPECT_EQ(index, 2);

    tokenizer.rewind();
    index = tokenizer.get_index();
    EXPECT_EQ(index, 0);
    EXPECT_EQ(tokenizer.next().type, Token_type::alias_keyword);
    index = tokenizer.get_index();
    EXPECT_EQ(index, 1);
    EXPECT_EQ(tokenizer.next().type, Token_type::assert_keyword);
    index = tokenizer.get_index();
    EXPECT_EQ(index, 2);

    tokenizer.set_index(0);
    index = tokenizer.get_index();
    EXPECT_EQ(index, 0);

    tokenizer.set_index(1);
    index = tokenizer.get_index();
    EXPECT_EQ(index, 1);
    EXPECT_EQ(tokenizer.next().type, Token_type::assert_keyword);
    index = tokenizer.get_index();
    EXPECT_EQ(index, 2);

    tokenizer.set_index(9);
    index = tokenizer.get_index();
    EXPECT_EQ(index, 9);
    EXPECT_EQ(tokenizer.peek().type, Token_type::if_keyword);

    EXPECT_THROW_CONTAINS_MSG(
        tokenizer.set_index(19), Tokenizer_error, std::format(fmt::index_out_of_range, 19).c_str());
    EXPECT_THROW_CONTAINS_MSG(
        tokenizer.set_index(110), Tokenizer_error, std::format(fmt::index_out_of_range, 110).c_str());
    EXPECT_THROW_CONTAINS_MSG(tokenizer.set_index(static_cast<size_t>(-1)), Tokenizer_error,
        std::format(fmt::index_out_of_range, static_cast<size_t>(-1)).c_str());
}

TEST_F(Tokenizer_test, unit_test_next)
{
#if defined(__clang__)
    const std::string expected_out_of_range_error_message{"vector"};
#elif defined(_MSC_VER)
    const std::string expected_out_of_range_error_message{"invalid vector subscript"};
#else
    const std::string expected_out_of_range_error_message{">= this->size() (which is 19)"};
#endif

    Tokenizer tokenizer;
    tokenizer.run(ctc::data_misc_dir / schema_keywords);

    EXPECT_EQ(tokenizer.next().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::assert_keyword);

    tokenizer.rewind();
    EXPECT_EQ(tokenizer.next().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::assert_keyword);

    tokenizer.rewind();
    for (int i{0}; i <= 18; ++i) {
        EXPECT_NO_THROW(tokenizer.next());
    }
    EXPECT_THROW_CONTAINS_MSG(
        static_cast<void>(tokenizer.next().type), std::out_of_range, expected_out_of_range_error_message.c_str());
}

TEST_F(Tokenizer_test, unit_test_back)
{
#if defined(__clang__)
    const std::string expected_out_of_range_error_message{"vector"};
#elif defined(_MSC_VER)
    const std::string expected_out_of_range_error_message{"invalid vector subscript"};
#else
    const std::string expected_out_of_range_error_message{">= this->size() (which is 19)"};
#endif

    Tokenizer tokenizer;
    tokenizer.run(ctc::data_misc_dir / schema_keywords);

    EXPECT_EQ(tokenizer.next().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.back().type, Token_type::assert_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::assert_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::capture_index_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::const_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::elif_keyword);
    EXPECT_EQ(tokenizer.back().type, Token_type::else_keyword);
    EXPECT_EQ(tokenizer.back().type, Token_type::elif_keyword);
    EXPECT_EQ(tokenizer.back().type, Token_type::const_keyword);
    EXPECT_EQ(tokenizer.back().type, Token_type::capture_index_keyword);
    EXPECT_EQ(tokenizer.back().type, Token_type::assert_keyword);
    EXPECT_EQ(tokenizer.back().type, Token_type::alias_keyword);

    constexpr size_t index{18};
    tokenizer.set_index(index);
    for (int i{18}; i >= 0; --i) {
        EXPECT_NO_THROW(tokenizer.back());
    }
    EXPECT_THROW_CONTAINS_MSG(
        static_cast<void>(tokenizer.back().type), std::out_of_range, expected_out_of_range_error_message.c_str());
}

TEST_F(Tokenizer_test, unit_test_peek)
{
    Tokenizer tokenizer;
    tokenizer.run(ctc::data_misc_dir / schema_keywords);

    EXPECT_EQ(tokenizer.peek().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.peek().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.peek().type, Token_type::assert_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::assert_keyword);

    tokenizer.rewind();
    EXPECT_EQ(tokenizer.peek().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.peek().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.peek().type, Token_type::assert_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::assert_keyword);
}

TEST_F(Tokenizer_test, unit_test_peek_ahead)
{
#if defined(__clang__)
    const std::string expected_out_of_range_error_message{"vector"};
#elif defined(_MSC_VER)
    const std::string expected_out_of_range_error_message{"invalid vector subscript"};
#else
    const std::string expected_out_of_range_error_message{">= this->size() (which is 19)"};
#endif

    Tokenizer tokenizer;
    tokenizer.run(ctc::data_misc_dir / schema_keywords);

    EXPECT_EQ(tokenizer.peek_ahead(3).type, Token_type::const_keyword);
    EXPECT_EQ(tokenizer.peek_ahead(0).type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.peek_ahead(2).type, Token_type::const_keyword);
    EXPECT_EQ(tokenizer.next().type, Token_type::assert_keyword);

    tokenizer.rewind();
    EXPECT_EQ(tokenizer.peek().type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.peek_ahead(0).type, Token_type::alias_keyword);
    EXPECT_EQ(tokenizer.peek_ahead(18).type, Token_type::meta_eos);
    EXPECT_THROW_CONTAINS_MSG(static_cast<void>(tokenizer.peek_ahead(19).type), std::out_of_range,
        expected_out_of_range_error_message.c_str());
}

TEST_F(Tokenizer_test, unit_test_replace_and_restore_type_name_token)
{
    // Initialize token stream
    const std::string complex_typename_type{std::string("int32 Length\nT[Length] Data")};
    std::stringstream str;
    str << complex_typename_type;
    EXPECT_NO_THROW(m_tokenizer.run(str));

    // Verify that the token at position 0 is "int32"
    const Token* token_int32{&m_tokenizer.peek()};
    EXPECT_STREQ(token_int32->value.c_str(), "int32");

    // Verify that the token at position 2 is "T"
    EXPECT_EQ(m_tokenizer.peek_ahead(2).type, Token_type::identifier);
    m_tokenizer.set_index(2);
    const Token* token_type_name{&m_tokenizer.peek()};
    EXPECT_STREQ(token_type_name->value.c_str(), "T");

    // Test replace_type_name_token
    m_tokenizer.replace_type_name_token(*token_int32);
    const Token* replacement_token{&m_tokenizer.peek()};
    EXPECT_STREQ(replacement_token->value.c_str(), "int32");

    // Test replace_type_name_token when a replacement already exists
    const Tokenizer_error expected_exception{make_ex<Tokenizer_error>(fmt::replace_typename_error, token_int32->loc)};
    const std::string expected_error{expected_exception.what()};
    EXPECT_THROW_CONTAINS_MSG(
        m_tokenizer.replace_type_name_token(*token_int32), Tokenizer_error, expected_error.c_str());

    // Test restore_type_name_token
    m_tokenizer.restore_type_name_token();
    EXPECT_STREQ(m_tokenizer.peek().value.c_str(), "T");

    // Test OK to call restore_type_name_token multiple times.
    EXPECT_NO_THROW(m_tokenizer.restore_type_name_token());
    EXPECT_NO_THROW(m_tokenizer.restore_type_name_token());
    EXPECT_NO_THROW(m_tokenizer.restore_type_name_token());

    // Test OK to call replace_type_name_token once more
    m_tokenizer.replace_type_name_token(*token_int32);
    EXPECT_STREQ(m_tokenizer.peek().value.c_str(), "int32");

    // Restore token and then try replacing a non-identifier token.
    EXPECT_NO_THROW(m_tokenizer.restore_type_name_token());
    EXPECT_NO_THROW(m_tokenizer.next());
    EXPECT_THROW_CONTAINS_MSG(
        m_tokenizer.replace_type_name_token(*token_int32), Tokenizer_error, expected_error.c_str());
}

TEST_F(Tokenizer_test, unit_test_using_actual_schema)
{
    EXPECT_NO_THROW(m_tokenizer.run(ctc::relative_root_path_doc / native::Path("BTS.schema")));
}

} // namespace c4lib::schema_parser
