/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include <fstream>
#include <iostream>
#include <string>

#include <tsl/robin_map.h>

#include <utils/FixedCapacityVector.h>
#include <utils/memalign.h>
#include <utils/Panic.h>

#include <uberz/ReadableArchive.h>
#include <uberz/WritableArchive.h>

#include <zstd.h>

using namespace std;
using namespace utils;
using namespace filament::uberz;

using StringMap = tsl::robin_map<std::string, std::string>;

static std::string g_outputFile = "materials.uberz";
static bool g_appendMode = false;
static bool g_quietMode = false;
static bool g_verboseMode = false;
static StringMap g_templateMap;

static const char* USAGE = R"TXT(
UBERZ aggregates and compresses a set of filamat files into a single archive file. It includes
metadata that specifies the feature set that each material supports. By default, it generates
a file called "materials.uberz" but this can be customized with -o.

Usage:
    UBERZ [options] <src_name_0> <src_name_1> ...

For each src_name, UBERZ looks for "src_name.filamat" and "src_name.spec" in the current
working directory. If either of these files do not exist, an error is reported. Each
pair of filamat/spec files corresponds to a material in the generated archive.

For more information on the format of the spec file, see the gltfio README.

Options:
   --append, -a
       Enable append mode
   --help, -h
       Print this message
   --license, -L
       Print copyright and license information
   --output=filename, -o filename
       Specify a custom filename.
    --quiet, -q
        Suppress console output
    --template <macro>=<string>, -T<macro>=<string>
        Replaces ${MACRO} with specified string before parsing spec file
)TXT";

static void printUsage(const char* name) {
    std::string execName(Path(name).getName());
    const std::string from("UBERZ");
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

// Parses a string with the form KEY=VALUE and inserts the key-value pair into the given map.
// If only a key is specified, then the given default value is used.
static void insertMapEntry(std::string assignment, StringMap& map, std::string defaultValue) {
    const char* const start = assignment.c_str();
    const char* cursor = start;
    const char* end = cursor + assignment.length();
    while (cursor < end && *cursor != '=') {
        cursor++;
    }
    if (UTILS_UNLIKELY(cursor == end)) {
        map.emplace(std::string(start, cursor - start), defaultValue);
    } else if (UTILS_LIKELY(cursor != start && cursor + 1 < end)) {
        map.emplace(std::string(start, cursor - start), cursor + 1);
    }
}

static int handleArguments(int argc, char* argv[]) {
    static constexpr const char* OPTSTR = "ahLqvo:T:";
    static const struct option OPTIONS[] = {
            { "append",   no_argument,       0, 'a' },
            { "help",     no_argument,       0, 'h' },
            { "license",  no_argument,       0, 'L' },
            { "quiet",    no_argument,       0, 'q' },
            { "verbose",  no_argument,       0, 'v' },
            { "output",   required_argument, 0, 'o' },
            { "template", required_argument, 0, 'T' },
            { 0, 0, 0, 0 }  // termination of the option list
    };

    int opt;
    int optionIndex = 0;

    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'a':
                g_appendMode = true;
                break;
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'L':
                license();
                exit(0);
            case 'o':
                g_outputFile = optarg;
                break;
            case 'T':
                insertMapEntry(optarg, g_templateMap, "missing");
                break;
            case 'q':
                g_quietMode = true;
                break;
            case 'v':
                g_verboseMode = true;
                break;
        }
    }

    return optind;
}

static size_t getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
};

int main(int argc, char* argv[]) {
    const int optionIndex = handleArguments(argc, argv);
    const size_t additionalMaterialsCount = argc - optionIndex;
    if (additionalMaterialsCount < 1) {
        printUsage(argv[0]);
        return 1;
    }

    size_t existingMaterialsCount = 0;
    ReadableArchive* existingArchive = nullptr;

    // In append mode, the first step is to consume the output file.
    if (g_appendMode) {
        const size_t archiveSize = getFileSize(g_outputFile.c_str());
        FixedCapacityVector<uint8_t> archiveBuffer(archiveSize);
        uint8_t* archiveData = archiveBuffer.data();
        std::ifstream in(g_outputFile.c_str(), std::ifstream::in | std::ifstream::binary);
        if (!in.read((char*) archiveData, archiveSize)) {
            cerr << "Unable to consume " << g_outputFile << endl;
            exit(1);
        }
        const uint64_t decompSize = ZSTD_getFrameContentSize(archiveData, archiveSize);
        if (decompSize == ZSTD_CONTENTSIZE_UNKNOWN || decompSize == ZSTD_CONTENTSIZE_ERROR) {
            PANIC_POSTCONDITION("Decompression error.");
        }
        uint64_t* basePointer = (uint64_t*) utils::aligned_alloc(decompSize, 8);
        ZSTD_decompress(basePointer, decompSize, archiveData, archiveSize);
        existingArchive = (ReadableArchive*) basePointer;
        convertOffsetsToPointers(existingArchive);
        existingMaterialsCount = existingArchive->specsCount;
    }

    WritableArchive outputArchive(existingMaterialsCount + additionalMaterialsCount);

    // In append mode, add the existing materials into the new WritableArchive.
    if (existingArchive) {
        for (size_t specIndex = 0; specIndex < existingMaterialsCount; ++specIndex) {
            // We do not know where this material was originally consumed from, so just use
            // a made-up string (it is only used for error messages).
            std::string materialName = "mat" + to_string(specIndex);
            const ArchiveSpec& spec = existingArchive->specs[specIndex];
            outputArchive.addMaterial(materialName.c_str(), spec.package, spec.packageByteCount);
            outputArchive.setShadingModel(spec.shadingModel);
            outputArchive.setBlendingModel(spec.blendingMode);
            for (uint16_t flagIndex = 0; flagIndex < spec.flagsCount; ++flagIndex) {
                auto flag = spec.flags[flagIndex];
                outputArchive.setFeatureFlag(flag.name, flag.value);
            }
        }
        utils::aligned_free(existingArchive);
    }

    for (int argIndex = optionIndex; argIndex < argc; ++argIndex) {
        std::string name(argv[argIndex]);
        const Path filamatPath(name + ".filamat");
        if (!filamatPath.exists()) {
            cerr << "Unable to open " << filamatPath << endl;
            exit(1);
        }
        const Path specPath(name + ".spec");
        if (!specPath.exists()) {
            cerr << "Unable to open " << specPath << endl;
            exit(1);
        }

        const size_t filamatSize = getFileSize(filamatPath.c_str());
        FixedCapacityVector<uint8_t> filamatBuffer(filamatSize);
        std::ifstream in(filamatPath.c_str(), std::ifstream::in | std::ifstream::binary);
        if (!in.read((char*) filamatBuffer.data(), filamatSize)) {
            cerr << "Unable to consume " << filamatPath << endl;
            exit(1);
        }

        // This tool is often invoked by CMake, so in verbose mode we print an index to stderr.
        // This allows us to see the spec index associated with each file for diagnostic purposes.
        if (g_verboseMode) {
            size_t specIndex = existingMaterialsCount + argIndex - optionIndex;
            fprintf(stderr, "uberz %2zu %s\n", specIndex, filamatPath.getName().c_str());
        }

        outputArchive.addMaterial(name.c_str(), filamatBuffer.data(), filamatSize);

        std::string specLine;
        ifstream specStream(specPath.c_str());
        while (std::getline(specStream, specLine)) {
            for (auto pair : g_templateMap) {
                auto [from, to] = pair;
                from = "${" + from + "}";
                for (size_t pos = specLine.find(from); pos != std::string::npos;
                        pos = specLine.find(from, pos)) {
                    specLine.replace(pos, from.length(), to);
                }
            }
            outputArchive.addSpecLine(specLine.c_str());
        }
    }

    FixedCapacityVector<uint8_t> binBuffer = outputArchive.serialize();

    ofstream binStream(g_outputFile, ios::binary);
    if (!binStream) {
        cerr << "Unable to open " << g_outputFile << endl;
        exit(1);
    }
    binStream.write((const char*) binBuffer.data(), binBuffer.size());
    binStream.close();

    return 0;
}
