/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <imageio-lite/ImageDecoder.h>

#include <image/ColorTransform.h>
#include <image/ImageOps.h>

#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/Panic.h>

#include <cstdint>
#include <cstring>
#include <istream>
#include <limits>
#include <memory>
#include <vector>

// for ntohs
#if defined(WIN32)
#include <Winsock2.h>
#include <utils/unwindows.h>
#else
#include <arpa/inet.h>
#endif

using namespace image;

namespace imageio_lite {

class TIFFDecoder : public ImageDecoder::Decoder {
public:
    static TIFFDecoder* create(std::istream& stream);
    static bool checkSignature(char const* buf);

    TIFFDecoder(const TIFFDecoder&) = delete;
    TIFFDecoder& operator=(const TIFFDecoder&) = delete;

private:
    explicit TIFFDecoder(std::istream& stream);
    ~TIFFDecoder() override = default;

    // ImageDecoder::Decoder interface
    LinearImage decode() override;

    std::istream& mStream;
    std::streampos mStreamStartPos;
};

// -----------------------------------------------------------------------------------------------

image::LinearImage ImageDecoder::decode(std::istream& stream, utils::CString const& sourceName,
        ColorSpace sourceSpace) {

    Format format = Format::NONE;

    std::streampos pos = stream.tellg();
    char buf[16];
    stream.read(buf, sizeof(buf));

    if (TIFFDecoder::checkSignature(buf)) {
        format = Format::TIFF;
    }

    stream.seekg(pos);

    std::unique_ptr<Decoder> decoder;
    switch (format) {
        case Format::NONE:
            return LinearImage();
        case Format::TIFF:
            decoder.reset(TIFFDecoder::create(stream));
            decoder->setColorSpace(sourceSpace);
            break;
    }

    return decoder->decode();
}

// -----------------------------------------------------------------------------------------------

static inline float read32(std::istream& istream) {
    uint32_t data;
    istream.read(reinterpret_cast<char*>(&data), sizeof(uint32_t));
    data = ntohl(data);
    return *reinterpret_cast<float*>(&data);
}

static inline float read16(std::istream& istream) {
    uint16_t data;
    istream.read(reinterpret_cast<char*>(&data), sizeof(uint16_t));
    return static_cast<float>(ntohs(data)) / std::numeric_limits<uint16_t>::max();
}

// -----------------------------------------------------------------------------------------------

namespace {

// TIFF Tag Definitions
constexpr uint16_t ImageWidth = 256;
constexpr uint16_t ImageLength = 257;
constexpr uint16_t BitsPerSample = 258;
constexpr uint16_t Compression = 259;
constexpr uint16_t PhotometricInterpretation = 262;
constexpr uint16_t StripOffsets = 273;
constexpr uint16_t SamplesPerPixel = 277;
constexpr uint16_t RowsPerStrip = 278;
constexpr uint16_t StripByteCounts = 279;
constexpr uint16_t PlanarConfiguration = 284;


// TIFF Type Definitions
constexpr uint16_t SHORT = 3;
constexpr uint16_t LONG = 4;

constexpr uint32_t MAX_STRIPS = 1000000;

} // namespace


TIFFDecoder* TIFFDecoder::create(std::istream& stream) { return new TIFFDecoder(stream); }

bool TIFFDecoder::checkSignature(char const* buf) {
    return memcmp(buf, "II\x2a\x00", 4) == 0 || memcmp(buf, "MM\x00\x2a", 4) == 0;
}

TIFFDecoder::TIFFDecoder(std::istream& stream)
        : mStream(stream),
          mStreamStartPos(stream.tellg()) {}

LinearImage TIFFDecoder::decode() {
    char header[8];
    mStream.read(header, 8);
    FILAMENT_CHECK_PRECONDITION(mStream.gcount() == 8) << "Incomplete TIFF header";

    bool const littleEndian = (header[0] == 0x49 && header[1] == 0x49);

    auto const read16 = [&](std::istream& s) -> uint16_t {
        uint8_t b[2];
        s.read(reinterpret_cast<char*>(b), 2);
        if (littleEndian) {
            return uint16_t(b[0]) | (uint16_t(b[1]) << 8);
        }
        return uint16_t(b[1]) | (uint16_t(b[0]) << 8);
    };

    auto const read32 = [&](std::istream& s) -> uint32_t {
        uint8_t b[4];
        s.read(reinterpret_cast<char*>(b), 4);
        if (littleEndian) {
            return uint32_t(b[0]) | (uint32_t(b[1]) << 8) | (uint32_t(b[2]) << 16) |
                   (uint32_t(b[3]) << 24);
        }
        return uint32_t(b[3]) | (uint32_t(b[2]) << 8) | (uint32_t(b[1]) << 16) |
               (uint32_t(b[0]) << 24);
    };

    auto const seek = [&](uint32_t offset) {
        mStream.seekg(mStreamStartPos + (std::streamoff) offset, std::ios::beg);
    };

    // IFD offset is bytes 4-7
    uint32_t const ifdOffset =
            littleEndian ? (uint8_t) header[4] | ((uint8_t) header[5] << 8) |
                                   ((uint8_t) header[6] << 16) | ((uint8_t) header[7] << 24)
                         : (uint8_t) header[7] | ((uint8_t) header[6] << 8) |
                                   ((uint8_t) header[5] << 16) | ((uint8_t) header[4] << 24);

    seek(ifdOffset);

    uint16_t const numEntries = read16(mStream);

    uint32_t width = 0;
    uint32_t height = 0;
    uint16_t compression = 1;
    uint16_t photometric = 0;
    uint16_t samplesPerPixel = 1;
    uint16_t bitsPerSample = 8;
    uint16_t planarConfiguration = 1;
    std::vector<uint32_t> stripOffsets;
    std::vector<uint32_t> stripByteCounts;

    for (int i = 0; i < numEntries; ++i) {
        uint16_t const tag = read16(mStream);
        uint16_t const type = read16(mStream);
        uint32_t const count = read32(mStream);

        // Value or offset field (4 bytes)
        uint8_t voBuf[4];
        mStream.read(reinterpret_cast<char*>(voBuf), 4);
        uint32_t const valueOrOffset =
                littleEndian ? (uint32_t) voBuf[0] | ((uint32_t) voBuf[1] << 8) |
                                       ((uint32_t) voBuf[2] << 16) | ((uint32_t) voBuf[3] << 24)
                             : (uint32_t) voBuf[3] | ((uint32_t) voBuf[2] << 8) |
                                       ((uint32_t) voBuf[1] << 16) | ((uint32_t) voBuf[0] << 24);

        // Save current position
        std::streampos const nextEntry = mStream.tellg();

        // Correct SHORT decoding for Big Endian
        uint32_t entryValue = valueOrOffset;
        if (!littleEndian && type == SHORT && count == 1) {
            entryValue >>= 16;
        }

        if (tag == ImageWidth) {
            width = entryValue;
        } else if (tag == ImageLength) {
            height = entryValue;
        } else if (tag == Compression) {
            compression = (uint16_t) entryValue;
        } else if (tag == PhotometricInterpretation) {
            photometric = (uint16_t) entryValue;
        } else if (tag == SamplesPerPixel) {
            samplesPerPixel = (uint16_t) entryValue;
        } else if (tag == PlanarConfiguration) {
            planarConfiguration = (uint16_t) entryValue;
        } else if (tag == BitsPerSample) {
            if (count == 1) {
                bitsPerSample = (uint16_t) entryValue;
            } else {
                seek(valueOrOffset);
                bitsPerSample = read16(mStream);
            }
        } else if (tag == StripOffsets) {
            FILAMENT_CHECK_PRECONDITION(count <= MAX_STRIPS) << "TIFF Decoder: Too many strips";
            stripOffsets.resize(count);
            if (count == 1) {
                stripOffsets[0] = valueOrOffset;
            } else {
                seek(valueOrOffset);
                for (uint32_t j = 0; j < count; ++j) {
                    stripOffsets[j] = (type == SHORT) ? read16(mStream) : read32(mStream);
                }
            }
        } else if (tag == StripByteCounts) {
            FILAMENT_CHECK_PRECONDITION(count <= MAX_STRIPS) << "TIFF Decoder: Too many strips";
            stripByteCounts.resize(count);
            if (count == 1) {
                stripByteCounts[0] = valueOrOffset;
            } else {
                seek(valueOrOffset);
                for (uint32_t j = 0; j < count; ++j) {
                    stripByteCounts[j] = (type == SHORT) ? read16(mStream) : read32(mStream);
                }
            }
        }

        mStream.seekg(nextEntry);
    }

    FILAMENT_CHECK_PRECONDITION(compression == 1) << "TIFF Decoder: Unsupported compression";
    FILAMENT_CHECK_PRECONDITION(photometric == 2)
            << "TIFF Decoder: Unsupported photometric interpretation (only RGB supported)";
    FILAMENT_CHECK_PRECONDITION(samplesPerPixel == 3 || samplesPerPixel == 4)
            << "TIFF Decoder: Unsupported samples per pixel (only 3 or 4 supported)";
    FILAMENT_CHECK_PRECONDITION(bitsPerSample == 8)
            << "TIFF Decoder: Unsupported bits per sample (only 8 supported)";
    FILAMENT_CHECK_PRECONDITION(planarConfiguration == 1)
            << "TIFF Decoder: Unsupported planar configuration (only chunky supported)";

    // Check for integer overflow
    FILAMENT_CHECK_PRECONDITION(width != 0 && height != 0) << "TIFF Decoder: Invalid dimensions";
    FILAMENT_CHECK_PRECONDITION(height <= std::numeric_limits<size_t>::max() / width)
            << "TIFF Decoder: Image size overflow";
    size_t const pixelCount = (size_t) width * height;
    FILAMENT_CHECK_PRECONDITION(pixelCount <= std::numeric_limits<size_t>::max() / samplesPerPixel)
            << "TIFF Decoder: Image size overflow";
    size_t const allocationSize = pixelCount * samplesPerPixel;

    // Verify total strip bytes
    size_t totalStripBytes = 0;
    for (uint32_t const count: stripByteCounts) {
        FILAMENT_CHECK_PRECONDITION(std::numeric_limits<size_t>::max() - totalStripBytes >= count)
                << "TIFF Decoder: Strip bytes overflow";
        totalStripBytes += count;
    }
    FILAMENT_CHECK_PRECONDITION(totalStripBytes <= allocationSize)
            << "TIFF Decoder: Strip bytes exceed allocation";

    // Assemble image
    auto imageData = std::make_unique<uint8_t[]>(allocationSize);
    uint8_t* dst = imageData.get();

    FILAMENT_CHECK_PRECONDITION(stripOffsets.size() == stripByteCounts.size())
            << "TIFF Decoder: Strip offsets and byte counts mismatch";

    size_t totalBytesRead = 0;
    for (size_t i = 0; i < stripOffsets.size(); ++i) {
        seek(stripOffsets[i]);
        mStream.read(reinterpret_cast<char*>(dst), stripByteCounts[i]);
        dst += stripByteCounts[i];
        totalBytesRead += stripByteCounts[i];
    }

    FILAMENT_CHECK_PRECONDITION(totalBytesRead >= allocationSize)
            << "TIFF Decoder: Insufficient data read";

    if (getColorSpace() == ImageDecoder::ColorSpace::SRGB) {
        if (samplesPerPixel == 4) {
            return toLinearWithAlpha<uint8_t>(
                    width, height, width * 4, imageData.get(),
                    [](uint8_t v) -> uint8_t { return v; },
                    image::sRGBToLinear<filament::math::float4>);
        } else {
            return toLinear<uint8_t>(
                    width, height, width * 3, imageData.get(),
                    [](uint8_t v) -> uint8_t { return v; },
                    image::sRGBToLinear<filament::math::float3>);
        }
    } else {
        if (samplesPerPixel == 4) {
            return toLinearWithAlpha<uint8_t>(
                    width, height, width * 4, imageData.get(),
                    [](uint8_t v) -> uint8_t { return v; },
                    [](filament::math::float4 const& color) -> filament::math::float4 {
                        return color;
                    });
        } else {
            return toLinear<uint8_t>(
                    width, height, width * 3, imageData.get(),
                    [](uint8_t v) -> uint8_t { return v; },
                    [](filament::math::float3 const& color) -> filament::math::float3 {
                        return color;
                    });
        }
    }
}

} // namespace imageio_lite
