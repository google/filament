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


#include <fstream>
#include <iostream>

#include <math/vec3.h>

#include <imageio/ImageDecoder.h>
#include <imageio/ImageEncoder.h>

#include <utils/Path.h>

#include <getopt/getopt.h>

using namespace filament::math;
using namespace image;

static image::ImageEncoder::Format g_format = image::ImageEncoder::Format::PNG;
static bool g_formatSpecified = false;
static std::string g_compression = "";

static void blend(const LinearImage& normal, const LinearImage& detail, LinearImage output);

static void printUsage(const char* name) {
    std::string execName(utils::Path(name).getName());
    std::string usage(
            "NORMALBLENDING is a tool for blending normal maps using Reoriented Normal Mapping\n"
            "Usage:\n"
            "    NORMALBLENDING [options] <normal-map> <detail-map> <output-map>\n"
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
            "   --format=[exr|hdr|psd|png|dds], -f [exr|hdr|psd|png|dds]\n"
            "       specify output file format, inferred from file name if omitted\n\n"
            "   --compression=COMPRESSION, -c COMPRESSION\n"
            "       format specific compression:\n"
            "           PNG: Ignored\n"
            "           Radiance: Ignored\n"
            "           Photoshop: 16 (default), 32\n"
            "           OpenEXR: RAW, RLE, ZIPS, ZIP, PIZ (default)\n"
            "           DDS: 8, 16 (default), 32\n\n"
    );

    const std::string from("NORMALBLENDING");
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
    static constexpr const char* OPTSTR = "hf:c:";
    static const struct option OPTIONS[] = {
            { "help",                 no_argument, 0, 'h' },
            { "license",              no_argument, 0, 'l' },
            { "format",         required_argument, 0, 'f' },
            { "compression",    required_argument, 0, 'c' },
            { 0, 0, 0, 0 }  // termination of the option list
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
            case 'l':
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
        }
    }

    return optind;
}

int main(int argc, char* argv[]) {
    int optionIndex = handleArguments(argc, argv);

    int numArgs = argc - optionIndex;
    if (numArgs < 3) {
        printUsage(argv[0]);
        return 1;
    }

    utils::Path normalMap(argv[optionIndex    ]);
    utils::Path detailMap(argv[optionIndex + 1]);
    utils::Path outputMap(argv[optionIndex + 2]);

    if (!normalMap.exists()) {
        std::cerr << "The input normal map does not exist: " << normalMap << std::endl;
        exit(1);
    }

    if (!detailMap.exists()) {
        std::cerr << "The input detail map does not exist: " << detailMap << std::endl;
        exit(1);
    }

    // make sure we load the normal maps as linear data
    std::ifstream inputStream(normalMap, std::ios::binary);
    LinearImage normalImage = ImageDecoder::decode(inputStream, normalMap,
            ImageDecoder::ColorSpace::LINEAR);
    if (!normalImage.isValid()) {
        std::cerr << "The input normal map is invalid: " << normalMap << std::endl;
        exit(1);
    }
    inputStream.close();

    inputStream.open(detailMap, std::ios::binary);
    LinearImage detailImage = ImageDecoder::decode(inputStream, detailMap,
            ImageDecoder::ColorSpace::LINEAR);
    if (!detailImage.isValid()) {
        std::cerr << "The detail normal map is invalid: " << detailMap << std::endl;
        exit(1);
    }
    inputStream.close();

    // TODO: handle normal maps of different sizes
    if (normalImage.getWidth() != detailImage.getWidth() ||
            normalImage.getHeight() != detailImage.getHeight()) {
        std::cerr << "The normal and detail maps must have the same dimensions:" << std::endl
                << "    Normal map: " << normalImage.getWidth() << "x" << normalImage.getHeight()
                << std::endl
                << "    Detail map: " << detailImage.getWidth() << "x" << detailImage.getHeight()
                << std::endl;
        exit(1);
    }

    size_t width = normalImage.getWidth();
    size_t height = normalImage.getHeight();
    LinearImage image(width, height, 3);

    blend(normalImage, detailImage, image);

    if (!g_formatSpecified) {
        g_format = ImageEncoder::chooseFormat(outputMap);
    }

    std::ofstream outputStream(outputMap, std::ios::binary | std::ios::trunc);
    if (!outputStream.good()) {
        std::cerr << "The output file cannot be opened: " << outputMap << std::endl;
        exit(1);
    }

    ImageEncoder::encode(outputStream, g_format, image, g_compression, outputMap.getPath());

    outputStream.close();
    if (!outputStream.good()) {
        std::cerr << "An error occurred while writing the output file: " << outputMap << std::endl;
        exit(1);
    }
}

void blend(const LinearImage& normal, const LinearImage& detail, LinearImage output) {
    const size_t width = output.getWidth();
    const size_t height = output.getHeight();

    for (size_t y = 0; y < height; y++) {
        auto normalRow = normal.get<float3>(0, y);
        auto detailRow = detail.get<float3>(0, y);
        auto outputRow = output.get<float3>(0, y);

        for (size_t x = 0; x < width; x++, normalRow++, detailRow++, outputRow++) {
            // Reoriented Normal Mapping
            float3 t = *normalRow * float3( 2,  2, 2) + float3(-1, -1,  0);
            float3 u = *detailRow * float3(-2, -2, 2) + float3( 1,  1, -1);
            float3 r = normalize(t * dot(t, u) - u * t.z);

            *outputRow = r * 0.5 + 0.5;
        }
    }
}
