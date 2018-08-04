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

#include <image/ImageDecoder.h>

#include <cstdint>
#include <istream>
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

#include "utilities.h"

namespace image {

class PNGDecoder : public ImageDecoder::Decoder {
public:
    static PNGDecoder* create(std::istream& stream);
    static bool checkSignature(char const* buf);

private:
    explicit PNGDecoder(std::istream& stream);
    PNGDecoder(const PNGDecoder&) = delete;
    ~PNGDecoder();

    PNGDecoder& operator = (const PNGDecoder&) = delete;

    void init();

    // ImageDecoder::Decoder interface
    virtual Image decode() override;

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
    friend class ImageDecoder;
    static HDRDecoder* create(std::istream& stream);
    static bool checkSignature(char const* buf);

private:
    explicit HDRDecoder(std::istream& stream);
    HDRDecoder(const HDRDecoder&) = delete;
    ~HDRDecoder();

    HDRDecoder& operator=(const HDRDecoder&) = delete;

    // ImageDecoder::Decoder interface
    virtual Image decode() override;

    static const char sigRadiance[];
    static const char sigRGBE[];
    std::istream& mStream;
    std::streampos mStreamStartPos;
};

// -----------------------------------------------------------------------------------------------

class PSDDecoder : public ImageDecoder::Decoder {
    friend class ImageDecoder;
    static PSDDecoder* create(std::istream& stream);
    static bool checkSignature(char const* buf);

private:
    explicit PSDDecoder(std::istream& stream);
    PSDDecoder(const PSDDecoder&) = delete;
    ~PSDDecoder();

    PSDDecoder& operator = (const PSDDecoder&) = delete;

    // ImageDecoder::Decoder interface
    virtual Image decode() override;

    static const char sig[];
    std::istream& mStream;
    std::streampos mStreamStartPos;
};

// -----------------------------------------------------------------------------------------------

class EXRDecoder : public ImageDecoder::Decoder {
    friend class ImageDecoder;
    static EXRDecoder* create(std::istream& stream, const std::string& sourceName);
    static bool checkSignature(char const* buf);

private:
    EXRDecoder(std::istream& stream, const std::string& sourceName);
    EXRDecoder(const EXRDecoder&) = delete;
    ~EXRDecoder();

    EXRDecoder& operator = (const EXRDecoder&) = delete;

    // ImageDecoder::Decoder interface
    virtual Image decode() override;

    static const char sig[];
    std::istream& mStream;
    std::streampos mStreamStartPos;
    std::string mSourceName;
};

// -----------------------------------------------------------------------------------------------

Image ImageDecoder::decode(std::istream& stream, const std::string& sourceName,
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
            return Image();
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

template<typename T, typename PROCESS, typename TRANSFORM>
static Image toLinear(size_t w, size_t h, size_t bpr,
        const std::unique_ptr<uint8_t[]>& src, PROCESS proc, TRANSFORM transform) {
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * sizeof(math::float3)]);
    math::float3* d = reinterpret_cast<math::float3*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src.get() + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 3) {
            math::float3 sRGB(proc(p[0]), proc(p[1]), proc(p[2]));
            sRGB /= std::numeric_limits<T>::max();
            *d++ = transform(sRGB);
        }
    }
    return Image(std::move(dst), w, h, w * sizeof(math::float3), sizeof(math::float3));
}

template<typename T, typename PROCESS, typename TRANSFORM>
static Image toLinearWithAlpha(size_t w, size_t h, size_t bpr,
        const std::unique_ptr<uint8_t[]>& src, PROCESS proc, TRANSFORM transform) {
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * sizeof(math::float4)]);
    math::float4* d = reinterpret_cast<math::float4*>(dst.get());
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src.get() + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 4) {
            math::float4 sRGB(proc(p[0]), proc(p[1]), proc(p[2]), proc(p[3]));
            sRGB /= std::numeric_limits<T>::max();
            *d++ = transform(sRGB);
        }
    }
    return Image(std::move(dst), w, h, w * sizeof(math::float4), sizeof(math::float4), 4);
}


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
        mPNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
}

void PNGDecoder::init() {
    png_set_error_fn(mPNG, this, cb_error, NULL);
    png_set_read_fn(mPNG, this, cb_stream);
}

PNGDecoder::~PNGDecoder() {
    png_destroy_read_struct(&mPNG, &mInfo, NULL);
}

Image PNGDecoder::decode() {
    std::unique_ptr<uint8_t[]> imageData;
    try {
        mInfo = png_create_info_struct(mPNG);
        png_read_info(mPNG, mInfo);

        int colorType = png_get_color_type(mPNG, mInfo);
        int bitDepth = png_get_bit_depth(mPNG, mInfo);

        if (colorType == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(mPNG);
        }
        if (colorType == PNG_COLOR_TYPE_GRAY) {
            png_set_gray_to_rgb(mPNG);
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
        uint32_t width  = png_get_image_width(mPNG, mInfo);
        uint32_t height = png_get_image_height(mPNG, mInfo);
        size_t rowBytes = png_get_rowbytes(mPNG, mInfo);

        imageData.reset(new uint8_t[width * height * rowBytes]);
        std::unique_ptr<png_bytep[]> rowPointers(new png_bytep[height]);
        for (size_t y = 0 ; y < height ; y++) {
            rowPointers[y] = &imageData[y * rowBytes];
        }
        png_read_image(mPNG, rowPointers.get());
        png_read_end(mPNG, mInfo);

        if (colorType == PNG_COLOR_TYPE_RGBA) {
            if (getColorSpace() == ImageDecoder::ColorSpace::SRGB) {
                return toLinearWithAlpha<uint16_t>(width, height, rowBytes, imageData,
                        [ ](uint16_t v) -> uint16_t { return ntohs(v); },
                        sRGBToLinear<math::float4>);
            } else {
                return toLinearWithAlpha<uint16_t>(width, height, rowBytes, imageData,
                        [ ](uint16_t v) -> uint16_t { return ntohs(v); },
                        [ ](const math::float4& color) -> math::float4 { return color; });
            }
        } else {
            // Convert to linear float (PNG 16 stores data in network order (big endian).
            if (getColorSpace() == ImageDecoder::ColorSpace::SRGB) {
                return toLinear<uint16_t>(width, height, rowBytes, imageData,
                        [ ](uint16_t v) -> uint16_t { return ntohs(v); },
                        sRGBToLinear<math::float3>);
            } else {
                return toLinear<uint16_t>(width, height, rowBytes, imageData,
                        [ ](uint16_t v) -> uint16_t { return ntohs(v); },
                        [ ](const math::float3& color) -> math::float3 { return color; });
            }
        }
    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while decoding PNG: " << e.what() << std::endl;
        mStream.seekg(mStreamStartPos);
        imageData.release();
    }
    return Image();
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

HDRDecoder::~HDRDecoder() {
}

Image HDRDecoder::decode() {
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
                sscanf(buf, "FORMAT=%127s", format);
                sscanf(buf, "GAMMA=%f", &gamma);
                sscanf(buf, "EXPOSURE=%f", &exposure);
                if ((sscanf(buf, "%cY %u %cX %u", &sy, &height, &sx, &width) == 4)||
                    (sscanf(buf, "%cX %u %cY %u", &sx, &width, &sy, &height) == 4)) {
                    break;
                }
            } while (true);
        }

        uint32_t flags = 0;
        if (sx == '-') flags |= Image::FLIP_X;

        // Experimentally, "-Y" means vertical origin at the top, "+Y" at the bottom
        if (sy == '+') flags |= Image::FLIP_Y;

        std::unique_ptr<uint8_t[]> data(new uint8_t[width * height * sizeof(math::float3)]);
        Image image(std::move(data), width, height, width*sizeof(math::float3), sizeof(math::float3));
        image.setFlags(flags);

        uint16_t w;
        uint16_t magic;
        std::unique_ptr<uint8_t[]> rgbe(new uint8_t[width*4]);
        for (size_t y=0 ; y<height ; y++) {
            //std::cout << "line: " << y << std::endl;
            mStream.read((char*)&magic, 2);
            if (magic != 0x0202) {
                throw std::runtime_error("invalid scanline (magic)");
            }
            mStream.read((char*)&w, 2);
            if (ntohs(w) != width) {
                throw std::runtime_error("invalid scanline (width)");
            }

            char *d = (char *)rgbe.get();
            for (size_t p=0 ; p<4 ; p++) {
                size_t num_bytes = 0;
                while (num_bytes < width) {
                    uint8_t rle_count;
                    mStream.read((char*)&rle_count, 1);
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
            uint8_t const* b = &rgbe[2*width];
            uint8_t const* e = &rgbe[3*width];
            math::float3* i = static_cast<math::float3*>(image.getPixelRef(0, y));
            // (rgb/256) * 2^(e-128)
            for (size_t x=0 ; x<width ; x++, r++, g++, b++, e++) {
                math::float3 v(r[0], g[0], b[0]);
                i[x] = v * std::ldexp(1.0f, e[0]-(128+8));
            }
        }
        return image;

    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while decoding HDR: " << e.what() << std::endl;
        mStream.seekg(mStreamStartPos);
    }
    return Image();
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

PSDDecoder::~PSDDecoder() {
}

Image PSDDecoder::decode() {
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
        Header h;
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

        std::unique_ptr<uint8_t[]> data(new uint8_t[width * height * sizeof(math::float3)]);
        Image image(std::move(data), width, height,
                    width * sizeof(math::float3), sizeof(math::float3));

        if (depth == 32) {
            for (size_t i = 0; i < 3; i++) {
                for (size_t y = 0; y < height; y++) {
                    for (size_t x = 0; x < width; x++) {
                        math::float3& pixel = *static_cast<math::float3*>(image.getPixelRef(x, y));
                        pixel[i] = read32(mStream);
                    }
                }
            }
        } else {
            for (size_t i = 0; i < 3; i++) {
                for (size_t y = 0; y < height; y++) {
                    for (size_t x = 0; x < width; x++) {
                        math::float3& pixel = *static_cast<math::float3*>(image.getPixelRef(x, y));
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

    return Image();
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

EXRDecoder::~EXRDecoder() {
}

Image EXRDecoder::decode() {
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
            mStream.seekg(mStreamStartPos);
            return Image();
        }

        src.resize(0);

        std::unique_ptr<uint8_t[]> data(new uint8_t[width * height * sizeof(math::float3)]);
        Image image(std::move(data), static_cast<size_t>(width), static_cast<size_t>(height),
                    width * sizeof(math::float3), sizeof(math::float3));

        size_t i = 0;
        for (size_t y = 0; y < height; y++) {
            for (size_t x = 0; x < width; x++) {
                math::float3& pixel = *static_cast<math::float3*>(image.getPixelRef(x, y));
                pixel.r = rgba[i++];
                pixel.g = rgba[i++];
                pixel.b = rgba[i++];
                // skip alpha
                i++;
            }
        }

        return image;
    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while decoding OpenEXR: " << e.what() << std::endl;
        mStream.seekg(mStreamStartPos);
    }

    return Image();
}

} // namespace image
