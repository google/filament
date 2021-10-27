/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <imageio/HDRDecoder.h>

#include <image/ImageOps.h>

#include <math/vec3.h>

#include <utils/Log.h>

#include <cstring>
#include <limits>
#include <memory>
#include <sstream>

// for ntohs
#if defined(WIN32)
#    include <Winsock2.h>
#    include <utils/unwindows.h>
#else
#    include <arpa/inet.h>
#endif

using namespace utils;

namespace image {

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
    float gamma;
    float exposure;
    char sy, sx;
    unsigned int height = 0, width = 0;
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

    if (sx == '-') image = (image);
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
                if (rgbe[pixel + 3] == 0.0f) {
                    dst[x] = filament::math::float3{0.0f};
                } else {
                    filament::math::float3 v(rgbe[pixel], rgbe[pixel + 1], rgbe[pixel + 2]);
                    dst[x] = (v + 0.5f) * std::ldexp(1.0f, rgbe[pixel + 3] - (128 + 8));
                }
            }
        }
    } else {
        for (uint32_t y = 0; y < height; y++) {
            uint16_t magic;
            mStream.read((char*) &magic, 2);
            if (magic != 0x0202) {
                slog.e << "invalid scanline (magic)" << io::endl;
                return {};
            }

            uint16_t w;
            mStream.read((char*) &w, 2);
            if (ntohs(w) != width) {
                slog.e << "invalid scanline (width)" << io::endl;
                return {};
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
                            slog.e << "run length is zero" << io::endl;
                            return {};
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
                if (e[0] == 0.0f) {
                    dst[x] = filament::math::float3{0.0f};
                } else {
                    filament::math::float3 v(r[0], g[0], b[0]);
                    dst[x] = (v + 0.5f) * std::ldexp(1.0f, e[0] - (128 + 8));
                }
            }
        }
    }

    return image;
}

#ifdef IMAGEIO_LITE

LinearImage ImageDecoder::decode(std::istream& stream, const std::string& sourceName,
        ColorSpace sourceSpace) {

    Format format = Format::NONE;

    std::streampos pos = stream.tellg();
    char buf[16];
    stream.read(buf, sizeof(buf));

    if (HDRDecoder::checkSignature(buf)) {
        format = Format::HDR;
    }

    stream.seekg(pos);

    std::unique_ptr<Decoder> decoder;
    switch (format) {
        case Format::HDR:
            decoder.reset(HDRDecoder::create(stream));
            decoder->setColorSpace(ColorSpace::LINEAR);
            break;
        default:
            return LinearImage();
    }

    return decoder->decode();
}

#endif

} // namespace image
