// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 7/25/2024.

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <include/exceptions.hpp>
#include <ios>
#include <iosfwd>
#include <iterator>
#include <lib/io/io.hpp>
#include <lib/layout/layout.hpp>
#include <lib/native/compiler-support.hpp>
#include <lib/native/path.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/limits.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/options.hpp>
#include <lib/zlib/constants.hpp>
#include <lib/zlib/zlib-engine.hpp>
#include <lib/zlib/zstream.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <zconf.h>
#include <zlib.h>

using namespace std::literals;

namespace {

// N.B.: The zerrors array is defined such that zreturn + 6 can be used as an index
// Z_VERSION_ERROR = -6 and subsequent values increase by 1 thereafter.
// std::pair allows this array to be used for both zreturn-to-string,
// and string-to-zreturn.
const std::array zerrors{
    std::pair<int, std::string>{Z_VERSION_ERROR, "Z_VERSION_ERROR"},
    std::pair<int, std::string>{Z_BUF_ERROR, "Z_BUF_ERROR"},
    std::pair<int, std::string>{Z_MEM_ERROR, "Z_MEM_ERROR"},
    std::pair<int, std::string>{Z_DATA_ERROR, "Z_DATA_ERROR"},
    std::pair<int, std::string>{Z_STREAM_ERROR, "Z_STREAM_ERROR"},
    std::pair<int, std::string>{Z_ERRNO, "Z_ERRNO"},
    std::pair<int, std::string>{Z_OK, "Z_OK"},
    std::pair<int, std::string>{Z_STREAM_END, "Z_STREAM_END"},
    std::pair<int, std::string>{Z_NEED_DICT, "Z_NEED_DICT"},
};

} // namespace

namespace c4lib::zlib {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
native::Path ZLib_engine::create_binary_filename(const native::Path& output_dir,
    const native::Path& original,
    const std::string& suffix,
    const std::string& extension)
{
    return create_base_binary_path_(output_dir, original, suffix).append(extension);
}

void ZLib_engine::deflate(const native::Path& savegame,
    std::istream& in,
    std::iostream& out,
    size_t count_footer,
    size_t& count_header,
    size_t& count_compressed,
    size_t& count_decompressed,
    size_t& count_total,
    std::unordered_map<std::string, std::string>& options)
{
    const ssize_t scount_footer{gsl::narrow<ssize_t>(count_footer)};
    ssize_t scount_header{gsl::narrow<ssize_t>(count_header)};
    ssize_t scount_compressed{gsl::narrow<ssize_t>(count_compressed)};
    ssize_t scount_decompressed{gsl::narrow<ssize_t>(count_decompressed)};
    ssize_t scount_total{gsl::narrow<ssize_t>(count_total)};

    deflate_(
        savegame, in, out, scount_footer, scount_header, scount_compressed, scount_decompressed, scount_total, options);

    count_header = gsl::narrow<size_t>(scount_header);
    count_compressed = gsl::narrow<size_t>(scount_compressed);
    count_decompressed = gsl::narrow<size_t>(scount_decompressed);
    count_total = gsl::narrow<size_t>(scount_total);
}

void ZLib_engine::inflate(const native::Path& savegame,
    std::iostream& out,
    size_t& count_header,
    size_t& count_compressed,
    size_t& count_decompressed,
    size_t& count_footer,
    size_t& count_total,
    std::unordered_map<std::string, std::string>& options)
{
    ssize_t scount_header{gsl::narrow<ssize_t>(count_header)};
    ssize_t scount_compressed{gsl::narrow<ssize_t>(count_compressed)};
    ssize_t scount_decompressed{gsl::narrow<ssize_t>(count_decompressed)};
    ssize_t scount_footer{gsl::narrow<ssize_t>(count_footer)};
    ssize_t scount_total{gsl::narrow<ssize_t>(count_total)};

    inflate_(
        savegame, out, scount_header, scount_compressed, scount_decompressed, scount_footer, scount_total, options);

    count_header = gsl::narrow<size_t>(scount_header);
    count_compressed = gsl::narrow<size_t>(scount_compressed);
    count_decompressed = gsl::narrow<size_t>(scount_decompressed);
    count_footer = gsl::narrow<size_t>(scount_footer);
    count_total = gsl::narrow<size_t>(scount_total);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
native::Path ZLib_engine::create_base_binary_path_(
    const native::Path& output_dir, const native::Path& original, const std::string& suffix)
{
    std::filesystem::path base_filename{std::filesystem::path{original}.replace_extension().filename()};
    base_filename += suffix;
    return output_dir / native::Path{base_filename};
}

void ZLib_engine::deflate_(const native::Path& savegame,
    std::istream& in,
    std::iostream& out,
    ssize_t count_footer,
    ssize_t& count_header,
    ssize_t& count_compressed,
    ssize_t& count_decompressed,
    ssize_t& count_total,
    std::unordered_map<std::string, std::string>& options)
{
    m_filename = savegame;

    // Get the offset to compressed data.
    m_compressed_data_offset = layout::get_civ4_compressed_data_offset(in, false);

    // The zlib offset is 4 bytes beyond the offset to compressed data.
    m_zlib_magic_offset = m_compressed_data_offset + 4LL;

    // Copy the uncompressed game header into the output stream.
    in.seekg(0);
    std::copy_n(std::istreambuf_iterator<char>{in}, m_compressed_data_offset, std::ostream_iterator<char>{out});

    // Deflate the Civ4 decompressed data, appending the deflated data to the file-memory stream.  Note that Civ4 writes
    // compressed data in chunks.  The size of the compressed data chunk is written, followed by the compressed data
    // proper.  Each chunk is 64K bytes long except for the last chunk which is shorter.  Due to the layout above,
    // straight-forward deflation of the compressed data will not work as chunk lengths are interspersed with
    // compressed data.  The ZLib_engine deflate_ method accommodates this layout.
    ZLib_engine zlib_engine;
    // Civ4 writes a 4-byte pad field prior to the deflated data proper.  Skip past the padding in the input stream.
    constexpr std::streamoff pad_size{4};
    zlib_engine.deflate_(
        in, out, m_compressed_data_offset + pad_size, count_footer, m_size_compressed, m_size_decompressed);

    // Copy the uncompressed game footer to the output stream
    in.seekg(m_compressed_data_offset + pad_size + m_size_decompressed);
    std::copy(std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}, std::ostream_iterator<char>{out});

    count_header = m_compressed_data_offset;
    count_compressed = m_size_compressed;
    count_decompressed = m_size_decompressed;
    count_total = out.tellp();

    if (!in || !out) {
        throw std::runtime_error{std::format(fmt::runtime_error_io, "deflate")};
    }

    if (options[options::debug_write_binaries] == "1") {
        write_binaries_(native::Path{options[options::debug_output_dir]}, constants::deflate_binaries_suffix, out, in);
    }
}

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)
void ZLib_engine::deflate_(std::istream& in,
    std::ostream& out,
    std::streampos offset,
    ssize_t count_footer,
    ssize_t& count_compressed_written,
    ssize_t& count_decompressed)
{
    count_compressed_written = 0;
    count_decompressed = 0;

    // Civilization 4 compresses data by calling zlib deflate numerous times, once for each simple type backed
    // with compressed data.  We use a similar technique below to exactly match the compressed data produced by
    // Civ4 except that we process uncompressed data in 2K chunks at a time.
    static constexpr ssize_t max_uncompressed_chunk_size{1UL << 11UL}; // 2K

    // Compute the number of decompressed bytes in the composite input stream
    const std::streampos in_count_total{io::stream_size(in)};
    if (offset + count_footer > in_count_total) {
        m_zreturn = Z_ERRNO;
        throw std::logic_error(fmt::bad_file_offset);
    }
    const ssize_t count_to_decompress{in_count_total - offset - count_footer};

    // To simplify our implementation we will first compress all decompressed bytes
    // into a single buffer.  Get an upper bound for the size of the compressed buffer needed
    // and allocate the buffer.
    const static ssize_t compressed_buffer_size{
        gsl::narrow<ssize_t>(compressBound(gsl::narrow<uLong>(count_to_decompress)))};
    std::vector<uint8_t> compressed_buffer(gsl::narrow<size_t>(compressed_buffer_size)); // use () for initialization

    // We will also need a staging buffer.  The staging buffer is used to store uncompressed
    // data read from the input stream prior to compression.
    static constexpr ssize_t staging_buffer_size{max_uncompressed_chunk_size};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    std::array<uint8_t, staging_buffer_size> staging_buffer;

    // Position the input stream to the offset where decompressed data begins.
    in.seekg(offset);
    if (!in) {
        m_zreturn = Z_ERRNO;
        throw std::runtime_error{fmt::runtime_error_seek};
    }

    // Initialize z_stream structure for deflation.
    ZStream zstream{ZStream::Type::deflate};
    if (m_zreturn != Z_OK) {
        const std::string error{std::format(
            fmt::zlib_error_deflate_init, zreturn_to_string(m_zreturn), m_zreturn, zstream_message_(zstream.msg))};
        throw ZLib_error{error};
    }

    // Compress data into the buffer until all uncompressed bytes have been processed.  The input
    // data stream contains the uncompressed civ4 header immediately following the bytes which
    // are to be compressed; it's therefore possible that all uncompressed bytes will be processed
    // prior to EOF.
    //
    // The algorithm below mirrors that used by Civ4.  In particular, it does not call Z_FINISH for
    // the last chuck, but instead uses
    uint32_t count_decompressed_ul{0};
    uint32_t count_decompressed_remaining_ul{gsl::narrow<uint32_t>(count_to_decompress)};
    uint32_t avail_out{gsl::narrow<uint32_t>(compressed_buffer_size)};
    uint8_t* next_out{compressed_buffer.data()};
    uint32_t count_compressed_by_zlib{0};
    int flush{Z_NO_FLUSH};
    do {
        // Read the next uncompressed data chunk into the staging buffer
        const uint32_t chunk_size{
            std::min(gsl::narrow<uint32_t>(staging_buffer_size), count_decompressed_remaining_ul)};
        in.read(reinterpret_cast<char*>(staging_buffer.data()), chunk_size);
        zstream.avail_in = gsl::narrow<uInt>(in.gcount());
        zstream.next_in = staging_buffer.data();
        if (!in && in.gcount() == 0) {
            // Note: If read attempts to read past EOF, the EOF and fail bits are set
            // and the amount of data that was copied prior to EOF is reflected by gcount.
            // To distinguish between complete read failure and a successful partial
            // read we must check both in and in.gcount.
            m_zreturn = Z_ERRNO;
            throw std::runtime_error{fmt::runtime_error_read};
        }
        count_decompressed_ul += gsl::narrow<uint32_t>(in.gcount());
        count_decompressed_remaining_ul -= gsl::narrow<uint32_t>(in.gcount());

        // N.B:
        // Z_SYNC_FLUSH is intentionally used instead of Z_FINISH, even though this use is
        // produces output that is not conformant with the zlib standard, because
        // use of Z_SYNC_FLUSH produces output consistent with that generated by Civ4,
        // whereas use of Z_FINISH does not.  Civ4's zlib output is certainly non-conformant
        // but its use of Z_SYNC_FLUSH has been confirmed under a debugger.
        flush = count_decompressed_remaining_ul ? Z_NO_FLUSH : Z_SYNC_FLUSH;

        zstream.avail_out = avail_out;
        zstream.next_out = next_out;
        m_zreturn = ::deflate(&zstream, flush); // no bad return value
        assert(m_zreturn != Z_STREAM_ERROR); // state not clobbered
        const uint32_t count_compressed_this_time_ul{avail_out - zstream.avail_out};
        avail_out -= count_compressed_this_time_ul;

        // avail_out should never reach zero because the single output buffer was sized to accommodate
        // all decompressed data using compressBound.  We'll check avail_out as a safety precaution.
        if (avail_out == 0) {
            throw ZLib_error(std::format(fmt::out_of_range_error, "avail_out", "deflate_"));
        }

        // Also check the output buffer as a safety precaution
        next_out += count_compressed_this_time_ul;
        if (next_out < compressed_buffer.data()
            || next_out + avail_out > compressed_buffer.data() + compressed_buffer_size) {
            throw ZLib_error(std::format(fmt::out_of_range_error, "next_out", "deflate_"));
        }
        count_compressed_by_zlib += count_compressed_this_time_ul;

        assert(zstream.avail_in == 0);
    }
    while (count_decompressed_remaining_ul);

    // Copy compressed data to the savegame.
    count_compressed_written = 0;
    ssize_t count_chunk_size_fields{0};
    // Use uint32_t b/c chunk_size is written to the stream
    uint32_t chunk_size{0};
    while (count_compressed_written < count_compressed_by_zlib) {
        const uint32_t count_remaining{count_compressed_by_zlib - gsl::narrow<uint32_t>(count_compressed_written)};
        chunk_size = std::min(count_remaining, constants::max_chunk_size);

        // Write the current compressed data chunk to the savegame, preceded by the 4-byte chunk length.
        io::write_bytes(out, reinterpret_cast<const char*>(&chunk_size), sizeof(chunk_size));
        ++count_chunk_size_fields;
        io::write_bytes(
            out, reinterpret_cast<const char*>(compressed_buffer.data()) + count_compressed_written, chunk_size);
        count_compressed_written += chunk_size;
    }

    // Write the final chunk size (the size will be zero).
    chunk_size = 0;
    io::write_bytes(out, reinterpret_cast<const char*>(&chunk_size), sizeof(chunk_size));
    ++count_chunk_size_fields;
    if (!out) {
        m_zreturn = Z_ERRNO;
        throw std::runtime_error{fmt::runtime_error_write};
    }

    // Update output size parameters.  Note that count_compressed_written includes the chunk sizes.
    count_compressed_written += count_chunk_size_fields * gsl::narrow<ssize_t>(sizeof(uint32_t));
    count_decompressed = count_decompressed_ul;

    // The state of the input stream is likely to reflect a successful partial read
    // past EOF.  Clear the input stream flags as this condition is expected and not
    // indicative of error.
    in.clear();
}

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)
void ZLib_engine::inflate_(const native::Path& savegame,
    std::iostream& out,
    ssize_t& count_header,
    ssize_t& count_compressed,
    ssize_t& count_decompressed,
    ssize_t& count_footer,
    ssize_t& count_total,
    std::unordered_map<std::string, std::string>& options)
{
    m_filename = savegame;

    // Open the save in binary mode and clear the whitespace removal flag.
    std::ifstream file{savegame, std::ios_base::in | std::ios_base::binary};
    if (!file.is_open() || file.bad()) {
        throw std::runtime_error{std::format(fmt::runtime_error_opening_file, savegame)};
    }
    file.unsetf(std::ios::skipws);

    // Get the offset to compressed data.
    m_compressed_data_offset = layout::get_civ4_compressed_data_offset(file, true);

    // The zlib offset is 4 bytes beyond the offset to compressed data.
    m_zlib_magic_offset = m_compressed_data_offset + 4LL;

    // Copy the uncompressed game header into the composite game copy.
    file.seekg(0);
    std::copy_n(std::istreambuf_iterator<char>{file}, m_compressed_data_offset, std::ostream_iterator<char>{out});

    // Civ4 writes a 4-byte pad field prior to the inflated data proper.  The pad value is 0.
    uint32_t pad{0};
    out.write(reinterpret_cast<char*>(&pad), sizeof(pad));

    // Inflate the Civ4 compressed data, appending the inflated data to the file-memory stream.  Note that Civ4 writes
    // compressed data in chunks.  The size of the compressed data chunk is written, followed by the compressed data
    // proper.  Each chunk is 64K bytes long except for the last chunk which is shorter.  Due to the layout above,
    // straight-forward inflation of the compressed data will not work as chunk lengths are interspersed with
    // compressed data.  The ZLib_engine inflate_ method accommodates this layout.
    ZLib_engine zlib_engine;
    zlib_engine.inflate_(file, out, m_compressed_data_offset, m_size_compressed, m_size_decompressed);

    // Copy the uncompressed game footer into the composite game copy.
    file.seekg(m_compressed_data_offset + m_size_compressed);
    const std::streampos pos_footer_begin{file.tellg()};
    std::copy(std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}, std::ostream_iterator<char>{out});
    const std::streampos pos_footer_end{file.tellg()};

    count_header = m_compressed_data_offset;
    count_compressed = m_size_compressed;
    count_decompressed = m_size_decompressed;
    count_footer = pos_footer_end - pos_footer_begin;
    count_total = out.tellp();

    if (!file || !out) {
        throw std::runtime_error{std::format(fmt::runtime_error_io, "inflate")};
    }

    if (options[options::debug_write_binaries] == "1") {
        write_binaries_(
            native::Path{options[options::debug_output_dir]}, constants::inflate_binaries_suffix, file, out);
    }
}

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)
void ZLib_engine::inflate_(
    std::istream& in, std::ostream& out, std::streampos offset, ssize_t& count_compressed, ssize_t& count_decompressed)
{
    count_compressed = 0;
    count_decompressed = 0;

    // Position input stream to the offset.  Note that offset points to a 4-byte size value for the first chunk
    // of compressed data.
    in.seekg(offset);
    if (!in) {
        m_zreturn = Z_ERRNO;
        throw std::runtime_error{fmt::runtime_error_seek};
    }

    // Allocate input and output buffers.
    auto in_buffer{std::make_unique<std::array<uint8_t, constants::buffer_size>>()};
    auto out_buffer{std::make_unique<std::array<uint8_t, constants::buffer_size>>()};
    // Civ4 writes compressed data in 64K chunks.  Ensure that buffer_size accommodates a full chunk of compressed data.
    assert(constants::buffer_size >= 0x10000);

    // Initialize z_stream structure for inflation.
    ZStream zstream{ZStream::Type::inflate};
    if (m_zreturn != Z_OK) {
        const std::string error{std::format(
            fmt::zlib_error_inflate_init, zreturn_to_string(m_zreturn), m_zreturn, zstream_message_(zstream.msg))};
        throw ZLib_error{error};
    }

    int count_chunks{0};
    do {
        // Read the next chunk size.
        uint32_t count_to_read{0};
        in.read(reinterpret_cast<char*>(&count_to_read), sizeof(count_to_read));
        if (!in) {
            m_zreturn = Z_ERRNO;
            throw std::runtime_error{fmt::runtime_error_read};
        }

        // Verify that count_to_read <= buffer_size.
        if (count_to_read > constants::buffer_size) {
            m_zreturn = Z_ERRNO;
            throw std::out_of_range{std::format(fmt::out_of_range_error, "Chunk size", "inflate_")};
        }

        in.read(reinterpret_cast<char*>(in_buffer.get()), gsl::narrow<int>(count_to_read));
        if (!in) {
            m_zreturn = Z_ERRNO;
            throw std::runtime_error{fmt::runtime_error_read};
        }
        ++count_chunks;
        zstream.avail_in = gsl::narrow<uInt>(in.gcount());
        if (zstream.avail_in == 0) {
            break;
        }
        zstream.next_in = in_buffer->data();

        // run inflate on input until output buffer not full
        uint32_t have{0};
        do {
            zstream.avail_out = constants::buffer_size;
            zstream.next_out = out_buffer->data();
            m_zreturn = ::inflate(&zstream, Z_NO_FLUSH);
            // state not clobbered
            assert(m_zreturn != Z_STREAM_ERROR);
            switch (m_zreturn) {
            case Z_NEED_DICT:
            case Z_MEM_ERROR:
            case Z_DATA_ERROR:
                throw ZLib_error{std::format(
                    fmt::zlib_error_inflate, zreturn_to_string(m_zreturn), m_zreturn, zstream_message_(zstream.msg))};
                break;

            default:
                // Fall through
                break;
            }
            have = constants::buffer_size - zstream.avail_out;
            out.write(reinterpret_cast<char*>(out_buffer.get()), have);
            if (!out) {
                m_zreturn = Z_ERRNO;
                throw std::runtime_error{fmt::runtime_error_write};
            }
        }
        while (zstream.avail_out == 0);

        // done when inflate says it's done
    }
    while (m_zreturn != Z_STREAM_END);

    // Update counts and return
    count_compressed = count_chunks * 4 + gsl::narrow<ssize_t>(zstream.total_in);
    count_decompressed = gsl::narrow<ssize_t>(zstream.total_out);
}

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)

// Writes various copies of the saved game to disk to facilitate testing/debugging.
void ZLib_engine::write_binaries_(
    const native::Path& dir, const std::string& suffix, std::istream& original, std::istream& composite) const
{
    const native::Path base_path{create_base_binary_path_(dir, m_filename, suffix)};

    struct Associations {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
        std::istream& source;
        std::streamoff offset{0};
        ssize_t size{limits::invalid_size};
        native::Path path;
    };

    std::array associations{
        Associations{
            .source = original, .offset = 0, .size = 0, .path = base_path.append_to_copy(constants::original_ext)},
        Associations{
            .source = composite, .offset = 0, .size = 0, .path = base_path.append_to_copy(constants::composite_ext)},
        Associations{.source = original,
            .offset = m_compressed_data_offset,
            .size = m_size_compressed,
            .path = base_path.append_to_copy(constants::compressed_ext)},
        Associations{.source = composite,
            .offset = m_zlib_magic_offset,
            .size = m_size_decompressed,
            .path = base_path.append_to_copy(constants::decompressed_ext)},
        Associations{.source = original,
            .offset = m_compressed_data_offset + m_size_compressed,
            .size = 0,
            .path = base_path.append_to_copy(constants::footer_ext)},
        Associations{.source = original,
            .offset = 0,
            .size = m_compressed_data_offset,
            .path = base_path.append_to_copy(constants::header_ext)},
    };

    for (auto& a : associations) {
        // Source may have EOF bit set at this point.  Clear flags so that source is available for use.
        a.source.clear();
        io::write_binary_stream_to_file(a.source, a.offset, gsl::narrow<size_t>(a.size), a.path);
    }
}

const std::string& ZLib_engine::zreturn_to_string(int zreturn)
{
    // Z_VERSION_ERROR is -6.  To use zreturn as an index we need to add + 6.
    return zerrors.at(gsl::narrow<size_t>(-Z_VERSION_ERROR + zreturn)).second;
}

const char* ZLib_engine::zstream_message_(const char* msg)
{
    if (msg == nullptr) {
        msg = constants::null_message;
    }
    return msg;
}

} // namespace c4lib::zlib
