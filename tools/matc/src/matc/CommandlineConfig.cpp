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

#include "CommandlineConfig.h"

#include <filament-matp/Config.h>

#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

#include <utils/Path.h>

#include <getopt/getopt.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

using namespace utils;
using namespace filament;

namespace matc {

static constexpr const char* OPTSTR = "hLxo:f:dm:a:l:p:D:T:P:OSEr:vV:gtwF1RW:";
static const option OPTIONS[] = {
        { "help",                    no_argument, nullptr, 'h' },
        { "license",                 no_argument, nullptr, 'L' },
        { "output",            required_argument, nullptr, 'o' },
        { "output-format",     required_argument, nullptr, 'f' },
        { "debug",                   no_argument, nullptr, 'd' },
        { "variant-filter",    required_argument, nullptr, 'V' },
        { "platform",          required_argument, nullptr, 'p' },
        { "optimize",                no_argument, nullptr, 'x' }, // for backward compatibility
        { "optimize",                no_argument, nullptr, 'O' }, // for backward compatibility
        { "optimize-size",           no_argument, nullptr, 'S' },
        { "optimize-none",           no_argument, nullptr, 'g' },
        { "preprocessor-only",       no_argument, nullptr, 'E' },
        { "api",               required_argument, nullptr, 'a' },
        { "feature-level",     required_argument, nullptr, 'l' },
        { "no-essl1",                no_argument, nullptr, '1' },
        { "define",            required_argument, nullptr, 'D' },
        { "template",          required_argument, nullptr, 'T' },
        { "material-parameter",required_argument, nullptr, 'P' },
        { "reflect",           required_argument, nullptr, 'r' },
        { "print",                   no_argument, nullptr, 't' },
        { "version",                 no_argument, nullptr, 'v' },
        { "raw",                     no_argument, nullptr, 'w' },
        { "no-sampler-validation",   no_argument, nullptr, 'F' },
        { "save-raw-variants",       no_argument, nullptr, 'R' },
        { "workarounds",       required_argument, nullptr, 'W' },
        { nullptr, 0, nullptr, 0 }  // termination of the option list
};

// A list of options that may contain PII(Personally Identifiable Information) data.
// We ignore these options when we call the `toPIISafeString` method.
static const std::string_view PII_OPTIONS[] = {
    "output",
};

static bool isPIIOption(const char* longOptionName) {
    if (!longOptionName) {
        return false;
    }
    for (auto const option : PII_OPTIONS) {
        if (option == longOptionName) {
            return true;
        }
    }
    return false;
}

static void usage(char* name) {
    std::string const exec_name(Path(name).getName());
    std::string usage(
            "MATC is a command-line tool to compile material definition.\n"
            "\n"
            "Caution! MATC was designed to operate on trusted inputs. To minimize the risk of\n"
            "triggering memory corruption vulnerabilities, please make sure that the files passed\n"
            "to MATC come from a trusted source, or run MATC in a sandboxed environment.\n"
            "\n"
            "Usages:\n"
            "    MATC [options] <input-file>\n"
            "\n"
            "Supported input formats:\n"
            "    Filament material definition (.mat)\n"
            "\n"
            "Options:\n"
            "   --help, -h\n"
            "       Print this message\n\n"
            "   --license\n"
            "       Print copyright and license information\n\n"
            "   --output, -o\n"
            "       Specify path to output file\n\n"
            "   --platform, -p\n"
            "       Shader family to generate: desktop, mobile or all (default)\n\n"
            "   --optimize-size, -S, -Os\n"
            "       Optimize generated shader code for size instead of just performance\n\n"
            "   --api, -a\n"
            "       Specify the target API: opengl (default), vulkan, metal, or all\n"
            "       This flag can be repeated to individually select APIs for inclusion:\n"
            "           MATC --api opengl --api metal ...\n\n"
            "   --feature-level, -l\n"
            "       Specify the maximum feature level allowed (default is 3).\n\n"
            "   --no-essl1, -1\n"
            "       Don't generate ESSL 1.0 code even for Feature Level 0 mobile shaders.\n"
            "       Shaders are still validated against ESSL 1.0.\n\n"
            "   --define, -D\n"
            "       Add a preprocessor define macro via <macro>=<value>. <value> defaults to 1 if omitted.\n"
            "       Can be repeated to specify multiple definitions:\n"
            "           MATC -Dfoo=1 -Dbar -Dbuzz=100 ...\n\n"
            "   --template <macro>=<string>, -T<macro>=<string>\n"
            "       Replaces ${MACRO} with specified string before parsing\n"
            "       Unlike --define, this applies to the material specification, not GLSL.\n"
            "       Can be repeated to specify multiple macros:\n"
            "           MATC -TBLENDING=fade -TDOUBLESIDED=false ...\n\n"
            "   --material-parameter <key>=<value>, -P<key>=<value>\n"
            "       Set the material property pointed to by <key> to <value>\n"
            "       This overwrites the value configured in the material file.\n"
            "       Material property of array type is not supported.\n"
            "           MATC -PflipUV=false -PshadingModel=lit -Pname=myMat ...\n\n"
            "   --reflect, -r\n"
            "       Reflect the specified metadata as JSON: parameters\n\n"
            "   --variant-filter=<filter>, -V <filter>\n"
            "       Filter out specified comma-separated variants:\n"
            "           directionalLighting, dynamicLighting, shadowReceiver, skinning, vsm, fog,"
            "           ssr (screen-space reflections), stereo\n"
            "       This variant filter is merged with the filter from the material, if any\n\n"
            "   --workarounds, -W\n"
            "       Workarounds to apply: all or none. (default is all).\n\n"
            "   --version, -v\n"
            "       Print the material version number\n\n"
            "Internal use and debugging only:\n"
            "   --optimize-none, -g, -O0\n"
            "       Disable all shader optimizations, for debugging\n\n"
            "   --preprocessor-only, -E\n"
            "       Optimize shaders by running only the preprocessor\n\n"
            "   --raw, -w\n"
            "       Compile a raw GLSL shader into a SPIRV binary chunk\n\n"
            "   --output-format, -f\n"
            "       Specify output format: blob (default) or header\n\n"
            "   --debug, -d\n"
            "       Generate extra data for debugging\n\n"
            "   --no-sampler-validation, -F\n"
            "       Skip validation of number of sampler used\n\n"
            "   --print, -t\n"
            "       Print generated shaders for debugging\n\n"
            "   --save-raw-variants, -R\n"
            "       Write the raw generated GLSL for each variant to a text file in the current directory.\n\n"
    );
    const std::string from("MATC");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    printf("%s", usage.c_str());
}

static void license() {
    static char const * const license[] = {
        #include "licenses/licenses.inc"
        nullptr
    };

    const char * const *p = &license[0];
    while (*p)
        std::cout << *p++ << std::endl;
}

static UserVariantFilterMask parseVariantFilter(const std::string& arg) {
    std::stringstream ss(arg);
    std::string item;
    UserVariantFilterMask variantFilter = 0;
    while (std::getline(ss, item, ',')) {
        if (item == "directionalLighting") {
            variantFilter |= uint32_t(UserVariantFilterBit::DIRECTIONAL_LIGHTING);
        } else if (item == "dynamicLighting") {
            variantFilter |= uint32_t(UserVariantFilterBit::DYNAMIC_LIGHTING);
        } else if (item == "shadowReceiver") {
            variantFilter |= uint32_t(UserVariantFilterBit::SHADOW_RECEIVER);
        } else if (item == "skinning") {
            variantFilter |= uint32_t(UserVariantFilterBit::SKINNING);
        } else if (item == "vsm") {
            variantFilter |= uint32_t(UserVariantFilterBit::VSM);
        } else if (item == "fog") {
            variantFilter |= uint32_t(UserVariantFilterBit::FOG);
        } else if (item == "ssr") {
            variantFilter |= uint32_t(UserVariantFilterBit::SSR);
        } else if (item == "stereo") {
            variantFilter |= uint32_t(UserVariantFilterBit::STE);
        }
    }
    return variantFilter;
}

CommandlineConfig::CommandlineConfig(int const argc, char** argv)
        : Config(), mArgc(argc), mArgv(argv) {
    // Add aliases for some optimization flags. We do this by pre-processing the arguments,
    // since getopt has trouble with short options that are longer than one character (e.g. -Os).
    for (int i = 1; i < mArgc; i++) {
        if (mArgv[i]) {
            if (strcmp(mArgv[i], "-Os") == 0) {
                mArgv[i] = (char *)"-S";
            } else if (strcmp(mArgv[i], "-O0") == 0) {
                mArgv[i] = (char *)"-g";
            }
        }
    }
    mIsValid = parse();
}

static void parseDefine(std::string const& defineString, matp::Config::StringReplacementMap& defines) {
    const char* const defineArg = defineString.c_str();
    const size_t length = defineString.length();

    const char* p = defineArg;
    const char* end = p + length;

    while (p < end && *p != '=') {
        p++;
    }

    if (*p == '=') {
        if (p == defineArg || p + 1 >= end) {
            // Edge-cases, missing define name or value.
            return;
        }
        std::string const def(defineArg, p - defineArg);
        defines.emplace(def, p + 1);
        return;
    }

    // No explicit assignment, use a default value of 1.
    std::string const def(defineArg, p - defineArg);
    defines.emplace(def, "1");
}

bool CommandlineConfig::parse() {
    int opt;
    int option_index = 0;

    while ((opt = getopt_long(mArgc, mArgv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        std::string const arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                usage(mArgv[0]);
                exit(0);
            case 'L':
                license();
                exit(0);
            case 'o':
                mOutput = new FilesystemOutput(arg.c_str());
                break;
            case 'f':
                if (arg == "blob") {
                    mOutputFormat = OutputFormat::BLOB;
                } else if (arg == "header") {
                    mOutputFormat = OutputFormat::C_HEADER;
                } else {
                    std::cerr << "Unrecognized output format flag. Must be 'blob'|'header'."
                            << std::endl;
                   return false;
                }
                break;
            case 'd':
                mDebug = true;
                break;
            case 'p':
                if (arg == "desktop") {
                   mPlatform = Platform::DESKTOP;
                } else if (arg == "mobile") {
                    mPlatform = Platform::MOBILE;
                } else if (arg == "all") {
                    mPlatform = Platform::ALL;
                } else {
                    std::cerr << "Unrecognized platform. Must be 'desktop'|'mobile'|'all'."
                            << std::endl;
                    return false;
                }
                break;
            case 'a':
                if (arg == "opengl") {
                    mTargetApi |= TargetApi::OPENGL;
                } else if (arg == "vulkan") {
                    mTargetApi |= TargetApi::VULKAN;
                } else if (arg == "metal") {
                    mTargetApi |= TargetApi::METAL;
                } else if (arg == "webgpu") {
                    mTargetApi |= TargetApi::WEBGPU;
                } else if (arg == "all") {
                    mTargetApi |= TargetApi::ALL;
                } else {
                    std::cerr << "Unrecognized target API. Must be 'opengl'|'vulkan'|'metal'|'all'."
                            << std::endl;
                    return false;
                }
                break;
            case 'l': {
                auto const featureLevel = backend::FeatureLevel(std::atoi(arg.c_str()));
                mFeatureLevel = backend::FeatureLevel::FEATURE_LEVEL_3;
                switch (featureLevel) {
                    case backend::FeatureLevel::FEATURE_LEVEL_0:
                    case backend::FeatureLevel::FEATURE_LEVEL_1:
                    case backend::FeatureLevel::FEATURE_LEVEL_2:
                    case backend::FeatureLevel::FEATURE_LEVEL_3:
                        mFeatureLevel = featureLevel;
                        break;
                }
                break;
            }
            case '1':
                mIncludeEssl1 = false;
                break;
            case 'D':
                parseDefine(arg, mDefines);
                break;
            case 'T':
                parseDefine(arg, mTemplateMap);
                break;
            case 'P':
                parseDefine(arg, mMaterialParameters);
                break;
            case 'v':
                // Similar to --help, the --version command does an early exit in order to avoid
                // subsequent error spew such as "Missing input filename" etc.
                std::cout << MATERIAL_VERSION << std::endl;
                exit(0);
            case 'V':
                mVariantFilter = parseVariantFilter(arg);
                break;
            case 'W':
                if (arg == "none") {
                    mWorkarounds = Workarounds::NONE;
                } else if (arg == "all") {
                    mWorkarounds = Workarounds::ALL;
                } else {
                    std::cerr << "Unrecognized workaround. Must be 'all'|'none'." << std::endl;
                    return false;
                }
                break;
            // These 2 flags are supported for backward compatibility
            case 'O':
            case 'x':
                mOptimizationLevel = Optimization::PERFORMANCE;
                break;
            case 'S':
                mOptimizationLevel = Optimization::SIZE;
                break;
            case 'E':
                mOptimizationLevel = Optimization::PREPROCESSOR;
                break;
            case 'g':
                mOptimizationLevel = Optimization::NONE;
                break;
            case 'r':
                mReflectionTarget = Metadata::PARAMETERS;
                break;
            case 't':
                mPrintShaders = true;
                break;
            case 'w':
                mRawShaderMode = true;
                break;
            case 'F':
                mNoSamplerValidation = true;
            case 'R':
                mSaveRawVariants = true;
                break;
        }
    }

    if (mArgc - optind > 1) {
        std::cerr << "Only one input file should be specified on the command line." << std::endl;
        return false;
    }
    if (mArgc - optind > 0) {
        mInput = new FilesystemInput(mArgv[optind]);
    }

    return true;
}

// This method concatenates all options and arguments, excluding the executable name, input file
// name, and any options found in PII_OPTIONS due to potential PII data.
std::string CommandlineConfig::toPIISafeString() const noexcept {
    std::string result;
    optind = 1; // Reset getopt's internal index before parsing

    while (true) {
        // getopt_long will only set `long_index` if a long option is parsed.
        int long_index = -1;
        int const opt = getopt_long(mArgc, mArgv, OPTSTR, OPTIONS, &long_index);
        if (opt == -1) {
            break; // End of options
        }

        // Find the matched option.
        const struct option* matched_option = nullptr;
        if (long_index != -1) {
            // A long option was parsed (e.g., --help)
             matched_option = &OPTIONS[long_index];
        } else if (opt > 0) {
            // A short option was parsed (e.g., -h)
            for (int i = 0; OPTIONS[i].name != nullptr; ++i) {
                if (OPTIONS[i].val == opt) {
                    matched_option = &OPTIONS[i];
                    break;
                }
            }
        }

        if (!matched_option) {
            std::cerr << "Failed to find the matched option: long_index=" << long_index
                      << ", opt=" << opt << std::endl;
            continue;
        }

        // Skip if it's a PII option.
        if (isPIIOption(matched_option->name)) {
            continue;
        }

        // Reconstruct the option.
        if (long_index != -1) {
            result += "--";
            result += matched_option->name;
            result += " ";
        } else if (opt > 0) {
            result += "-";
            result += (char)matched_option->val;
            result += " ";
        }

        // Add an argument if available.
        if (optarg && matched_option->has_arg != no_argument) {
            result += optarg;
            result += " ";
        }
    }

    // We're ignoring the last (remaining) argument because it's user input with PII.

    // Trim trailing space
    if (!result.empty()) {
        result.pop_back();
    }

    return result;
}

} // namespace matc
