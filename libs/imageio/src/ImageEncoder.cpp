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

#include <imageio/ImageEncoder.h>

#include <algorithm>
#include <cstdint>
#include <cstring> // for memset
#include <functional>
#include <limits>
#include <memory>
#include <iostream> // for cerr

#if defined(WIN32)
    #include <Winsock2.h>
    #include <utils/unwindows.h>
#else
    #include <arpa/inet.h>
#endif

#include <png.h>

#include <tinyexr.h>

#include <math/half.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/compiler.h>

#include <image/ColorTransform.h>

using namespace filament::math;

namespace image {

class PNGEncoder : public ImageEncoder::Encoder {
public:
    enum class PixelFormat {
        sRGB,           // 8-bits sRGB
        RGBM,           // 8-bits RGBM
        LINEAR_RGB,     // 8-bits RGB
        RGB_10_11_11_REV,
    };

    static PNGEncoder* create(std::ostream& stream, PixelFormat format = PixelFormat::sRGB);

    PNGEncoder(const PNGEncoder&) = delete;
    PNGEncoder& operator=(const PNGEncoder&) = delete;

private:
    PNGEncoder(std::ostream& stream, PixelFormat format);
    ~PNGEncoder() override;

    void init();

    // ImageEncoder::Encoder interface
    bool encode(const LinearImage& image) override;

    int chooseColorType(const LinearImage& image) const;
    uint32_t getChannelsCount(int colorType) const;

    static void cb_error(png_structp png, png_const_charp error);
    static void cb_stream(png_structp png, png_bytep buffer, png_size_t size);

    void error();
    void stream(void* buffer, size_t size);

    png_structp mPNG = nullptr;
    png_infop mInfo = nullptr;
    std::ostream& mStream;
    std::streampos mStreamStartPos;

    PixelFormat mFormat;
};

// ------------------------------------------------------------------------------------------------

class HDREncoder : public ImageEncoder::Encoder {
public:
    static HDREncoder* create(std::ostream& stream);

    HDREncoder(const HDREncoder&) = delete;
    HDREncoder& operator=(const HDREncoder&) = delete;

private:
    explicit HDREncoder(std::ostream& stream);
    ~HDREncoder() override = default;

    // ImageEncoder::Encoder interface
    bool encode(const LinearImage& image) override;

    static void float2rgbe(uint8_t rgbe[4], const float3& in);
    static size_t countRepeats(uint8_t const* data, size_t length);
    static size_t countNonRepeats(uint8_t const* data, size_t length);
    static void rle(std::ostream& out, uint8_t const* data, size_t length);

    std::ostream& mStream;
    std::streampos mStreamStartPos;
};

// ------------------------------------------------------------------------------------------------

class PSDEncoder : public ImageEncoder::Encoder {
public:
    static PSDEncoder* create(std::ostream& stream, const std::string& compression);

    PSDEncoder(const PSDEncoder&) = delete;
    PSDEncoder& operator=(const PSDEncoder&) = delete;

private:
    PSDEncoder(std::ostream& stream, const std::string& compression);
    ~PSDEncoder() override = default;

    // ImageEncoder::Encoder interface
    bool encode(const LinearImage& image) override;

    std::ostream& mStream;
    std::streampos mStreamStartPos;
    std::string mCompression;

    static const char sig[];
};

// ------------------------------------------------------------------------------------------------

class EXREncoder : public ImageEncoder::Encoder {
public:
    static EXREncoder* create(std::ostream& stream, const std::string& compression,
            const std::string& destName);

    EXREncoder(const EXREncoder&) = delete;
    EXREncoder& operator=(const EXREncoder&) = delete;

private:
    EXREncoder(std::ostream& stream, const std::string& compression, const std::string& destName);
    ~EXREncoder() override = default;

    // ImageEncoder::Encoder interface
    bool encode(const LinearImage& image) override;

    std::ostream& mStream;
    std::streampos mStreamStartPos;
    std::string mDestName;
    std::string mCompression;
};

// ------------------------------------------------------------------------------------------------

class DDSEncoder : public ImageEncoder::Encoder {
public:
    enum class PixelFormat {
        sRGB,        // sRGB
        LINEAR_RGB,  // RGB
    };

    static DDSEncoder* create(std::ostream& stream, const std::string& compression,
                              PixelFormat format = PixelFormat::sRGB);

    DDSEncoder(const DDSEncoder&) = delete;
    DDSEncoder& operator=(const DDSEncoder&) = delete;

private:
    DDSEncoder(std::ostream& stream, const std::string& compression, PixelFormat format);
    ~DDSEncoder() override = default;

    // ImageEncoder::Encoder interface
    bool encode(const LinearImage& image) override;

    std::ostream& mStream;
    std::streampos mStreamStartPos;
    std::string mCompression;
    PixelFormat mFormat;
};

// ------------------------------------------------------------------------------------------------

bool ImageEncoder::encode(std::ostream& stream, Format format, const LinearImage& image,
        const std::string& compression, const std::string& destName) {
    std::unique_ptr<Encoder> encoder;
    switch(format) {
        case Format::PNG:
            encoder.reset(PNGEncoder::create(stream));
            break;
        case Format::PNG_LINEAR:
            encoder.reset(PNGEncoder::create(stream, PNGEncoder::PixelFormat::LINEAR_RGB));
            break;
        case Format::RGB_10_11_11_REV:
            encoder.reset(PNGEncoder::create(stream, PNGEncoder::PixelFormat::RGB_10_11_11_REV));
            break;
        case Format::HDR:
            encoder.reset(HDREncoder::create(stream));
            break;
        case Format::RGBM:
            encoder.reset(PNGEncoder::create(stream, PNGEncoder::PixelFormat::RGBM));
            break;
        case Format::PSD:
            encoder.reset(PSDEncoder::create(stream, compression));
            break;
        case Format::EXR:
            encoder.reset(EXREncoder::create(stream, compression, destName));
            break;
        case Format::DDS:
            encoder.reset(DDSEncoder::create(stream, compression));
            break;
        case Format::DDS_LINEAR:
            encoder.reset(DDSEncoder::create(stream, compression, DDSEncoder::PixelFormat::LINEAR_RGB));
            break;
    }
    return encoder->encode(image);
}

ImageEncoder::Format ImageEncoder::chooseFormat(const std::string& name, bool forceLinear) {
    std::string ext;
    size_t index = name.rfind('.');
    if (index != std::string::npos && index != 0) {
         ext = name.substr(index + 1);
    }

    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "png") return forceLinear ? Format::PNG_LINEAR : Format::PNG;

    if (ext == "rgbm") return Format::PNG;

    if (ext == "rgb32f") return Format::RGB_10_11_11_REV;

    if (ext == "hdr") return Format::HDR;

    if (ext == "psd") return Format::PSD;

    if (ext == "exr") return Format::EXR;

    if (ext == "dds") return forceLinear ? Format::DDS_LINEAR : Format::DDS;

    // PNG by default
    return forceLinear ? Format::PNG_LINEAR : Format::PNG;
}

std::string ImageEncoder::chooseExtension(ImageEncoder::Format format) {
    switch (format) {
        case Format::PNG:
        case Format::PNG_LINEAR:
            return ".png";
        case Format::RGB_10_11_11_REV:
            return ".rgb32f";
        case Format::RGBM:
            return ".rgbm";
        case Format::HDR:
            return ".hdr";
        case Format::PSD:
            return ".psd";
        case Format::EXR:
            return ".exr";
        case Format::DDS:
        case Format::DDS_LINEAR:
            return ".dds";
    }
}

//-------------------------------------------------------------------------------------------------

PNGEncoder* PNGEncoder::create(std::ostream& stream, PixelFormat format) {
    PNGEncoder* encoder = new PNGEncoder(stream, format);
    encoder->init();
    return encoder;
}

PNGEncoder::PNGEncoder(std::ostream& stream, PixelFormat format)
    : mPNG(png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr)),
      mStream(stream), mStreamStartPos(stream.tellp()), mFormat(format) {
}

PNGEncoder::~PNGEncoder() {
    png_destroy_write_struct(&mPNG, &mInfo);
}

void PNGEncoder::init() {
    png_set_error_fn(mPNG, this, cb_error, nullptr);
    png_set_write_fn(mPNG, this, cb_stream, nullptr);
}

int PNGEncoder::chooseColorType(const LinearImage& image) const {
    size_t channels = image.getChannels();
    switch (channels) {
        case 1:
            return PNG_COLOR_TYPE_GRAY;
        case 3:
            switch (mFormat) {
                case PixelFormat::RGBM:
                case PixelFormat::RGB_10_11_11_REV:
                    return PNG_COLOR_TYPE_RGBA;
                default:
                    return PNG_COLOR_TYPE_RGB;
            }
        case 4:
            return PNG_COLOR_TYPE_RGBA;
        default:
            std::cerr << "Warning: strange number of channels in PNG" << std::endl;
            return PNG_COLOR_TYPE_RGB;
    }
}

uint32_t PNGEncoder::getChannelsCount(int colorType) const {
    switch (mFormat) {
        case PixelFormat::RGBM:
        case PixelFormat::RGB_10_11_11_REV:
            return 4;
        default:
            switch (colorType) {
                case PNG_COLOR_TYPE_GRAY: return 1;
                case PNG_COLOR_TYPE_RGB:  return 3;
                case PNG_COLOR_TYPE_RGBA: return 4;
            }
            return 3;
    }
}

bool PNGEncoder::encode(const LinearImage& image) {
    size_t srcChannels = image.getChannels();

    switch (mFormat) {
        case PixelFormat::RGBM:
        case PixelFormat::RGB_10_11_11_REV:
            if (srcChannels != 3) {
                std::cerr << "Cannot encode PNG: " << srcChannels << " channels." << std::endl;
                return false;
            }
            break;
        default:
            if (srcChannels != 1 && srcChannels != 3 && srcChannels != 4) {
                std::cerr << "Cannot encode PNG: " << srcChannels << " channels." << std::endl;
                return false;
            }
            break;
    }

    try {
        mInfo = png_create_info_struct(mPNG);

        // Write header (8 bit colour depth)
        size_t width = image.getWidth();
        size_t height = image.getHeight();
        int colorType = chooseColorType(image);

        png_set_IHDR(mPNG, mInfo, width, height,
                     8, colorType, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        if (mFormat == PixelFormat::LINEAR_RGB || mFormat == PixelFormat::RGB_10_11_11_REV) {
            png_set_gAMA(mPNG, mInfo, 1.0);
        } else {
            png_set_sRGB_gAMA_and_cHRM(mPNG, mInfo, PNG_sRGB_INTENT_PERCEPTUAL);
        }

        png_write_info(mPNG, mInfo);

        std::unique_ptr<png_bytep[]> row_pointers(new png_bytep[height]);
        std::unique_ptr<uint8_t[]> data;

        uint32_t dstChannels;
        if (srcChannels == 1) {
            dstChannels = 1;
            data = fromLinearToGrayscale<uint8_t>(image);
        } else {
            dstChannels = getChannelsCount(colorType);
            switch (mFormat) {
                case PixelFormat::RGBM:
                    data = fromLinearToRGBM<uint8_t>(image);
                    break;
                case PixelFormat::RGB_10_11_11_REV:
                    data = fromLinearToRGB_10_11_11_REV(image);
                    break;
                case PixelFormat::sRGB:
                    if (dstChannels == 4) {
                        data = fromLinearTosRGB<uint8_t, 4>(image);
                    } else {
                        data = fromLinearTosRGB<uint8_t, 3>(image);
                    }
                    break;
                case PixelFormat::LINEAR_RGB:
                    if (dstChannels == 4) {
                        data = fromLinearToRGB<uint8_t, 4>(image);
                    } else {
                        data = fromLinearToRGB<uint8_t, 3>(image);
                    }
                    break;
            }
        }

        for (size_t y = 0; y < height; y++) {
            row_pointers[y] = reinterpret_cast<png_bytep>
                    (&data[y * width * dstChannels * sizeof(uint8_t)]);
        }

        png_write_image(mPNG, row_pointers.get());
        png_write_end(mPNG, mInfo);
        mStream.flush();
    } catch (std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while encoding PNG: " << e.what() << std::endl;
        mStream.seekp(mStreamStartPos);
        return false;
    }
    return true;
}

void PNGEncoder::cb_stream(png_structp png, png_bytep buffer, png_size_t size) {
    PNGEncoder* that = static_cast<PNGEncoder*>(png_get_io_ptr(png));
    that->stream(buffer, size);
}

void PNGEncoder::stream(void* buffer, size_t size) {
    mStream.write(static_cast<char *>(buffer), size);
    if (!mStream.good()) {
        throw std::runtime_error("Problem with the PNG stream.");
    }
}

void PNGEncoder::cb_error(png_structp png, png_const_charp) {
    PNGEncoder* that = static_cast<PNGEncoder*>(png_get_error_ptr(png));
    that->error();
}

void PNGEncoder::error() {
    throw std::runtime_error("Error while encoding PNG stream.");
}

//-------------------------------------------------------------------------------------------------

HDREncoder* HDREncoder::create(std::ostream& stream) {
    HDREncoder* encoder = new HDREncoder(stream);
    return encoder;
}

HDREncoder::HDREncoder(std::ostream& stream)
    : mStream(stream), mStreamStartPos(stream.tellp()) {
}

void HDREncoder::float2rgbe(uint8_t rgbe[4], const float3& in) {
    int e;

    // RGBE can't handle negative floats
    float3 color(in);
    if (color.r < 0) color.r = 0;
    if (color.g < 0) color.g = 0;
    if (color.b < 0) color.b = 0;

    float v = std::max(color.r, std::max(color.g, color.b));
    if (v < 1e-32f) {
        rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
    } else {
        v = std::frexp(v, &e) * 256 / v;  // m*2^e = v
        rgbe[0] = uint8_t(color.r * v);
        rgbe[1] = uint8_t(color.g * v);
        rgbe[2] = uint8_t(color.b * v);
        rgbe[3] = uint8_t(e + 128);
    }
}

size_t HDREncoder::countRepeats(uint8_t const* data, size_t length) {
    length = std::min(size_t(127), length);
    uint8_t v = data[0];
    for (size_t i=1 ; i<length ; i++) {
        if (data[i] != v) {
            return i;
        }
    }
    return length;
}

size_t HDREncoder::countNonRepeats(uint8_t const* data, size_t length) {
    length = std::min(size_t(128), length);
    size_t same = 1;
    uint8_t v = data[0];
    for (size_t i=1 ; i<length ; i++) {
        if (data[i] == v) {
            same++;
            if (same >= 3) {
                // non-repeats are always at least 3 bytes long
                return i;
            }
        } else {
            same = 1;
            v = data[i];
        }
    }
    return length;
}

void HDREncoder::rle(std::ostream& out, uint8_t const* data, size_t length) {
    uint8_t const* const end = data + length;
    while (data < end) {
        size_t c = countRepeats(data, end-data);
        if (c >= 3) {
            out.put((char)(c + 128));
            out.put(data[0]);
            data += c;
            continue;
        }
        c = countNonRepeats(data, end-data);
        out.put((char)c);
        out.write((char const*)data , c);
        data += c;
    }
}

bool HDREncoder::encode(const LinearImage& image) {
    if (image.getChannels() != 3) {
        return false;
    }

    try {
        // Write header (8 bit color depth)
        size_t width = image.getWidth();
        size_t height = image.getHeight();

        mStream << "#?RADIANCE" << std::endl;
        mStream << "# cmgen" << std::endl;
        mStream << "FORMAT=32-bit_rle_rgbe" << std::endl;
        mStream << "GAMMA=" << std::to_string(1) << std::endl;
        mStream << "EXPOSURE=" << std::to_string(0) << std::endl;
        mStream << std::endl;
        mStream << "-Y " << std::to_string(height) << " "
                << "+X " << std::to_string(width) << std::endl;

        // The Radiance format is not expected to use RLE encoding when
        // scanlines are less than 8 pixels or more than 32,767 pixels
        if (width < 8 || width > 32767) {
            for (uint32_t y = 0; y < height; y++) {
                uint8_t p[4];
                auto data = image.get<float3>(0, y);
                for (size_t x = 0; x < width; ++x, ++data) {
                    float2rgbe(p, *data);
                    mStream.write((char*) &p, 4);
                }
            }
        } else {
            std::unique_ptr<uint8_t[]> rgbe(new uint8_t[width*4]);
            uint8_t* const r = &rgbe[0];
            uint8_t* const g = &rgbe[width];
            uint8_t* const b = &rgbe[2*width];
            uint8_t* const e = &rgbe[3*width];
            uint16_t magic = 0x0202;
            uint16_t widthNetwork = htons(width);

            for (uint32_t y = 0; y < height; y++) {
                // convert one scanline to RGBE
                uint8_t p[4];
                auto data = image.get<float3>(0, y);
                for (size_t x = 0; x < width; ++x, ++data) {
                    float2rgbe(p, *data);
                    r[x] = p[0];
                    g[x] = p[1];
                    b[x] = p[2];
                    e[x] = p[3];
                }
                // now RLE-compress each plane
                mStream.write((char*) &magic, 2);
                mStream.write((char*) &widthNetwork, 2);
                rle(mStream, r, width);
                rle(mStream, g, width);
                rle(mStream, b, width);
                rle(mStream, e, width);
            }
        }

        mStream.flush();
    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while encoding HDR: " << e.what() << std::endl;
        mStream.seekp(mStreamStartPos);
        return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------

const char PSDEncoder::sig[] = { '8', 'B', 'P', 'S', 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

PSDEncoder* PSDEncoder::create(std::ostream& stream, const std::string& compression) {
    PSDEncoder* encoder = new PSDEncoder(stream, compression);
    return encoder;
}

PSDEncoder::PSDEncoder(std::ostream& stream, const std::string& compression)
        : mStream(stream), mStreamStartPos(stream.tellp()), mCompression(compression) {
}

static inline void write32(std::ostream& stream, float f) {
    uint32_t data = htonl(*reinterpret_cast<uint32_t*>(&f));
    stream.write(reinterpret_cast<char*>(&data), sizeof(uint32_t));
}

static inline void write16(std::ostream& stream, float f) {
    uint16_t data = htons(static_cast<uint16_t>(std::min(std::max(0.0f, f), 1.0f) * 65535.0f));
    stream.write(reinterpret_cast<char*>(&data), sizeof(uint16_t));
}

static inline void write32i(std::ostream& stream, uint32_t v) {
    uint32_t word = htonl(v);
    stream.write(reinterpret_cast<char*>(&word), sizeof(uint32_t));
}

static inline void write16i(std::ostream& stream, uint16_t v) {
    uint16_t word = htons(v);
    stream.write(reinterpret_cast<char*>(&word), sizeof(uint16_t));
}

static inline void write8i(std::ostream& stream, uint8_t v) {
    stream.write(reinterpret_cast<const char*>(&v), sizeof(uint8_t));
}

bool PSDEncoder::encode(const LinearImage& image) {
    static const uint16_t kColorModeRGB = 3;
    static const uint16_t kCompressionRAW = 0;
    // preview mode: 0 = highlight compression, 1 = exposure & gamma
    static const uint32_t kToningPreviewExposureGamma = 1;

    if (image.getChannels() != 3) {
        return false;
    }

    try {
        size_t width = image.getWidth();
        size_t height = image.getHeight();

        uint16_t depth = mCompression == "32" ?
                         static_cast<uint16_t>(32) : static_cast<uint16_t>(16);

        mStream.write(sig, sizeof(sig));
        write16i(mStream, 3); // channels
        write32i(mStream, static_cast<uint32_t>(height));
        write32i(mStream, static_cast<uint32_t>(width));
        write16i(mStream, depth);
        write16i(mStream, kColorModeRGB);

        // color mode data section
        // 32 bits images need a lot of magic HDR toning information
        // this information is undocumented so we simply use what seems
        // to be Photoshop's default toning data (as of Photoshop CC 2015)
        if (depth == 16) {
            write32i(mStream, 0);
        } else {
            write32i(mStream, 112);
            mStream.write("hdrt", 4);

            write32i(mStream, 3); // version?
            write32(mStream, 0.23f); // edge glow strength
            write32i(mStream, 2); // ??

            write32i(mStream, 8);
            const uint8_t presetName[] = {
                    // "Default" in UTF-16, null-terminated
                    0x00, 0x44, 0x00, 0x65, 0x00, 0x66, 0x00, 0x61,
                    0x00, 0x75, 0x00, 0x6C, 0x00, 0x74, 0x00, 0x00
            };
            mStream.write(reinterpret_cast<const char*>(presetName), sizeof(presetName));

            // toning curve
            write16i(mStream, 2); // ??
            write16i(mStream, 2); // number of points
            // point 1
            write16i(mStream, 0); // input
            write16i(mStream, 0); // output
            // point 2
            write16i(mStream, 255); // input
            write16i(mStream, 255); // output
            // corners (0 = corner, 1 = not a corner)
            write8i(mStream, 1); // point 1
            write8i(mStream, 1); // point 2

            write32i(mStream, 0); // ??
            write32i(mStream, 0); // ??

            write32(mStream, 16.0f); // edge glow radius

            write32i(mStream, kToningPreviewExposureGamma); // preview mode

            write32(mStream, 0.0f); // exposure
            write32(mStream, 1.0f); // gamma

            mStream.write("hdra", 4);
            write32i(mStream, 6); // version?

            write32(mStream,  0.0f); // exposure
            write32(mStream, 20.0f); // saturation, in %
            write32(mStream, 30.0f); // detail, in %
            write32(mStream,  0.0f); // shadow, in %
            write32(mStream,  0.0f); // highlight, in %
            write32(mStream,  1.0f); // gamma
            write32(mStream,  0.0f); // vibrance

            write16i(mStream, 0); // flags, 0x0 = smooth edges off
        }

        // image resources section
        write32i(mStream, 0);
        // layer and mask info section
        write32i(mStream, 0);
        // compression format
        write16i(mStream, kCompressionRAW);

        if (depth == 32) {
            for (size_t channel = 0; channel < 3; channel++) {
                for (uint32_t y = 0; y < height; y++) {
                    auto data = image.get<float3>(0, y);
                    for (size_t x = 0; x < width; x++) {
                        write32(mStream, (*data)[channel]);
                        data++;
                    }
                }
            }
        } else {
            for (size_t channel = 0; channel < 3; channel++) {
                for (uint32_t y = 0; y < height; y++) {
                    auto data = image.get<float3>(0, y);
                    for (size_t x = 0; x < width; x++) {
                        write16(mStream, linearTosRGB((*data)[channel]));
                        data++;
                    }
                }
            }
        }

        mStream.flush();
    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while encoding PSD: " << e.what() << std::endl;
        mStream.seekp(mStreamStartPos);
        return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------

EXREncoder* EXREncoder::create(std::ostream& stream, const std::string& compression,
                               const std::string& destName) {
    EXREncoder* encoder = new EXREncoder(stream, compression, destName);
    return encoder;
}

EXREncoder::EXREncoder(std::ostream& stream, const std::string& compression,
                       const std::string& destName)
        : mStream(stream), mStreamStartPos(stream.tellp()),
          mDestName(destName), mCompression(compression) {
}

static int toEXRCompression(const std::string& c) {
    if (c.empty()) {
        return TINYEXR_COMPRESSIONTYPE_PIZ;
    } else if (c == "RAW") {
        return TINYEXR_COMPRESSIONTYPE_NONE;
    } else if (c == "RLE") {
        return TINYEXR_COMPRESSIONTYPE_ZIPS;
    } else if (c == "ZIPS") {
        return TINYEXR_COMPRESSIONTYPE_ZIPS;
    } else if (c == "ZIP") {
        return TINYEXR_COMPRESSIONTYPE_ZIP;
    } else if (c == "PIZ") {
        return TINYEXR_COMPRESSIONTYPE_PIZ;
    }
    throw std::runtime_error("unknown compression scheme " + c);
}

bool EXREncoder::encode(const LinearImage& image) {
    if (image.getChannels() != 3) {
        return false;
    }

    try {
        EXRHeader header;
        InitEXRHeader(&header);

        EXRImage exrImage;
        InitEXRImage(&exrImage);

        size_t width = image.getWidth();
        size_t height = image.getHeight();

        exrImage.num_channels = 3;
        exrImage.width = static_cast<int>(width);
        exrImage.height = static_cast<int>(height);

        std::unique_ptr<float[]> r(new float[width * height]);
        std::unique_ptr<float[]> g(new float[width * height]);
        std::unique_ptr<float[]> b(new float[width * height]);

        size_t i = 0;
        for (uint32_t y = 0; y < height; y++) {
            auto data = image.get<float3>(0, y);
            for (size_t x = 0; x < width; x++, data++) {
                r[i] = data->r;
                g[i] = data->g;
                b[i] = data->b;
                i++;
            }
        }

        float* imageData[3];
        imageData[0] = &b[0];
        imageData[1] = &g[0];
        imageData[2] = &r[0];

        exrImage.images = (unsigned char**) imageData;

        header.num_channels = 3;
        header.compression_type = toEXRCompression(mCompression);
        header.channels = (EXRChannelInfo*) malloc(sizeof(EXRChannelInfo) * header.num_channels);

        header.channels[0].name[0] = 'B';
        header.channels[0].name[1] = '\0';
        header.channels[1].name[0] = 'G';
        header.channels[1].name[1] = '\0';
        header.channels[2].name[0] = 'R';
        header.channels[2].name[1] = '\0';

        header.pixel_types = (int*) malloc(sizeof(int) * header.num_channels);
        header.requested_pixel_types = (int*) malloc(sizeof(int) * header.num_channels);
        for (i = 0; i < header.num_channels; i++) {
            header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
            header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF;
        }

        unsigned char* outData;
        const char* error;
        size_t size = SaveEXRImageToMemory(&exrImage, &header, &outData, &error);
        if (size > 0 && outData) {
            mStream.write(reinterpret_cast<char*>(outData), size);
            free(outData);
        } else {
            std::cerr << "Runtime error while encoding EXR: " << error << std::endl;
            mStream.seekp(mStreamStartPos);
        }

        free(header.channels);
        free(header.pixel_types);
        free(header.requested_pixel_types);
    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while encoding EXR: " << e.what() << std::endl;
        mStream.seekp(mStreamStartPos);
        return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------

const uint32_t DDS_MAGIC       = 0x20534444; // "DDS"
const uint32_t DDS_FOURCC_DX10 = 0x30315844; // "DX10"

#pragma pack(push, 1)
struct DDS_PIXELFORMAT {
    uint32_t UTILS_UNUSED dwSize;
    uint32_t              dwFlags;
    uint32_t UTILS_UNUSED dwFourCC;
    uint32_t UTILS_UNUSED dwRGBBitCount;
    uint32_t UTILS_UNUSED dwRBitMask;
    uint32_t UTILS_UNUSED dwGBitMask;
    uint32_t UTILS_UNUSED dwBBitMask;
    uint32_t UTILS_UNUSED dwABitMask;
};

struct DDS_HEADER {
    uint32_t        UTILS_UNUSED dwSize;
    uint32_t                     dwFlags;
    uint32_t        UTILS_UNUSED dwHeight;
    uint32_t        UTILS_UNUSED dwWidth;
    uint32_t        UTILS_UNUSED dwPitchOrLinearSize;
    uint32_t        UTILS_UNUSED dwDepth;
    uint32_t        UTILS_UNUSED dwMipMapCount;
    uint32_t        UTILS_UNUSED dwReserved1[11];
    DDS_PIXELFORMAT UTILS_UNUSED ddspf;
    uint32_t        UTILS_UNUSED dwCaps;
    uint32_t        UTILS_UNUSED dwCaps2;
    uint32_t        UTILS_UNUSED dwCaps3;
    uint32_t        UTILS_UNUSED dwCaps4;
    uint32_t        UTILS_UNUSED dwReserved2;
};

struct DDS_HEADER_DXT10 {
    uint32_t              dxgiFormat;
    uint32_t UTILS_UNUSED resourceDimension;
    uint32_t UTILS_UNUSED miscFlag;
    uint32_t UTILS_UNUSED arraySize;
    uint32_t UTILS_UNUSED miscFlags2;
};
#pragma pack(pop)

#define DDSD_CAPS	     0x1
#define DDSD_HEIGHT	     0x2
#define DDSD_WIDTH	     0x4
#define DDSD_PITCH       0x8
#define DDSD_PIXELFORMAT 0x1000

#define DDSCAPS_TEXTURE  0x1000

#define DDPF_FOURCC      0x4

#define DDS_RESOURCE_DIMENSION_TEXTURE2D 0x3

#define DXGI_FORMAT_R32G32B32A32_FLOAT 2
#define DXGI_FORMAT_R16G16B16A16_FLOAT 10
#define DXGI_FORMAT_R32G32_FLOAT       16
#define DXGI_FORMAT_R8G8B8A8_UINT      30
#define DXGI_FORMAT_R16G16_FLOAT       34
#define DXGI_FORMAT_R32_FLOAT          41
#define DXGI_FORMAT_R8G8_UINT          50
#define DXGI_FORMAT_R16_FLOAT          54
#define DXGI_FORMAT_R8_UINT            62

DDSEncoder* DDSEncoder::create(std::ostream& stream, const std::string& compression,
                               PixelFormat format) {
    DDSEncoder* encoder = new DDSEncoder(stream, compression, format);
    return encoder;
}

DDSEncoder::DDSEncoder(std::ostream& stream, const std::string& compression, PixelFormat format)
        : mStream(stream), mStreamStartPos(stream.tellp()),
          mCompression(compression), mFormat(format) {
    if (format == PixelFormat::sRGB) {
        if (compression != "8") mFormat = PixelFormat::LINEAR_RGB;
    }
}

static uint32_t chooseBpp(const LinearImage& image, const std::string& compression) {
    size_t depth = 16;
    if (compression == "8") depth = 8;
    if (compression == "32") depth = 32;

    size_t index = static_cast<size_t>(std::log2(depth)) - 3;

    static const uint32_t formats[3][3] = {
            { 1, 2,  4 },
            { 2, 4,  8 },
            { 4, 8, 16 },
    };

    return formats[image.getChannels() - 1][index];
}

static uint32_t chooseDXGIFormat(const LinearImage& image, const std::string& compression) {
    size_t depth = 16;
    if (compression == "8") depth = 8;
    if (compression == "32") depth = 32;

    size_t index = static_cast<size_t>(std::log2(depth)) - 3;

    static const uint32_t formats[3][3] = {
            { DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_R32_FLOAT },
            { DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_R16G16_FLOAT, DXGI_FORMAT_R32G32_FLOAT },
            { DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT },
    };

    return formats[image.getChannels() - 1][index];
}

bool DDSEncoder::encode(const LinearImage& image) {
    try {
        size_t width = image.getWidth();
        size_t height = image.getHeight();

        DDS_PIXELFORMAT ddspf = { };
        ddspf.dwSize   = sizeof(ddspf);
        ddspf.dwFlags  = DDPF_FOURCC;
        ddspf.dwFourCC = DDS_FOURCC_DX10;

        DDS_HEADER header = { };
        header.dwSize              = sizeof(header);
        header.dwFlags             = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH |
                                     DDSD_PIXELFORMAT | DDSD_PITCH;
        header.dwHeight            = static_cast<uint32_t>(height);
        header.dwWidth             = static_cast<uint32_t>(width);
        header.dwDepth             = 1;
        header.dwMipMapCount       = 1;
        header.dwPitchOrLinearSize = static_cast<uint32_t>(width * chooseBpp(image, mCompression));
        header.ddspf               = ddspf;
        header.dwCaps              = DDSCAPS_TEXTURE;

        DDS_HEADER_DXT10 headerDX10 = { };
        headerDX10.dxgiFormat        = chooseDXGIFormat(image, mCompression);
        headerDX10.resourceDimension = DDS_RESOURCE_DIMENSION_TEXTURE2D;
        headerDX10.arraySize         = 1;

        mStream.write((const char*) &DDS_MAGIC, sizeof(DDS_MAGIC));
        mStream.write((const char*) &header, sizeof(header));
        mStream.write((const char*) &headerDX10, sizeof(headerDX10));

        switch (headerDX10.dxgiFormat) {
            case DXGI_FORMAT_R8_UINT: {
                switch (mFormat) {
                    case PixelFormat::sRGB:
                        for (uint32_t y = 0; y < height; y++) {
                            const float* data = image.getPixelRef(0, y);
                            for (size_t x = 0; x < width; x++) {
                                uint8_t b = (uint8_t) (linearTosRGB(saturate(*data)) * 255);
                                mStream.write((const char*) &b, 1);
                                data++;
                            }
                        }
                        break;
                    case PixelFormat::LINEAR_RGB:
                        for (uint32_t y = 0; y < height; y++) {
                            const float* data = image.getPixelRef(0, y);
                            for (size_t x = 0; x < width; x++) {
                                uint8_t b = (uint8_t) (saturate(*data) * 255);
                                mStream.write((const char*) &b, 1);
                                data++;
                            }
                        }
                        break;
                }
                break;
            }
            case DXGI_FORMAT_R16_FLOAT: {
                for (uint32_t y = 0; y < height; y++) {
                    const float* data = image.getPixelRef(0, y);
                    for (size_t x = 0; x < width; x++) {
                        half p = half(*data);
                        mStream.write((const char*) &p, 2);
                        data++;
                    }
                }
                break;
            }
            case DXGI_FORMAT_R32_FLOAT: {
                for (uint32_t y = 0; y < height; y++) {
                    const float* data = image.getPixelRef(0, y);
                    mStream.write((const char*) data, width * sizeof(float));
                }
                break;
            }
            case DXGI_FORMAT_R8G8_UINT: {
                switch (mFormat) {
                    case PixelFormat::sRGB:
                        for (uint32_t y = 0; y < height; y++) {
                            const float2* data = reinterpret_cast<float2 const*>(image.getPixelRef(0, y));
                            for (size_t x = 0; x < width; x++) {
                                uint8_t b;
                                b = (uint8_t) (linearTosRGB(saturate(data->g)) * 255);
                                mStream.write((const char*) &b, 1);
                                b = (uint8_t) (linearTosRGB(saturate(data->r)) * 255);
                                mStream.write((const char*) &b, 1);
                                data++;
                            }
                        }
                        break;
                    case PixelFormat::LINEAR_RGB:
                        for (uint32_t y = 0; y < height; y++) {
                            const float2* data = reinterpret_cast<float2 const*>(image.getPixelRef(0, y));
                            for (size_t x = 0; x < width; x++) {
                                uint8_t b;
                                b = (uint8_t) (saturate(data->g) * 255);
                                mStream.write((const char*) &b, 1);
                                b = (uint8_t) (saturate(data->r) * 255);
                                mStream.write((const char*) &b, 1);
                                data++;
                            }
                        }
                        break;
                }
                break;
            }
            case DXGI_FORMAT_R16G16_FLOAT: {
                for (uint32_t y = 0; y < height; y++) {
                    const float2* data = reinterpret_cast<float2 const*>(image.getPixelRef(0, y));
                    for (size_t x = 0; x < width; x++) {
                        half2 p = half2(*data);
                        mStream.write((const char*) &p, sizeof(p));
                        data++;
                    }
                }
                break;
            }
            case DXGI_FORMAT_R32G32_FLOAT: {
                for (uint32_t y = 0; y < height; y++) {
                    const float2* data = reinterpret_cast<float2 const*>(image.getPixelRef(0, y));
                    mStream.write((const char*) data, width * sizeof(float2));
                }
                break;
            }
            case DXGI_FORMAT_R8G8B8A8_UINT: {
                switch (mFormat) {
                    case PixelFormat::sRGB:
                        for (uint32_t y = 0; y < height; y++) {
                            auto data = image.get<float3>(0, y);
                            for (size_t x = 0; x < width; x++) {
                                uint8_t r = (uint8_t) (linearTosRGB(saturate(data->r)) * 255);
                                uint8_t g = (uint8_t) (linearTosRGB(saturate(data->g)) * 255);
                                uint8_t b = (uint8_t) (linearTosRGB(saturate(data->b)) * 255);
                                uint32_t p = (uint8_t) 0xff << 24 | b << 16 | g << 8 | r;
                                mStream.write((const char*) &p, 4);
                                data++;
                            }
                        }

                        break;
                    case PixelFormat::LINEAR_RGB:
                        for (uint32_t y = 0; y < height; y++) {
                            auto data = image.get<float3>(0, y);
                            for (size_t x = 0; x < width; x++) {
                                uint8_t r = (uint8_t) (saturate(data->r) * 255);
                                uint8_t g = (uint8_t) (saturate(data->g) * 255);
                                uint8_t b = (uint8_t) (saturate(data->b) * 255);
                                uint32_t p = (uint8_t) 0xff << 24 | b << 16 | g << 8 | r;
                                mStream.write((const char*) &p, 4);
                                data++;
                            }
                        }
                        break;
                }
                break;
            }
            case DXGI_FORMAT_R16G16B16A16_FLOAT: {
                for (uint32_t y = 0; y < height; y++) {
                    auto data = image.get<float3>(0, y);
                    for (size_t x = 0; x < width; x++) {
                        half4 p = half4(half3(*data), 1);
                        mStream.write((const char*) &p, sizeof(ushort4));
                        data++;
                    }
                }
                break;
            }
            case DXGI_FORMAT_R32G32B32A32_FLOAT: {
                for (uint32_t y = 0; y < height; y++) {
                    auto data = image.get<float3>(0, y);
                    for (size_t x = 0; x < width; x++) {
                        float4 p = float4(3.0f, 3.0f, 3.0f, 1.0f);
                        mStream.write((const char*) &p, sizeof(float4));
                        data++;
                    }
                }
                break;
            }
            default:
                break;
        }

        mStream.flush();
    } catch(std::runtime_error& e) {
        // reset the stream, like we found it
        std::cerr << "Runtime error while encoding PSD: " << e.what() << std::endl;
        mStream.seekp(mStreamStartPos);
        return false;
    }
    return true;
}

} // namespace image
