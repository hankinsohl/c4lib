// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/22/2024.

#include <array>
#include <boost/property_tree/ptree.hpp>
#include <gtest/gtest.h>
#include <include/node-attributes.hpp>
#include <include/node-type.hpp>
#include <lib/expression-parser/infix-representation.hpp>
#include <lib/expression-parser/parser.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <sstream>
#include <string>

namespace bpt = boost::property_tree;
namespace cpt = c4lib::property_tree;
namespace csp = c4lib::schema_parser;

namespace {
struct TestInfo {
    std::string expression;
    int value;
    std::string infixNotation;
};

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
const std::array test_info{
    // Test various numeric literals.  Hex and decimal format.
    TestInfo{.expression = "1 + 1", .value = 2, .infixNotation = "(1 + 1)"},
    TestInfo{.expression = "0x10 + 0xD", .value = 29, .infixNotation = "(0x10 + 0xD)"},
    TestInfo{.expression = "2 + 0xc", .value = 14, .infixNotation = "(2 + 0xc)"},

    // Test arithmetic operators
    TestInfo{.expression = "2 + 1", .value = 3, .infixNotation = "(2 + 1)"},
    TestInfo{.expression = "1 - 1", .value = 0, .infixNotation = "(1 - 1)"},
    TestInfo{.expression = "2 * 3", .value = 6, .infixNotation = "(2 * 3)"},
    TestInfo{.expression = "4 / 2", .value = 2, .infixNotation = "(4 / 2)"},
    TestInfo{.expression = "5 % 3", .value = 2, .infixNotation = "(5 % 3)"},

    // Test logical operators
    TestInfo{.expression = "2 && 1", .value = 1, .infixNotation = "(2 && 1)"},
    TestInfo{.expression = "32 && 0", .value = 0, .infixNotation = "(32 && 0)"},
    TestInfo{.expression = "0 && 10", .value = 0, .infixNotation = "(0 && 10)"},
    TestInfo{.expression = "0 && 0", .value = 0, .infixNotation = "(0 && 0)"},
    TestInfo{.expression = "2 || 1", .value = 1, .infixNotation = "(2 || 1)"},
    TestInfo{.expression = "32 || 0", .value = 1, .infixNotation = "(32 || 0)"},
    TestInfo{.expression = "0 || 10", .value = 1, .infixNotation = "(0 || 10)"},
    TestInfo{.expression = "0 || 0", .value = 0, .infixNotation = "(0 || 0)"},
    TestInfo{.expression = "!0", .value = 1, .infixNotation = "(!0)"},
    TestInfo{.expression = "!10", .value = 0, .infixNotation = "(!10)"},
    TestInfo{.expression = "!-10", .value = 0, .infixNotation = "(!-10)"},

    // Test comparison operators
    TestInfo{.expression = "2 > 1", .value = 1, .infixNotation = "(2 > 1)"},
    TestInfo{.expression = "1 > 2", .value = 0, .infixNotation = "(1 > 2)"},
    TestInfo{.expression = "1 > 1", .value = 0, .infixNotation = "(1 > 1)"},
    TestInfo{.expression = "2 >= 1", .value = 1, .infixNotation = "(2 >= 1)"},
    TestInfo{.expression = "1 >= 2", .value = 0, .infixNotation = "(1 >= 2)"},
    TestInfo{.expression = "1 >= 1", .value = 1, .infixNotation = "(1 >= 1)"},
    TestInfo{.expression = "2 == 1", .value = 0, .infixNotation = "(2 == 1)"},
    TestInfo{.expression = "1 == 2", .value = 0, .infixNotation = "(1 == 2)"},
    TestInfo{.expression = "1 == 1", .value = 1, .infixNotation = "(1 == 1)"},
    TestInfo{.expression = "2 != 1", .value = 1, .infixNotation = "(2 != 1)"},
    TestInfo{.expression = "1 != 2", .value = 1, .infixNotation = "(1 != 2)"},
    TestInfo{.expression = "1 != 1", .value = 0, .infixNotation = "(1 != 1)"},
    TestInfo{.expression = "2 <= 1", .value = 0, .infixNotation = "(2 <= 1)"},
    TestInfo{.expression = "1 <= 2", .value = 1, .infixNotation = "(1 <= 2)"},
    TestInfo{.expression = "1 <= 1", .value = 1, .infixNotation = "(1 <= 1)"},
    TestInfo{.expression = "2 < 1", .value = 0, .infixNotation = "(2 < 1)"},
    TestInfo{.expression = "1 < 2", .value = 1, .infixNotation = "(1 < 2)"},
    TestInfo{.expression = "1 < 1", .value = 0, .infixNotation = "(1 < 1)"},

    // Test grouping
    TestInfo{.expression = "2 + 3 * 3", .value = 11, .infixNotation = "(2 + (3 * 3))"},
    TestInfo{.expression = "(2 + 3) * 3", .value = 15, .infixNotation = "((2 + 3) * 3)"},

    // Test precedence
    TestInfo{.expression = "2 - 3 / 3", .value = 1, .infixNotation = "(2 - (3 / 3))"},
    TestInfo{.expression = "2 < 3 && 3 > -1", .value = 1, .infixNotation = "((2 < 3) && (3 > -1))"},

    // Test associativity
    TestInfo{.expression = "1 - 2 - 3", .value = -4, .infixNotation = "((1 - 2) - 3)"},
    TestInfo{.expression = "9 / 3 / 3", .value = 1, .infixNotation = "((9 / 3) / 3)"},
    TestInfo{.expression = "1 && 0 || 1", .value = 1, .infixNotation = "((1 && 0) || 1)"},

    // Test use of variables
    TestInfo{.expression = "i2 + j17", .value = 19, .infixNotation = "(i2 + j17)"},

    // Test use of node reference
    TestInfo{.expression = "r.cn1.cn2.[3] - 2", .value = 1, .infixNotation = "(r.cn1.cn2.[3] - 2)"},
};
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

} // namespace

namespace c4lib::expression_parser {

class Expression_parser_test : public testing::Test {
public:
    Expression_parser_test() = default;

    ~Expression_parser_test() override = default;

    Expression_parser_test(const Expression_parser_test&) = delete;

    Expression_parser_test& operator=(const Expression_parser_test&) = delete;

    Expression_parser_test(Expression_parser_test&&) noexcept = delete;

    Expression_parser_test& operator=(Expression_parser_test&&) noexcept = delete;

protected:
    void SetUp() override
    {
        m_variable_manager.init(&m_ptree, &m_ptree_parent, m_definition_table);

        // Add variables i2 and j17.
        m_variable_manager.push();
        m_variable_manager.add("i2", 2);
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
        m_variable_manager.add("j17", 17);

        // Add ptree node for int value
        bpt::ptree& value{m_ptree.put_child("r.cn1.cn2.[3]", bpt::ptree{})};
        bpt::ptree& attributes{value.put_child(cpt::nn_attributes, bpt::ptree{})};
        attributes.put<cpt::Node_type>(cpt::nn_type, cpt::Node_type::int_type);
        attributes.put<std::string>(cpt::nn_data, std::string("3"));

        // BEGIN TROUBLESHOOTING CODE
        // std::cout << "m_root_ptree:" << '\n';
        // bpt::write_info(std::cout, m_root_ptree);
        // END TROUBLESHOOTING CODE
    }

    void TearDown() override {}

    static void tokenize_expression(const std::string& expression, csp::Tokenizer& tokenizer);

    csp::Def_tbl* m_definition_table{nullptr};
    bpt::ptree m_ptree;
    bpt::ptree* m_ptree_parent{&m_ptree};
    Variable_manager m_variable_manager;
};

void Expression_parser_test::tokenize_expression(const std::string& expression, csp::Tokenizer& tokenizer)
{
    std::stringstream expression_stream;
    expression_stream << expression;
    tokenizer.run(expression_stream);
}

TEST_F(Expression_parser_test, unit_test_constant_expressions)
{
    csp::Tokenizer tokenizer;
    Parser parser;
    for (const auto& info : test_info) {
        tokenize_expression(info.expression, tokenizer);
        Infix_representation logger;
        const int value{parser.parse(tokenizer, m_variable_manager, &logger)};
        const std::string infixNotation{logger.pop()};
        EXPECT_EQ(value, info.value) << "Failed evaluating expression: " << info.expression;
        EXPECT_STREQ(infixNotation.c_str(), info.infixNotation.c_str());
    }
}

} // namespace c4lib::expression_parser
