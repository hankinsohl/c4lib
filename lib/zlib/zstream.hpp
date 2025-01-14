// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 8/15/2024.

#pragma once

#include <zlib.h>

namespace c4lib::zlib {

// Wrapper for z_stream.  Handles initialization and teardown.  Provides exception safety.
class ZStream : public z_stream {
public:
    enum class Type { inflate, deflate };

    // Construct a ZStream object.  If type is Inflate, the level parameter is not used.
    explicit ZStream(Type type, int level = Z_DEFAULT_COMPRESSION);

    ~ZStream();

    ZStream(const ZStream&) = delete;

    ZStream& operator=(const ZStream&) = delete;

    ZStream(ZStream&&) noexcept = delete;

    ZStream& operator=(ZStream&&) noexcept = delete;

    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    operator z_stream*();

    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    operator const z_stream*() const;

    Type m_type;
};

} // namespace c4lib::zlib
