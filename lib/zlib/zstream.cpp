// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 8/15/2024.

#include <cassert>
#include <format>
#include <include/exceptions.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/zlib/zstream.hpp>
#include <string>
#include <zlib.h>

namespace c4lib::zlib {

ZStream::ZStream(ZStream::Type type, int level)
    : z_stream_s(), m_type(type)
{
    zalloc = nullptr;
    zfree = nullptr;
    opaque = nullptr;
    avail_in = 0;
    next_in = nullptr;

    int zreturn{limits::invalid_value};
    if (m_type == ZStream::Type::deflate) {
        zreturn = deflateInit(this, level);
    }
    else {
        zreturn = inflateInit(this);
    }

    if (zreturn != Z_OK) {
        const std::string error{std::format(fmt::zlib_initialization_error, zreturn)};
        throw ZLib_error{error};
    }
}

ZStream::~ZStream()
{
    if (m_type == ZStream::Type::deflate) {
        // We cannot check the return from deflateEnd because when deflate_ is used,
        // deflateEnd will return an error.  This is caused by Civ4's incorrect use of Z_SYNC_FLUSH
        // (use of Z_SYNC_FLUSH has been confirmed under a debugger) instead of Z_FINISH when processing
        // its final block.
        // Our implementation of deflate_ uses Z_SYNC_FLUSH instead of Z_FINISH in order to exactly
        // match the compressed data stream generated by Civ4.
        static_cast<void>(deflateEnd(this));
    }
    else {
#ifdef _DEBUG
        int zreturn{inflateEnd(this)};
        assert(zreturn == Z_OK);
#else
        static_cast<void>(inflateEnd(this));
#endif
    }
}

ZStream::operator z_stream*()
{
    return this;
}

ZStream::operator const z_stream*() const
{
    return this;
}

} // namespace c4lib::zlib
