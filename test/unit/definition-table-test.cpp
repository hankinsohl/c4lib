// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/17/2024.

#include <boost/property_tree/ptree.hpp>
#include <cstddef>
#include <fstream>
#include <gtest/gtest.h>
#include <ios>
#include <lib/native/path.hpp>
#include <lib/schema-parser/def-tbl.hpp>
#include <lib/schema-parser/def-type.hpp>
#include <lib/schema-parser/parser-phase-one.hpp>
#include <lib/schema-parser/tokenizer.hpp>
#include <lib/util/limits.hpp>
#include <lib/variable-manager/variable-manager.hpp>
#include <stdexcept>
#include <string>
#include <test/util/constants.hpp>

using namespace std::string_literals;
namespace bpt = boost::property_tree;
namespace ctc = c4lib::test::constants;

namespace c4lib::schema_parser {

class Definition_table_test : public testing::Test {
public:
    Definition_table_test() = default;

    ~Definition_table_test() override = default;

    Definition_table_test(const Definition_table_test&) = delete;

    Definition_table_test& operator=(const Definition_table_test&) = delete;

    Definition_table_test(Definition_table_test&&) noexcept = delete;

    Definition_table_test& operator=(Definition_table_test&&) noexcept = delete;

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

    static void export_(const Def_tbl& table, Def_type type, const std::string& filename);

    native::Path m_custom_assets_path;
    Def_tbl m_definition_table;
    native::Path m_install_root;
    bpt::ptree m_ptree;
    bpt::ptree* m_ptree_parent{&m_ptree};
    std::string m_mod_name;
    size_t m_root_name_index{limits::invalid_size};
    Tokenizer m_tokenizer;
    bool m_use_modular_loading{false};
    Variable_manager m_variable_manager;
};

void Definition_table_test::export_(const Def_tbl& table, Def_type type, const std::string& filename)
{
    std::ofstream out(filename, std::ios_base::out);
    if (!out.is_open() || out.bad()) {
        throw std::runtime_error("Error opening file '" + filename + "'.");
    }
    table.export_definitions(type, out);
}

TEST_F(Definition_table_test, unit_test_export)
{
    const native::Path schema{ctc::relative_root_path_doc / native::Path{"BTS.schema"}};
    Parser_phase_one parser{schema, m_install_root, m_custom_assets_path, m_mod_name, m_use_modular_loading,
        m_tokenizer, m_definition_table, m_root_name_index, m_variable_manager};
    EXPECT_NO_THROW(parser.parse());

    EXPECT_NO_THROW(
        export_(m_definition_table, Def_type::const_type, ctc::out_common_dir / native::Path{"ConstDefinitions.txt"}));
    EXPECT_NO_THROW(
        export_(m_definition_table, Def_type::enum_type, ctc::out_common_dir / native::Path{"EnumDefinitions.txt"}));
}

} // namespace c4lib::schema_parser
