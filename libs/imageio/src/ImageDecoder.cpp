#include <memory>

/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <imageio/ImageDecoder.h>

#include <utils/Panic.h>

#include <cstdint>
#include <cstring> // for memcmp
#include <iostream> // for cerr
#include <limits>
#include <memory>
#include <sstream>

#include <png.h>

// for ntohs
#if defined(WIN32)
#    include <Winsock2.h>
#    include <utils/unwindows.h>
#else
#    include <arpa/inet.h>
#endif

#include <math/vec3.h>
#include <math/vec4.h>

#include <tinyexr.h>

#include <vector>

#include <image/ColorTransform.h>
#include <image/ImageOps.h>

#include <imageio/HDRDecoder.h>

namespace image {

class PNGDecoder : public ImageDecoder::Decoder {
public:
    static PNGDecoder* create(std::istream& stream);
    static bool checkSignature(char const* buf);

    PNGDecoder(const PNGDecoder&) = delete;
    PNGDecoder& operator=(const PNGDecoder&) = delete;

private:
    explicit PNGDecoder(std::istream& stream);
    ~PNGDecoder() override;

    void init();

    // ImageDecoder::Decoder interface
    LinearImage decode() override;

    static void cb_error(png_structp, png_const_charp);
    static void cb_stream(png_structp png, png_bytep buffer, png_size_t size);

    void error();
    void stream(void* buffer, size_t size);

    png_structp mPNG = nullptr;
    png_infop mInfo = nullptr;
    std::istream& mStream;
    std::streampos mStreamStartPos;
};

// -----------------------------------------------------------------------------------------------

class PSDDecoder : public ImageDecoder::Decoder {
public:
    static PSDDecoder* create(std::istream& stream);
    static bool checkSignature(char const* buf);

    PSDDecoder(const PSDDecoder&) = delete;
    PSDDecoder& operator=(const PSDDecoder&) = delete;

private:
    explicit PSDDecoder(std::istream& stream);
    ~PSDDecoder() override;

    // ImageDecoder::Decoder interface
    LinearImage decode() override;

    static const char sig[];
    std::istream& mStream;
    std::streampos mStreamStartPos;
};

// -----------------------------------------------------------------------------------------------

class EXRDecoder : public ImageDecoder::Decoder {
public:
    static EXRDecoder* create(std::istream& stream, const std::string& sourceName);
    static bool checkSignature(char const* buf);

    EXRDecoder(const EXRDecoder&) = delete;
    EXRDecoder& operator = (const EXRDecoder&) = delete;

private:
    EXRDecoder(std::istream& stream, const std::string& sourceName);
    ~EXRDecoder() override;

    // ImageDecoder::Decoder interface
    LinearImage decode() override;

    static const char sig[];
    std::istream& mStream;
    std::streampos mStreamStartPos;
    std::string mSourceName;
};

// -----------------------------------------------------------------------------------------------

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

LinearImage ImageDecoder::decode(std::istream& stream, const std::string& sourceName,
        ColorSpace sourceSpace) {

    Format format = Format::NONE;

    std::streampos pos = stream.tellg();
    char buf[16];
    stream.read(buf, sizeof(buf));

    if (PNGDecoder::checkSignature(buf)) {
        format = Format::PNG;
    } else if (HDRDecoder::checkSignature(buf)) {
        format = Format::HDR;
    } else if (PSDDecoder::checkSignature(buf)) {
        format = Format::PSD;
    } else if (EXRDecoder::checkSignature(buf)) {
        format = Format::EXR;
    } else if (TIFFDecoder::checkSignature(buf)) {
        format = Format::TIFF;
    }

    stream.seekg(pos);

    std::unique_ptr<Decoder> decoder;
    switch (format) {
        case Format::NONE:
            return LinearImage();
        case Format::PNG:
            decoder.reset(PNGDecoder::create(stream));
            decoder->setColorSpace(sourceSpace);
            break;
        case Format::HDR:
            decoder.reset(HDRDecoder::create(stream));
            decoder->setColorSpace(ColorSpace::LINEAR);
            break;
        case Format::PSD:
            decoder.reset(PSDDecoder::create(stream));
            decoder->setColorSpace(ColorSpace::LINEAR);
            break;
        case Format::EXR:
            decoder.reset(EXRDecoder::create(stream, sourceName));
            decoder->setColorSpace(ColorSpace::LINEAR);
            break;
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

PNGDecoder* PNGDecoder::create(std::istream& stream) {
    PNGDecoder* decoder = new PNGDecoder(stream);
    decoder->init();
    return decoder;
}

bool PNGDecoder::checkSignature(char const* buf) {
    return png_check_sig(png_const_bytep(buf), 8);
}

PNGDecoder::PNGDecoder(std::istream& stream)
    : mStream(stream), mStreamStartPos(stream.tellg()) {
        mPNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
}

void PNGDecoder::init() {
    png_set_error_fn(mPNG, this, cb_error, nullptr);
    png_set_read_fn(mPNG, this, cb_stream);
}

PNGDecoder::~PNGDecoder() {
    png_destroy_read_struct(&mPNG, &mInfo, nullptr);
}

LinearImage PNGDecoder::decode() {
    std::unique_ptr<uint8_t[]> imageData;
    try {
        mInfo = png_create_info_struct(mPNG);
        png_read_info(mPNG, mInfo);

        int colorType = png_get_color_type(mPNG, mInfo);
        int bitDepth = png_get_bit_depth(mPNG, mInfo);

        if (colorType == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(mPNG);
        }
        if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
            if (bitDepth < 8) {
                png_set_expand_gray_1_2_4_to_8(mPNG);
            }
            png_set_gray_to_rgb(mPNG);
        }
        if (png_get_valid(mPNG, mInfo, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(mPNG);
        }
        if (getColorSpace() == ImageDecoder::ColorSpace::SRGB) {
            double gamma = 1.0;
            png_get_gAMA(mPNG, mInfo, &gamma);
            if (gamma != 1.0) {
                png_set_alpha_mode(mPNG, PNG_ALPHA_PNG, PNG_DEFAULT_sRGB);
            }
        } else {
            png_set_gamma_fixed(mPNG, PNG_FP_1, PNG_FP_1);
            png_set_alpha_mode(mPNG, PNG_ALPHA_PNG, PNG_GAMMA_LINEAR);
        }
        if (bitDepth < 16) {
            png_set_expand_16(mPNG);
        }

        png_read_update_info(mPNG, mInfo);

        // Read updated color type since we may have asked for a conversion before
        colorType = png_get_color_type(mPNG, mInfo);

        uint32_t width  = png_get_image_width(mPNG, mInfo);
        uint32_t height = png_get_image_height(mPNG, mInfo);
        size_t rowBytes = png_get_rowbytes(mPNG, mInfo);

        imageData = std::make_unique<uint8_t[]>(height * rowBytes);
        std::unique_ptr<png_bytep[]> rowPointers(new png_bytep[height]);
        for (size_t y = 0 ; y < height ; y++) {
            rowPointers[y] = &imageData[y * rowBytes];
        }
        png_read_image(mPNG, rowPointers.get());
        png_read_end(mPNG, mInfo);

        if (colorType == PNG_COLOR_TYPE_RGBA) {
            if (getColorSpace() == ImageDecoder::ColorSpace::SRGB) {
                return toLinearWithAlpha<uint16_t>(width, height, rowBytes, imageData,
                        [](uint16_t v) -> uint16_t { return ntohs(v); },
                        sRGBToLinear<filament::math::float4>);
            } else {
                return toLinearWithAlpha<uint16_t>(width, height, rowBytes, imageData,
                        [](uint16_t v) -> uint16_t { return ntohs(v); },
                        [](const filament::math::float4& color) ->  filament::math::float4 { return color; });
            }
        } else {
            // Convert to linear float (PNG 16 stores data in network order (big endian).
            if (getColorSpace() == ImageDecoder::ColorSpace::SRGB) {
                return toLinear<uint16_t>(width, height, rowBytes, imageData,
                        [](uint16_t v) -> uint16_t { return ntohs(v); },
                        sRGBToLinear< filament::math::float3>);
            } else {
                return toLinear<uint16_t>(width, height, rowBytes, imageData,
                        [](uint16_t v) -> uint16_t { return ntohs(v); },
                        [](const filament::math::float3& color) ->  filament::math::float3 { return color; });
            }
        }
    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while decoding PNG: " << e.what() << std::endl;
        mStream.seekg(mStreamStartPos);
        imageData.release();
    }
    return LinearImage();
}

void PNGDecoder::cb_stream(png_structp png, png_bytep buffer, png_size_t size) {
    PNGDecoder* that = static_cast<PNGDecoder*>(png_get_io_ptr(png));
    that->stream(buffer, size);
}

void PNGDecoder::stream(void* buffer, size_t size) {
    mStream.read(static_cast<char *>(buffer), size);
    if (!mStream.good()) {
        throw std::runtime_error("Problem with the PNG stream.");
    }
}

void PNGDecoder::cb_error(png_structp png, png_const_charp) {
    PNGDecoder* that = static_cast<PNGDecoder*>(png_get_error_ptr(png));
    that->error();
}

void PNGDecoder::error() {
    throw std::runtime_error("Error while decoding PNG stream.");
}

// -----------------------------------------------------------------------------------------------

const char PSDDecoder::sig[] = { '8', 'B', 'P', 'S', 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

PSDDecoder* PSDDecoder::create(std::istream& stream) {
    PSDDecoder* decoder = new PSDDecoder(stream);
    return decoder;
}

bool PSDDecoder::checkSignature(char const* buf) {
    return !memcmp(buf, sig, sizeof(sig));
}

PSDDecoder::PSDDecoder(std::istream& stream)
        : mStream(stream), mStreamStartPos(stream.tellg()) {
}

PSDDecoder::~PSDDecoder() = default;

LinearImage PSDDecoder::decode() {
    #pragma pack(push, 1)
    // IMPORTANT NOTE: PSD files use big endian storage
    struct Header {
        char signature[4];
        uint16_t version;
        char padding[6];
        uint16_t channels;
        uint32_t height;
        uint32_t width;
        uint16_t depth;
        uint16_t mode;
    };
    #pragma pack(pop)

    static const uint16_t kColorModeRGB = 3;
    static const uint16_t kCompressionRAW = 0;

    try {
        Header h = { };
        mStream.read(reinterpret_cast<char*>(&h), sizeof(Header));

        if (ntohs(h.channels) != 3) {
            throw std::runtime_error("the image must have 3 channels only");
        }

        uint16_t depth = ntohs(h.depth);
        if (depth != 16 && depth != 32) {
            throw std::runtime_error("the image depth must be 16 or 32 bits per pixel");
        }

        if (ntohs(h.mode) != kColorModeRGB) {
            throw std::runtime_error("the image must be RGB");
        }

        uint32_t width = ntohl(h.width);
        uint32_t height = ntohl(h.height);

        uint32_t length;

        // color mode data section
        mStream.read(reinterpret_cast<char*>(&length), sizeof(uint32_t));
        mStream.seekg(ntohl(length), std::istream::cur);

        // image resources
        mStream.read(reinterpret_cast<char*>(&length), sizeof(uint32_t));
        mStream.seekg(ntohl(length), std::istream::cur);

        // layer and mask info section
        mStream.read(reinterpret_cast<char*>(&length), sizeof(uint32_t));
        mStream.seekg(ntohl(length), std::istream::cur);

        // compression format
        uint16_t compression;
        mStream.read(reinterpret_cast<char*>(&compression), sizeof(uint16_t));
        if (ntohs(compression) != kCompressionRAW) {
            throw std::runtime_error("compressed images are not supported");
        }

        LinearImage image(width, height, 3);

        if (depth == 32) {
            for (size_t i = 0; i < 3; i++) {
                for (uint32_t y = 0; y < height; y++) {
                    for (uint32_t x = 0; x < width; x++) {
                         filament::math::float3& pixel =
                                *reinterpret_cast< filament::math::float3*>(image.getPixelRef(x, y));
                        pixel[i] = read32(mStream);
                    }
                }
            }
        } else {
            for (size_t i = 0; i < 3; i++) {
                for (uint32_t y = 0; y < height; y++) {
                    for (uint32_t x = 0; x < width; x++) {
                         filament::math::float3& pixel =
                                *reinterpret_cast< filament::math::float3*>(image.getPixelRef(x, y));
                        pixel[i] = read16(mStream);
                    }
                }
            }
        }

        return image;
    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while decoding PSD: " << e.what() << std::endl;
        mStream.seekg(mStreamStartPos);
    }

    return LinearImage();
}

// -----------------------------------------------------------------------------------------------

const char EXRDecoder::sig[] = { 0x76, 0x2f, 0x31, 0x01 };

EXRDecoder* EXRDecoder::create(std::istream& stream, const std::string& sourceName) {
    EXRDecoder* decoder = new EXRDecoder(stream, sourceName);
    return decoder;
}

bool EXRDecoder::checkSignature(char const* buf) {
    return !memcmp(buf, sig, sizeof(sig));
}

EXRDecoder::EXRDecoder(std::istream& stream, const std::string& sourceName)
        : mStream(stream), mStreamStartPos(stream.tellg()), mSourceName(sourceName) {
}

EXRDecoder::~EXRDecoder() = default;

LinearImage EXRDecoder::decode() {
    try {
        // copy the EXR data in memory
        std::vector<unsigned char> src;
        unsigned char buffer[4096];
        while (mStream.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
            src.insert(src.end(), &buffer[0], &buffer[4096]);
        }
        src.insert(src.end(), &buffer[0], &buffer[mStream.gcount()]);

        int width;
        int height;
        float* rgba;
        const char* error;

        int ret = LoadEXRFromMemory(&rgba, &width, &height, src.data(), src.size(), &error);
        if (ret != TINYEXR_SUCCESS) {
            std::cerr << "Could not decode OpenEXR: " << error << std::endl;
            FreeEXRErrorMessage(error);
            mStream.seekg(mStreamStartPos);
            return LinearImage();
        }

        src.clear();
        src.shrink_to_fit();

        LinearImage image((uint32_t) width, (uint32_t) height, 3);

        size_t i = 0;
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                filament::math::float3& pixel =
                        *reinterpret_cast< filament::math::float3*>(image.getPixelRef(x, y));
                pixel.r = rgba[i++];
                pixel.g = rgba[i++];
                pixel.b = rgba[i++];
                // skip alpha
                i++;
            }
        }

        free(rgba);

        return image;
    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while decoding OpenEXR: " << e.what() << std::endl;
        mStream.seekg(mStreamStartPos);
    }

    return LinearImage();
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


TIFFDecoder* TIFFDecoder::create(std::istream& stream) {
    return new TIFFDecoder(stream);
}

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
                    [](const filament::math::float4& color) -> filament::math::float4 {
                        return color;
                    });
        } else {
            return toLinear<uint8_t>(
                    width, height, width * 3, imageData.get(),
                    [](uint8_t v) -> uint8_t { return v; },
                    [](const filament::math::float3& color) -> filament::math::float3 {
                        return color;
                    });
        }
    }
}

} // namespace image
