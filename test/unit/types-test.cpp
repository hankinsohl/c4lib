// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/6/2024.

#include <gtest/gtest.h>
#include <include/node-type.hpp>
#include <lib/schema-parser/def-mem-type.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/token-type.hpp>
#include <lib/util/enum-range.hpp>

namespace cpt = c4lib::property_tree;
namespace csp = c4lib::schema_parser;

namespace {
void test_def_mem_type_to_string()
{
    for (const auto& e : c4lib::enum_range(csp::Def_mem_type::begin, csp::Def_mem_type::end)) {
        static_cast<void>(to_string(e));
    }
}

void test_def_type_to_string()
{
    for (const auto& e : c4lib::enum_range(csp::Def_type::begin, csp::Def_type::end)) {
        static_cast<void>(to_string(e));
    }
}

void test_token_type_to_string()
{
    for (const auto& e : c4lib::enum_range(csp::Token_type::begin, csp::Token_type::end)) {
        static_cast<void>(to_string(e));
    }
}

void test_tree_node_type_to_string()
{
    for (const auto& e : c4lib::enum_range(cpt::Node_type::begin, cpt::Node_type::end)) {
        static_cast<void>(to_string(e));
    }
}

} // namespace

namespace c4lib {

class Types_test : public testing::Test {
public:
    Types_test() = default;

    ~Types_test() override = default;

    Types_test(const Types_test&) = delete;

    Types_test& operator=(const Types_test&) = delete;

    Types_test(Types_test&&) noexcept = delete;

    Types_test& operator=(Types_test&&) noexcept = delete;

protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(Types_test, unit_test_def_mem_type_to_string)
{
    EXPECT_NO_THROW(test_def_mem_type_to_string());
}

TEST_F(Types_test, unit_test_def_type_to_string)
{
    EXPECT_NO_THROW(test_def_type_to_string());
}

TEST_F(Types_test, unit_test_token_type_to_string)
{
    EXPECT_NO_THROW(test_token_type_to_string());
}

TEST_F(Types_test, unit_test_tree_node_type_to_string)
{
    EXPECT_NO_THROW(test_tree_node_type_to_string());
}

} // namespace c4lib
