// //////////////////////////////////////////////////////////
// md5.cpp
// Copyright (c) 2014,2015 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//
// Source altered by Hankinsohl
//     * changed file extension from .h to .hpp
//     * updated include to work with c4lib directory structure.
//     * commented out use of <endian.h>.
//     * added pragmas to disable some compiler warnings
//     * disabled clang-tidy warnings

#include <lib/md5/md5.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

// NOLINTBEGIN

// The following was commented out
// #ifndef _MSC_VER
// #include <endian.h>
// #endif

/// same as reset()
MD5::MD5()
{
    reset();
}

/// restart
void MD5::reset()
{
    m_numBytes = 0;
    m_bufferSize = 0;

    // according to RFC 1321
    m_hash[0] = 0x67'45'23'01;
    m_hash[1] = 0xef'cd'ab'89;
    m_hash[2] = 0x98'ba'dc'fe;
    m_hash[3] = 0x10'32'54'76;
}

namespace {
// mix functions for processBlock()
inline uint32_t f1(uint32_t b, uint32_t c, uint32_t d)
{
    return d ^ (b & (c ^ d)); // original: f = (b & c) | ((~b) & d);
}

inline uint32_t f2(uint32_t b, uint32_t c, uint32_t d)
{
    return c ^ (d & (b ^ c)); // original: f = (b & d) | (c & (~d));
}

inline uint32_t f3(uint32_t b, uint32_t c, uint32_t d)
{
    return b ^ c ^ d;
}

inline uint32_t f4(uint32_t b, uint32_t c, uint32_t d)
{
    return c ^ (b | ~d);
}

inline uint32_t rotate(uint32_t a, uint32_t c)
{
    return (a << c) | (a >> (32 - c));
}

#if defined(__BYTE_ORDER) && (__BYTE_ORDER != 0) && (__BYTE_ORDER == __BIG_ENDIAN)
inline uint32_t swap(uint32_t x)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(x);
#endif
#ifdef MSC_VER
    return _byteswap_ulong(x);
#endif

    return (x >> 24) | ((x >> 8) & 0x00'00'FF'00) | ((x << 8) & 0x00'FF'00'00) | (x << 24);
}
#endif
} // namespace

/// process 64 bytes
void MD5::processBlock(const void* data)
{
    // get last hash
    uint32_t a = m_hash[0];
    uint32_t b = m_hash[1];
    uint32_t c = m_hash[2];
    uint32_t d = m_hash[3];

    // data represented as 16x 32-bit words
    const uint32_t* words = (uint32_t*)data;

    // computations are little endian, swap data if necessary
#if defined(__BYTE_ORDER) && (__BYTE_ORDER != 0) && (__BYTE_ORDER == __BIG_ENDIAN)
#define LITTLEENDIAN(x) swap(x)
#else
#define LITTLEENDIAN(x) (x)
#endif

    // first round
    uint32_t word0 = LITTLEENDIAN(words[0]);
    a = rotate(a + f1(b, c, d) + word0 + 0xd7'6a'a4'78, 7) + b;
    uint32_t word1 = LITTLEENDIAN(words[1]);
    d = rotate(d + f1(a, b, c) + word1 + 0xe8'c7'b7'56, 12) + a;
    uint32_t word2 = LITTLEENDIAN(words[2]);
    c = rotate(c + f1(d, a, b) + word2 + 0x24'20'70'db, 17) + d;
    uint32_t word3 = LITTLEENDIAN(words[3]);
    b = rotate(b + f1(c, d, a) + word3 + 0xc1'bd'ce'ee, 22) + c;

    uint32_t word4 = LITTLEENDIAN(words[4]);
    a = rotate(a + f1(b, c, d) + word4 + 0xf5'7c'0f'af, 7) + b;
    uint32_t word5 = LITTLEENDIAN(words[5]);
    d = rotate(d + f1(a, b, c) + word5 + 0x47'87'c6'2a, 12) + a;
    uint32_t word6 = LITTLEENDIAN(words[6]);
    c = rotate(c + f1(d, a, b) + word6 + 0xa8'30'46'13, 17) + d;
    uint32_t word7 = LITTLEENDIAN(words[7]);
    b = rotate(b + f1(c, d, a) + word7 + 0xfd'46'95'01, 22) + c;

    uint32_t word8 = LITTLEENDIAN(words[8]);
    a = rotate(a + f1(b, c, d) + word8 + 0x69'80'98'd8, 7) + b;
    uint32_t word9 = LITTLEENDIAN(words[9]);
    d = rotate(d + f1(a, b, c) + word9 + 0x8b'44'f7'af, 12) + a;
    uint32_t word10 = LITTLEENDIAN(words[10]);
    c = rotate(c + f1(d, a, b) + word10 + 0xff'ff'5b'b1, 17) + d;
    uint32_t word11 = LITTLEENDIAN(words[11]);
    b = rotate(b + f1(c, d, a) + word11 + 0x89'5c'd7'be, 22) + c;

    uint32_t word12 = LITTLEENDIAN(words[12]);
    a = rotate(a + f1(b, c, d) + word12 + 0x6b'90'11'22, 7) + b;
    uint32_t word13 = LITTLEENDIAN(words[13]);
    d = rotate(d + f1(a, b, c) + word13 + 0xfd'98'71'93, 12) + a;
    uint32_t word14 = LITTLEENDIAN(words[14]);
    c = rotate(c + f1(d, a, b) + word14 + 0xa6'79'43'8e, 17) + d;
    uint32_t word15 = LITTLEENDIAN(words[15]);
    b = rotate(b + f1(c, d, a) + word15 + 0x49'b4'08'21, 22) + c;

    // second round
    a = rotate(a + f2(b, c, d) + word1 + 0xf6'1e'25'62, 5) + b;
    d = rotate(d + f2(a, b, c) + word6 + 0xc0'40'b3'40, 9) + a;
    c = rotate(c + f2(d, a, b) + word11 + 0x26'5e'5a'51, 14) + d;
    b = rotate(b + f2(c, d, a) + word0 + 0xe9'b6'c7'aa, 20) + c;

    a = rotate(a + f2(b, c, d) + word5 + 0xd6'2f'10'5d, 5) + b;
    d = rotate(d + f2(a, b, c) + word10 + 0x02'44'14'53, 9) + a;
    c = rotate(c + f2(d, a, b) + word15 + 0xd8'a1'e6'81, 14) + d;
    b = rotate(b + f2(c, d, a) + word4 + 0xe7'd3'fb'c8, 20) + c;

    a = rotate(a + f2(b, c, d) + word9 + 0x21'e1'cd'e6, 5) + b;
    d = rotate(d + f2(a, b, c) + word14 + 0xc3'37'07'd6, 9) + a;
    c = rotate(c + f2(d, a, b) + word3 + 0xf4'd5'0d'87, 14) + d;
    b = rotate(b + f2(c, d, a) + word8 + 0x45'5a'14'ed, 20) + c;

    a = rotate(a + f2(b, c, d) + word13 + 0xa9'e3'e9'05, 5) + b;
    d = rotate(d + f2(a, b, c) + word2 + 0xfc'ef'a3'f8, 9) + a;
    c = rotate(c + f2(d, a, b) + word7 + 0x67'6f'02'd9, 14) + d;
    b = rotate(b + f2(c, d, a) + word12 + 0x8d'2a'4c'8a, 20) + c;

    // third round
    a = rotate(a + f3(b, c, d) + word5 + 0xff'fa'39'42, 4) + b;
    d = rotate(d + f3(a, b, c) + word8 + 0x87'71'f6'81, 11) + a;
    c = rotate(c + f3(d, a, b) + word11 + 0x6d'9d'61'22, 16) + d;
    b = rotate(b + f3(c, d, a) + word14 + 0xfd'e5'38'0c, 23) + c;

    a = rotate(a + f3(b, c, d) + word1 + 0xa4'be'ea'44, 4) + b;
    d = rotate(d + f3(a, b, c) + word4 + 0x4b'de'cf'a9, 11) + a;
    c = rotate(c + f3(d, a, b) + word7 + 0xf6'bb'4b'60, 16) + d;
    b = rotate(b + f3(c, d, a) + word10 + 0xbe'bf'bc'70, 23) + c;

    a = rotate(a + f3(b, c, d) + word13 + 0x28'9b'7e'c6, 4) + b;
    d = rotate(d + f3(a, b, c) + word0 + 0xea'a1'27'fa, 11) + a;
    c = rotate(c + f3(d, a, b) + word3 + 0xd4'ef'30'85, 16) + d;
    b = rotate(b + f3(c, d, a) + word6 + 0x04'88'1d'05, 23) + c;

    a = rotate(a + f3(b, c, d) + word9 + 0xd9'd4'd0'39, 4) + b;
    d = rotate(d + f3(a, b, c) + word12 + 0xe6'db'99'e5, 11) + a;
    c = rotate(c + f3(d, a, b) + word15 + 0x1f'a2'7c'f8, 16) + d;
    b = rotate(b + f3(c, d, a) + word2 + 0xc4'ac'56'65, 23) + c;

    // fourth round
    a = rotate(a + f4(b, c, d) + word0 + 0xf4'29'22'44, 6) + b;
    d = rotate(d + f4(a, b, c) + word7 + 0x43'2a'ff'97, 10) + a;
    c = rotate(c + f4(d, a, b) + word14 + 0xab'94'23'a7, 15) + d;
    b = rotate(b + f4(c, d, a) + word5 + 0xfc'93'a0'39, 21) + c;

    a = rotate(a + f4(b, c, d) + word12 + 0x65'5b'59'c3, 6) + b;
    d = rotate(d + f4(a, b, c) + word3 + 0x8f'0c'cc'92, 10) + a;
    c = rotate(c + f4(d, a, b) + word10 + 0xff'ef'f4'7d, 15) + d;
    b = rotate(b + f4(c, d, a) + word1 + 0x85'84'5d'd1, 21) + c;

    a = rotate(a + f4(b, c, d) + word8 + 0x6f'a8'7e'4f, 6) + b;
    d = rotate(d + f4(a, b, c) + word15 + 0xfe'2c'e6'e0, 10) + a;
    c = rotate(c + f4(d, a, b) + word6 + 0xa3'01'43'14, 15) + d;
    b = rotate(b + f4(c, d, a) + word13 + 0x4e'08'11'a1, 21) + c;

    a = rotate(a + f4(b, c, d) + word4 + 0xf7'53'7e'82, 6) + b;
    d = rotate(d + f4(a, b, c) + word11 + 0xbd'3a'f2'35, 10) + a;
    c = rotate(c + f4(d, a, b) + word2 + 0x2a'd7'd2'bb, 15) + d;
    b = rotate(b + f4(c, d, a) + word9 + 0xeb'86'd3'91, 21) + c;

    // update hash
    m_hash[0] += a;
    m_hash[1] += b;
    m_hash[2] += c;
    m_hash[3] += d;
}

/// add arbitrary number of bytes
void MD5::add(const void* data, size_t numBytes)
{
    const uint8_t* current = (const uint8_t*)data;

    if (m_bufferSize > 0) {
        while (numBytes > 0 && m_bufferSize < BlockSize) {
            m_buffer[m_bufferSize++] = *current++;
            numBytes--;
        }
    }

    // full buffer
    if (m_bufferSize == BlockSize) {
        processBlock(m_buffer);
        m_numBytes += BlockSize;
        m_bufferSize = 0;
    }

    // no more data ?
    if (numBytes == 0)
        return;

    // process full blocks
    while (numBytes >= BlockSize) {
        processBlock(current);
        current += BlockSize;
        m_numBytes += BlockSize;
        numBytes -= BlockSize;
    }

    // keep remaining bytes in buffer
    while (numBytes > 0) {
// Suppress spurious buffer overrun warning from MSVC when compiling with /analyze
#pragma warning(suppress : 6386)
        m_buffer[m_bufferSize++] = *current++;
        numBytes--;
    }
}

/// process final block, less than 64 bytes
void MD5::processBuffer()
{
    // the input bytes are considered as bits strings, where the first bit is the most significant bit of the byte

    // - append "1" bit to message
    // - append "0" bits until message length in bit mod 512 is 448
    // - append length as 64 bit integer

    // number of bits
    size_t paddedLength = m_bufferSize * 8;

    // plus one bit set to 1 (always appended)
    paddedLength++;

    // number of bits must be (numBits % 512) = 448
    size_t lower11Bits = paddedLength & 511;
    if (lower11Bits <= 448)
        paddedLength += 448 - lower11Bits;
    else
        paddedLength += 512 + 448 - lower11Bits;
    // convert from bits to bytes
    paddedLength /= 8;

    // only needed if additional data flows over into a second block
    unsigned char extra[BlockSize];

    // append a "1" bit, 128 => binary 10000000
    if (m_bufferSize < BlockSize)
        m_buffer[m_bufferSize] = 128;
    else
        extra[0] = 128;

    size_t i;
    for (i = m_bufferSize + 1; i < BlockSize; i++)
        m_buffer[i] = 0;
    for (; i < paddedLength; i++)
        extra[i - BlockSize] = 0;

    // add message length in bits as 64 bit number
    uint64_t msgBits = 8 * (m_numBytes + m_bufferSize);
    // find right position
    unsigned char* addLength;
    if (paddedLength < BlockSize)
        addLength = m_buffer + paddedLength;
    else
        addLength = extra + paddedLength - BlockSize;

    // must be little endian
    *addLength++ = msgBits & 0xFF;
    msgBits >>= 8;
    *addLength++ = msgBits & 0xFF;
    msgBits >>= 8;
    *addLength++ = msgBits & 0xFF;
    msgBits >>= 8;
    *addLength++ = msgBits & 0xFF;
    msgBits >>= 8;
    *addLength++ = msgBits & 0xFF;
    msgBits >>= 8;
    *addLength++ = msgBits & 0xFF;
    msgBits >>= 8;
    *addLength++ = msgBits & 0xFF;
    msgBits >>= 8;
    *addLength++ = msgBits & 0xFF;

    // process blocks
    processBlock(m_buffer);
    // flowed over into a second block ?
    if (paddedLength > BlockSize)
        processBlock(extra);
}

/// return latest hash as 32 hex characters
std::string MD5::getHash()
{
    // compute hash (as raw bytes)
    unsigned char rawHash[HashBytes];
    getHash(rawHash);

    // convert to hex string
    std::string result;
    result.reserve(2 * HashBytes);
    for (int i = 0; i < HashBytes; i++) {
        static const char dec2hex[16 + 1] = "0123456789abcdef";
        result += dec2hex[(rawHash[i] >> 4) & 15];
        result += dec2hex[rawHash[i] & 15];
    }

    return result;
}

/// return latest hash as bytes
void MD5::getHash(unsigned char buffer[MD5::HashBytes])
{
    // save old hash if buffer is partially filled
    uint32_t oldHash[HashValues];
    for (int i = 0; i < HashValues; i++)
        oldHash[i] = m_hash[i];

    // process remaining bytes
    processBuffer();

    unsigned char* current = buffer;
    for (int i = 0; i < HashValues; i++) {
        *current++ = m_hash[i] & 0xFF;
        *current++ = (m_hash[i] >> 8) & 0xFF;
        *current++ = (m_hash[i] >> 16) & 0xFF;
        *current++ = (m_hash[i] >> 24) & 0xFF;

        // restore old hash
        m_hash[i] = oldHash[i];
    }
}

/// compute MD5 of a memory block
std::string MD5::operator()(const void* data, size_t numBytes)
{
    reset();
    add(data, numBytes);
    return getHash();
}

/// compute MD5 of a string, excluding final zero
std::string MD5::operator()(const std::string& text)
{
    reset();
    add(text.c_str(), text.size());
    return getHash();
}

// NOLINTEND

#pragma GCC diagnostic push
