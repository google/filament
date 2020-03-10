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

#include <getopt/getopt.h>

#include <utils/Path.h>

#include <filaflat/BlobDictionary.h>
#include <filaflat/ChunkContainer.h>

#include <matdbg/DebugServer.h>
#include <matdbg/ShaderExtractor.h>
#include <matdbg/ShaderInfo.h>
#include <matdbg/TextWriter.h>

#include <spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <thread>

using namespace filament::matdbg;

using filaflat::BlobDictionary;
using filaflat::ChunkContainer;
using filament::backend::Backend;
using utils::Path;

struct Config {
    bool printGLSL = false;
    bool printSPIRV = false;
    bool printMetal = false;
    bool printDictionaryGLSL = false;
    bool printDictionarySPIRV = false;
    bool printDictionaryMetal = false;
    bool transpile = false;
    bool binary = false;
    bool analyze = false;
    uint64_t shaderIndex;
    int serverPort = 0;
};

static void printUsage(const char* name) {
    std::string execName(utils::Path(name).getName());
    std::string usage(
            "MATINFO prints information about material files compiled with matc\n"
            "Usage:\n"
            "    MATINFO [options] <material file>\n"
            "\n"
            "Options:\n"
            "   --help, -h\n"
            "       Print this message\n\n"
            "   --print-glsl=[index], -g\n"
            "       Print GLSL for the nth shader (0 is the first OpenGL shader)\n\n"
            "   --print-spirv=[index], -s\n"
            "       Print disasm for the nth shader (0 is the first Vulkan shader)\n\n"
            "   --print-metal=[index], -m\n"
            "       Print Metal Shading Language for the nth shader (0 is the first Metal shader)\n\n"
            "   --print-vkglsl=[index], -v\n"
            "       Print the nth Vulkan shader transpiled into GLSL\n\n"
            "   --print-dic-glsl\n"
            "       Print the GLSL dictionary\n\n"
            "   --print-dic-metal\n"
            "       Print the Metal dictionary\n\n"
            "   --print-dic-vk\n"
            "       Print the Vulkan dictionary\n\n"
            "   --web-server=[port], -w\n"
            "       Serve a web page at the given port (e.g. 8080)\n\n"
            "   --dump-binary=[index], -b\n"
            "       Dump binary SPIRV for the nth Vulkan shader to 'out.spv'\n\n"
            "   --license\n"
            "       Print copyright and license information\n\n"
            "   --analyze-spirv=[index], -a\n"
            "       Print annotated GLSL for the nth shader (0 is the first Vulkan shader)\n\n"
    );

    const std::string from("MATINFO");
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

static int handleArguments(int argc, char* argv[], Config* config) {
    static constexpr const char* OPTSTR = "hla:g:s:v:b:m:b:w:xyz";
    static const struct option OPTIONS[] = {
            { "help",            no_argument,       0, 'h' },
            { "license",         no_argument,       0, 'l' },
            { "analyze-spirv",   required_argument, 0, 'a' },
            { "print-glsl",      required_argument, 0, 'g' },
            { "print-spirv",     required_argument, 0, 's' },
            { "print-vkglsl",    required_argument, 0, 'v' },
            { "print-metal",     required_argument, 0, 'm' },
            { "print-dic-glsl",  no_argument,       0, 'x' },
            { "print-dic-metal", no_argument,       0, 'y' },
            { "print-dic-vk",    no_argument,       0, 'z' },
            { "dump-binary",     required_argument, 0, 'b' },
            { "web-server",      required_argument, 0, 'w' },
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
            case 'l':
                license();
                exit(0);
            case 'g':
                config->printGLSL = true;
                config->shaderIndex = static_cast<uint64_t>(std::stoi(arg));
                break;
            case 's':
                config->printSPIRV = true;
                config->shaderIndex = static_cast<uint64_t>(std::stoi(arg));
                break;
            case 'v':
                config->printSPIRV = true;
                config->shaderIndex = static_cast<uint64_t>(std::stoi(arg));
                config->transpile = true;
                break;
            case 'a':
                config->printSPIRV = true;
                config->shaderIndex = static_cast<uint64_t>(std::stoi(arg));
                config->analyze = true;
                break;
            case 'b':
                config->printSPIRV = true;
                config->shaderIndex = static_cast<uint64_t>(std::stoi(arg));
                config->binary = true;
                break;
            case 'm':
                config->printMetal = true;
                config->shaderIndex = static_cast<uint64_t>(std::stoi(arg));
                break;
            case 'w':
                config->serverPort = 8080;
                break;
            case 'x':
                config->printDictionaryGLSL = true;
                break;
            case 'y':
                config->printDictionaryMetal = true;
                break;
            case 'z':
                config->printDictionarySPIRV = true;
                break;
        }
    }

    return optind;
}

template<typename T>
static bool read(const filaflat::ChunkContainer& container, filamat::ChunkType type,
        T* value) noexcept {
    if (!container.hasChunk(type)) {
        return false;
    }

    filaflat::Unflattener unflattener(container.getChunkStart(type), container.getChunkEnd(type));
    return unflattener.read(value);
}

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

// Consumes SPIRV binary and produces a GLSL-ES string.
static void transpileSpirv(const std::vector<uint32_t>& spirv) {
    std::cout << ShaderExtractor::spirvToGLSL(spirv.data(), spirv.size()).c_str();
}

// Consumes SPIRV binary and produces an ordered map from "line number" to "GLSL string" where
// the line number is determined by line directives, and the GLSL string is one or more lines of
// transpiled GLSL-ES.
static std::map<int, std::string> transpileSpirvToLines(const std::vector<uint32_t>& spirv) {
    using namespace spirv_cross;

    CompilerGLSL::Options emitOptions;
    emitOptions.es = true;
    emitOptions.vulkan_semantics = true;
    emitOptions.emit_line_directives = true;

    CompilerGLSL glslCompiler(move(spirv));
    glslCompiler.set_common_options(emitOptions);
    std::string transpiled = glslCompiler.compile();

    std::map<int, std::string> result;
    const std::regex lineDirectivePattern("\\#line ([0-9]+)");

    std::istringstream ss(glslCompiler.compile());
    std::string glslCodeline;
    int currentLineNumber = -1;
    while (std::getline(ss, glslCodeline, '\n')) {
        std::smatch matchResult;
        if (std::regex_search(glslCodeline, matchResult, lineDirectivePattern)) {
            currentLineNumber = stoi(matchResult[1].str());
        } else {
            result[currentLineNumber] += glslCodeline + "\n";
        }
    }

    return result;
}

static void analyzeSpirv(const std::vector<uint32_t>& spirv, const char* disassembly) {
    using namespace std;

    const map<int, string> glsl = transpileSpirvToLines(spirv);
    const regex globalDecoratorPattern("OpDecorate (\\%[A-Za-z_0-9]+) RelaxedPrecision");
    const regex typeDefinitionPattern("(\\%[A-Za-z_0-9]+) = OpType[A-Z][a-z]+");
    const regex memberDecoratorPattern("OpMemberDecorate (\\%[A-Za-z_0-9]+) ([0-9]+) RelaxedPrecision");
    const regex lineDirectivePattern("OpLine (\\%[A-Za-z_0-9]+) ([0-9]+)");
    const regex binaryFunctionPattern("(\\%[A-Za-z_0-9]+).*(\\%[A-Za-z_0-9]+)");
    const regex operandPattern("(\\%[A-Za-z_0-9]+)");
    const regex sensitivePattern("OpComposite[A-Za-z]+");

    string spirvInstruction;
    smatch matchResult;

    // In the first pass, collect types and variables that are decorated as "relaxed".
    //
    // NOTE: We also collect struct members but do not use them in any analysis (yet).
    // In SPIR-V, struct fields are accessed through pointers, so we would need to:
    //
    // 1) Create a map of all OpConstant values (integers only, %int and %uint)
    // 2) Parse all "OpAccessChain" instructions and dereference their OpConstant arguments
    // 3) Follow through to the downstream OpStore / OpLoad
    //
    // Regarding struct field precision, the SPIR-V specification says:
    //
    //   When applied to a variable or structure member, all loads and stores from the decorated
    //   object may be treated as though they were decorated with RelaxedPrecision. Loads may also
    //   be decorated with RelaxedPrecision, in which case they are treated as operating at
    //   relaxed precision.

    set<string> relaxedPrecisionVariables;
    set<string> typeIds;
    {
        istringstream ss(disassembly);
        while (getline(ss, spirvInstruction, '\n')) {
            if (regex_search(spirvInstruction, matchResult, typeDefinitionPattern)) {
                typeIds.insert(matchResult[1].str());
            } else if (regex_search(spirvInstruction, matchResult, globalDecoratorPattern)) {
                relaxedPrecisionVariables.insert(matchResult[1].str());
            } else if (regex_search(spirvInstruction, matchResult, memberDecoratorPattern)) {
                string member = matchResult[1].str() + "." + matchResult[2].str();
                relaxedPrecisionVariables.insert(member);
            }
        }
    }

    // In the second pass, track line numbers and detect potential mixed precision.
    // The line directive are not necessarily in order (!) so we also track their ordering.

    map<int, string> mixedPrecisionInfo;
    vector<int> lineNumbers = {-1};
    {
        istringstream ss(disassembly);
        int currentLineNumber = -1;
        while (getline(ss, spirvInstruction, '\n')) {
            if (regex_search(spirvInstruction, matchResult, lineDirectivePattern)) {
                currentLineNumber = stoi(matchResult[2].str());
                lineNumbers.push_back(currentLineNumber);
            } else {

                // Trim out the leftmost whitespace.
                const string trimmed = regex_replace(spirvInstruction, regex("^\\s+"), string(""));

                // Check for mixed precision.
                bool mixed = false;
                int relaxed = -1;
                string remaining = trimmed;
                string info;
                while (regex_search(remaining, matchResult, operandPattern)) {
                    const string arg = matchResult[1].str();
                    if (typeIds.count(arg) == 0) {
                        if (relaxedPrecisionVariables.count(arg) > 0) {
                            mixed = relaxed == 0 ? true : mixed;
                            relaxed = 1;
                            info += arg + " ";
                        } else {
                            mixed = relaxed == 1 ? true : mixed;
                            relaxed = 0;
                        }
                    }
                    remaining = matchResult.suffix();
                }
                if (mixed && currentLineNumber > -1) {
                    if (regex_search(trimmed, matchResult, sensitivePattern)) {
                        info = "// " + trimmed + " ; relaxed = " + info + "\n";
                        mixedPrecisionInfo[currentLineNumber] += info;
                    }
                }
            }
        }
        if (currentLineNumber == -1) {
            cerr << "No #line directives found, did you use a debug version of matc?" << endl;
            return;
        }
    }

    // Finally, dump out the annotated GLSL.

    for (int lineNumber : lineNumbers) {
        if (!glsl.count(lineNumber)) {
            continue;
        }
        istringstream ss(glsl.at(lineNumber));
        string glslCodeline;
        bool firstLine = true;
        while (getline(ss, glslCodeline, '\n')) {
            cout << glslCodeline << endl;
            if (firstLine) {
                if (mixedPrecisionInfo.count(lineNumber)) {
                    string info = mixedPrecisionInfo.at(lineNumber);
                    cout << "// POTENTIAL MIXED PRECISION\n" << info;
                }
                firstLine = false;
            }
        }
    }
}

static void disassembleSpirv(const std::vector<uint32_t>& spirv, bool analyze) {
    utils::CString text = ShaderExtractor::spirvToText(spirv.data(), spirv.size());
    if (analyze) {
        analyzeSpirv(spirv, text.c_str());
    } else {
        std::cout << text.c_str() << std::endl;
    }
}

static void dumpSpirvBinary(const std::vector<uint32_t>& spirv, std::string filename) {
    std::ofstream out(filename, std::ofstream::binary);
    out.write((const char*) spirv.data(), spirv.size() * 4);
    std::cout << "Binary SPIR-V dumped to " << filename << std::endl;
}

static bool parseChunks(Config config, void* data, size_t size) {
    using namespace filament::matdbg;
    ChunkContainer container(data, size);
    if (!container.parse()) {
        return false;
    }

    if (config.serverPort) {
        // Spin up a web server on a secondary thread.
        DebugServer server(Backend::DEFAULT, config.serverPort);

        // Notify the server that we have a filamat file.
        utils::CString name;
        read(container, filamat::MaterialName, &name);
        server.addMaterial(name, data, size);

        // Keep listening until the user does Ctrl+C.
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        return true;
    }

    if (config.printGLSL || config.printSPIRV || config.printMetal) {
        filaflat::ShaderBuilder builder;
        std::vector<ShaderInfo> info;

        if (config.printGLSL) {
            ShaderExtractor parser(Backend::OPENGL, data, size);
            if (!parser.parse()) {
                return false;
            }

            info.resize(getShaderCount(container, filamat::ChunkType::MaterialGlsl));
            if (!getGlShaderInfo(container, info.data())) {
                std::cerr << "Failed to parse GLSL chunk." << std::endl;
                return false;
            }

            if (config.shaderIndex >= info.size()) {
                std::cerr << "Shader index out of range." << std::endl;
                return false;
            }

            const auto& item = info[config.shaderIndex];
            parser.getShader(item.shaderModel, item.variant, item.pipelineStage, builder);

            // Casted to char* to print as a string rather than hex value.
            std::cout << (const char*) builder.data();

            return true;
        }

        if (config.printSPIRV) {
            ShaderExtractor parser(Backend::VULKAN, data, size);
            if (!parser.parse()) {
                return false;
            }

            info.resize(getShaderCount(container, filamat::ChunkType::MaterialSpirv));
            if (!getVkShaderInfo(container, info.data())) {
                std::cerr << "Failed to parse SPIRV chunk." << std::endl;
                return false;
            }

            if (config.shaderIndex >= info.size()) {
                std::cerr << "Shader index out of range." << std::endl;
                return false;
            }

            const auto& item = info[config.shaderIndex];
            parser.getShader(item.shaderModel, item.variant, item.pipelineStage, builder);

            // Build std::vector<uint32_t> since that's what the Khronos libraries consume.
            uint32_t const* words = reinterpret_cast<uint32_t const*>(builder.data());
            assert(0 == (builder.size() % 4));
            const std::vector<uint32_t> spirv(words, words + builder.size() / 4);

            if (config.transpile) {
                transpileSpirv(spirv);
            } else if (config.binary) {
                dumpSpirvBinary(spirv, "out.spv");
            } else {
                disassembleSpirv(spirv, config.analyze);
            }

            return true;
        }

        if (config.printMetal) {
            ShaderExtractor parser(Backend::METAL, data, size);
            if (!parser.parse()) {
                return false;
            }

            info.resize(getShaderCount(container, filamat::ChunkType::MaterialMetal));
            if (!getMetalShaderInfo(container, info.data())) {
                std::cerr << "Failed to parse Metal chunk." << std::endl;
                return false;
            }

            if (config.shaderIndex >= info.size()) {
                std::cerr << "Shader index out of range." << std::endl;
                return false;
            }

            const auto& item = info[config.shaderIndex];
            parser.getShader(item.shaderModel, item.variant, item.pipelineStage, builder);
            std::cout << (const char*) builder.data();

            return true;
        }
    }

    TextWriter writer;

    if (config.printDictionaryGLSL || config.printDictionarySPIRV || config.printDictionaryMetal) {
        ShaderExtractor parser(
                (config.printDictionaryGLSL  ? Backend::OPENGL :
                (config.printDictionarySPIRV ? Backend::VULKAN :
                (config.printDictionaryMetal ? Backend::METAL  : Backend::DEFAULT))), data, size);

        if (!parser.parse()) {
            return false;
        }

        BlobDictionary dictionary;
        if (!parser.getDictionary(dictionary)) {
            return false;
        }

        for (size_t i = 0; i < dictionary.size(); i++) {
            std::cout << dictionary.getString(i) << std::endl;
        }

        return true;
    }

    if (!writer.writeMaterialInfo(container)) {
        return false;
    }

    std::cout << writer.getString();

    return true;
}

// Parse the contents of .inc files, which look like: "0xba, 0xdf, 0xf0" etc. Happily, istream
// skips over whitespace and commas, and stoul takes care of leading "0x" when parsing hex.
static bool parseTextBlob(Config config, std::istream& in) {
    std::vector<char> buffer;
    std::string hexcode;
    while (in >> hexcode) {
        buffer.push_back(static_cast<char>(std::stoul(hexcode, nullptr, 16)));
    }
    return parseChunks(config, buffer.data(), buffer.size());
}

static bool parseBinary(Config config, std::istream& in, long fileSize) {
    std::vector<char> buffer(static_cast<unsigned long>(fileSize));
    if (in.read(buffer.data(), fileSize)) {
        return parseChunks(config, buffer.data(), buffer.size());
    }
    std::cerr << "Could not read the source material." << std::endl;
    return false;
}

int main(int argc, char* argv[]) {
    Config config;
    int optionIndex = handleArguments(argc, argv, &config);

    int numArgs = argc - optionIndex;
    if (numArgs < 1) {
        printUsage(argv[0]);
        return 1;
    }

    Path src(argv[optionIndex]);
    if (!src.exists()) {
        std::cerr << "The source material " << src << " does not exist." << std::endl;
        return 1;
    }

    long fileSize = static_cast<long>(getFileSize(src.c_str()));
    if (fileSize <= 0) {
        std::cerr << "The source material " << src << " is invalid." << std::endl;
        return 1;
    }

    std::ifstream in(src.c_str(), std::ifstream::in | std::ios::binary);
    if (in.is_open()) {
        if (src.getExtension() == "inc") {
            return parseTextBlob(config, in) ? 0 : 1;
        } else {
            return parseBinary(config, in, fileSize) ? 0 : 1;
        }
    } else {
        std::cerr << "Could not open the source material " << src << std::endl;
        return 1;
    };

    return 0;
}
