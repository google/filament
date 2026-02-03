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

#include <imageio-lite/ImageEncoder.h>

#include <image/ColorTransform.h>

#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/Panic.h>
#include <utils/compiler.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <ostream>
#include <vector>

#if defined(WIN32)
#include <Winsock2.h>
#include <utils/unwindows.h>
#else
#include <arpa/inet.h>
#endif

using namespace filament::math;
using namespace image;

namespace imageio_lite {

class TIFFEncoder : public ImageEncoder::Encoder {
public:
    static TIFFEncoder* create(std::ostream& stream);

    TIFFEncoder(TIFFEncoder const&) = delete;
    TIFFEncoder& operator=(TIFFEncoder const&) = delete;

private:
    explicit TIFFEncoder(std::ostream& stream);
    ~TIFFEncoder() override = default;

    // ImageEncoder::Encoder interface
    bool encode(LinearImage const& image) override;

    std::ostream& mStream;
    std::streampos mStreamStartPos;
};

// ------------------------------------------------------------------------------------------------

bool ImageEncoder::encode(std::ostream& stream, Format format, image::LinearImage const& image,
        utils::CString const& compression, utils::CString const& destName) {
    std::unique_ptr<Encoder> encoder;
    switch (format) {
        case Format::TIFF:
            encoder.reset(TIFFEncoder::create(stream));
            break;
    }
    return encoder->encode(image);
}

ImageEncoder::Format ImageEncoder::chooseFormat(utils::CString const& name, bool forceLinear) {
    utils::CString ext;
    std::string_view nameSv = name;
    size_t index = nameSv.rfind('.');
    if (index != std::string_view::npos && index != 0) {
        ext = utils::CString(nameSv.substr(index + 1));
    }

    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "tif" || ext == "tiff") return Format::TIFF;

    // Default to TIFF for this lite library if no other format matches?
    // Or should we return something indicating failure/default?
    // The original defaulted to PNG. Here we only support TIFF.
    return Format::TIFF;
}

utils::CString ImageEncoder::chooseExtension(ImageEncoder::Format format) {
    switch (format) {
        case Format::TIFF:
            return ".tif";
        default:
            FILAMENT_CHECK_POSTCONDITION(false) << "Format not supported";
            return "";
    }
}

//-------------------------------------------------------------------------------------------------

namespace {

// TIFF Header Structure
struct TIFFHeader {
    uint16_t byteOrder;
    uint16_t magicNumber;
    uint32_t firstIFDOffset;
};

static_assert(sizeof(TIFFHeader) == 8);

// Image File Directory (IFD) Entry Structure
struct IFDEntry {
    uint16_t tag;
    uint16_t type;
    uint32_t count;
    uint32_t valueOffset;
};

static_assert(sizeof(IFDEntry) == 12);

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
constexpr uint16_t XResolution = 282;
constexpr uint16_t YResolution = 283;
constexpr uint16_t ResolutionUnit = 296;
constexpr uint16_t PlanarConfiguration = 284;

// TIFF Type Definitions
constexpr uint16_t SHORT = 3;
constexpr uint16_t LONG = 4;
constexpr uint16_t RATIONAL = 5;

// TIFF resolution unit
constexpr uint16_t Inch = 2;

constexpr uint16_t getType(uint16_t const tag) {
    switch (tag) {
        case ImageWidth:
            return LONG;
        case ImageLength:
            return LONG;
        case BitsPerSample:
            return SHORT;
        case Compression:
            return SHORT;
        case PhotometricInterpretation:
            return SHORT;
        case StripOffsets:
            return LONG;
        case SamplesPerPixel:
            return SHORT;
        case RowsPerStrip:
            return LONG;
        case StripByteCounts:
            return LONG;
        case PlanarConfiguration:
            return SHORT;
        case XResolution:
            return RATIONAL;
        case YResolution:
            return RATIONAL;
        case ResolutionUnit:
            return SHORT;
        default:
            return SHORT;
    }
}

// Photometric Interpretation
constexpr uint16_t RGBA = 2;

// Compression
constexpr uint16_t NoCompression = 1;

// Planar Configuration
constexpr uint16_t Chunky = 1;

// According to spec, 8K is the recommended max strip size.
constexpr uint32_t MAX_STRIP_SIZE = 8192;

#define ROWS_PER_STRIP(width) (std::max(1u, MAX_STRIP_SIZE / (width * 4)))

void buildStrips(uint32_t width, uint32_t height, std::vector<uint32_t>& stripSizes,
        std::vector<uint32_t>& stripOffsets) {
    uint32_t const totalSize = width * height * 4;
    uint32_t const rowSize = width * 4;
    uint32_t const rowsPerStrip = ROWS_PER_STRIP(width);
    uint32_t const maxStripSize = rowSize * rowsPerStrip;

    uint32_t size = totalSize;
    uint32_t offset = 0;
    while (size > 0) {
        uint32_t const stripSize = std::min(size, maxStripSize);
        size -= stripSize;

        stripSizes.push_back(stripSize);
        stripOffsets.push_back(offset);
        offset += stripSize;
    }
}

using AddIFDEntryFunc = std::function<void(uint16_t, uint32_t, uint32_t)>;

struct Offsets {
    uint32_t bitsPerSample;
    uint32_t xResolution;
    uint32_t yResolution;
    uint32_t stripByteCounts;
    uint32_t strips;
};

void buildIFDEntries(uint32_t width, uint32_t height, Offsets offsets, uint32_t stripCount,
        AddIFDEntryFunc addIFD, uint32_t* size) {

    auto addIFDEntry = [&](uint16_t tag, uint32_t count, uint32_t val) {
        addIFD(tag, count, val);
        if (size) {
            (*size) += sizeof(IFDEntry);
        }
    };

    addIFDEntry(ImageWidth, 1, width);
    addIFDEntry(ImageLength, 1, height);
    addIFDEntry(Compression, 1, NoCompression);
    addIFDEntry(PhotometricInterpretation, 1, RGBA);
    addIFDEntry(SamplesPerPixel, 1, 4);
    addIFDEntry(RowsPerStrip, 1, ROWS_PER_STRIP(width));
    addIFDEntry(StripByteCounts, stripCount, offsets.stripByteCounts);
    addIFDEntry(PlanarConfiguration, 1, Chunky);
    addIFDEntry(BitsPerSample, 4, offsets.bitsPerSample);
    addIFDEntry(StripOffsets, stripCount, offsets.strips);
    addIFDEntry(XResolution, 1, offsets.xResolution);
    addIFDEntry(YResolution, 1, offsets.yResolution);
    addIFDEntry(ResolutionUnit, 1, Inch);
}

} // namespace

TIFFEncoder* TIFFEncoder::create(std::ostream& stream) {
    TIFFEncoder* encoder = new TIFFEncoder(stream);
    return encoder;
}

TIFFEncoder::TIFFEncoder(std::ostream& stream)
        : mStream(stream),
          mStreamStartPos(stream.tellp()) {}

bool TIFFEncoder::encode(LinearImage const& image) {
    uint32_t const width = image.getWidth();
    uint32_t const height = image.getHeight();

    // Convert to sRGB 8-bit RGBA
    size_t const srcChannels = image.getChannels();
    std::unique_ptr<uint8_t[]> rgbaData;
    if (srcChannels == 4) {
        rgbaData = fromLinearTosRGB<uint8_t, 4>(image);
    } else if (srcChannels == 3) {
        rgbaData = std::make_unique<uint8_t[]>(width * height * 4);
        auto rgb = fromLinearTosRGB<uint8_t, 3>(image);
        for (size_t i = 0, n = width * height; i < n; ++i) {
            rgbaData[i * 4 + 0] = rgb[i * 3 + 0];
            rgbaData[i * 4 + 1] = rgb[i * 3 + 1];
            rgbaData[i * 4 + 2] = rgb[i * 3 + 2];
            rgbaData[i * 4 + 3] = 255;
        }
    } else {
        rgbaData = std::make_unique<uint8_t[]>(width * height * 4);
        uint8_t* d = rgbaData.get();
        for (size_t y = 0; y < height; ++y) {
            float const* p = image.getPixelRef(0, y);
            for (size_t x = 0; x < width; ++x, p += srcChannels, d += 4) {
                if (srcChannels == 1) {
                    uint8_t g = uint8_t(
                            filament::math::saturate(image::linearTosRGB(p[0])) * 255.0f + 0.5f);
                    d[0] = d[1] = d[2] = g;
                } else {
                    for (int i = 0; i < 3; i++) {
                        if (i < srcChannels) {
                            d[i] = uint8_t(
                                    filament::math::saturate(image::linearTosRGB(p[i])) * 255.0f +
                                    0.5f);
                        } else {
                            d[i] = 0;
                        }
                    }
                }
                d[3] = 255;
            }
        }
    }

    auto write = [this](void const* data, size_t size) {
        mStream.write(reinterpret_cast<char const*>(data), size);
    };
    auto write16 = [this](uint16_t v) {
        uint8_t b[2] = { uint8_t(v & 0xFF), uint8_t(v >> 8) };
        mStream.write((char*) b, 2);
    };
    auto write32 = [this](uint32_t v) {
        uint8_t b[4] = { uint8_t(v & 0xFF), uint8_t((v >> 8) & 0xFF), uint8_t((v >> 16) & 0xFF),
            uint8_t(v >> 24) };
        mStream.write((char*) b, 4);
    };
    auto tell = [this]() { return (uint32_t) (mStream.tellp() - mStreamStartPos); };

    // TIFF Header
    write16(0x4949); // Little-endian (II)
    write16(42);
    write32(8); // firstIFDOffset

    auto noopIFD = [](uint16_t tag, uint32_t count, uint32_t val) {};

    uint32_t ifdSize = 0;
    // We do a no-op to gather the size of the IFD entries
    buildIFDEntries(width, height, {}, 1, noopIFD, &ifdSize);

    uint16_t const ifdCount = ifdSize / sizeof(IFDEntry);
    write16(ifdCount);

    std::vector<uint32_t> stripByteCounts;
    std::vector<uint32_t> stripOffsets;

    buildStrips(width, height, stripByteCounts, stripOffsets);

    // Next IFD Offset (0 for none)
    uint32_t const nextIFDOffset = 0;

    // At this point, we've written the header plus the number of IFD entries (a uint16_t).
    uint32_t offsetCursor = tell();

    constexpr uint16_t bitsPerSample[4] = { 8, 8, 8, 8 };
    constexpr uint32_t xResolution[2] = { 1, 1 };
    constexpr uint32_t yResolution[2] = { 1, 1 };

    Offsets const offsets = {
        .bitsPerSample = (offsetCursor += (ifdSize + sizeof(nextIFDOffset))),
        .xResolution = (offsetCursor += sizeof(bitsPerSample)),
        .yResolution = (offsetCursor += sizeof(xResolution)),
        .stripByteCounts = (offsetCursor += sizeof(yResolution)),
        .strips = (offsetCursor += sizeof(uint32_t) * stripByteCounts.size()),
    };

    uint32_t const stripStart = offsets.strips + stripOffsets.size() * sizeof(uint32_t);
    std::for_each(stripOffsets.begin(), stripOffsets.end(),
            [stripStart](uint32_t& offset) { offset += stripStart; });

    // Really build the IFD entries with the proper offsets and putting them into a vector.
    std::vector<IFDEntry> ifdEntries;
    auto addIFD = [&ifdEntries](uint16_t tag, uint32_t count, uint32_t val) {
        ifdEntries.push_back({});
        auto& entry = ifdEntries.back();
        entry.tag = tag;
        entry.type = getType(tag);
        entry.count = count;
        entry.valueOffset = val;
    };
    buildIFDEntries(width, height, offsets, (uint32_t) stripOffsets.size(), addIFD, nullptr);

    // Fix for count=1: the ValueOffset should contain the value directly if it fits.
    if (stripOffsets.size() == 1) {
        for (auto& entry: ifdEntries) {
            if (entry.tag == StripOffsets) {
                entry.valueOffset = stripOffsets[0];
            }
            if (entry.tag == StripByteCounts) {
                entry.valueOffset = stripByteCounts[0];
            }
        }
    }

    // IFD entries must be sorted by tag.
    std::sort(ifdEntries.begin(), ifdEntries.end(),
            [](auto const& a, auto const& b) { return a.tag < b.tag; });

    // Begin writing IFD and all the other metadata arrays.
    for (const auto& entry: ifdEntries) {
        write16(entry.tag);
        write16(entry.type);
        write32(entry.count);
        write32(entry.valueOffset);
    }
    write32(nextIFDOffset);

    FILAMENT_CHECK_POSTCONDITION(tell() == offsets.bitsPerSample)
            << "TIFF Encode error: offset mismatch bitsPerSample";
    for (int i = 0; i < 4; i++) {
        write16(bitsPerSample[i]);
    }
    FILAMENT_CHECK_POSTCONDITION(tell() == offsets.xResolution)
            << "TIFF Encode error: offset mismatch xResolution";
    for (int i = 0; i < 2; i++) {
        write32(xResolution[i]);
    }
    FILAMENT_CHECK_POSTCONDITION(tell() == offsets.yResolution)
            << "TIFF Encode error: offset mismatch yResolution";
    for (int i = 0; i < 2; i++) {
        write32(yResolution[i]);
    }
    FILAMENT_CHECK_POSTCONDITION(tell() == offsets.stripByteCounts)
            << "TIFF Encode error: offset mismatch stripByteCounts";
    for (uint32_t v: stripByteCounts) {
        write32(v);
    }
    FILAMENT_CHECK_POSTCONDITION(tell() == offsets.strips)
            << "TIFF Encode error: offset mismatch strips";
    for (uint32_t v: stripOffsets) {
        write32(v);
    }

    uint32_t const totalSize = width * height * 4;
    uint32_t const maxStripSize = ROWS_PER_STRIP(width) * width * 4;
    for (uint32_t i = 0, count = 0; i < totalSize;) {
        uint32_t const stripSize = std::min(totalSize - i, maxStripSize);

        FILAMENT_CHECK_POSTCONDITION(tell() == stripOffsets[count++])
                << "TIFF Encode error: offset mismatch stripOffsets";
        write(((uint8_t*) rgbaData.get()) + i, stripSize);

        i += stripSize;
    };
    mStream.flush();

    return true;
}

} // namespace imageio_lite
