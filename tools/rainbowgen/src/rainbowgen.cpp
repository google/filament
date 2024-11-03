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

#include "targa.h"
#include "srgb.h"

#include <utils/Path.h>
#include <utils/JobSystem.h>

#include <math/vec3.h>

#include <getopt/getopt.h>

#include <iostream>
#include <string>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static const char* USAGE = R"TXT(
RAINBOWGEN generates a rainbow:

Usage:
    RAINBOWGEN [options] output

Options:
   --help, -h
       Print this message
   --license, -L
       Print copyright and license information

Examples:
    RAINBOWGEN -h
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

static int handleArguments(int argc, char* argv[], RainbowGenerator& builder) {
    static constexpr const char* OPTSTR = "hL";
    static const struct option OPTIONS[] = {
            { "help",                 no_argument, 0, 'h' },
            { "license",              no_argument, 0, 'L' },
            { 0, 0, 0, 0 }  // termination of the option list
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
        }
    }

    return optind;
}

int main(int argc, char* argv[]) {
    RainbowGenerator builder;
    const int optionIndex = handleArguments(argc, argv, builder);
    const int numArgs = argc - optionIndex;
    if (numArgs < 1) {
        printUsage(argv[0]);
        return 1;
    }

    utils::JobSystem js;
    js.adopt();

    Rainbow const rainbow = builder.build(js);

    // save to tga...
    using namespace filament::math;
    size_t const angleCount = rainbow.data.size();
    auto* image = tga_new(angleCount, 32);
    for (size_t index = 0; index < angleCount; index++) {
        float3 c = rainbow.data[index];
        c = srgb::XYZ_to_sRGB(c);
        printf("vec3( %g, %g, %g ),\n", c.r, c.g, c.b);
        c = srgb::linear_to_sRGB(c * 1075);
        uint3 const rgb = uint3(saturate(c) * 255);
        for (int y = 0; y < 32; y++) {
            tga_set_pixel(image, index, y, {
                    .b = (uint8_t)rgb.v[2],
                    .g = (uint8_t)rgb.v[1],
                    .r = (uint8_t)rgb.v[0]
            });
        }
    }
    tga_write("toto.tga", image);
    tga_free(image);
}

#define TARGALIB_IMPLEMENTATION
#include "targa.h"
