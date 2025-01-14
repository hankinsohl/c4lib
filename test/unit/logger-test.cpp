// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/5/2024.

#include <filesystem>
#include <gtest/gtest.h>
#include <include/logger.hpp>
#include <iostream>
#include <lib/native/path.hpp>
#include <sstream>
#include <string>
#include <test/util/constants.hpp>

using namespace std::string_literals;
namespace ctc = c4lib::test::constants;

namespace c4lib {

class Logger_test : public testing::Test {
public:
    Logger_test() = default;

    ~Logger_test() override = default;

    Logger_test(const Logger_test&) = delete;

    Logger_test& operator=(const Logger_test&) = delete;

    Logger_test(Logger_test&&) noexcept = delete;

    Logger_test& operator=(Logger_test&&) noexcept = delete;

protected:
    void SetUp() override
    {
        Logger::start(std::cout, Logger::Severity::info);
    }

    void TearDown() override
    {
        Logger::stop();
    }
};

TEST_F(Logger_test, unit_test_logging)
{
    EXPECT_NO_THROW(Logger::info("This is an info message"));
    EXPECT_NO_THROW(Logger::info("This is an info message with {} argument", 1));
    EXPECT_NO_THROW(Logger::warn("This is a warning"));
    EXPECT_NO_THROW(Logger::warn("This is a warning with {} arguments and this is the 2nd one: {}", 2, "two"));
    EXPECT_NO_THROW(Logger::error("This is an error"));
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    EXPECT_NO_THROW(Logger::warn("This is an error message with arguments: {}, {}, {}", 1, "two", 3.0));
}

TEST_F(Logger_test, unit_test_start_with_stream)
{
    std::stringstream ss;
    Logger::start(ss, Logger::Severity::info);
    Logger::warn("This is a warning");
    EXPECT_NE(ss.str().find("This is a warning\n"), std::string::npos);
    // Stop the logger b/c ss is about to go out of scope.
    Logger::stop();
}

TEST_F(Logger_test, unit_test_start_with_filename)
{
    const native::Path test_file{ctc::out_common_dir / native::Path("logger_output.txt")};
    std::filesystem::remove(test_file);
    EXPECT_NO_THROW(Logger::start(test_file, Logger::Severity::info));
    EXPECT_NO_THROW(Logger::warn("This is a warning"));
    EXPECT_NO_THROW(Logger::info("This is an info message"));
    EXPECT_NO_THROW(Logger::info("This is an info message with {} argument", 1));
    EXPECT_NO_THROW(Logger::warn("This is a warning"));
    EXPECT_NO_THROW(Logger::warn("This is a warning with {} arguments and this is the 2nd one: {}", 2, "two"));
    EXPECT_NO_THROW(Logger::error("This is an error"));
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    EXPECT_NO_THROW(Logger::warn("This is an error message with arguments: {}, {}, {}", 1, "two", 3.0));
}

TEST_F(Logger_test, unit_test_set_threshold)
{
    std::stringstream ss;
    Logger::start(ss, Logger::Severity::info);

    Logger::set_threshold(Logger::Severity::info);
    ss.str("");
    ss.clear();
    Logger::info("This is an informational message");
    EXPECT_NE(ss.str().find("This is an informational message\n"), std::string::npos);
    ss.str("");
    ss.clear();
    Logger::warn("This is a warning");
    EXPECT_NE(ss.str().find("This is a warning\n"), std::string::npos);
    ss.str("");
    ss.clear();
    Logger::error("This is an error");
    EXPECT_NE(ss.str().find("This is an error\n"), std::string::npos);

    Logger::set_threshold(Logger::Severity::warn);
    ss.str("");
    ss.clear();
    Logger::info("This is an informational message");
    EXPECT_STREQ(ss.str().c_str(), "");
    ss.str("");
    ss.clear();
    Logger::warn("This is a warning");
    EXPECT_NE(ss.str().find("This is a warning\n"), std::string::npos);
    ss.str("");
    ss.clear();
    Logger::error("This is an error");
    EXPECT_NE(ss.str().find("This is an error\n"), std::string::npos);

    Logger::set_threshold(Logger::Severity::error);
    ss.str("");
    ss.clear();
    Logger::info("This is an informational message");
    EXPECT_STREQ(ss.str().c_str(), "");
    ss.str("");
    ss.clear();
    Logger::warn("This is a warning");
    EXPECT_STREQ(ss.str().c_str(), "");
    ss.str("");
    ss.clear();
    Logger::error("This is an error");
    EXPECT_NE(ss.str().find("This is an error\n"), std::string::npos);

    // Stop the logger b/c ss is about to go out of scope.
    Logger::stop();
}

} // namespace c4lib
