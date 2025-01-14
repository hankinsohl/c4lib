// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/8/2024.

#include <boost/property_tree/ptree.hpp>
#include <cstddef>
#include <gtest/gtest.h>
#include <lib/native/path.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/parser-phase-one.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/limits.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <string>
#include <test/util/constants.hpp>

using namespace std::string_literals;
namespace bpt = boost::property_tree;

namespace c4lib::schema_parser {

class Schema_parser_p1_test : public testing::Test {
public:
    Schema_parser_p1_test() = default;

    ~Schema_parser_p1_test() override = default;

    Schema_parser_p1_test(const Schema_parser_p1_test&) = delete;

    Schema_parser_p1_test& operator=(const Schema_parser_p1_test&) = delete;

    Schema_parser_p1_test(Schema_parser_p1_test&&) noexcept = delete;

    Schema_parser_p1_test& operator=(Schema_parser_p1_test&&) noexcept = delete;

protected:
    void SetUp() override
    {
        m_custom_assets_path = native::Path{R"(C:\Users\Passenger\Documents\My Games\beyond the sword\CustomAssets)"};
        m_definition_table.reset();
        m_install_root
            = native::Path{R"(C:\Program Files (x86)\GOG Galaxy\Games\Civilization IV Complete\Civ4\Beyond the Sword)"};
        m_mod_name = "";
        m_root_name_index = limits::invalid_size;
        m_tokenizer.reset();
        m_use_modular_loading = false;
        m_variable_manager.init(&m_ptree, &m_ptree_parent, &m_definition_table);
    }

    void TearDown() override {}

    native::Path m_custom_assets_path;
    Def_tbl m_definition_table;
    native::Path m_install_root;
    std::string m_mod_name;
    bpt::ptree m_ptree;
    bpt::ptree* m_ptree_parent{&m_ptree};
    size_t m_root_name_index{limits::invalid_size};
    Tokenizer m_tokenizer;
    bool m_use_modular_loading{false};
    Variable_manager m_variable_manager;
};

TEST_F(Schema_parser_p1_test, unit_test_schema)
{
    const native::Path schema = test::constants::relative_root_path_doc / native::Path{"BTS.schema"};
    Parser_phase_one parser(schema, m_install_root, m_custom_assets_path, m_mod_name, m_use_modular_loading,
        m_tokenizer, m_definition_table, m_root_name_index, m_variable_manager);
    EXPECT_NO_THROW(parser.parse());
}

} // namespace c4lib::schema_parser
