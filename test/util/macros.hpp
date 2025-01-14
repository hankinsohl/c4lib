// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/6/2024.

#pragma once

// Check if expected exception is thrown and exception message contains expected message
#define EXPECT_THROW_CONTAINS_MSG(statement, expected_exception, expected_message)                                     \
    EXPECT_THROW(                                                                                                      \
        {                                                                                                              \
            try {                                                                                                      \
                statement;                                                                                             \
            }                                                                                                          \
            catch (const expected_exception& ex) {                                                                     \
                const std::string what{ex.what()};                                                                     \
                if (what.find(expected_message) == std::string::npos) {                                                \
                    EXPECT_STREQ(expected_message, ex.what());                                                         \
                }                                                                                                      \
                throw;                                                                                                 \
            }                                                                                                          \
        },                                                                                                             \
        expected_exception)
