// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/12/2024.

#include <gtest/gtest.h>
#include <lib/native/path.hpp>
#include <lib/options/exceptions.hpp>
#include <lib/options/options-manager.hpp>
#include <span>
#include <string>
#include <test/unit/options-manager-test-data.hpp>
#include <test/util/constants.hpp>
#include <test/util/macros.hpp>
#include <unordered_map>

using namespace std::string_literals;
namespace ctc = c4lib::test::constants;

namespace hankinsohl::options {

class Options_manager_test : public testing::Test {
public:
    Options_manager_test() = default;

    ~Options_manager_test() override = default;

    Options_manager_test(const Options_manager_test&) = delete;

    Options_manager_test& operator=(const Options_manager_test&) = delete;

    Options_manager_test(Options_manager_test&&) noexcept = delete;

    Options_manager_test& operator=(Options_manager_test&&) noexcept = delete;

protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(Options_manager_test, unit_test_add_options)
{
    // Test that add options adds new options
    Options_manager manager;
    manager.add_options(all_good_options);
    const std::unordered_map<std::string, std::string> copy_of_good_options_all_1{manager.get_options()};
    EXPECT_EQ(all_good_options, copy_of_good_options_all_1);

    // Test that check_options doesn't throw an exception
    manager.add_info(all_options_info_lookup);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());

    // Test that add_options merges options
    manager.reset();
    manager.add_info(all_options_info_lookup);
    manager.add_options(good_boolean_options);
    const std::unordered_map<std::string, std::string> copy_of_good_options_boolean_1{manager.get_options()};
    EXPECT_EQ(good_boolean_options, copy_of_good_options_boolean_1);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());

    manager.add_options(good_integer_options);
    manager.add_options(good_text_options);
    const std::unordered_map<std::string, std::string> merged_good_options_1{manager.get_options()};
    EXPECT_EQ(all_good_options, merged_good_options_1);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());

    // Test that add_info merges
    manager.reset();
    manager.add_info(boolean_options_info_lookup);
    manager.add_options(good_boolean_options);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());
    manager.add_options(good_integer_options);
    EXPECT_THROW_CONTAINS_MSG(manager.set_defaults_then_check_options(), Options_error, "Unknown option:");
    manager.add_info(integer_options_info_lookup);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());
    manager.add_options(good_text_options);
    EXPECT_THROW_CONTAINS_MSG(manager.set_defaults_then_check_options(), Options_error, "Unknown option:");
    manager.add_info(text_options_info_lookup);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());
    const std::unordered_map<std::string, std::string> merged_good_options_2{manager.get_options()};
    EXPECT_EQ(all_good_options, merged_good_options_2);
}

TEST_F(Options_manager_test, unit_test_add_options_from_command_line)
{
    Options_manager manager;
    const std::span<const char* const> args{good_options_all_cli.data(), good_options_all_cli.size()};
    manager.add_options_from_command_line(args);

    const std::unordered_map<std::string, std::string> copy_of_good_options_all_1{manager.get_options()};
    EXPECT_EQ(all_good_options, copy_of_good_options_all_1);

    manager.add_info(all_options_info_lookup);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());
}

TEST_F(Options_manager_test, unit_test_add_options_from_config_file)
{
    const c4lib::native::Path config_file{ctc::data_config_dir / c4lib::native::Path{"options-manager-test-config.xml"}};
    Options_manager manager;
    manager.add_options_from_config_file(config_file);

    const std::unordered_map<std::string, std::string> copy_of_good_options_all_1{manager.get_options()};
    EXPECT_EQ(all_good_options, copy_of_good_options_all_1);

    manager.add_info(all_options_info_lookup);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());
}

TEST_F(Options_manager_test, unit_test_get_options_exclusive_of)
{
    const c4lib::native::Path config_file{ctc::data_config_dir / c4lib::native::Path{"options-manager-test-config.xml"}};
    Options_manager manager;
    manager.add_options_from_config_file(config_file);

    const std::unordered_map<std::string, std::string> copy_of_good_options_all_1{manager.get_options()};
    EXPECT_EQ(all_good_options, copy_of_good_options_all_1);

    const std::unordered_map<std::string, std::string> good_options_less_text_1{
        manager.get_options_exclusive_of(text_options_info_lookup)};
    manager.reset();
    manager.add_options(good_options_less_text_1);
    const std::unordered_map<std::string, std::string> good_options_less_text_and_integers_1{
        manager.get_options_exclusive_of(integer_options_info_lookup)};
    EXPECT_EQ(good_boolean_options, good_options_less_text_and_integers_1);
}

TEST_F(Options_manager_test, unit_test_write_help_message)
{
    // Test that add options adds new options
    Options_manager manager;
    manager.add_options(all_good_options);
    const std::unordered_map<std::string, std::string> copy_of_good_options_all_1{manager.get_options()};
    EXPECT_EQ(all_good_options, copy_of_good_options_all_1);

    // Test that check_options doesn't throw an exception
    manager.add_info(all_options_info_lookup);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());

    // Add help options
    manager.add_options(good_option_help);

    // Change format for help message
    manager.set_help_format(help_fmt);

    std::string help;
    try {
        manager.set_defaults_then_check_options();
    }
    catch (const Display_help_error& ex) {
        help = ex.what();
    }

    EXPECT_STREQ(help.c_str(), help_message);
}

TEST_F(Options_manager_test, unit_test_bad_options)
{
    // Test bad boolean - boolean out of range
    Options_manager manager;
    manager.add_info(all_options_info_lookup);
    manager.add_options(bad_boolean_option_out_of_range);
    EXPECT_THROW_CONTAINS_MSG(manager.set_defaults_then_check_options(), Options_error, "must be '0' or '1'");

    // Test bad boolean - boolean as text
    manager.reset();
    manager.add_info(all_options_info_lookup);
    manager.add_options(bad_boolean_option_as_text);
    EXPECT_THROW_CONTAINS_MSG(manager.set_defaults_then_check_options(), Options_error, "must be '0' or '1'");

    // Test bad integer - integer out of range, overflow
    manager.reset();
    manager.add_info(all_options_info_lookup);
    manager.add_options(bad_integer_option_out_of_range_overflow);
    EXPECT_THROW_CONTAINS_MSG(manager.set_defaults_then_check_options(), Options_error, "value out of range");

    // Test bad integer - integer out of range, underflow
    manager.reset();
    manager.add_info(all_options_info_lookup);
    manager.add_options(bad_integer_option_out_of_range_underflow);
    EXPECT_THROW_CONTAINS_MSG(manager.set_defaults_then_check_options(), Options_error, "value out of range");

    // Test bad integer - integer out of range, underflow
    manager.reset();
    manager.add_info(all_options_info_lookup);
    manager.add_options(bad_integer_option_as_text);
    EXPECT_THROW_CONTAINS_MSG(manager.set_defaults_then_check_options(), Options_error, "must be an integer");

    // Test missing dependency
    manager.reset();
    manager.add_info(options_dependant_on_3_info_lookup);
    manager.add_options(good_dependant_option);
    EXPECT_THROW_CONTAINS_MSG(manager.set_defaults_then_check_options(), Options_error, "depends on option");

    manager.add_options(good_prerequisite_1_option);
    EXPECT_THROW_CONTAINS_MSG(manager.set_defaults_then_check_options(), Options_error, "depends on option");

    manager.add_options(good_prerequisite_2_option);
    EXPECT_THROW_CONTAINS_MSG(manager.set_defaults_then_check_options(), Options_error, "depends on option");

    manager.add_options(good_prerequisite_3_option);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());
}

TEST_F(Options_manager_test, unit_test_aggregate_info_requires_one_of)
{
    Options_manager manager;
    manager.add_aggregate_checks({
        {requires_one_of, check_requires_at_least_one_of},
    });
    manager.add_info(options_info_for_aggregate);
    EXPECT_THROW_CONTAINS_MSG(
        manager.set_defaults_then_check_options(), Options_error, "Required option from set not specified");

    manager.add_options(good_option_one_of_three_bool);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());

    manager.add_options(good_option_two_of_three_int);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());

    manager.add_options(good_option_three_of_three_text);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());
}

TEST_F(Options_manager_test, unit_test_aggregate_info_incompatible)
{
    Options_manager manager;
    manager.add_aggregate_checks({
        {incompatible_options, check_compatibility},
    });
    manager.add_info(options_info_for_aggregate);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());

    manager.add_options(good_option_one_of_three_bool);
    EXPECT_NO_THROW(manager.set_defaults_then_check_options());

    manager.add_options(good_option_two_of_three_int);
    EXPECT_THROW_CONTAINS_MSG(
        manager.set_defaults_then_check_options(), Options_error, "Incompatible options specified");

    manager.add_options(good_option_three_of_three_text);
    EXPECT_THROW_CONTAINS_MSG(
        manager.set_defaults_then_check_options(), Options_error, "Incompatible options specified");
}

} // namespace hankinsohl::options
