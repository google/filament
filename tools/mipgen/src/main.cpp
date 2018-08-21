/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <image/ImageSampler.h>
#include <image/LinearImage.h>

#include <imageio/ImageDecoder.h>
#include <imageio/ImageEncoder.h>

#include <utils/Path.h>

#include <getopt/getopt.h>

#include <fstream>
#include <iostream>

using namespace image;
using namespace std;
using namespace utils;

static ImageEncoder::Format g_format = ImageEncoder::Format::PNG_LINEAR;
static bool g_formatSpecified = false;
static string g_compression = "";
static Filter g_filter = Filter::DEFAULT;

static const char* USAGE = R"TXT(
MIPGEN generates mipmaps for an image down to the 1x1 level.
Usage:
    MIPGEN [options] <input_file> <output_pattern>

Options:
   --help, -h
       print this message
   --license
       print copyright and license information
   --format=[exr|hdr|rgbm|psd|png|dds], -f [exr|hdr|rgbm|psd|png|dds]
       specify output file format, inferred from output pattern if omitted
   --kernel=[box|nearest|hermite|gaussian|normals|mitchell|lanczos|min], -k [filter]
       specify filter kernel type (defaults to LANCZOS)
   --compression=COMPRESSION, -c COMPRESSION
       format specific compression:
           PNG: Ignored
           Radiance: Ignored
           Photoshop: 16 (default), 32
           OpenEXR: RAW, RLE, ZIPS, ZIP, PIZ (default)
           DDS: 8, 16 (default), 32

Example:
    MIPGEN --kernel=hermite grassland.png mip_%03d.png
)TXT";

static const char* HTML_PREFIX = R"HTML(<!DOCTYPE html>
<html>
<head>
<style>
img {
    image-rendering: pixelated;
    border: solid 2px;
    padding: 2px;
    display: block;
}
</style>
</head>
<body>
)HTML";

static const char* HTML_SUFFIX = R"HTML(</body>
</html>
)HTML";

static void printUsage(const char* name) {
    string execName(Path(name).getName());
    const string from("MIPGEN");
    string usage(USAGE);
    for (size_t pos = usage.find(from); pos != string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), execName);
    }
    puts(usage.c_str());
}

static void license() {
    cout <<
    #include "licenses/licenses.inc"
    ;
}

static int handleArguments(int argc, char* argv[]) {
    static constexpr const char* OPTSTR = "hlf:c:k:";
    static const struct option OPTIONS[] = {
            { "help",                 no_argument, 0, 'h' },
            { "license",              no_argument, 0, 'l' },
            { "format",         required_argument, 0, 'f' },
            { "compression",    required_argument, 0, 'c' },
            { "kernel",         required_argument, 0, 'k' },
            { 0, 0, 0, 0 }  // termination of the option list
    };

    int opt;
    int optionIndex = 0;

    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'l':
                license();
                exit(0);
            case 'k': {
                bool isvalid;
                g_filter = filterFromString(arg.c_str(), &isvalid);
                if (!isvalid) {
                    cerr << "Warning: unknown filter: " << arg << endl;
                }
            }
            case 'f':
                if (arg == "png") {
                    g_format = ImageEncoder::Format::PNG;
                    g_formatSpecified = true;
                }
                if (arg == "hdr") {
                    g_format = ImageEncoder::Format::HDR;
                    g_formatSpecified = true;
                }
                if (arg == "rgbm") {
                    g_format = ImageEncoder::Format::RGBM;
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
    if (numArgs < 2) {
        printUsage(argv[0]);
        return 1;
    }
    Path inputPath(argv[optionIndex++]);
    string outputPattern(argv[optionIndex]);
    if (!g_formatSpecified) {
        constexpr bool forceLinear = true;
        g_format = ImageEncoder::chooseFormat(outputPattern, forceLinear);
    }

    puts("Reading image...");
    ifstream inputStream(inputPath.getPath(), ios::binary);
    LinearImage sourceImage = ImageDecoder::decode(inputStream, inputPath.getPath());
    if (!sourceImage.isValid()) {
        cerr << "Unable to open image: " << inputPath.getPath() << endl;
        exit(1);
    }

    puts("Generating miplevels...");
    uint32_t count = getMipmapCount(sourceImage);
    vector<LinearImage> miplevels(count);
    generateMipmaps(sourceImage, g_filter, miplevels.data(), count);

    puts("Writing image files to disk...");
    char path[256];
    uint32_t mip = 1; // start at 1 because 0 is the original image
    for (auto image: miplevels) {
        int result = snprintf(path, sizeof(path), outputPattern.c_str(), mip++);
        if (result < 0 || result >= sizeof(path)) {
            puts("Output pattern is too long.");
            exit(1);
        }
        ofstream outputStream(path, ios::binary | ios::trunc);
        if (!outputStream) {
            cerr << "The output file cannot be opened: " << path << endl;
        } else {
            ImageEncoder::encode(outputStream, g_format, image, g_compression, path);
            outputStream.close();
            if (!outputStream) {
                cerr << "An error occurred while writing the output file: " << path << endl;
            }
        }
    }

    puts("Generating mipmaps.html...");
    char tag[256];
    mip = 1;
    const char* pattern = R"(<image src="%s" width="%dpx" height="%dpx">)";
    const uint32_t width = sourceImage.getWidth();
    const uint32_t height = sourceImage.getHeight();
    ofstream html("mipmaps.html", ios::trunc);
    html << HTML_PREFIX;
    int result = snprintf(tag, sizeof(tag), pattern, inputPath.c_str(), width, height);
    if (result < 0 || result >= sizeof(tag)) {
        puts("Output pattern is too long.");
        exit(1);
    }
    html << tag << std::endl;
    for (auto image: miplevels) {
        snprintf(path, sizeof(path), outputPattern.c_str(), mip++);
        result = snprintf(tag, sizeof(tag), pattern, path, width, height);
        if (result < 0 || result >= sizeof(tag)) {
            puts("Output pattern is too long.");
            exit(1);
        }
        html << tag << std::endl;
    }
    html << HTML_SUFFIX;
    html.close();
    puts("Done.");
}
