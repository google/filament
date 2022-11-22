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

#include <image/ColorTransform.h>
#include <image/ImageOps.h>
#include <image/ImageSampler.h>
#include <image/Ktx1Bundle.h>
#include <image/LinearImage.h>

#include <imageio/BasisEncoder.h>
#include <imageio/ImageDecoder.h>
#include <imageio/ImageEncoder.h>

#include <utils/Path.h>

#include <getopt/getopt.h>

#include <fstream>
#include <iostream>
#include <string>

using namespace image;
using namespace std;
using namespace utils;

enum KtxCompression {
    NONE,
    UASTC,
    ETC1S,
    UASTC_NORMALS,
    ETC1S_NORMALS,
};

static ImageEncoder::Format g_format = ImageEncoder::Format::PNG;
static bool g_formatSpecified = false;
static bool g_createGallery = false;
static KtxCompression g_ktxCompression = NONE;
static std::string g_compressionString;
static Filter g_filter = Filter::DEFAULT;
static bool g_addAlpha = false;
static bool g_stripAlpha = false;
static bool g_grayscale = false;
static bool g_ktx1Container = false;
static bool g_ktx2Container = false;
static bool g_sourceIsLinear = false;
static bool g_quietMode = false;
static uint32_t g_mipLevelCount = 0;

static const char* USAGE = R"TXT(
MIPGEN generates mipmaps for an image down to the 1x1 level.

The <output_pattern> argument is a printf-style pattern.
For example, "mip%2d.png" generates mip01.png, mip02.png, etc.
Miplevel 0 is not generated since it is the original image.

If the output format is a container format like KTX, then
<output_pattern> is simply a filename.

Usage:
    MIPGEN [options] <input_file> <output_pattern>

Options:
   --help, -h
       print this message
   --license, -L
       print copyright and license information
   --linear, -l
       specifies that the source image is linear (converts to floats without transformation)
   --page, -p
       generate HTML page for review purposes (mipmap.html)
   --quiet, -q
       suppress console output from the mipgen tool
   --grayscale, -g
       create a single-channel image
   --format=[exr|hdr|rgbm|psd|png|dds|ktx|ktx2], -f [extension]
       specify output file format, inferred from output pattern if omitted
   --kernel=[box|nearest|hermite|gaussian|normals|mitchell|lanczos|min], -k [filter]
       specify filter kernel type (defaults to lanczos)
   --add-alpha
       if the source image has 3 channels, this adds a fourth channel filled with 1.0
   --strip-alpha
       ignore the alpha component of the input image
   --mip-levels=N, -m N
       specifies the number of mip levels to generate
       if 0 (default), all levels are generated
   --compression=COMPRESSION, -c COMPRESSION
       format specific compression:
           KTX, PNG, Radiance: Ignored
           KTX2: uastc, etc1s, uastc_normals, or etc1s_normals
           Photoshop: 16 (default), 32
           OpenEXR: RAW, RLE, ZIPS, ZIP, PIZ (default)
           DDS: 8, 16 (default), 32

Examples:
    MIPGEN -g --kernel=hermite grassland.png mip_%03d.png
    MIPGEN -f ktx2 --compression=uastc grassland.png mips.ktx
    MIPGEN -f ktx grassland.png mips.ktx
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
    std::string execName(Path(name).getName());
    const std::string from("MIPGEN");
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

static int handleArguments(int argc, char* argv[]) {
    static constexpr const char* OPTSTR = "hLlgpf:c:k:saqm:";
    static const struct option OPTIONS[] = {
            { "help",                 no_argument, 0, 'h' },
            { "license",              no_argument, 0, 'L' },
            { "linear",               no_argument, 0, 'l' },
            { "grayscale",            no_argument, 0, 'g' },
            { "page",                 no_argument, 0, 'p' },
            { "format",         required_argument, 0, 'f' },
            { "compression",    required_argument, 0, 'c' },
            { "kernel",         required_argument, 0, 'k' },
            { "strip-alpha",          no_argument, 0, 's' },
            { "add-alpha",            no_argument, 0, 'a' },
            { "quiet",                no_argument, 0, 'q' },
            { "mip-levels",     required_argument, 0, 'm' },
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
            case 'L':
                license();
                exit(0);
            case 'l':
                g_sourceIsLinear = true;
                break;
            case 'g':
                g_grayscale = true;
                break;
            case 'p':
                g_createGallery = true;
                break;
            case 'k': {
                g_filter = filterFromString(arg.c_str());
                if (g_filter == Filter::DEFAULT) {
                    cerr << "Warning: unrecognized filter, falling back to DEFAULT." << endl;
                }
                break;
            }
            case 's':
                g_stripAlpha = true;
                break;
            case 'a':
                g_addAlpha = true;
                break;
            case 'q':
                g_quietMode = true;
                break;
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
                if (arg == "ktx") {
                    g_ktx1Container = true;
                    g_formatSpecified = true;
                }
                if (arg == "ktx2") {
                    g_ktx2Container = true;
                    g_formatSpecified = true;
                }
                break;
            case 'c':
                if (arg == "uastc") {
                    g_ktxCompression = UASTC;
                } else if (arg == "etc1s") {
                    g_ktxCompression = ETC1S;
                } else if (arg == "uastc_normals") {
                    g_ktxCompression = UASTC_NORMALS;
                } else if (arg == "etc1s_normals") {
                    g_ktxCompression = ETC1S_NORMALS;
                }
                g_compressionString = arg;
                break;
            case 'm':
                try {
                    g_mipLevelCount = std::stoi(arg);
                } catch (std::invalid_argument &e) {
                    // keep default value
                }
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
    std::string outputPattern(argv[optionIndex]);
    if (Path(outputPattern).getExtension() == "ktx") {
        g_ktx1Container = true;
        g_formatSpecified = true;
    } else if (Path(outputPattern).getExtension() == "ktx2") {
        g_ktx2Container = true;
        g_formatSpecified = true;
    } else if (!g_formatSpecified) {
        g_format = ImageEncoder::chooseFormat(outputPattern, g_sourceIsLinear);
    }

    if (!g_quietMode) {
        puts("Reading image...");
    }

    ifstream inputStream(inputPath.getPath(), ios::binary);
    LinearImage sourceImage = ImageDecoder::decode(inputStream, inputPath.getPath(),
            g_sourceIsLinear ? ImageDecoder::ColorSpace::LINEAR : ImageDecoder::ColorSpace::SRGB);
    if (!sourceImage.isValid()) {
        cerr << "Unable to open image: " << inputPath.getPath() << endl;
        return 1;
    }
    if (g_stripAlpha && sourceImage.getChannels() == 4) {
        auto r = extractChannel(sourceImage, 0);
        auto g = extractChannel(sourceImage, 1);
        auto b = extractChannel(sourceImage, 2);
        sourceImage = combineChannels({r, g, b});
    }
    if (g_addAlpha && sourceImage.getChannels() == 3) {
        auto r = extractChannel(sourceImage, 0);
        auto g = extractChannel(sourceImage, 1);
        auto b = extractChannel(sourceImage, 2);
        auto a = LinearImage(sourceImage.getWidth(), sourceImage.getHeight(), 1);
        clearToValue(a, 1.0f);
        sourceImage = combineChannels({r, g, b, a});
    }
    if (g_grayscale) {
        sourceImage = extractChannel(sourceImage, 0);
    }

    if (g_filter == Filter::GAUSSIAN_NORMALS) {
        sourceImage = colorsToVectors(sourceImage);
    }

    if (!g_quietMode) {
        puts("Generating miplevels...");
    }

    uint32_t count = getMipmapCount(sourceImage);
    count = g_mipLevelCount == 0 ? count : min(g_mipLevelCount - 1, count);
    vector<LinearImage> miplevels(count);
    generateMipmaps(sourceImage, g_filter, miplevels.data(), count);

    if (g_ktx1Container) {
        if (!g_quietMode) {
            puts("Writing KTX file to disk...");
        }

        // The libimage API does not include the original image in the mip array,
        // which might make sense when generating individual files, but for a KTX
        // bundle, we want to include level 0, so add 1 to the KTX level count.
        Ktx1Bundle container(1 + miplevels.size(), 1, false);
        auto& info = container.info();
        info = {
            .endianness = Ktx1Bundle::ENDIAN_DEFAULT,
            .glType = Ktx1Bundle::UNSIGNED_BYTE,
            .glTypeSize = 1,
            .pixelWidth = sourceImage.getWidth(),
            .pixelHeight = sourceImage.getHeight(),
            .pixelDepth = 0,
        };
        size_t componentCount = sourceImage.getChannels();

        // Try to choose an internal format that has the same transformation function as the
        // source format. This varible may be adjusted later, after the destination format has
        // been resolved.
        bool destIsLinear = g_sourceIsLinear;

        if (componentCount == 1) {
            info.glFormat = info.glBaseInternalFormat = Ktx1Bundle::RED;
            info.glInternalFormat = Ktx1Bundle::R8;
            destIsLinear = true;
        } else if (componentCount == 3) {
            info.glFormat = info.glBaseInternalFormat = Ktx1Bundle::RGB;
            info.glInternalFormat = destIsLinear ? Ktx1Bundle::RGB8 : Ktx1Bundle::SRGB8;
        } else if (componentCount == 4) {
            info.glFormat = info.glBaseInternalFormat = Ktx1Bundle::RGBA;
            info.glInternalFormat = destIsLinear ? Ktx1Bundle::RGBA8 : Ktx1Bundle::SRGB8_ALPHA8;
        } else {
            cerr << "Bad component count." << endl;
            return 1;
        }
        if (g_ktxCompression != NONE) {
            cerr << "Compression not supported with KTX1." << endl;
            return 1;
        }
        uint32_t mip = 0;
        auto addLevel = [&](LinearImage image) {
            if (g_filter == Filter::GAUSSIAN_NORMALS) {
                image = vectorsToColors(image);
            }
            std::unique_ptr<uint8_t[]> data;
            if (g_grayscale && destIsLinear) {
                data = fromLinearToGrayscale<uint8_t>(image);
            } else if (g_grayscale) {
                data = fromLinearTosRGB<uint8_t, 1>(image);
            } else if (destIsLinear) {
                if (componentCount == 3) {
                    data = fromLinearToRGB<uint8_t, 3>(image);
                } else {
                    data = fromLinearToRGB<uint8_t, 4>(image);
                }
            } else {
                if (componentCount == 3) {
                    data = fromLinearTosRGB<uint8_t, 3>(image);
                } else {
                    data = fromLinearTosRGB<uint8_t, 4>(image);
                }
            }
            container.setBlob({mip++, 0, 0}, data.get(), image.getWidth() * image.getHeight() *
                    container.info().glTypeSize * componentCount);
        };
        addLevel(sourceImage);
        for (auto image : miplevels) {
            addLevel(image);
        }
        vector<uint8_t> fileContents(container.getSerializedLength());
        container.serialize(fileContents.data(), fileContents.size());
        Path(outputPattern).getParent().mkdirRecursive();
        ofstream outputStream(outputPattern, ios::out | ios::binary);
        outputStream.write((const char*) fileContents.data(), fileContents.size());
        outputStream.close();
        if (!g_quietMode) {
            puts("Done.");
        }
        return 0;
    }

    if (g_ktx2Container) {
        if (!g_quietMode) {
            puts("Writing KTX2 file to disk...");
        }

        BasisEncoder::Builder builder(miplevels.size() + 1, 1);
        using IntermediateFormat = BasisEncoder::IntermediateFormat;

        size_t mipIndex = 0;
        builder
            .intermediateFormat((g_ktxCompression == UASTC || g_ktxCompression == UASTC_NORMALS) ?
                    IntermediateFormat::UASTC : IntermediateFormat::ETC1S)
            .grayscale(g_grayscale)
            .linear(g_sourceIsLinear)
            .quiet(g_quietMode)
            .normals(g_ktxCompression == ETC1S_NORMALS || g_ktxCompression == UASTC_NORMALS)
            .miplevel(mipIndex++, 0, sourceImage);

        for (auto image : miplevels) {
            builder.miplevel(mipIndex++, 0, image);
        }

        BasisEncoder* encoder = builder.build();
        if (!encoder) {
            puts("Error while creating BasisU encoder.");
            return 1;
        }

        bool success = encoder->encode();
        if (!success) {
            // Error message has already been printed.
            return 1;
        }

        Path(outputPattern).getParent().mkdirRecursive();
        ofstream outputStream(outputPattern, ios::out | ios::binary);
        outputStream.write((const char*) encoder->getKtx2Data(), encoder->getKtx2ByteCount());
        outputStream.close();
        if (!g_quietMode) {
            printf("Wrote %zu bytes to %s.\n", encoder->getKtx2ByteCount(), outputPattern.c_str());
        }
        delete encoder;
        return 0;
    }

    if (!g_quietMode) {
        puts("Writing image files to disk...");
    }

    char path[256];
    uint32_t mip = 1; // start at 1 because 0 is the original image
    for (auto image : miplevels) {
        int result = snprintf(path, sizeof(path), outputPattern.c_str(), mip++);
        if (result < 0 || result >= sizeof(path)) {
            cerr << "Output pattern is too long." << endl;
            return 1;
        }
        Path(path).getParent().mkdirRecursive();
        ofstream outputStream(path, ios::binary | ios::trunc);
        if (!outputStream) {
            cerr << "The output file cannot be opened: " << path << endl;
        } else {
            if (g_filter == Filter::GAUSSIAN_NORMALS) {
                image = vectorsToColors(image);
            }
            if (!ImageEncoder::encode(outputStream, g_format, image, g_compressionString, path)) {
                cerr << "An error occurred while encoding the image." << endl;
                return 1;
            }
            outputStream.close();
            if (!outputStream) {
                cerr << "An error occurred while writing the output file: " << path << endl;
                return 1;
            }
        }
    }

    if (g_createGallery) {
        if (!g_quietMode) {
            puts("Generating mipmaps.html...");
        }

        char tag[256];
        mip = 1;
        const char* pattern = R"(<image src="%s" width="%dpx" height="%dpx">)";
        const uint32_t width = sourceImage.getWidth();
        const uint32_t height = sourceImage.getHeight();
        ofstream html("mipmaps.html", ios::trunc);
        html << HTML_PREFIX;
        int result = snprintf(tag, sizeof(tag), pattern, inputPath.c_str(), width, height);
        if (result < 0 || result >= sizeof(tag)) {
            cerr << "Output pattern is too long." << endl;
            return 1;
        }
        html << tag << std::endl;
        for (auto image: miplevels) {
            snprintf(path, sizeof(path), outputPattern.c_str(), mip++);
            result = snprintf(tag, sizeof(tag), pattern, path, width, height);
            if (result < 0 || result >= sizeof(tag)) {
                cerr << "Output pattern is too long." << endl;
                return 1;
            }
            html << tag << std::endl;
        }
        html << HTML_SUFFIX;
    }

    if (!g_quietMode) {
        puts("Done.");
    }
}
