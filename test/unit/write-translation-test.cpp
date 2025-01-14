// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/10/2024.

#include <boost/property_tree/ptree.hpp>
#include <format>
#include <gtest/gtest.h>
#include <include/c4lib.hpp>
#include <lib/native/path.hpp>
#include <lib/util/options.hpp>
#include <string>
#include <test/util/constants.hpp>
#include <unordered_map>

using namespace std::string_literals;
namespace bpt = boost::property_tree;
namespace ctc = c4lib::test::constants;

namespace c4lib::property_tree {

class Write_translation_test : public testing::Test {
public:
    Write_translation_test() = default;

    ~Write_translation_test() override = default;

    Write_translation_test(const Write_translation_test&) = delete;

    Write_translation_test& operator=(const Write_translation_test&) = delete;

    Write_translation_test(Write_translation_test&&) noexcept = delete;

    Write_translation_test& operator=(Write_translation_test&&) noexcept = delete;

protected:
    void SetUp() override
    {
        m_filename = ctc::data_saves_dir / native::Path{"Tiny-Map-BC-4000.CivBeyondSwordSave"};
        m_options[options::schema] = ctc::relative_root_path_doc / native::Path{"BTS.schema"};
        m_options[options::debug_output_dir] = ctc::out_common_dir;
        m_options[options::bts_install_dir]
            = R"(C:\Program Files (x86)\GOG Galaxy\Games\Civilization IV Complete\Civ4\Beyond the Sword)";
        m_options[options::custom_assets_dir]
            = R"(C:\Users\Passenger\Documents\My Games\beyond the sword\CustomAssets)";
        m_options[options::debug_write_binaries] = "1";
    }

    void TearDown() override {}

    std::string m_filename;
    std::unordered_map<std::string, std::string> m_options;
};

TEST_F(Write_translation_test, unit_test_write_translation)
{
    bpt::ptree ptree;
    EXPECT_NO_THROW(read_save(ptree, m_filename, m_options));

    const native::Path translation_info_filename{ctc::out_common_dir / native::Path{"write-translation-test.info"}};
    EXPECT_NO_THROW(write_info(ptree, translation_info_filename, m_options));

    const native::Path translation_filename{ctc::out_common_dir / native::Path{"write-translation-test.txt"}};
    EXPECT_NO_THROW(write_translation(ptree, translation_filename, m_options));
}

TEST_F(Write_translation_test, unit_test_write_translation_disable_columns)
{
    bpt::ptree ptree;
    m_options[options::debug_write_binaries] = "0";

    // Use a small map for the disable-columns test.  We have 3 binary column options
    // which means there are 2^3=8 different combinations to test.
    const native::Path tiny_save{ctc::data_saves_dir / native::Path{"Tiny-Map-BC-4000.CivBeyondSwordSave"}};
    EXPECT_NO_THROW(read_save(ptree, tiny_save, m_options));

    const native::Path translation_info_filename{
        ctc::out_common_dir / native::Path{"write-translation-disable_columns-test.info"}};
    EXPECT_NO_THROW(write_info(ptree, translation_info_filename, m_options));

    int test{1};
    for (int disable_offset = 0; disable_offset < 2; ++disable_offset) {
        for (int disable_hex = 0; disable_hex < 2; ++disable_hex) {
            for (int disable_ascii = 0; disable_ascii < 2; ++disable_ascii) {
                m_options[options::omit_offset_column] = std::to_string(disable_offset);
                m_options[options::omit_hex_column] = std::to_string(disable_hex);
                m_options[options::omit_ascii_column] = std::to_string(disable_ascii);
                const std::string fmt{ctc::out_common_dir.str()
                                      + R"(\write-translation-test-disable-columns-{}-offset={}-hex={}-ascii={}.txt)"};
                const std::string filename{
                    std::vformat(fmt, std::make_format_args(test, disable_offset, disable_hex, disable_ascii))};
                EXPECT_NO_THROW(write_translation(ptree, filename, m_options));
                ++test;
            }
        }
    }
}

} // namespace c4lib::property_tree
