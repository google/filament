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

#include <image/ColorTransform.h>
#include <image/ImageOps.h>
#include <image/ImageSampler.h>
#include <image/KtxBundle.h>

#include <imageio/ImageDecoder.h>
#include <imageio/ImageEncoder.h>

#include <utils/JobSystem.h>
#include <utils/Path.h>

#include <getopt/getopt.h>

using namespace filament::math;
using namespace image;
using namespace utils;

static ImageEncoder::Format g_format = ImageEncoder::Format::PNG_LINEAR;
static bool g_formatSpecified = false;
static bool g_ktxContainer = false;
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
            "   --format=[exr|hdr|psd|png|dds|ktx], -f [exr|hdr|psd|png|dds|ktx]\n"
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
    static const char *license[] = {
        #include "licenses/licenses.inc"
        nullptr
    };

    const char **p = &license[0];
    while (*p)
        std::cout << *p++ << std::endl;
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
                if (arg == "ktx") {
                    g_ktxContainer = true;
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

float normalFiltering(float roughness, float variance, float threshold) {
    return std::sqrt(roughness * roughness + std::min(2.0f * variance, threshold * threshold));
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
    float variance = 0.0f;

    if (r < 1.0f) {
        float r2 = r * r;
        float kappa = (3.0f * r - r * r2) / (1.0f - r2);
        variance = 0.25f / kappa;
    }

    // See: Karis, 2018, "Normal map filtering using vMF (part 3)"
    // The original formulation in Neubelt et al. 2013,
    // "Crafting a Next-Gen Material Pipeline for The Order: 1886" contains an error
    // and defines alpha' = sqrt(alpha^2 + 1 / kappa)
    return normalFiltering(roughness, variance, 0.2f);
}

void prefilter(const LinearImage& normal, const size_t mipLevel, LinearImage& output) {
    const size_t width = output.getWidth();
    const size_t height = output.getHeight();
    const size_t sampleCount = 1u << mipLevel;

    for (size_t y = 0; y < height; y++) {
        auto outputRow = output.get<float>(0, y);
        for (size_t x = 0; x < width; x++, outputRow++) {
            const float2 uv = (float2(x, y) + 0.5f) / float2(width, height);
            const float2 pos = uv * normal.getWidth();
            const float roughness = g_roughness;
            *outputRow = solveVMF(pos, sampleCount, roughness, normal);
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
        auto outputRow = output.get<float>(0, y);
        for (size_t x = 0; x < width; x++, outputRow++) {
            auto data = roughness.get<float>(x, y);
            if (FIRST_MIP) {
                *outputRow = *data;
            } else {
                const float2 uv = (float2(x, y) + 0.5f) / float2(width, height);
                const float2 pos = uv * normal.getWidth();
                *outputRow = solveVMF(pos, sampleCount, *data, normal);
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
            std::cerr << "The input normal and roughness maps must have the same dimensions"
                    << std::endl;
            exit(1);
        }

        roughnessImage = extractChannel(roughnessImage, 0);
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

    if (hasRoughnessMap) {
        mipImages.resize(mipLevels);
        mipImages[0] = roughnessImage;
        image::generateMipmaps(roughnessImage, image::Filter::BOX, &mipImages[1], mipLevels - 1);
    }

    JobSystem js;
    js.adopt();

    // For thread safety, we allocate each KTX blob now, before invoking the job system.
    image::KtxBundle bundle(mipLevels, 1, false);
    if (g_ktxContainer) {
        bundle.info() = {
            .endianness = KtxBundle::ENDIAN_DEFAULT,
            .glType = KtxBundle::UNSIGNED_BYTE,
            .glTypeSize = 1,
            .glFormat = KtxBundle::LUMINANCE,
            .glInternalFormat = KtxBundle::LUMINANCE,
            .glBaseInternalFormat = KtxBundle::LUMINANCE,
            .pixelWidth = (uint32_t) width,
            .pixelHeight = (uint32_t) height,
            .pixelDepth = 0,
        };
        for (uint32_t i = 0; i < mipLevels; i++) {
            bundle.allocateBlob({i}, (width >> i) * (height >> i));
        }
    }

    JobSystem::Job* parent = js.createJob();
    for (size_t i = 0; i < mipLevels; i++) {
        JobSystem::Job* mip = jobs::createJob(js, parent, [&bundle, &normalImage, &mipImages,
                outputMap, i, width, height, hasRoughnessMap]() {
            const size_t w = width >> i;
            const size_t h = height >> i;

            LinearImage image(w, h, 1);

            if (i == 0) {
                if (hasRoughnessMap) {
                    if (mipImages.at(0).getWidth() == image.getWidth()) {
                        const size_t size = image.getWidth() * image.getHeight() * sizeof(float);
                        memcpy(image.getPixelRef(), mipImages.at(0).getPixelRef(), size);
                    } else {
                        prefilter<true>(normalImage, mipImages.at(0), 0, image);
                    }
                } else {
                    std::fill_n(image.get<float>(), w * h, g_roughness);
                }
            } else {
                if (hasRoughnessMap) {
                    prefilter<false>(normalImage, mipImages.at(i), i, image);
                } else {
                    prefilter(normalImage, i, image);
                }
            }

            if (g_ktxContainer) {
                std::unique_ptr<uint8_t[]> data;
                if (g_linearOutput) {
                    data = image::fromLinearToGrayscale<uint8_t>(image);
                } else {
                    data = image::fromLinearTosRGB<uint8_t, 1>(image);
                }
                bundle.setBlob({(uint32_t) i}, data.get(), w * h);
                return;
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

    if (g_ktxContainer) {
        using namespace std;
        vector<uint8_t> fileContents(bundle.getSerializedLength());
        bundle.serialize(fileContents.data(), fileContents.size());
        ofstream outputStream(outputMap.c_str(), ios::out | ios::binary);
        outputStream.write((const char*) fileContents.data(), fileContents.size());
        outputStream.close();
    }
}
