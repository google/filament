/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <utils/Path.h>

#include <getopt/getopt.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include <string.h>

using namespace utils;

// Holds all command-line configuration for the resource generator.
struct AppConfig {
    const char* packageName = "resources";
    const char* deployDir = ".";
    bool keepExtension = false;
    bool appendNull = false;
    bool generateC = false;
    bool quietMode = false;
    bool embedJson = false;
    std::vector<Path> inputPaths;
};

// A special marker used to identify where to insert the JSON summary blob.
static const char* g_jsonMagicString = "__RESGEN__";

// Defines the help message text displayed to the user.
static const char* USAGE = R"TXT(
RESGEN aggregates a sequence of binary blobs, each of which becomes a "resource" whose id
is the basename of the input file. It produces the following set of files:

    resources.h ......... declares sizes and offsets for each resource
    resources.S ......... small assembly file with incbin directive and rodata section
    resources.apple.S ... ditto but with different rodata name and underscore prefixes
    resources.bin ....... the aggregated binary blob that the incbin refers to

Usage:
    RESGEN [options] <input_file_0> <input_file_1> ...

Options:
   --help, -h
       Print this message
   --license, -L
       Print copyright and license information
   --package=string, -p string
       Name of the resource package (defaults to "resources")
       This is used to generate filenames and symbol prefixes
   --deploy=dir, -x dir (defaults to ".")
       Generate everything needed for deployment into <dir>
   --keep, -k
       Keep file extensions when generating symbols
   --text, -t
       Append a null terminator to each data blob
   --cfile, -c
       Generate xxd-style C file (useful for WebAssembly)
   --json, -j
       Embed a JSON string in the output that provides a summary
       of all resource sizes and names. Useful for size analysis.
    --quiet, -q
        Suppress console output

Examples:
    RESGEN -cp textures jungle.png beach.png
    > Generated files: textures.h, textures.S, textures.apple.S, textures.bin, textures.c
    > Generated symbols: TEXTURES_JUNGLE_DATA, TEXTURES_JUNGLE_SIZE,
                         TEXTURES_BEACH_DATA, TEXTURES_BEACH_SIZE
)TXT";

// Template for the Apple-specific assembly file.
static const char* APPLE_ASM_TEMPLATE = R"ASM(
    .global _{RESOURCES}PACKAGE
    .section __TEXT,__const
_{RESOURCES}PACKAGE:
    .incbin "{resources}.bin"
)ASM";

// Template for the standard assembly file.
static const char* ASM_TEMPLATE = R"ASM(
    .global {RESOURCES}PACKAGE
    .section .rodata
{RESOURCES}PACKAGE:
    .incbin "{resources}.bin"
)ASM";

// Prints the usage string, replacing the placeholder with the actual executable name.
static void printUsage(const char* name) {
    std::string const execName(Path(name).getName());
    const std::string from("RESGEN");
    std::string usage(USAGE);
    std::size_t pos = 0;
    while ((pos = usage.find(from, pos)) != std::string::npos) {
        usage.replace(pos, from.length(), execName);
        pos += execName.length();
    }
    std::cout << usage;
}

// Prints the license text to the console.
static void license() {
    static const char *license[] = {
        #include "licenses/licenses.inc"
        nullptr
    };

    for (const char **p = &license[0]; *p; ++p) {
        std::cout << *p << std::endl;
    }
}

// A helper function to replace all occurrences of a substring within a string.
static std::string& replaceAll(std::string& context, const std::string& from, const std::string& to) {
    std::size_t pos = 0;
    while ((pos = context.find(from, pos)) != std::string::npos) {
        context.replace(pos, from.length(), to);
        pos += to.length();
    }
    return context;
}

// Parses command-line arguments using getopt and populates the AppConfig struct.
static AppConfig handleArguments(int const argc, char* argv[]) {
    static constexpr const char* OPTSTR = "hLp:x:ktcqj";
    static const option OPTIONS[] = {
            { "help",         no_argument,       nullptr, 'h' },
            { "license",      no_argument,       nullptr, 'L' },
            { "package",      required_argument, nullptr, 'p' },
            { "deploy",       required_argument, nullptr, 'x' },
            { "keep",         no_argument,       nullptr, 'k' },
            { "text",         no_argument,       nullptr, 't' },
            { "cfile",        no_argument,       nullptr, 'c' },
            { "quiet",        no_argument,       nullptr, 'q' },
            { "json",         no_argument,       nullptr, 'j' },
            { nullptr, 0, nullptr, 0 }
    };

    AppConfig config;
    int opt;
    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, nullptr)) >= 0) {
        switch (opt) {
            case 'h':
                printUsage(argv[0]);
                std::exit(0);
            case 'L':
                license();
                std::exit(0);
            case 'p':
                config.packageName = optarg;
                break;
            case 'x':
                config.deployDir = optarg;
                break;
            case 'k':
                config.keepExtension = true;
                break;
            case 't':
                config.appendNull = true;
                break;
            case 'c':
                config.generateC = true;
                break;
            case 'q':
                config.quietMode = true;
                break;
            case 'j':
                config.embedJson = true;
                break;
            default:
                printUsage(argv[0]);
                std::exit(1);
        }
    }

    // Treat remaining arguments as input file paths.
    for (int i = optind; i < argc; ++i) {
        config.inputPaths.emplace_back(argv[i]);
    }

    return config;
}

// Opens a file for writing, printing an error and exiting on failure.
static std::ofstream openOutputFile(const Path& path, std::ios_base::openmode const mode = std::ios_base::out) {
    std::ofstream stream(path.getPath(), mode);
    if (!stream) {
        std::cerr << "Unable to open " << path << std::endl;
        std::exit(1);
    }
    return stream;
}

// Reads an entire binary file into a vector of bytes.
static std::vector<std::uint8_t> readFile(const Path& path) {
    std::ifstream inStream(path.getPath(), std::ios::binary);
    if (!inStream) {
        std::cerr << "Unable to open " << path << std::endl;
        std::exit(1);
    }
    return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(inStream)),
            (std::istreambuf_iterator<char>()));
}

// Writes the generated header file, but only if its content has changed.
// This helps avoid unnecessary recompilation in build systems.
static void writeHeaderIfChanged(const Path& path, const std::string& newContents) {
    std::ifstream headerInStream(path.getPath(), std::ifstream::ate);
    if (headerInStream) {
        long const fileSize = headerInStream.tellg();
        if (fileSize == newContents.size()) {
            std::vector<char> previous(fileSize);
            headerInStream.seekg(0);
            headerInStream.read(previous.data(), fileSize);
            if (0 == memcmp(previous.data(), newContents.c_str(), fileSize)) {
                return;
            }
        }
    }
    std::ofstream headerOutStream = openOutputFile(path);
    headerOutStream << newContents;
}

// Writes a resource's data to a C-style char array (similar to 'xxd -i').
static void writeXxdEntry(std::ofstream& xxdStream, const std::string& resourceName,
        const std::vector<std::uint8_t>& content) {
    xxdStream << "// " << resourceName << "\n";
    xxdStream << std::setfill('0') << std::hex;
    for (std::size_t i = 0; i < content.size(); i++) {
        if (i > 0 && i % 20 == 0) {
            xxdStream << "\n";
        }
        xxdStream << "0x" << std::setw(2) << int(content[i]) << ", ";
    }
    if (!content.empty() && content.size() % 20 != 0) {
        xxdStream << "\n";
    }
    xxdStream << "\n";
}

int main(int argc, char* argv[]) {
    // Parse arguments and check for input files.
    const AppConfig config = handleArguments(argc, argv);
    if (config.inputPaths.empty()) {
        printUsage(argv[0]);
        return 1;
    }

    // If JSON embedding is enabled, add a placeholder to the input list.
    std::vector<Path> inputPaths = config.inputPaths;
    if (config.embedJson) {
        inputPaths.emplace_back(g_jsonMagicString);
    }

    // Generate names and symbols based on the package name.
    const std::string packageFile = config.packageName;
    std::string packagePrefix = std::string(config.packageName) + "_";
    std::transform(packagePrefix.begin(), packagePrefix.end(), packagePrefix.begin(), [](unsigned char c) { return std::toupper(c); });
    const std::string packageSymbol = packagePrefix + "PACKAGE";

    // Create the deployment directory if it doesn't exist.
    const Path deployDir(config.deployDir);
    if (!deployDir.exists()) {
        deployDir.mkdirRecursive();
    }

    // Define all output file paths.
    const Path appleAsmPath(deployDir + (packageFile + ".apple.S"));
    const Path asmPath(deployDir + (packageFile + ".S"));
    const Path binPath(deployDir + (packageFile + ".bin"));
    const Path headerPath(deployDir + (packageFile + ".h"));
    const Path xxdPath(deployDir + (packageFile + ".c"));

    // Prepare assembly file content by replacing placeholders.
    std::string aasmstr(APPLE_ASM_TEMPLATE);
    replaceAll(aasmstr, "{RESOURCES}", packagePrefix);
    replaceAll(aasmstr, "{resources}", packageFile);

    std::string asmstr(ASM_TEMPLATE);
    replaceAll(asmstr, "{RESOURCES}", packagePrefix);
    replaceAll(asmstr, "{resources}", packageFile);

    // Open all output file streams.
    auto appleAsmStream = openOutputFile(appleAsmPath);
    auto asmStream = openOutputFile(asmPath);
    auto binStream = openOutputFile(binPath, std::ios::binary);

    // Begin constructing the C header file content.
    std::ostringstream headerStream;
    headerStream << "#ifndef " << packagePrefix << "H_\n"
                 << "#define " << packagePrefix << "H_\n\n"
                 << "#include <stdint.h>\n\n"
                 << "extern \"C\" {\n"
                 << "    extern const uint8_t " << packageSymbol << "[];\n";

    // String streams to accumulate header macros and the JSON summary.
    std::ostringstream headerMacros;
    std::ostringstream jsonStream;

    // If generating a C file, open the stream and write the initial boilerplate.
    std::ofstream xxdStream;
    if (config.generateC) {
        xxdStream = openOutputFile(xxdPath);
        xxdStream << "#include <stdint.h>\n"
                  << "const uint8_t " << packageSymbol << "[] = {\n";
    }

    // Process each input file to build the resource collection.
    jsonStream << "{";
    std::size_t offset = 0;
    for (const auto& inPath : inputPaths) {
        std::vector<std::uint8_t> content;
        if (inPath != g_jsonMagicString) {
            // For a regular file, read its binary content.
            content = readFile(inPath);
        } else {
            // For the JSON placeholder, finalize and embed the JSON summary blob.
            // The blob is formatted as: __RESGEN__\0<size>\0{...json...}
            std::string jsonString = jsonStream.str();
            jsonString[jsonString.size() - 1] = '}';
            std::ostringstream jsonBlob;
            jsonBlob << g_jsonMagicString << '\0';
            jsonBlob << jsonString.size() << '\0';
            jsonBlob << jsonString;
            jsonString = jsonBlob.str();
            const auto* jsonPtr = reinterpret_cast<const std::uint8_t*>(jsonString.c_str());
            content.assign(jsonPtr, jsonPtr + jsonBlob.str().size());
        }

        // Optionally append a null terminator, useful for text resources.
        if (config.appendNull) {
            content.push_back(0);
        }

        // Generate the resource's symbol name from its file name.
        std::string rname = config.keepExtension ? inPath.getName() : inPath.getNameWithoutExtension();
        replaceAll(rname, ".", "_");
        std::transform(rname.begin(), rname.end(), rname.begin(), [](unsigned char c) { return std::toupper(c); });
        const std::string prname = packagePrefix + rname;

        // Write the resource content to the aggregate binary file.
        binStream.write(reinterpret_cast<const char*>(content.data()), content.size());

        // Generate C preprocessor macros for the resource's offset, size, and data pointer.
        headerMacros << "#define " << prname << "_OFFSET " << offset << "\n"
                     << "#define " << prname << "_SIZE " << content.size() << "\n"
                     << "#define " << prname << "_DATA (" << packageSymbol << " + " << prname << "_OFFSET)\n\n";

        // If enabled, write the content to the C source file.
        if (config.generateC) {
            writeXxdEntry(xxdStream, rname, content);
        }

        // Add an entry to the JSON summary and update the running offset.
        jsonStream << "\"" << rname << "\":" << content.size() << ",";
        offset += content.size();
    }

    // Finalize the header file content.
    headerStream << "}\n\n";
    headerStream << headerMacros.str();
    headerStream << "#endif\n";

    // Write the header and assembly files.
    writeHeaderIfChanged(headerPath, headerStream.str());
    asmStream << asmstr << std::endl;
    appleAsmStream << aasmstr << std::endl;

    // Report generated files to the console unless in quiet mode.
    if (!config.quietMode) {
        std::cout << "Generated files: " << headerPath << " " << asmPath << " " << appleAsmPath << " "
             << binPath;
    }

    // Finalize the C file and report it.
    if (config.generateC) {
        xxdStream << "};\n\n";
        if (!config.quietMode) {
            std::cout << " " << xxdPath;
        }
    }

    if (!config.quietMode) {
        std::cout << std::endl;
    }

    return 0;
}