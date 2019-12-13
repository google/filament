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

#include "GlslMinify.h"

#include <utils/Path.h>

#include <getopt/getopt.h>

#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace utils;
using namespace glslminifier;

static bool g_writeToStdOut = true;
static const char* g_outputFile = "";
static const char* g_inputFile = "";
GlslMinifyOptions g_optimizationLevel = GlslMinifyOptions::ALL;

static const char* USAGE = R"TXT(
GLSLMINIFIER minifies GLSL shader code by removing comments, blank lines and indentation.

Usage:
    GLSLMINIFIER [options] <input file>

Options:
   --help, -h
       Print this message.
   --license, -L
       Print copyright and license information.
   --output, -o
       Specify path to output file. If none provided, writes to stdout.
   --optimization, -O [none]
       Set the level of optimization. "none" performs a simple passthrough.

Example:
    GLSLMINIFIER -o output.fs.min input.fs
    > Output file: output.fs.min
)TXT";

static void printUsage(const char* name) {
    std::string execName(Path(name).getName());
    const std::string from("GLSLMINIFIER");
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
    static constexpr const char* OPTSTR = "hLo:O:";
    static const struct option OPTIONS[] = {
            { "help",                 no_argument, nullptr, 'h' },
            { "license",              no_argument, nullptr, 'L' },
            { "output",         required_argument, nullptr, 'o' },
            { "optimization",   required_argument, nullptr, 'O' },
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
            case 'L':
                license();
                exit(0);
            case 'o':
                g_outputFile = optarg;
                g_writeToStdOut = false;
                break;
            case 'O':
                if (arg == "none") {
                    g_optimizationLevel = GlslMinifyOptions::NONE;
                } else {
                    std::cerr << "Warning: unknown optimization level." << std::endl;
                }
                break;
        }
    }

    return optind;
}

int main(int argc, char* argv[]) {
    const int optionIndex = handleArguments(argc, argv);
    const int numArgs = argc - optionIndex;
    if (numArgs < 1) {
        printUsage(argv[0]);
        return 1;
    }
    if (numArgs > 1) {
        cerr << "Only one input file should be specified on the command line." << endl;
        return 1;
    }
    g_inputFile = argv[optionIndex];

    // Read the contents of the input file.
    ifstream inStream(g_inputFile, ios::binary);
    if (!inStream) {
        cerr << "Unable to read " << g_inputFile << endl;
        exit(1);
    }
    string inputStr((istreambuf_iterator<char>(inStream)), istreambuf_iterator<char>());

    // Minify the GLSL.
    string result = minifyGlsl(inputStr, g_optimizationLevel);

    if (g_writeToStdOut) {
        cout << result;
        return 0;
    }

    // Create the output path if it doesn't exist already.
    const Path outputDir = Path(g_outputFile).getParent();
    if (!outputDir.exists()) {
        outputDir.mkdirRecursive();
    }

    // Open the output file for writing.
    ofstream outStream(g_outputFile, ios::binary);
    if (!outStream) {
        cerr << "Unable to open " << g_outputFile << endl;
        exit(1);
    }
    outStream << result;
    return 0;
}
