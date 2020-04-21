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

#include <cstdint>
#include <cstring> // for memcmp
#include <iostream> // for cerr
#include <limits>
#include <memory>
#include <sstream>

#include <png.h>

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

class HDRDecoder : public ImageDecoder::Decoder {
public:
    static HDRDecoder* create(std::istream& stream);
    static bool checkSignature(char const* buf);

    HDRDecoder(const HDRDecoder&) = delete;
    HDRDecoder& operator=(const HDRDecoder&) = delete;

private:
    explicit HDRDecoder(std::istream& stream);
    ~HDRDecoder() override;

    // ImageDecoder::Decoder interface
    LinearImage decode() override;

    static const char sigRadiance[];
    static const char sigRGBE[];
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
            png_set_alpha_mode(mPNG, PNG_ALPHA_PNG, PNG_DEFAULT_sRGB);
        } else {
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

const char HDRDecoder::sigRadiance[] = { '#', '?', 'R', 'A', 'D', 'I', 'A', 'N', 'C', 'E', 0xa };
const char HDRDecoder::sigRGBE[]     = { '#', '?', 'R', 'G', 'B', 'E', 0xa };

HDRDecoder* HDRDecoder::create(std::istream& stream) {
    HDRDecoder* decoder = new HDRDecoder(stream);
    return decoder;
}

bool HDRDecoder::checkSignature(char const* buf) {
    return !memcmp(buf, sigRadiance, sizeof(sigRadiance)) ||
            !memcmp(buf, sigRGBE, sizeof(sigRGBE));
}

HDRDecoder::HDRDecoder(std::istream& stream)
    : mStream(stream), mStreamStartPos(stream.tellg()) {
}

HDRDecoder::~HDRDecoder() = default;

LinearImage HDRDecoder::decode() {
    try {
        float gamma;
        float exposure;
        char sy, sx;
        unsigned int height, width;

        {
            char buf[1024];
            do {
                char format[128];
                mStream.getline(buf, sizeof(buf), 0xa);
                if (buf[0] == '#') continue;
                sscanf(buf, "FORMAT=%127s", format); // NOLINT
                sscanf(buf, "GAMMA=%f", &gamma); // NOLINT
                sscanf(buf, "EXPOSURE=%f", &exposure); // NOLINT
                if ((sscanf(buf, "%cY %u %cX %u", &sy, &height, &sx, &width) == 4)||   // NOLINT
                    (sscanf(buf, "%cX %u %cY %u", &sx, &width, &sy, &height) == 4)) {  // NOLINT
                    break;
                }
            } while (true);
        }

        LinearImage image(width, height, 3);

        if (sx == '-') image = horizontalFlip(image);
        if (sy == '+') image = verticalFlip(image);

        // Allocate memory to hold one row of decoded pixel data.
        std::unique_ptr<uint8_t[]> rgbe(new uint8_t[width * 4]);

        // First, test for non-RLE images.
        const auto pos = mStream.tellg();
        mStream.read((char*) rgbe.get(), 3);
        mStream.seekg(pos);

        if (rgbe[0] != 0x2 || rgbe[1] != 0x2 || (rgbe[2] & 0x80) || width < 8 || width > 32767) {
            for (uint32_t y = 0; y < height; y++) {
                filament::math::float3* dst = reinterpret_cast<filament::math::float3*>(image.getPixelRef(0, y));
                mStream.read((char*) rgbe.get(), width * 4);
                // (rgb/256) * 2^(e-128)
                size_t pixel = 0;
                for (size_t x = 0; x < width; x++, pixel += 4) {
                    filament::math::float3 v(rgbe[pixel], rgbe[pixel + 1], rgbe[pixel + 2]);
                    dst[x] = v * std::ldexp(1.0f, rgbe[pixel + 3] - (128 + 8));
                }
            }
        } else {
            for (uint32_t y = 0; y < height; y++) {
                uint16_t magic;
                mStream.read((char*) &magic, 2);
                if (magic != 0x0202) {
                    throw std::runtime_error("invalid scanline (magic)");
                }

                uint16_t w;
                mStream.read((char*) &w, 2);
                if (ntohs(w) != width) {
                    throw std::runtime_error("invalid scanline (width)");
                }

                char* d = (char*) rgbe.get();
                for (size_t p = 0; p < 4; p++) {
                    size_t num_bytes = 0;
                    while (num_bytes < width) {
                        uint8_t rle_count;
                        mStream.read((char*) &rle_count, 1);
                        if (rle_count > 128) {
                            char v;
                            mStream.read(&v, 1);
                            memset(d, v, size_t(rle_count - 128));
                            d += rle_count - 128;
                            num_bytes += rle_count - 128;
                        } else {
                            if (rle_count == 0) {
                                throw std::runtime_error("run length is zero");
                            }
                            mStream.read(d, rle_count);
                            d += rle_count;
                            num_bytes += rle_count;
                        }
                    }
                }

                uint8_t const* r = &rgbe[0];
                uint8_t const* g = &rgbe[width];
                uint8_t const* b = &rgbe[2 * width];
                uint8_t const* e = &rgbe[3 * width];
                filament::math::float3* dst = reinterpret_cast<filament::math::float3*>(image.getPixelRef(0, y));
                // (rgb/256) * 2^(e-128)
                for (size_t x = 0; x < width; x++, r++, g++, b++, e++) {
                    filament::math::float3 v(r[0], g[0], b[0]);
                    dst[x] = v * std::ldexp(1.0f, e[0] - (128 + 8));
                }
            }
        }

        return image;

    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while decoding HDR: " << e.what() << std::endl;
        mStream.seekg(mStreamStartPos);
    }
    return LinearImage();
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
                 filament::math::float3& pixel = *reinterpret_cast< filament::math::float3*>(image.getPixelRef(x, y));
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

} // namespace image
