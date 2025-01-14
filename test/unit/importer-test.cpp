// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/11/2024.

#include <gtest/gtest.h>
#include <lib/importer/importer.hpp>
#include <lib/native/path.hpp>
#include <lib/schema-parser/def-tbl.hpp>

namespace csp = c4lib::schema_parser;

namespace c4lib {

class Importer_test : public testing::Test {
public:
    Importer_test() = default;

    ~Importer_test() override = default;

    Importer_test(const Importer_test&) = delete;

    Importer_test& operator=(const Importer_test&) = delete;

    Importer_test(Importer_test&&) noexcept = delete;

    Importer_test& operator=(Importer_test&&) noexcept = delete;

protected:
    void SetUp() override
    {
        m_custom_assets_path = native::Path{R"(C:\Users\Passenger\Documents\My Games\beyond the sword\CustomAssets)"};
        m_install_root
            = native::Path{R"(C:\Program Files (x86)\GOG Galaxy\Games\Civilization IV Complete\Civ4\Beyond the Sword)"};
        m_mod_name = "";
        m_use_modular_loading = false;
    }

    void TearDown() override {}

    native::Path m_custom_assets_path;
    native::Path m_install_root;
    std::string m_mod_name;
    bool m_use_modular_loading{false};
};

// TODO - Test for duplicate enum and const imports as well as both import and definition in schema.

TEST_F(Importer_test, unit_test_import)
{
    // FIXME - this test doesn't do anything.  Either remove it or add definitions for import.
    csp::Def_tbl definitionTable;
    Importer importer;
    EXPECT_NO_THROW(importer.import_definitions(
        definitionTable, m_install_root, m_custom_assets_path, m_mod_name, m_use_modular_loading));
}

} // namespace c4lib
