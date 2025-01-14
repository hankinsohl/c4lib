// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/23/2024.

#include <algorithm>
#include <filesystem>
#include <format>
#include <gtest/gtest.h>
#include <lib/native/path.hpp>
#include <string>

namespace c4lib::test {

class Path_test : public testing::Test {
public:
    Path_test() = default;

    ~Path_test() override = default;

    Path_test(const Path_test&) = delete;

    Path_test& operator=(const Path_test&) = delete;

    Path_test(Path_test&&) noexcept = delete;

    Path_test& operator=(Path_test&&) noexcept = delete;

protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(Path_test, unit_test_path_directory_separator)
{
    const native::Path p1{"a/b"};
    const native::Path p2{"a\\b"};
    EXPECT_EQ(p1.c_str()[1], native::directory_separator);
    EXPECT_STREQ(p1.c_str(), p2.c_str());

#ifdef linux
    EXPECT_EQ(native::directory_separator, '/');
#else
    EXPECT_EQ(native::directory_separator, '\\');
#endif
}

TEST_F(Path_test, unit_test_remove_trailing_directory_separator)
{
    const native::Path p1{"a/b/"};
    EXPECT_EQ(static_cast<std::string>(p1).length(), 3);
}

TEST_F(Path_test, unit_test_mnt_conversion)
{
    const native::Path p1{R"(c:\Program Files)"};
    const native::Path p2{"/mnt/c/Program Files"};
    EXPECT_STREQ(p1.c_str(), p2.c_str());

    const std::string s{p1};
#ifdef linux
    EXPECT_EQ(s.find("/mnt/"), 0);

    // Test that drive conversion on Linux generates lowercase drive letters.
    native::Path p3{R"(C:\Program Files)"};
    EXPECT_STREQ(p3.c_str(), p2.c_str());
    EXPECT_EQ(p3.c_str()[5], 'c');

#else
    EXPECT_EQ(s.find("/mnt/"), std::string::npos);
#endif
}

TEST_F(Path_test, unit_test_conversion_to_string)
{
    std::string s1{"a/b/c"};
    std::ranges::replace(s1, '/', native::directory_separator);
    const std::string s2{native::Path{s1}};
    EXPECT_STREQ(s1.c_str(), s2.c_str());
}

TEST_F(Path_test, unit_test_conversion_to_filesystem_path)
{
    std::filesystem::path p1{"a/b/c"};
    p1.make_preferred();
    const std::filesystem::path p2{native::Path{"a/b/c"}};
    EXPECT_STREQ(p1.c_str(), p2.c_str());
}

TEST_F(Path_test, unit_test_operator_slash)
{
    const native::Path pa{"a"};
    const native::Path pb{"b"};
    const native::Path pc{"c"};
    const native::Path p_a_b_c1 = pa / pb / pc;
    const native::Path p_a_b_c2{"a/b/c"};
    EXPECT_STREQ(p_a_b_c1.c_str(), p_a_b_c2.c_str());
}

TEST_F(Path_test, unit_test_operator_slash_equals)
{
    const native::Path pa{"a"};
    const native::Path pb{"b"};
    const native::Path pc{"c"};
    const native::Path p_a_b_c1 = pa / pb / pc;
    native::Path p_a_b_c2{};
    p_a_b_c2 /= pa;
    p_a_b_c2 /= pb;
    p_a_b_c2 /= pc;
    EXPECT_STREQ(p_a_b_c1.c_str(), p_a_b_c2.c_str());
}

TEST_F(Path_test, unit_test_support_for_format)
{
    native::Path p{"a/b/c"};

    EXPECT_STREQ(p.c_str(), std::format("{}", p).c_str());
}

} // namespace c4lib::test
