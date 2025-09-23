/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "TIFFExport.h"

#include <utils/debug.h>
#include <utils/Log.h>
#include <utils/Panic.h>

#include <algorithm>
#include <functional>
#include <vector>

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
        case ImageWidth: return LONG;
        case ImageLength: return LONG;
        case BitsPerSample: return SHORT;
        case Compression: return SHORT;
        case PhotometricInterpretation: return SHORT;
        case StripOffsets: return LONG;
        case SamplesPerPixel: return SHORT;
        case RowsPerStrip: return LONG;
        case StripByteCounts: return LONG;
        case PlanarConfiguration: return SHORT;
        case XResolution: return RATIONAL;
        case YResolution: return RATIONAL;
        case ResolutionUnit: return SHORT;
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

#define ROWS_PER_STRIP(width) (MAX_STRIP_SIZE / (width * 4))

void buildStrips(uint32_t width, uint32_t height,
        std::vector<uint32_t>& stripSizes, std::vector<uint32_t>& stripOffsets) {
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

} // anonymous

void exportTIFF(void* rgbaData, uint32_t width, uint32_t height, std::ostream& file) {
    FILAMENT_CHECK_PRECONDITION(width * 4 < MAX_STRIP_SIZE)
            << "output image's width is too large. width=" << width
            << ", max-width=" << (MAX_STRIP_SIZE / 4);

    uint32_t cursor = 0;
    auto write = [&file, &cursor](auto const& obj) {
        uint32_t const len = sizeof(obj);
        file.write(reinterpret_cast<char const*>(&obj), sizeof(obj));
        cursor += len;
    };

    auto writeBytes = [&file, &cursor](uint8_t* bytes, uint32_t size) {
        file.write(reinterpret_cast<char const*>(bytes), size);
        cursor += size;
    };

    // TIFF Header
    TIFFHeader header = {
        .byteOrder = 0x4949, // Little-endian
        .magicNumber = 42,
        .firstIFDOffset = sizeof(TIFFHeader),
    };
    write(header);

    auto noopIFD = [](uint16_t tag, uint32_t count, uint32_t val) {};

    uint32_t ifdSize = 0;
    // We do a no-op to gather the size of the IFD entries
    buildIFDEntries(width, height, {}, 1, noopIFD, &ifdSize);

    uint16_t const ifdCount = ifdSize / sizeof(IFDEntry);
    write(ifdCount);

    std::vector<uint32_t> stripByteCounts;
    std::vector<uint32_t> stripOffsets;

    buildStrips(width, height, stripByteCounts, stripOffsets);

    // Next IFD Offset (0 for none)
    uint32_t const nextIFDOffset = 0;

    // At this point, we've written the header plus the number of IFD entries (a uint16_t).
    uint32_t offsetCursor = cursor;

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
    buildIFDEntries(width, height, offsets, stripOffsets.size(), addIFD, nullptr);
    // IFD entries must be sorted by tag.
    std::sort(ifdEntries.begin(), ifdEntries.end(), [](auto const& a, auto const& b) {
        return a.tag < b.tag;
    });

    // Begin writing IFD and all the other metadata arrays.
    std::for_each(ifdEntries.begin(), ifdEntries.end(), write);
    write(nextIFDOffset);

    assert_invariant(cursor == offsets.bitsPerSample);
    write(bitsPerSample);
    assert_invariant(cursor == offsets.xResolution);
    write(xResolution);
    assert_invariant(cursor == offsets.yResolution);
    write(yResolution);
    assert_invariant(cursor == offsets.stripByteCounts);
    writeBytes((uint8_t*) stripByteCounts.data(), stripByteCounts.size() * sizeof(uint32_t));
    assert_invariant(cursor == offsets.strips);
    writeBytes((uint8_t*) stripOffsets.data(), stripOffsets.size() * sizeof(uint32_t));

    uint32_t const totalSize = width * height * 4;
    uint32_t const maxStripSize = ROWS_PER_STRIP(width) * width * 4;
    for (uint32_t i = 0, count = 0; i < totalSize;) {
        uint32_t const stripSize = std::min(totalSize - i, maxStripSize);

        assert_invariant(cursor == stripOffsets[count++]);
        writeBytes(((uint8_t*) rgbaData) + i, stripSize);

        i += stripSize;
    };
}
