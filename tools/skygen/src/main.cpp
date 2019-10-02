/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <cstdlib>
#include <fstream>
#include <iostream>

#include <math/mat3.h>
#include <math/scalar.h>
#include <math/vec3.h>

#include <image/ColorTransform.h>
#include <image/LinearImage.h>
#include <imageio/ImageEncoder.h>

#include <utils/JobSystem.h>
#include <utils/Path.h>

#include <getopt/getopt.h>

extern "C" {
#include <ArHosekSkyModel.h>
}

using namespace filament::math;
using namespace image;
using namespace utils;

static const uint32_t DEFAULT_ENV_MAP_WIDTH = 4096;

static ImageEncoder::Format g_format = ImageEncoder::Format::PNG_LINEAR;
static bool g_formatSpecified = false;
static std::string g_compression;

static uint32_t g_outputWidth = DEFAULT_ENV_MAP_WIDTH;

static float  g_turbidity    = 4.0f;          // between 1 and 32
static float  g_elevation    = 0.785398f;     // sun elevation in radians (45 degrees)
static float  g_azimuth      = 0.0f;          // sun azimuth in radians (0.0 degrees)
static float3 g_groundAlbedo = float3{0.25f}; // ground albedo
static bool   g_normalize    = false;
static bool   g_tonemap      = false;
static bool   g_gammaCorrect = false;

float luminance(const float3 linear) {
    // Luminance coefficients from Rec. 709
    return dot(linear, float3(0.2126, 0.7152, 0.0722));
}

static float3 tonemapACES(const float3& x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

static inline float3 XYZ_to_sRGB(float3 const& v) {
    // XYZ to linear sRGB
    const mat3f XYZ_sRGB{
            3.2404542f, -0.9692660f,  0.0556434f,
            -1.5371385f,  1.8760108f, -0.2040259f,
            -0.4985314f,  0.0415560f,  1.0572252f
    };
    return XYZ_sRGB * v;
}

static float angleBetween(float thetav, float phiv, float theta, float phi) {
    float cosGamma = sinf(thetav) * sinf(theta) * cosf(phi - phiv) + cosf(thetav) * cosf(theta);
    return acosf(cosGamma);
}

static void generateSky(LinearImage image) {
    printf("Sky parameters\n");
    printf("    Elevation: %.2f°\n", g_elevation * 180.0 * F_1_PI);
    printf("    Azimuth:   %.2f°\n", g_azimuth * 180.0 * F_1_PI);
    printf("    Turbidity: %.2f\n", g_turbidity);
    printf("\n");

    struct {
        std::vector<float> maximas;
        std::mutex maximasMutex;

        float solarElevation = clamp(g_elevation, 0.0f, float(F_PI_2));
        float sunTheta = float(F_PI_2 - solarElevation);
        float sunPhi = 0.0f;

        float3 integral = 0.0f;

        ArHosekSkyModelState* skyState[9] = {
                arhosek_xyz_skymodelstate_alloc_init(g_turbidity, g_groundAlbedo.r, solarElevation),
                arhosek_xyz_skymodelstate_alloc_init(g_turbidity, g_groundAlbedo.g, solarElevation),
                arhosek_xyz_skymodelstate_alloc_init(g_turbidity, g_groundAlbedo.b, solarElevation)
        };
    } jobData{};

    jobData.maximas.reserve(256);

    // init the job system for parallel_for
    static JobSystem js;
    js.adopt();

    // generate the sky
    auto job = jobs::parallel_for<char>(js, nullptr, nullptr, uint32_t(image.getHeight()),
        [&image, &jobData](const char* d, size_t c) {
            const size_t w = image.getWidth();
            const size_t h = image.getHeight();

            float maxSample = 0.00001f;
            float3 integral{0.0f};

            size_t y0 = size_t(d);
            for (size_t y = y0; y < y0 + c; y++) {
                float3* UTILS_RESTRICT data = image.get<float3>(0, (uint32_t) y);

                float v = (y + 0.5f) / h;
                float theta = float(F_PI * v);
                if (theta > F_PI_2) return;

                float integralDelta = sin(theta) / (w * h / 2.0f);

                for (size_t x = 0; x < w; x++, data++) {
                    float u = (x + 0.5f) / w;
                    float phi = float(-2.0 * F_PI * u + F_PI + g_azimuth);

                    float gamma = angleBetween(theta, phi, jobData.sunTheta, jobData.sunPhi);

                    float3 sample{
                        arhosek_tristim_skymodel_radiance(jobData.skyState[0], theta, gamma, 0),
                        arhosek_tristim_skymodel_radiance(jobData.skyState[1], theta, gamma, 1),
                        arhosek_tristim_skymodel_radiance(jobData.skyState[2], theta, gamma, 2)
                    };

                    if (g_normalize) {
                        sample *= float(4.0 * F_PI / 683.0);
                    }

                    maxSample = std::max(maxSample, sample.y);
                    *data = XYZ_to_sRGB(sample);

                    integral += *data * integralDelta;
                }
            }

            std::lock_guard<std::mutex> guard(jobData.maximasMutex);
            jobData.maximas.push_back(maxSample);
            jobData.integral += integral;
        },
        jobs::CountSplitter<1, 8>()
    );

    js.runAndWait(job);

    // cleanup sky data
    arhosekskymodelstate_free(jobData.skyState[0]);
    arhosekskymodelstate_free(jobData.skyState[1]);
    arhosekskymodelstate_free(jobData.skyState[2]);

    float maxValue = *std::max_element(jobData.maximas.cbegin(), jobData.maximas.cend());
    // remap to the range 0..16 to fit in our RGBM format
    float hdrScale = 1.0f / (g_normalize ? maxValue : maxValue / 16.0f);
    switch (g_format) {
        case ImageEncoder::Format::PNG: break;
        case ImageEncoder::Format::PNG_LINEAR: break;
        case ImageEncoder::Format::HDR: break;
        case ImageEncoder::Format::RGBM: break;
        case ImageEncoder::Format::RGB_10_11_11_REV: break;
        case ImageEncoder::Format::PSD:
            if (g_compression != "32") {
                hdrScale = 1.0f / maxValue;
            } else {
                hdrScale = g_normalize ? 1.0f / maxValue : 1.0f;
            }
            break;
        case ImageEncoder::Format::EXR:
            hdrScale = g_normalize ? 1.0f / maxValue : 1.0f;
            break;
        case ImageEncoder::Format::DDS:
        case ImageEncoder::Format::DDS_LINEAR:
            if (g_compression != "8") {
                hdrScale = g_normalize ? 1.0f / maxValue : 1.0f;
            }
            break;
    }
    if (g_normalize) maxValue /= float(4.0 * F_PI / 683.0);

    const size_t w = image.getWidth();
    const size_t h = image.getHeight();

    for (size_t y = 0; y < h; y++) {
        float3* UTILS_RESTRICT data = image.get<float3>(0, (uint32_t) y);
        for (size_t x = 0; x < w; x++, data++) {
            if (y >= h / 2) {
                *data = g_groundAlbedo * jobData.integral;
            }

            *data *= hdrScale;

            if (g_tonemap) {
                *data = tonemapACES(*data);
            }

            if (g_gammaCorrect) {
                *data = image::sRGBToLinear(*data);
            }
        }
    }

    printf("Information\n");
    printf("    Max radiance:    %.2f W/(m^2.sr.nm)\n", maxValue);
    printf("    Max luminance:   %.2f nt\n", maxValue * 683.0f);
    printf("    Max illuminance: %.2f lx\n", maxValue * 683.0f * 2.0 * F_PI);
    printf("\n");

    printf("Rendering parameters\n");
    printf("    Luminance multiplier: %.2f\n", (1.0f / hdrScale) * 683.0f);
    printf("\n");
}

static void printUsage(const char* name) {
    std::string execName(Path(name).getName());
    std::string usage(
            "SKYGEN generates an environment map of a simulated sky\n"
            "Usage:\n"
            "    SKYGEN [options] <output>\n"
            "\n"
            "Options:\n"
            "   --help, -h\n"
            "       print this message\n\n"
            "   --license\n"
            "       Print copyright and license information\n\n"
            "   --format=[exr|hdr|rgbm|rgb32f|psd|png|dds], -f [exr|hdr|rgbm|rgb32f|psd|png|dds]\n"
            "       specify output file format, inferred from file name if omitted\n\n"
            "   --compression=COMPRESSION, -c COMPRESSION\n"
            "       format specific compression:\n"
            "           PNG: Ignored\n"
            "           Radiance: Ignored\n"
            "           Photoshop: 16 (default), 32\n"
            "           OpenEXR: RAW, RLE, ZIPS, ZIP, PIZ (default)\n"
            "           DDS: 8, 16 (default), 32\n\n"
            "   --width=WIDTH, -w WIDTH\n"
            "       specify the width in pixels of the generated environment map\n"
            "       the default width is 4096px\n\n"
            "   --turbidity=[1.0..11.0], -t [1.0..11.0]\n"
            "       specify the atmospheric turbidity, default is 4.0\n\n"
            "   --elevation=ELEVATION, -e ELEVATION\n"
            "       specify the sun elevation in degrees, default is 45.0\n\n"
            "   --azimuth=AZIMUTH, -a AZIMUTH\n"
            "       specify the sun azimuth in degrees, default is 0.0 (-Z)\n\n"
            "   --ground=ALBEDO, -g ALBEDO\n"
            "       specify the ground albedo, between 0.0 and 1.0, default is 0.25\n\n"
            "   --normalize, -n\n"
            "       normalizes output values between 0.0 and 1.0 (implied with PNG)\n\n"
            "   --tonemap, -m\n"
            "       tone-mapped output (implied with PNG)\n\n"
            "   --srgb, -s\n"
            "       applies sRGB gamma correction (implies --normalize, implied with PNG)\n\n"
    );

    const std::string from("SKYGEN");
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
    static constexpr const char* OPTSTR = "hf:c:w:t:e:a:sg:mns";
    static const struct option OPTIONS[] = {
            { "help",                 no_argument, nullptr, 'h' },
            { "license",              no_argument, nullptr, 'l' },
            { "format",         required_argument, nullptr, 'f' },
            { "compression",    required_argument, nullptr, 'c' },
            { "width",          required_argument, nullptr, 'w' },
            { "turbidity",      required_argument, nullptr, 't' },
            { "elevation",      required_argument, nullptr, 'e' },
            { "azimuth",        required_argument, nullptr, 'a' },
            { "ground",         required_argument, nullptr, 'g' },
            { "tonemap",              no_argument, nullptr, 'm' },
            { "normalize",            no_argument, nullptr, 'n' },
            { "srgb",                 no_argument, nullptr, 's' },
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
                if (arg == "rgbm") {
                    g_format = ImageEncoder::Format::RGBM;
                    g_formatSpecified = true;
                }
                if (arg == "rgb32f") {
                    g_format = ImageEncoder::Format::RGB_10_11_11_REV;
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
            case 'w': {
                long width = strtol(arg.c_str(), nullptr, 10);
                if (width > 2) {
                    g_outputWidth = (uint32_t) width;
                }
                break;
            }
            case 't':
                g_turbidity = clamp(strtof(arg.c_str(), nullptr), 1.0f, 11.0f);
                break;
            case 'e':
                g_elevation = (float) (strtof(arg.c_str(), nullptr) * F_PI / 180.0);
                break;
            case 'a':
                g_azimuth = (float) (strtof(arg.c_str(), nullptr) * F_PI / 180.0);
                break;
            case 'g':
                g_groundAlbedo = clamp(strtof(arg.c_str(), nullptr), 0.0f, 1.0f);
                break;
            case 'm':
                g_tonemap = true;
                break;
            case 'n':
                g_normalize = true;
                break;
            case 's':
                g_gammaCorrect = true;
                break;
        }
    }

    return optind;
}

int main(int argc, char* argv[]) {
    int optionIndex = handleArguments(argc, argv);

    int numArgs = argc - optionIndex;
    if (numArgs < 1) {
        printUsage(argv[0]);
        return 1;
    }

    Path outputMap(argv[optionIndex]);

    // guess the format from the file name
    if (!g_formatSpecified) {
        g_format = ImageEncoder::chooseFormat(outputMap, false);
    }

    // normalize output for formats that can only encode data between 0 and 1
    if (g_format == ImageEncoder::Format::PNG
            || (g_format == ImageEncoder::Format::DDS && g_compression == "8")) {
        g_normalize = true;
    }

    // tonemap 8 bit formats
    if (g_format == ImageEncoder::Format::PNG) {
        g_tonemap = true;
    }

    if (g_gammaCorrect) {
        g_normalize = true;
    }

    // the aspect ratio is always 2:1 for equirectangular environment maps
    const uint32_t width = g_outputWidth;
    const uint32_t height = std::max(1u, g_outputWidth >> 1);

    // allocate map
    LinearImage image(width, height, 3);

    // render the sky and sun disk
    generateSky(image);

    // write the environment map to disk
    std::ofstream outputStream(outputMap, std::ios::binary | std::ios::trunc);
    if (!outputStream.good()) {
        std::cerr << "The output file cannot be opened: " << outputMap << std::endl;
    } else {
        ImageEncoder::encode(outputStream, g_format, image, g_compression, outputMap.getPath());
        outputStream.close();
        if (!outputStream.good()) {
            std::cerr << "An error occurred while writing the output file: " << outputMap <<
                    std::endl;
        }
    }
}
