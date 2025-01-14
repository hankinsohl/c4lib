// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/8/2024.

#include <gtest/gtest.h>
#include <lib/md5/md5.hpp>
#include <string>

class Md5_test : public testing::Test {
public:
    Md5_test() = default;

    ~Md5_test() override = default;

    Md5_test(const Md5_test&) = delete;

    Md5_test& operator=(const Md5_test&) = delete;

    Md5_test(Md5_test&&) noexcept = delete;

    Md5_test& operator=(Md5_test&&) noexcept = delete;

protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(Md5_test, unit_test_md5)
{
    MD5 md5;
    const std::string hash{md5("Hello world!")};
    EXPECT_EQ(hash, "86fb269d190d2c85f6e0468ceca42a20");
}
