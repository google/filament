/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <math/scalar.h>
#include <math/vec3.h>

#include <image/LinearImage.h>

#include <imageio/ImageEncoder.h>

#include <utils/compiler.h>
#include <utils/Path.h>

#include <getopt/getopt.h>

using namespace filament::math;
using namespace image;
using namespace utils;

static ImageEncoder::Format g_format = ImageEncoder::Format::PNG_LINEAR;
static bool g_formatSpecified = false;
static bool g_groundTruth = false;
static std::string g_compression;

static void printUsage(const char* name) {
    std::string execName(Path(name).getName());
    std::string usage(
            "CSO-LUT generates a lookup table for cone/sphere occlusions.\n"
            "Usage:\n"
            "    CSO-LUT [options] <X Y Z> output_file\n"
            "\n"
            "Where X, Y and Z are the LUT dimensions on the X, Y and Z axis respectively.\n"
            "If omitted, the dimensions will be 32 pixels by default."
            "\n"
            "Options:\n"
            "   --help, -h\n"
            "       Print this message\n\n"
            "   --license\n"
            "       Print copyright and license information\n\n"
            "   --ground-truth, -g\n"
            "       Uses a Monte Carlo simulation to generate the LUT\n\n"
            "   --format=[exr|hdr|psd|png|dds|inc|h|hpp|c|cpp|txt], -f [exr|hdr|psd|png|dds|inc|h|hpp|c|cpp|txt]\n"
            "       Specify output file format, inferred from file name if omitted\n\n"
            "   --compression=COMPRESSION, -c COMPRESSION\n"
            "       Format specific compression:\n"
            "           PNG: Ignored\n"
            "           Radiance: Ignored\n"
            "           Photoshop: 16 (default), 32\n"
            "           OpenEXR: RAW, RLE, ZIPS, ZIP, PIZ (default)\n"
            "           DDS: 8, 16 (default), 32\n\n"
    );

    const std::string from("CSO-LUT");
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
    static constexpr const char* OPTSTR = "hc:f:g";
    static const struct option OPTIONS[] = {
            { "help",                 no_argument, nullptr, 'h' },
            { "license",              no_argument, nullptr, 's' },
            { "ground-truth",         no_argument, nullptr, 'g' },
            { "format",         required_argument, nullptr, 'f' },
            { "compression",    required_argument, nullptr, 'c' },
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
                    g_format = ImageEncoder::Format::PNG_LINEAR;
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
            case 'g':
                g_groundTruth = true;
                break;
        }
    }

    return optind;
}

static bool isIncludeFile(const Path& filename) {
    std::string extension(filename.getExtension());
    return extension == "inc";
}

static bool isTextFile(const utils::Path& filename) {
    std::string extension(filename.getExtension());
    return extension == "h" || extension == "hpp" ||
           extension == "c" || extension == "cpp" ||
           extension == "inc" || extension == "txt";
}

static float sq(float x) { return x * x; }

constexpr float PI_F = 3.141592653589793238f;

UTILS_UNUSED static float sphericalCapsIntersection(float cosCap1, float cosCap2, float cosDistance) {
    // Oat and Sander 2007, "Ambient Aperture Lighting"
    float r1 = std::acos(cosCap1);
    float r2 = std::acos(cosCap2);
    float d  = std::acos(cosDistance);

    if (min(r1, r2) <= max(r1, r2) - d) {
        return 2.0f * PI_F * (1.0f - max(cosCap1, cosCap2));
    } else if (r1 + r2 <= d) {
        return 0.0f;
    }

    float delta = abs(r1 - r2);
    float x = 1.0f - saturate((d - delta) / max(r1 + r2 - delta, 1e-5f));
    // simplified smoothstep()
    float area = sq(x) * (-2.0f * x + 3.0f);
    return area * 2.0f * PI_F * (1.0f - max(cosCap1, cosCap2));
}

static float conesIntersection(float cosTheta1, float cosTheta2, float cosAlpha) {
    // Mazonka 2012, "Solid Angle of Conical Surfaces, Polyhedral Cones, and
    // Intersecting Spherical Caps"
    const float sinAlpha = std::sqrt(1.0f - sq(cosAlpha));

    auto omega = [cosAlpha, sinAlpha](float cosTheta1, float cosTheta2) {
        float sinTheta1 = std::sqrt(1.0f - sq(cosTheta1));

        float ty = cosTheta2 - cosAlpha * cosTheta1;
        float tx = sinAlpha * cosTheta1;

        float cosPhi = clamp((ty * cosTheta1) / (tx * sinTheta1), -1.0f, 1.0f);
        float cosBeta = clamp(ty / (sinTheta1 * std::sqrt(sq(tx) + sq(ty))), -1.0f, 1.0f);

        float phi = std::acos(cosPhi);
        float beta = std::acos(cosBeta);

        return 2.0f * (beta - phi * cosTheta1);
    };

    const float omega1 = omega(cosTheta1, cosTheta2);
    const float omega2 = omega(cosTheta2, cosTheta1);

    if (cosAlpha > 0 && std::abs(1.0 - cosAlpha) < 1e-6f) {
        return min(omega1, omega2);
    }

    if (cosAlpha < 0 && std::abs(1.0 + cosAlpha) < 1e-6f) {
        return max(omega1 + omega2 - 4.0f * PI_F, 0.0f);
    }

    if (std::abs(1.0f - cosTheta1) < 1e-6f || std::abs(1.0f - cosTheta2) < 1e-6f) {
        auto corrected = [cosAlpha](float omega1, float cosTheta1, float cosTheta2) {
            float gamma = std::acos(cosAlpha) - std::acos(cosTheta2);
            float theta1 = std::acos(cosTheta1);
            if (gamma > theta1) {
                return 0.0f;
            } else if (gamma < -theta1) {
                return omega1;
            }
            return omega1 * ((gamma + theta1) / (2.0f * theta1));
        };
        if (std::abs(1.0f - cosTheta1) < 1e-6f) {
            return corrected(omega1, cosTheta1, cosTheta2);
        }
        return corrected(omega2, cosTheta2, cosTheta1);
    }

    if (std::abs(cosTheta1) < 1e-6f && std::abs(cosTheta2) < 1e-6f) {
        return 2.0f * (PI_F - std::acos(cosAlpha));
    }

    if (std::abs(cosTheta1) < 1e-6f) {
        return omega2;
    }

    if (std::abs(cosTheta2) < 1e-6f) {
        return omega1;
    }

    return omega1 + omega2;
}

static void generateSampleRays(float cosAngle, float3* samples, size_t sampleCount) {
    for (size_t i = 0; i < sampleCount; i++) {
        float r1 = rand() / float(RAND_MAX);
        float r2 = rand() / float(RAND_MAX);

        float cosTheta = r1 * (1.0f - cosAngle) + cosAngle;
        float sinTheta = std::sqrt(1.0f - sq(cosTheta));

        float phi = 2.0f * PI_F * r2 - 1.0f;
        float cosPhi = std::cos(phi);
        float sinPhi = std::sin(phi);

        samples[i] = float3{ sinTheta * cosPhi, sinTheta * sinPhi, cosTheta };
    }
}

static float groundTruthVisibility(float cosTheta, float cosAlpha,
        float3* samples, size_t sampleCount) {

    float sinAlpha = std::sqrt(1.0f - sq(cosAlpha));
    float3 occluderDirection{ sinAlpha, 0.0f, cosAlpha };

    size_t hits = 0;
    for (size_t i = 0; i < sampleCount; i++) {
        if (dot(occluderDirection, samples[i]) > cosTheta) {
            hits++;
        }
    }

    return 1.0f - hits / float(sampleCount);
}

int main(int argc, char* argv[]) {
    int optionIndex = handleArguments(argc, argv);

    int numArgs = argc - optionIndex;
    if (numArgs < 1) {
        printUsage(argv[0]);
        return 1;
    }

    size_t w = 32;
    size_t h = 32;
    size_t d = 32;

    Path output;

    if (numArgs >= 4) {
        try {
            w = std::stoi(argv[optionIndex]);
            h = std::stoi(argv[optionIndex + 1]);
            d = std::stoi(argv[optionIndex + 2]);
        } catch (std::invalid_argument &e) {
            // keep default value
        }
        output = argv[optionIndex + 3];
    } else {
        output = argv[optionIndex];
    }

    if (!g_formatSpecified) {
        g_format = ImageEncoder::chooseFormat(output);
    }

    std::cout << "Generating " << w << "x" << h << "x" << d << " cone/sphere occlusion LUT in " <<
            output << "..." << std::endl;

    srand(time(nullptr));

    constexpr size_t sampleCount = 1024;
    float3 samples[sampleCount];

    LinearImage image(w * d, h, 3);

    for (size_t z = 0; z < d; z++) {
        float cosTheta2 = clamp(z / (d - 1.0f), 1e-6f, 1.0f - 1e-6f);

        if (g_groundTruth) {
            generateSampleRays(cosTheta2, samples, sampleCount);
        }

        for (size_t y = 0; y < h; y++) {
            float cosTheta1 = clamp(y / (h - 1.0f), 1e-6f, 1.0f - 1e-6f);
            for (size_t x = 0; x < w; x++) {
                float cosAlpha = clamp(2.0f * (x / (w - 1.0f)) - 1.0f, -1.0f + 1e-6f, 1.0f - 1e-6f);

                float visibility;
                if (g_groundTruth) {
                    visibility = groundTruthVisibility(cosTheta1, cosAlpha, samples, sampleCount);
                } else {
                    float area = conesIntersection(cosTheta1, cosTheta2, cosAlpha);
                    visibility = 1.0f - area / (2.0f * PI_F * max(1.0f - cosTheta2, 1e-6f));
                }

                float3* pixel = image.get<float3>(x + z * w, y);
                *pixel = float3{ saturate(visibility) };
            }
        }
    }

    if (!isTextFile(output)) {
        std::ofstream outputStream(output, std::ios::binary | std::ios::trunc);
        if (!outputStream.good()) {
            std::cerr << "The output file cannot be opened: " << output << std::endl;
            exit(1);
        }

        if (g_format == ImageEncoder::Format::PNG) {
            g_format = ImageEncoder::Format::PNG_LINEAR;
        }
        ImageEncoder::encode(outputStream, g_format, image, g_compression, output.getPath());

        outputStream.close();
        if (!outputStream.good()) {
            std::cerr << "An error occurred while writing the output file: " << output << std::endl;
            exit(1);
        }
    } else {
        const bool isInclude = isIncludeFile(output);
        std::ofstream outputStream(output, std::ios::trunc);

        outputStream << "// Generated with: cso-lut " << w << " " << h << " " << d << " " <<
                output.c_str() << std::endl;
        outputStream << "// LUT stored as a 3D R8 texture, in GL order" << std::endl;
        outputStream << "// theta1 = angle of the first cone" << std::endl;
        outputStream << "// theta2 = angle of the second cone" << std::endl;
        outputStream << "// phi = angle between the two cones" << std::endl;
        outputStream << "// X = cos(phi)    in range -1..1" << std::endl;
        outputStream << "// Y = cos(theta1) in range 0..1" << std::endl;
        outputStream << "// Z = cos(theta2) in range 0..1" << std::endl;

        if (!isInclude) {
            outputStream << "const uint8_t CSO_LUT[] = {";
        }

        for (size_t z = 0; z < d; z++) {
            for (size_t y = 0; y < h; y++) {
                for (size_t x = 0; x < w; x++) {
                    if (x % 4 == 0) outputStream << std::endl << "    ";
                    float3* pixel = image.get<float3>(x + z * w, h - 1 - y);
                    const int o = pixel->r * 255.0f;
                    outputStream << "0x" << std::setfill('0') << std::setw(2) << std::hex << o << ", ";
                }
            }
        }

        if (!isInclude) {
            outputStream << std::endl << "};" << std::endl;
        }

        outputStream << std::endl;
        outputStream.flush();
        outputStream.close();
    }
}
