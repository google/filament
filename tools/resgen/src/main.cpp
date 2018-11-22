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

#include <utils/Path.h>

#include <getopt/getopt.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace utils;

static const char* g_packageName = "resources";
static const char* g_deployDir = ".";

static const char* USAGE = R"TXT(
RESGEN aggregates a sequence of binary blobs, each of which becomes a "resource" whose id
is the basename of the input file. It produces the following set of files:

    resources.h ......... declares sizes and offsets for each resource
    resources.S ......... small assembly file with incbin directive and rodata section
    resources.apple.S ... ditto but with different rodata name and underscore prefixes
    resources.bin ....... the aggregated binary blob that the incbin refers to
    resources.c ......... large xxd-style array (useful for WebAssembly)

Usage:
    RESGEN [options] <input_file_0> <input_file_1> ...

Options:
   --help, -h
       print this message
   --license, -L
       print copyright and license information
   --package=string, -p string
       Name of the resource package (defaults to "resources")
       This is used to generate filenames and symbol prefixes
   --deploy=dir, -x dir (defaults to ".")
       Generate everything needed for deployment into <dir>

Examples:
    RESGEN -p textures jungle.png beach.png
    > Generated files: textures.h, textures.S, textures.apple.S, textures.bin, textures.c
    > Generated symbols: TEXTURES_JUNGLE_DATA, TEXTURES_JUNGLE_SIZE,
                         TEXTURES_BEACH_DATA, TEXTURES_BEACH_SIZE
)TXT";

static const char* APPLE_ASM_TEMPLATE = R"ASM(
    .global _{RESOURCES}PACKAGE
    .global _{RESOURCES}PACKAGE_SIZE
    .section __TEXT,__const
_{RESOURCES}PACKAGE:
    .incbin "{resources}.bin"
1:
_{RESOURCES}PACKAGE_SIZE:
    .int 1b - _{RESOURCES}PACKAGE
)ASM";

static const char* ASM_TEMPLATE = R"ASM(
    .global {RESOURCES}PACKAGE
    .global {RESOURCES}PACKAGE_SIZE
    .section .rodata
{RESOURCES}PACKAGE:
    .incbin "{resources}.bin"
1:
{RESOURCES}PACKAGE_SIZE:
    .int 1b - {RESOURCES}PACKAGE
)ASM";

static void printUsage(const char* name) {
    string execName(Path(name).getName());
    const string from("RESGEN");
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
    static constexpr const char* OPTSTR = "hLp:x:";
    static const struct option OPTIONS[] = {
            { "help",                 no_argument, 0, 'h' },
            { "license",              no_argument, 0, 'L' },
            { "package",        required_argument, 0, 'p' },
            { "deploy",         required_argument, 0, 'x' },
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
            case 'L':
                license();
                exit(0);
            case 'p':
                g_packageName = optarg;
                break;
            case 'x':
                g_deployDir = optarg;
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

    vector<Path> inputPaths;
    for (int argIndex = optionIndex; argIndex < argc; ++argIndex) {
        inputPaths.emplace_back(argv[argIndex]);
    }

    string packageFile = g_packageName;
    string packagePrefix = string(g_packageName) + "_";
    transform(packageFile.begin(), packageFile.end(), packageFile.begin(), ::tolower);
    transform(packagePrefix.begin(), packagePrefix.end(), packagePrefix.begin(), ::toupper);

    const Path deployDir(g_deployDir);
    if (!deployDir.exists()) {
        deployDir.mkdirRecursive();
    }

    const Path appleAsmPath(deployDir + (packageFile + ".apple.S"));
    const Path asmPath(deployDir + (packageFile + ".S"));
    const Path binPath(deployDir + (packageFile + ".bin"));
    const Path headerPath(deployDir + (packageFile + ".h"));
    const Path xxdPath(deployDir + (packageFile + ".c"));

    // In the assembly language templates, replace {RESOURCES} with packagePrefix and replace
    // {resources} with packageFile.
    const string k1("{RESOURCES}");
    const string k2("{resources}");
    string aasmstr(APPLE_ASM_TEMPLATE);
    string asmstr(ASM_TEMPLATE);
    for (size_t pos = aasmstr.find(k1); pos != string::npos; pos = aasmstr.find(k1, pos))
        aasmstr.replace(pos, k1.length(), packagePrefix);
    for (size_t pos = aasmstr.find(k2); pos != string::npos; pos = aasmstr.find(k2, pos))
        aasmstr.replace(pos, k2.length(), packageFile);
    for (size_t pos = asmstr.find(k1); pos != string::npos; pos = asmstr.find(k1, pos))
        asmstr.replace(pos, k1.length(), packagePrefix);
    for (size_t pos = asmstr.find(k2); pos != string::npos; pos = asmstr.find(k2, pos))
        asmstr.replace(pos, k2.length(), packageFile);

    // Generate the Apple-friendly assembly language file.
    ofstream appleAsmStream(appleAsmPath.getPath());
    if (!appleAsmStream) {
        cerr << "Unable to open " << appleAsmPath << endl;
        exit(1);
    }
    appleAsmStream << aasmstr << endl;
    appleAsmStream.close();

    // Generate the non-Apple assembly language file.
    ofstream asmStream(asmPath.getPath());
    if (!asmStream) {
        cerr << "Unable to open " << asmPath << endl;
        exit(1);
    }
    asmStream << asmstr << endl;
    asmStream.close();

    // Open the bin file for writing.
    ofstream binStream(binPath.getPath(), ios::binary);
    if (!binStream) {
        cerr << "Unable to open " << binPath << endl;
        exit(1);
    }

    // Open the header file for writing.
    ofstream headerStream(headerPath.getPath());
    if (!headerStream) {
        cerr << "Unable to open " << headerPath << endl;
        exit(1);
    }
    headerStream << "#ifndef " << packagePrefix << "H_" << endl
            << "#define " << packagePrefix << "H_" << endl << endl
            << "#include <stdint.h>" << endl << endl
            << "extern \"C\" {" << endl
            << "    extern const uint8_t " << packagePrefix << "PACKAGE[];" << endl
            << "    extern const int " << packagePrefix << "PACKAGE_SIZE;" << endl
            << "}" << endl << endl;

    // Open the generated C file for writing.
    ofstream xxdStream(xxdPath.getPath());
    if (!xxdStream) {
        cerr << "Unable to open " << xxdPath << endl;
        exit(1);
    }
    xxdStream << "#include <stdint.h>" << endl << endl
            << "const uint8_t " << packagePrefix << "PACKAGE[] = {" << endl;

    // Iterate through each input file and consume its contents.
    size_t offset = 0;
    for (const auto& inPath : inputPaths) {
        ifstream inStream(inPath.getPath(), ios::binary);
        if (!inStream) {
            cerr << "Unable to open " << inPath << endl;
            exit(1);
        }
        vector<uint8_t> content((istreambuf_iterator<char>(inStream)), {});

        // Formulate the resource name and the prefixed resource name.
        string rname = inPath.getNameWithoutExtension();
        transform(rname.begin(), rname.end(), rname.begin(), ::toupper);
        const string prname = packagePrefix + rname;

        // Write the binary blob into the bin file.
        binStream.write((const char*) content.data(), content.size());

        // Write the offset and size into the header file.
        headerStream << "static const size_t " << prname << "_OFFSET = " << offset << ";" << endl
                << "static const size_t " << prname << "_SIZE = " << content.size() << ";" << endl
                << "static const uint8_t* " << prname << "_DATA = "
                << packagePrefix << "PACKAGE + " << prname << "_OFFSET;" << endl;

        // Write the xxd-style ASCII array, followed by a blank line.
        xxdStream << "// " << rname << endl;
        size_t i = 0;
        for (; i < content.size(); i++) {
            if (i > 0 && i % 20 == 0) {
                xxdStream << endl;
            }
            xxdStream << "0x" << setfill('0') << setw(2) << hex << (int) content[i] << ", ";
        }
        if (i % 20 != 0) xxdStream << endl;
        xxdStream << endl;
        offset += content.size();
    }

    headerStream << endl << "#endif" << endl;

    xxdStream << "};" << endl << endl
            << "const int " << packagePrefix << "PACKAGE_SIZE = " << dec << offset << ";" << endl;

    cout << "Generated files: "
        << headerPath << " "
        << asmPath << " "
        << appleAsmPath << " "
        << binPath << " "
        << xxdPath << endl;
}
