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

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>

#include <math/vec3.h>

#include <image/ImageOps.h>

#include <imageio/ImageDecoder.h>
#include <imageio/ImageEncoder.h>

#include <utils/JobSystem.h>
#include <utils/Path.h>

#include <getopt/getopt.h>

using namespace math;
using namespace image;
using namespace utils;

static ImageEncoder::Format g_format = ImageEncoder::Format::PNG_LINEAR;
static bool g_formatSpecified = false;
static bool g_linearOutput = false;
static std::string g_compression;

static Path g_roughnessMap;
static float g_roughness = 0.0;

static void printUsage(const char* name) {
    std::string execName(Path(name).getName());
    std::string usage(
            "ROUGHNESSPREFILTER generates pre-filtered roughness maps from normal maps\n"
            "to help mitigate specular aliasing.\n"
            "Usage:\n"
            "    ROUGHNESSPREFILTER [options] <normal-map> <output-roughness-map>\n"
            "\n"
            "Output note:\n"
            "    One file will be generated per mip-level. The size of the first level will be\n"
            "    the greater of the input normal map and input roughness map.\n"
            "\n"
            "Supported input formats:\n"
            "    PNG, 8 and 16 bits\n"
            "    Radiance (.hdr)\n"
            "    Photoshop (.psd), 16 and 32 bits\n"
            "    OpenEXR (.exr)\n"
            "\n"
            "Options:\n"
            "   --help, -h\n"
            "       print this message\n\n"
            "   --license\n"
            "       Print copyright and license information\n\n"
            "   --roughness=[0..1], -r [0..1]\n"
            "       desired constant roughness, ignored if --roughness-map is specified\n\n"
            "   --roughness-map=<input-roughness-map>, -m <input-roughness-map>\n"
            "       input roughness map\n\n"
            "   --format=[exr|hdr|psd|png|dds], -f [exr|hdr|psd|png|dds]\n"
            "       specify output file format, inferred from file name if omitted\n\n"
            "   --compression=COMPRESSION, -c COMPRESSION\n"
            "       format specific compression:\n"
            "           PNG: Ignored\n"
            "           Radiance: Ignored\n"
            "           Photoshop: 16 (default), 32\n"
            "           OpenEXR: RAW, RLE, ZIPS, ZIP, PIZ (default)\n"
            "           DDS: 8, 16 (default), 32\n\n"
            "   --linear, -l\n"
            "       force linear output when the PNG format is selected\n\n"
    );

    const std::string from("ROUGHNESSPREFILTER");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), execName);
    }
    printf("%s", usage.c_str());
}

static void license() {
    std::cout <<
    #include "licenses/licenses.inc"
    ;
}

static int handleArguments(int argc, char* argv[]) {
    static constexpr const char* OPTSTR = "hr:c:f:m:l";
    static const struct option OPTIONS[] = {
            { "help",                 no_argument, nullptr, 'h' },
            { "license",              no_argument, nullptr, 's' },
            { "format",         required_argument, nullptr, 'f' },
            { "compression",    required_argument, nullptr, 'c' },
            { "roughness",      required_argument, nullptr, 'r' },
            { "roughness-map",  required_argument, nullptr, 'm' },
            { "linear",               no_argument, nullptr, 'l' },
            { nullptr, 0, nullptr, 0 }  // termination of the option list
    };

    int opt;
    int optionIndex = 0;

    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
                // break;
            case 's':
                license();
                exit(0);
                // break;
            case 'f':
                if (arg == "png") {
                    g_format = ImageEncoder::Format::PNG;
                    g_formatSpecified = true;
                }
                if (arg == "hdr") {
                    g_format = ImageEncoder::Format::HDR;
                    g_formatSpecified = true;
                }
                if (arg == "exr") {
                    g_format = ImageEncoder::Format::EXR;
                    g_formatSpecified = true;
                }
                if (arg == "psd") {
                    g_format = ImageEncoder::Format::PSD;
                    g_formatSpecified = true;
                }
                if (arg == "dds") {
                    g_format = ImageEncoder::Format::DDS_LINEAR;
                    g_formatSpecified = true;
                }
                break;
            case 'c':
                g_compression = arg;
                break;
            case 'r':
                try {
                    g_roughness = std::stof(arg);
                } catch (std::invalid_argument& e) {
                    std::cerr << "The specified roughness must be [0..1]: " << arg << std::endl;
                    exit(1);
                } catch (std::out_of_range& e) {
                    std::cerr << "The specified roughness must be [0..1]: " << arg << std::endl;
                    exit(1);
                }
                break;
            case 'm':
                g_roughnessMap = arg;
                break;
            case 'l':
                g_linearOutput = true;
                break;
        }
    }

    return optind;
}

inline bool isPOT(size_t x) {
    return !(x & (x - 1));
}

float solveVMF(const float2& pos, const size_t sampleCount, const float roughness,
        const LinearImage& normal) {

    float3 averageNormal(0.0f);
    float2 topLeft(-float(sampleCount) / 2.0f + 0.5f);

    for (size_t y = 0; y < sampleCount; y++) {
        for (size_t x = 0; x < sampleCount; x++) {
            float2 offset(topLeft + float2(x, y));
            float2 samplePos(floor(pos + offset) + 0.5f);
            float3 sampleNormal = *normal.get<float3>(size_t(samplePos.x), size_t(samplePos.y));
            sampleNormal = sampleNormal * 2.0f - 1.0f;

            averageNormal += normalize(sampleNormal);
        }
    }

    averageNormal /= (sampleCount * sampleCount);

    float r = length(averageNormal);
    float kappa = 10000.0f;

    if (r < 1.0f) {
        kappa = (3.0f * r - r * r * r) / (1.0f - r * r);
    }

    // See: Karis, 2018, "Normal map filtering using vMF (part 3)"
    // The original formulation in Neubelt et al. 2013,
    // "Crafting a Next-Gen Material Pipeline for The Order: 1886" contains an error
    // and defines alpha' = sqrt(alpha^2 + 1 / kappa)
    return std::sqrt(roughness * roughness + (2.0f / kappa));
}

void prefilter(const LinearImage& normal, const size_t mipLevel, LinearImage& output) {
    const size_t width = output.getWidth();
    const size_t height = output.getHeight();
    const size_t sampleCount = 1u << mipLevel;

    for (size_t y = 0; y < height; y++) {
        auto outputRow = output.get<float3>(0, y);
        for (size_t x = 0; x < width; x++, outputRow++) {
            const float2 uv = (float2(x, y) + 0.5f) / float2(width, height);
            const float2 pos = uv * normal.getWidth();
            const float roughness = g_roughness;
            *outputRow = float3(solveVMF(pos, sampleCount, roughness, normal));
        }
    }
}

template<bool FIRST_MIP>
void prefilter(const LinearImage& normal, const LinearImage& roughness, const size_t mipLevel,
        LinearImage& output) {
    const size_t width = output.getWidth();
    const size_t height = output.getHeight();
    const size_t sampleCount = 1u << mipLevel;

    for (size_t y = 0; y < height; y++) {
        auto outputRow = output.get<float3>(0, y);
        for (size_t x = 0; x < width; x++, outputRow++) {
            auto data = roughness.get<float3>(x, y);
            if (FIRST_MIP) {
                *outputRow = *data;
            } else {
                const float2 uv = (float2(x, y) + 0.5f) / float2(width, height);
                const float2 pos = uv * normal.getWidth();
                *outputRow = float3(solveVMF(pos, sampleCount, data->r, normal));
            }
        }
    }
}

int main(int argc, char* argv[]) {
    int optionIndex = handleArguments(argc, argv);

    int numArgs = argc - optionIndex;
    if (numArgs < 2) {
        printUsage(argv[0]);
        return 1;
    }

    Path normalMap(argv[optionIndex    ]);
    Path outputMap(argv[optionIndex + 1]);

    if (!normalMap.exists()) {
        std::cerr << "The input normal map does not exist: " << normalMap << std::endl;
        exit(1);
    }

    // make sure we load the normal maps as linear data
    std::ifstream inputStream(normalMap, std::ios::binary);
    LinearImage normalImage = ImageDecoder::decode(inputStream, normalMap,
            ImageDecoder::ColorSpace::LINEAR);
    inputStream.close();

    if (!normalImage.isValid()) {
        std::cerr << "The input normal map is invalid: " << normalMap << std::endl;
        exit(1);
    }

    if (!isPOT(normalImage.getWidth()) || !isPOT(normalImage.getHeight()) ||
            normalImage.getWidth() != normalImage.getHeight()) {
        std::cerr << "The input normal map must have equal power-of-2 dimensions: " <<
                normalImage.getWidth() << "x" << normalImage.getHeight() << std::endl;
        exit(1);
    }

    LinearImage roughnessImage;
    std::vector<LinearImage> mipImages;
    bool hasRoughnessMap = false;

    if (!g_roughnessMap.isEmpty()) {
        Path roughnessMap(g_roughnessMap);
        if (!roughnessMap.exists()) {
            std::cerr << "The input roughness map does not exist: " << roughnessMap << std::endl;
            exit(1);
        }

        hasRoughnessMap = true;

        inputStream.open(roughnessMap, std::ios::binary);
        roughnessImage = ImageDecoder::decode(inputStream, roughnessMap,
                ImageDecoder::ColorSpace::LINEAR);
        inputStream.close();

        if (!roughnessImage.isValid()) {
            std::cerr << "The input roughness map is invalid: " << roughnessMap << std::endl;
            exit(1);
        }

        if (!isPOT(roughnessImage.getWidth()) || !isPOT(roughnessImage.getHeight()) ||
                roughnessImage.getWidth() != roughnessImage.getHeight()) {
            std::cerr << "The input roughness map must have equal power-of-2 dimensions: " <<
                    roughnessImage.getWidth() << "x" << roughnessImage.getHeight() << std::endl;
            exit(1);
        }

        if (roughnessImage.getWidth() != normalImage.getWidth() ||
                roughnessImage.getHeight() != normalImage.getHeight()) {
            std::cerr << "The input normal and roughness maps must have the same dimensions" << std::endl;
            exit(1);
        }
    }

    if (!g_formatSpecified) {
        g_format = ImageEncoder::chooseFormat(outputMap, g_linearOutput);
    } else {
        if (g_format == ImageEncoder::Format::PNG && g_linearOutput) {
            g_format = ImageEncoder::Format::PNG_LINEAR;
        }
    }

    const size_t width = hasRoughnessMap ? roughnessImage.getWidth() : normalImage.getWidth();
    const size_t height = hasRoughnessMap ? roughnessImage.getHeight() : normalImage.getHeight();
    const size_t mipLevels = size_t(std::log2f(width)) + 1;

    bool exportGrayscale = false;
    switch (g_format) {
        case ImageEncoder::Format::DDS:
        case ImageEncoder::Format::DDS_LINEAR:
        case ImageEncoder::Format::PNG:
        case ImageEncoder::Format::PNG_LINEAR:
            exportGrayscale = true;
            break;
        default:
            break;
    }

    if (hasRoughnessMap) {
        mipImages.push_back(std::move(roughnessImage));
        LinearImage* prevMip = &mipImages.at(0);

        for (size_t i = 1; i <= mipLevels; i++) {
            const size_t w = width >> i;
            const size_t h = height >> i;

            LinearImage image(w, h, 3);

            for (size_t y = 0; y < h; y++) {
                auto dst = image.get<float3>(0, y);
                for (size_t x = 0; x < w; x++, dst++) {
                    float3 aa = *prevMip->get<float3>(x * 2,     y * 2);
                    float3 ba = *prevMip->get<float3>(x * 2 + 1, y * 2);
                    float3 ab = *prevMip->get<float3>(x * 2,     y * 2 + 1);
                    float3 bb = *prevMip->get<float3>(x * 2 + 1, y * 2 + 1);
                    *dst = (aa + ba + ab + bb) / 4.0f;
                }
            }

            mipImages.push_back(std::move(image));
            prevMip = &mipImages.at(i);
        }
    }

    JobSystem js;
    js.adopt();

    JobSystem::Job* parent = js.createJob();
    for (size_t i = 0; i < mipLevels; i++) {
        JobSystem::Job* mip = jobs::createJob(js, parent,
                [&normalImage, &mipImages, outputMap, i, width, height, exportGrayscale, hasRoughnessMap]() {
            const size_t w = width >> i;
            const size_t h = height >> i;

            LinearImage image(w, h, 3);

            if (i == 0) {
                if (hasRoughnessMap) {
                    if (mipImages.at(0).getWidth() == image.getWidth()) {
                        const size_t size = image.getWidth() * image.getHeight() * 12;
                        memcpy(image.getPixelRef(), mipImages.at(0).getPixelRef(), size);
                    } else {
                        prefilter<true>(normalImage, mipImages.at(0), 0, image);
                    }
                } else {
                    std::fill_n(image.get<float3>(), w * h, float3(g_roughness));
                }
            } else {
                if (hasRoughnessMap) {
                    prefilter<false>(normalImage, mipImages.at(i), i, image);
                } else {
                    prefilter(normalImage, i, image);
                }
            }

            const std::string ext = outputMap.getExtension();
            const std::string name = outputMap.getNameWithoutExtension();
            Path out = Path(outputMap.getParent()).concat(
                    name + "_" + std::to_string(i) + "." + ext); // NOLINT

            std::ofstream outputStream(out, std::ios::binary | std::ios::trunc);
            if (!outputStream.good()) {
                std::cerr << "The output file cannot be opened: " << out << std::endl;
                return;
            }
            if (exportGrayscale) {
                image = extractChannel(image, 0);
            }
            ImageEncoder::encode(outputStream, g_format, image, g_compression, out.getPath());
            outputStream.close();
            if (!outputStream.good()) {
                std::cerr << "An error occurred while writing the output file: " << out <<
                        std::endl;
            }
        });
        js.run(mip);
    }
    js.runAndWait(parent);
}
