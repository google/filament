/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "RainbowGenerator.h"

#include <image/LinearImage.h>
#include <imageio/ImageEncoder.h>

#include <utils/Path.h>
#include <utils/JobSystem.h>

#include <math/vec3.h>
#include <math/scalar.h>

#include <getopt/getopt.h>

#include <fstream>
#include <iostream>
#include <string>
#include <optional>

#include <stdio.h>
#include <stdlib.h>

using namespace filament::math;
using namespace image;

static const char* USAGE = R"TXT(
RAINBOWGEN generates a rainbow:

Usage:
    RAINBOWGEN [options] output

Options:
    --help, -h
        Print this message

    --license, -L
        Print copyright and license information

    --format=[exr|hdr|psd|rgbm|rgb32f|png|dds], -f [format]
        Specify output file format.

    --cosine, -c
        LUT indexed by 1-cos(deviation)

    --size=integer, -s integer
        Size of the output rainbow texture, 256 by default.

    --min-deviation=float, -m float
        Minimum deviation in degrees encoded in the texture, 30 by default.

    --max-deviation=float, -M float
        Maximum deviation in degrees encoded in the texture, 60 by default.

    --samples=integer, -S integer
        Number of samples for the simulation, 10,000,000 by default.

Private use only:
    --debug, -d
        Increases the height of the output texture.

Examples:
    RAINBOWGEN -h
    RAINBOWGEN rainbow.png
)TXT";

static void printUsage(const char* name) {
    std::string const execName(utils::Path(name).getName());
    const std::string from("RAINBOWGEN");
    std::string usage(USAGE);
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), execName);
    }
    puts(usage.c_str());
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

static std::optional<image::ImageEncoder::Format> g_format;
static bool g_debug = false;

static int handleArguments(int argc, char* argv[], RainbowGenerator& builder) {
    static constexpr const char* OPTSTR = "hLs:m:M:S:f:dc";
    static const struct option OPTIONS[] = {
            { "help",           no_argument, nullptr, 'h' },
            { "license",        no_argument, nullptr, 'L' },
            { "cosine",         no_argument, nullptr, 'c' },
            { "format",         required_argument, nullptr, 'f' },
            { "size",           required_argument, nullptr, 's' },
            { "min-deviation",  required_argument, nullptr, 'm' },
            { "max-deviation",  required_argument, nullptr, 'M' },
            { "samples",        required_argument, nullptr, 'S' },
            { "debug",          no_argument, nullptr, 'd' },
            { nullptr, 0, nullptr, 0 }  // termination of the option list
    };

    int opt;
    int optionIndex = 0;

    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        std::string const arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'L':
                license();
                exit(0);
            case 'f':
                if (arg == "png") {
                    g_format = ImageEncoder::Format::PNG_LINEAR;
                } else if (arg == "hdr") {
                    g_format = ImageEncoder::Format::HDR;
                } else if (arg == "rgbm") {
                    g_format = ImageEncoder::Format::RGBM;
                } else if (arg == "rgb32f") {
                    g_format = ImageEncoder::Format::RGB_10_11_11_REV;
                } else if (arg == "exr") {
                    g_format = ImageEncoder::Format::EXR;
                } else if (arg == "psd") {
                    g_format = ImageEncoder::Format::PSD;
                } else if (arg == "dds") {
                    g_format = ImageEncoder::Format::DDS_LINEAR;
                }
                break;
            case 'c':
                builder.cosine(true);
                break;
            case 's':
                builder.lut(stoul(arg));
                break;
            case 'm':
                builder.minDeviation(stof(arg) * f::DEG_TO_RAD);
                break;
            case 'M':
                builder.maxDeviation(stof(arg) * f::DEG_TO_RAD);
                break;
            case 'S':
                builder.samples(stoul(arg));
                break;
            case 'd':
                g_debug = true;
                break;
        }
    }

    return optind;
}

int main(int argc, char* argv[]) {
    using namespace filament::math;
    using namespace image;

    // process options
    RainbowGenerator builder;
    const int optionIndex = handleArguments(argc, argv, builder);
    const int numArgs = argc - optionIndex;
    if (numArgs < 1) {
        printUsage(argv[0]);
        return 1;
    }

    utils::Path const path{ argv[optionIndex] };

    // actual computation
    utils::JobSystem js;
    js.adopt();
    Rainbow const rainbow = builder.build(js);

    // save result
    size_t const angleCount = rainbow.data.size();
    size_t const height = g_debug ? 32 : 1;

    LinearImage image(angleCount, height, 3);
    float3* const pData = reinterpret_cast<float3*>(image.getPixelRef());
    for (size_t index = 0; index < angleCount; index++) {
        float3 const c = rainbow.data[index];
        //printf("vec3( %g, %g, %g ),\n", c.r, c.g, c.b);
        for (int y = 0; y < height; y++) {
            pData[angleCount * y + index] = c;
        }
    }

    // choose a format, either the one specified or from the name, PNG by default
    ImageEncoder::Format const format =
            g_format.has_value() ? g_format.value() :
            ImageEncoder::chooseFormat(path.getPath(), true);

    // if we don't have an extension, pick one from the format
    std::string filename = path.getPath();
    if (path.getExtension().empty()) {
        std::string const ext = ImageEncoder::chooseExtension(format);
        filename += ext;
    }

    std::ofstream outputStream(filename, std::ios::binary | std::ios::trunc);
    ImageEncoder::encode(outputStream, format, image, "", "");
}
