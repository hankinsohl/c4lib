// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/29/2024.

#include <cstddef>
#include <gtest/gtest.h>
#include <ios>
#include <lib/io/io.hpp>
#include <lib/native/path.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/options.hpp>
#include <lib/zlib/zlib-engine.hpp>
#include <sstream>
#include <string>
#include <test/util/constants.hpp>
#include <test/util/util.hpp>
#include <unordered_map>

using namespace std::string_literals;
namespace ctc = c4lib::test::constants;

namespace c4lib::zlib {

class ZLib_engine_test : public testing::Test {
public:
    ZLib_engine_test() = default;

    ~ZLib_engine_test() override = default;

    ZLib_engine_test(const ZLib_engine_test&) = delete;

    ZLib_engine_test& operator=(const ZLib_engine_test&) = delete;

    ZLib_engine_test(ZLib_engine_test&&) noexcept = delete;

    ZLib_engine_test& operator=(ZLib_engine_test&&) noexcept = delete;

protected:
    void SetUp() override
    {
        m_options[options::debug_write_binaries] = "1";
        m_options[options::debug_output_dir] = ctc::out_common_dir;
    }

    void TearDown() override {}

    std::unordered_map<std::string, std::string> m_options;
};

TEST_F(ZLib_engine_test, unit_test_inflate)
{
    ZLib_engine engine;
    size_t count_header{limits::invalid_size};
    size_t count_compressed{limits::invalid_size};
    size_t count_decompressed{limits::invalid_size};
    size_t count_footer{limits::invalid_size};
    size_t count_total{limits::invalid_size};
    const native::Path savegame{ctc::data_saves_dir / native::Path{"Brennus BC-4000-2.CivBeyondSwordSave"}};
    std::stringstream out;
    EXPECT_NO_THROW(engine.inflate(
        savegame, out, count_header, count_compressed, count_decompressed, count_footer, count_total, m_options));
}

TEST_F(ZLib_engine_test, unit_test_deflate)
{
    // Obtain a composite, decompressed stream by first inflating a savegame.
    ZLib_engine engine;
    size_t count_header_from_inflate{limits::invalid_size};
    size_t count_compressed_from_inflate{limits::invalid_size};
    size_t count_decompressed_from_inflate{limits::invalid_size};
    size_t count_footer{limits::invalid_size};
    size_t count_total{limits::invalid_size};
    const native::Path savegame{ctc::data_saves_dir / native::Path{"Brennus BC-4000-2.CivBeyondSwordSave"}};
    std::stringstream composite;
    EXPECT_NO_THROW(engine.inflate(savegame, composite, count_header_from_inflate, count_compressed_from_inflate,
        count_decompressed_from_inflate, count_footer, count_total, m_options));

    // Next, deflate the composite stream
    size_t count_header_from_deflate{limits::invalid_size};
    size_t count_compressed_from_deflate{limits::invalid_size};
    size_t count_decompressed_from_deflate{limits::invalid_size};
    std::stringstream compressed;
    EXPECT_NO_THROW(engine.deflate(savegame, composite, compressed, count_footer, count_header_from_deflate,
        count_compressed_from_deflate, count_decompressed_from_deflate, count_total, m_options));

    // Verify that the various output sizes obtained from inflate and deflate match.
    EXPECT_EQ(count_header_from_inflate, count_header_from_deflate);
    EXPECT_EQ(count_compressed_from_inflate, count_compressed_from_deflate);
    EXPECT_EQ(count_decompressed_from_inflate, count_decompressed_from_deflate);

    // Save stream to disk
    const std::string round_trip{ctc::out_common_dir / native::Path("zlib_engine_test.CivBeyondSwordSave")};
    io::write_binary_stream_to_file(compressed, 0, 0, round_trip);

    // Compare the original savegame to the savegame generated using deflate.
    std::stringstream original;
    original.unsetf(std::ios::skipws);
    io::read_binary_file_to_stream(savegame, 0, 0, original);
    std::stringstream errors;
    EXPECT_EQ(test::compare_binary_streams(original, compressed, errors), 0) << errors.str();
}

} // namespace c4lib::zlib
