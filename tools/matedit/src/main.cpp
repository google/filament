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

#include <getopt/getopt.h>

#include "ExternalCompile.h"

#include <utils/Path.h>

#include <iostream>
#include <string>
#include <vector>

struct Config {
    utils::Path inputFile;
    utils::Path outputFile;
    std::vector<std::string> commandArgs;
};

static void printUsage(const char* name) {
    std::string execName(utils::Path(name).getName());
    std::string usage(
        "MATEDIT allows editing material files compiled with matc\n"
        "\n"
        "Caution! MATEDIT was designed to operate on trusted inputs. To minimize the risk of triggering\n"
        "memory corruption vulnerabilities, please make sure that the files passed to MATEDIT come from a\n"
        "trusted source, or run MATEDIT in a sandboxed environment.\n"
        "\n"
        "Usage:\n"
        "    MATEDIT [options] -o <output file> -i <input file> external-compile -- <script> [<script args>...]\n"
        "\n"
        "Options:\n"
        "   --help, -h\n"
        "       Print this message\n"
        "\n"
        "   --input=[input file], -i\n"
        "       Specify path to input compiled material file\n"
        "\n"
        "   --output=[output file], -o\n"
        "       Specify path to output compiled material file file\n"
        "\n"
        "   --type=[shader type], -t\n"
        "       Specify the shader type, currently only metal is supported\n"
        "\n"
        "Commands:\n"
        "   external-compile\n"
        "       Transforms all the text-based shaders of the specified type in the input file into binaries\n"
        "       by using the provided script. For each text shader, script is invoked as follows:\n"
        "\n"
        "           script <input shader> <output binary> <shader stage> <shader platform> [<script args>...]\n"
        "\n"
        "       The arguments to the script are as follows:\n"
        "           ${1}   :  a temporary input file containing the shader text contents\n"
        "           ${2}   :  a temporary output file the script should write the compiled shader to\n"
        "           ${3}   :  the shader stage for this shader, either 'vertex', 'fragment', or 'compute'\n"
        "           ${4}   :  the platform for this shader, either 'desktop' or 'mobile'\n"
        "           ${@:5} :  user-provided arguments <script args> passed to MATEDIT\n"
        "\n"
        "       script should compile the shader located at <input shader> into binary, writing its results\n"
        "       to the <output binary> file. The specifics of the binary format depend on the shader type:\n"
        "\n"
        "           metal: precompiled Metal library (.metallib)\n"
        "               <input shader> is guaranteed to have a .metal extension.\n"
        "               <output binary> is guaranteed to have a .metallib extension.\n"
        "\n"
        "       This command will remove the text-based shaders when writing the output material file.\n"
        "\n"
        "       If script exits with a non-zero exit code, MATEDIT will terminate with error. Multiple\n"
        "       invocations of script may be launched in parallel.\n"
        "\n"
        "Example:\n"
        "   MATEDIT -o out.cmat -i in.cmat --type metal external-compile -- ./my_compile-script.sh --sdk iphones\n"
    );

    const std::string from("MATEDIT");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), execName);
    }
    printf("%s", usage.c_str());
}

static int handleArguments(int argc, char* argv[], Config* config) {
    static constexpr const char* OPTSTR = "hi:o:t:";
    static const struct option OPTIONS[] = {
            { "help",               no_argument,       nullptr, 'h' },
            { "input",              required_argument, nullptr, 'i' },
            { "output",             required_argument, nullptr, 'o' },
            { "type",               required_argument, nullptr, 't' },
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
            case 'i':
                config->inputFile = arg;
                break;
            case 'o':
                config->outputFile = arg;
                break;
            case 't':
                if (arg == "metal") {
                    // no-op for now
                } else {
                    std::cerr << "Unrecognized shader type flag: '" << arg << "'. Must be 'metal'"
                              << std::endl;
                    exit(1);
                }
        }
    }

    return optind;
}

int main(int argc, char* argv[]) {
    Config config;
    int optionIndex = handleArguments(argc, argv, &config);

    if (config.inputFile.isEmpty() || config.outputFile.isEmpty()) {
        printUsage(argv[0]);
        return 1;
    }

    if (!config.inputFile.exists()) {
        std::cerr << "The source material " << config.inputFile << " does not exist." << std::endl;
        return 1;
    }

    int numArgs = argc - optionIndex;
    if (numArgs < 1) {
        printUsage(argv[0]);
        return 1;
    }

    const std::string command = argv[optionIndex++];
    if (command != "external-compile") {
        std::cerr << "Unrecognized command: '" << command << "'. Must be 'external-compile'"
                  << std::endl;
        return 1;
    }

    // Ignore any "--" arguments between "external-compile" and the name of the script.
    int i = optionIndex;
    while (i < argc && strcmp(argv[i], "--") == 0) {
        i++;
    }

    for (; i < argc; ++i) {
        config.commandArgs.emplace_back(argv[i]);
    }

    if (config.commandArgs.empty()) {
        std::cerr << "external-compile requires the path to a script to execute" << std::endl
                  << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    return matedit::externalCompile(config.inputFile, config.outputFile, config.commandArgs);
}
