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

#include <filaflat/ChunkContainer.h>
#include <filaflat/FilaflatDefs.h>
#include <filaflat/MaterialParser.h>
#include <filaflat/Unflattener.h>
#include <filaflat/ShaderBuilder.h>

#include <filament/EngineEnums.h>
#include <filament/MaterialEnums.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/UniformInterfaceBlock.h>
#include <filament/driver/DriverEnums.h>

#include <utils/Path.h>

#include <spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>

#include <fstream>
#include <iomanip>
#include <iostream>

using namespace filaflat;
using namespace utils;

static const int alignment = 17;

struct Config {
    bool printGLSL = false;
    bool printSPIRV = false;
    bool transpile = false;
    bool binary = false;
    uint64_t shaderIndex;
};

struct ShaderInfo {
    filament::driver::ShaderModel shaderModel;
    uint8_t variant;
    filament::driver::ShaderType pipelineStage;
    uint32_t offset;
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
                    "   --print-vkglsl=[index], -v\n"
                    "       Print the nth Vulkan shader transpiled into GLSL\n\n"
                    "   --dump-binary=[index], -b\n"
                    "       Dump binary SPIRV for the nth Vulkan shader to 'out.spv'\n\n"
                    "   --license\n"
                    "       Print copyright and license information\n\n"
    );

    const std::string from("MATINFO");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), execName);
    }
    printf("%s", usage.c_str());
}

static void license() {
    std::cout <<
    #include "licenses/licenses.inc"
    ;
}

static int handleArguments(int argc, char* argv[], Config* config) {
    static constexpr const char* OPTSTR = "hlg:s:v:b:";
    static const struct option OPTIONS[] = {
            { "help",         no_argument,       0, 'h' },
            { "license",      no_argument,       0, 'l' },
            { "print-glsl",   required_argument, 0, 'g' },
            { "print-spirv",  required_argument, 0, 's' },
            { "print-vkglsl", required_argument, 0, 'v' },
            { "dump-binary",  required_argument, 0, 'b' },
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
            case 'b':
                config->printSPIRV = true;
                config->shaderIndex = static_cast<uint64_t>(std::stoi(arg));
                config->binary = true;
                break;
        }
    }

    return optind;
}

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

template<typename T>
static bool read(const ChunkContainer& container, filamat::ChunkType type, T* value) noexcept {
    if (!container.hasChunk(type)) {
        return false;
    }

    Unflattener unflattener(container.getChunkStart(type), container.getChunkEnd(type));
    return unflattener.read(value);
}

template<typename T>
static const char* toString(T value);

template<>
const char* toString(filament::Shading shadingModel) {
    switch (shadingModel) {
        case filament::Shading::UNLIT: return "unlit";
        case filament::Shading::LIT: return "lit";
        case filament::Shading::SUBSURFACE: return "subsurface";
        case filament::Shading::CLOTH: return "cloth";
    }
}

template<>
const char* toString(filament::BlendingMode blendingMode) {
    switch (blendingMode) {
        case filament::BlendingMode::OPAQUE: return "opaque";
        case filament::BlendingMode::TRANSPARENT: return "transparent";
        case filament::BlendingMode::FADE: return "fade";
        case filament::BlendingMode::ADD: return "add";
        case filament::BlendingMode::MASKED: return "masked";
    }
}

template<>
const char* toString(filament::Interpolation interpolation) {
    switch (interpolation) {
        case filament::Interpolation::SMOOTH: return "smooth";
        case filament::Interpolation::FLAT: return "flat";
    }
}

template<>
const char* toString(filament::VertexDomain domain) {
    switch (domain) {
        case filament::VertexDomain::OBJECT: return "object";
        case filament::VertexDomain::WORLD: return "world";
        case filament::VertexDomain::VIEW: return "view";
        case filament::VertexDomain::DEVICE: return "device";
    }
}

template<>
const char* toString(filament::driver::CullingMode cullingMode) {
    switch (cullingMode) {
        case filament::driver::CullingMode::NONE: return "none";
        case filament::driver::CullingMode::FRONT: return "front";
        case filament::driver::CullingMode::BACK: return "back";
        case filament::driver::CullingMode::FRONT_AND_BACK: return "front & back";
    }
}

template<>
const char* toString(filament::TransparencyMode transparencyMode) {
    switch (transparencyMode) {
        case filament::TransparencyMode::DEFAULT: return "default";
        case filament::TransparencyMode::TWO_PASSES_ONE_SIDE: return "two passes, one side";
        case filament::TransparencyMode::TWO_PASSES_TWO_SIDES: return "two passes, two sides";
    }
}

template<>
const char* toString(filament::VertexAttribute attribute) {
    switch (attribute) {
        case filament::POSITION: return "position";
        case filament::TANGENTS: return "tangents";
        case filament::COLOR: return "color";
        case filament::UV0: return "uv0";
        case filament::UV1: return "uv1";
        case filament::BONE_INDICES: return "bone indices";
        case filament::BONE_WEIGHTS: return "bone weights";
    }
    return "--";
}

template<>
const char* toString(bool value) {
    return value ? "true" : "false";
}

template<>
const char* toString(filament::driver::ShaderType stage) {
    switch (stage) {
        case filament::driver::ShaderType::VERTEX: return "vs";
        case filament::driver::ShaderType::FRAGMENT: return "fs";
        default: break;
    }
    return "--";
}

template<>
const char* toString(filament::driver::ShaderModel model) {
    switch (model) {
        case filament::driver::ShaderModel::UNKNOWN: return "--";
        case filament::driver::ShaderModel::GL_ES_30: return "gles30";
        case filament::driver::ShaderModel::GL_CORE_41: return "gl41";
    }
}

template<>
const char* toString(filament::UniformInterfaceBlock::Type type) {
    switch (type) {
        case filament::driver::UniformType::BOOL:   return "bool";
        case filament::driver::UniformType::BOOL2:  return "bool2";
        case filament::driver::UniformType::BOOL3:  return "bool3";
        case filament::driver::UniformType::BOOL4:  return "bool4";
        case filament::driver::UniformType::FLOAT:  return "float";
        case filament::driver::UniformType::FLOAT2: return "float2";
        case filament::driver::UniformType::FLOAT3: return "float3";
        case filament::driver::UniformType::FLOAT4: return "float4";
        case filament::driver::UniformType::INT:    return "int";
        case filament::driver::UniformType::INT2:   return "int2";
        case filament::driver::UniformType::INT3:   return "int3";
        case filament::driver::UniformType::INT4:   return "int4";
        case filament::driver::UniformType::UINT:   return "uint";
        case filament::driver::UniformType::UINT2:  return "uint2";
        case filament::driver::UniformType::UINT3:  return "uint3";
        case filament::driver::UniformType::UINT4:  return "uint4";
        case filament::driver::UniformType::MAT3:   return "float3x3";
        case filament::driver::UniformType::MAT4:   return "float4x4";
    }
}

template<>
const char* toString(filament::SamplerInterfaceBlock::Type type) {
    switch (type) {
        case filament::driver::SamplerType::SAMPLER_2D: return "sampler2D";
        case filament::driver::SamplerType::SAMPLER_CUBEMAP: return "samplerCubemap";
        case filament::driver::SamplerType::SAMPLER_EXTERNAL: return "samplerExternal";
    }
}

template<>
const char* toString(filament::SamplerInterfaceBlock::Precision precision) {
    switch (precision) {
        case filament::driver::Precision::LOW: return "lowp";
        case filament::driver::Precision::MEDIUM: return "mediump";
        case filament::driver::Precision::HIGH: return "highp";
        case filament::driver::Precision::DEFAULT: return "default";
    }
}

template<>
const char* toString(filament::SamplerInterfaceBlock::Format format) {
    switch (format) {
        case filament::driver::SamplerFormat::INT: return "int";
        case filament::driver::SamplerFormat::UINT: return "uint";
        case filament::driver::SamplerFormat::FLOAT: return "float";
        case filament::driver::SamplerFormat::SHADOW: return "shadow";
    }
}

static std::string arraySizeToString(uint64_t size) {
    if (size > 1) {
        std::string s = "[";
        s += size;
        s += "]";
        return s;
    }
    return "";
}

template<typename T, typename V>
static void printChunk(const ChunkContainer& container, filamat::ChunkType type, const char* title) {
    T value;
    if (read(container, type, reinterpret_cast<V*>(&value))) {
        std::cout << "    " << std::setw(alignment) << std::left << title;
        std::cout << toString(value) << std::endl;
    }
}

static void printFloatChunk(const ChunkContainer& container, filamat::ChunkType type,
        const char* title) {
    float value;
    if (read(container, type, &value)) {
        std::cout << "    " << std::setw(alignment) << std::left << title;
        std::cout << std::setprecision(2) << value << std::endl;
    }
}

static void printUint32Chunk(const ChunkContainer& container, filamat::ChunkType type,
        const char* title) {
    uint32_t value;
    if (read(container, type, &value)) {
        std::cout << "    " << std::setw(alignment) << std::left << title;
        std::cout << value << std::endl;
    }
}

static bool printMaterial(const ChunkContainer& container) {
    std::cout << "Material:" << std::endl;

    uint32_t version;
    if (read(container, filamat::MaterialVersion, &version)) {
        std::cout << "    " << std::setw(alignment) << std::left << "Version: ";
        std::cout << version << std::endl;
    }

    printUint32Chunk(container, filamat::PostProcessVersion, "Post process version: ");

    CString name;
    if (read(container, filamat::MaterialName, &name)) {
        std::cout << "    " << std::setw(alignment) << std::left << "Name: ";
        std::cout << name.c_str() << std::endl;
    }

    std::cout << std::endl;

    std::cout << "Shading:" << std::endl;
    printChunk<filament::Shading, uint8_t>(container, filamat::MaterialShading, "Model: ");
    printChunk<filament::VertexDomain, uint8_t>(container, filamat::MaterialVertexDomain,
            "Vertex domain: ");
    printChunk<filament::Interpolation, uint8_t>(container, filamat::MaterialInterpolation,
            "Interpolation: ");
    printChunk<bool, bool>(container, filamat::MaterialShadowMultiplier, "Shadow multiply: ");

    std::cout << std::endl;

    std::cout << "Raster state:" << std::endl;
    printChunk<filament::BlendingMode, uint8_t>(container, filamat::MaterialBlendingMode, "Blending: ");
    printFloatChunk(container, filamat::MaterialMaskThreshold, "Mask threshold: ");
    printChunk<bool, bool>(container, filamat::MaterialColorWrite, "Color write: ");
    printChunk<bool, bool>(container, filamat::MaterialDepthWrite, "Depth write: ");
    printChunk<bool, bool>(container, filamat::MaterialDepthTest, "Depth test: ");
    printChunk<bool, bool>(container, filamat::MaterialDoubleSided, "Double sided: ");
    printChunk<filament::driver::CullingMode, uint8_t>(container, filamat::MaterialCullingMode,
            "Culling: ");
    printChunk<filament::TransparencyMode, uint8_t>(container, filamat::MaterialTransparencyMode,
            "Transparency: ");

    std::cout << std::endl;

    uint32_t requiredAttributes;
    if (read(container, filamat::MaterialRequiredAttributes, &requiredAttributes)) {
        filament::AttributeBitset bitset;
        bitset.setValue(requiredAttributes);

        if (bitset.count() > 0) {
            std::cout << "Required attributes:" << std::endl;
            for (size_t i = 0; i < bitset.size(); i++) {
                if (bitset.test(i)) {
                    std::cout << "    " <<
                              toString(static_cast<filament::VertexAttribute>(i)) << std::endl;
                }
            }
            std::cout << std::endl;
        }
    }

    return true;
}

static bool printParametersInfo(ChunkContainer container) {
    if (!container.hasChunk(filamat::ChunkType::MaterialUib)) {
        return true;
    }

    Unflattener uib(
            container.getChunkStart(filamat::ChunkType::MaterialUib),
            container.getChunkEnd(filamat::ChunkType::MaterialUib));

    CString name;
    if (!uib.read(&name)) {
        return false;
    }

    uint64_t uibCount;
    if (!uib.read(&uibCount)) {
        return false;
    }

    Unflattener sib(
            container.getChunkStart(filamat::ChunkType::MaterialSib),
            container.getChunkEnd(filamat::ChunkType::MaterialSib));

    if (!sib.read(&name)) {
        return false;
    }

    uint64_t sibCount;
    if (!sib.read(&sibCount)) {
        return false;
    }

    if (uibCount == 0 && sibCount == 0) {
        return true;
    }

    std::cout << "Parameters:" << std::endl;

    for (uint64_t i = 0; i < uibCount; i++) {
        CString fieldName;
        uint64_t fieldSize;
        uint8_t fieldType;
        uint8_t fieldPrecision;

        if (!uib.read(&fieldName)) {
            return false;
        }

        if (!uib.read(&fieldSize)) {
            return false;
        }

        if (!uib.read(&fieldType)) {
            return false;
        }

        if (!uib.read(&fieldPrecision)) {
            return false;
        }

        std::cout << "    "
                  << std::setw(alignment) << fieldName.c_str()
                  << std::setw(alignment) << toString(filament::UniformInterfaceBlock::Type(fieldType))
                  << arraySizeToString(fieldSize)
                  << std::setw(10) << toString(filament::UniformInterfaceBlock::Precision(fieldPrecision))
                  << std::endl;
    }

    for (uint64_t i = 0; i < sibCount; i++) {
        CString fieldName;
        uint8_t fieldType;
        uint8_t fieldFormat;
        uint8_t fieldPrecision;
        bool fieldMultisample;

        if (!sib.read(&fieldName)) {
            return false;
        }

        if (!sib.read(&fieldType)) {
            return false;
        }

        if (!sib.read(&fieldFormat))
            return false;

        if (!sib.read(&fieldPrecision)) {
            return false;
        }

        if (!sib.read(&fieldMultisample)) {
            return false;
        }

        std::cout << "    "
                << std::setw(alignment) << fieldName.c_str()
                << std::setw(alignment) << toString(filament::SamplerInterfaceBlock::Type(fieldType))
                << std::setw(10) << toString(filament::SamplerInterfaceBlock::Precision(fieldPrecision))
                << toString(filament::SamplerInterfaceBlock::Format(fieldFormat))
                << std::endl;
    }

    std::cout << std::endl;

    return true;
}

static void printChunks(const ChunkContainer& container) {
    std::cout << "Chunks:" << std::endl;

    std::cout << "    " << std::setw(9) << std::left << "Name ";
    std::cout << std::setw(7) << std::right << "Size" << std::endl;

    size_t count = container.getChunkCount();
    for (size_t i = 0; i < count; i++) {
        auto chunk = container.getChunk(i);
        std::cout << "    " << typeToString(chunk.type).c_str() << " ";
        std::cout << std::setw(7) << std::right << chunk.desc.size << std::endl;
    }
}

static bool getGlShaderInfo(ChunkContainer container, std::vector<ShaderInfo>* info) {
    if (!container.hasChunk(filamat::ChunkType::MaterialGlsl)) {
        return true; // that's not an error, a material can have no glsl stuff
    }

    Unflattener unflattener(
            container.getChunkStart(filamat::ChunkType::MaterialGlsl),
            container.getChunkEnd(filamat::ChunkType::MaterialGlsl));

    uint64_t shaderCount;
    if (!unflattener.read(&shaderCount) || shaderCount == 0) {
        return false;
    }

    info->clear();
    info->reserve(shaderCount);

    for (uint64_t i = 0; i < shaderCount; i++) {
        uint8_t shaderModelValue;
        uint8_t variantValue;
        uint8_t pipelineStageValue;
        uint32_t offsetValue;

        if (!unflattener.read(&shaderModelValue)) {
            return false;
        }

        if (!unflattener.read(&variantValue)) {
            return false;
        }

        if (!unflattener.read(&pipelineStageValue)) {
            return false;
        }

        if (!unflattener.read(&offsetValue)) {
            return false;
        }

        info->push_back({
            .shaderModel = filament::driver::ShaderModel(shaderModelValue),
            .variant = variantValue,
            .pipelineStage = filament::driver::ShaderType(pipelineStageValue),
            .offset = offsetValue
        });
    }
    return true;
}

static bool getVkShaderInfo(ChunkContainer container, std::vector<ShaderInfo>* info) {
    if (!container.hasChunk(filamat::ChunkType::MaterialSpirv)) {
        return true; // that's not an error, a material can have no spirv stuff
    }

    Unflattener unflattener(
            container.getChunkStart(filamat::ChunkType::MaterialSpirv),
            container.getChunkEnd(filamat::ChunkType::MaterialSpirv));

    uint64_t shaderCount;
    if (!unflattener.read(&shaderCount) || shaderCount == 0) {
        return false;
    }

    info->clear();
    info->reserve(shaderCount);

    for (uint64_t i = 0; i < shaderCount; i++) {
        uint8_t shaderModelValue;
        uint8_t variantValue;
        uint8_t pipelineStageValue;
        uint32_t dictionaryIndex;

        if (!unflattener.read(&shaderModelValue)) {
            return false;
        }

        if (!unflattener.read(&variantValue)) {
            return false;
        }

        if (!unflattener.read(&pipelineStageValue)) {
            return false;
        }

        if (!unflattener.read(&dictionaryIndex)) {
            return false;
        }

        info->push_back({
            .shaderModel = filament::driver::ShaderModel(shaderModelValue),
            .variant = variantValue,
            .pipelineStage = filament::driver::ShaderType(pipelineStageValue),
            .offset = dictionaryIndex
        });
    }
    return true;
}

static bool printGlslInfo(ChunkContainer container) {
    std::vector<ShaderInfo> info;
    if (!getGlShaderInfo(container, &info)) {
        return false;
    }
    std::cout << "GLSL shaders:" << std::endl;
    for (uint64_t i = 0; i < info.size(); ++i) {
        const auto& item = info[i];
        std::cout << "    #";
        std::cout << std::setw(4) << std::left << i;
        std::cout << std::setw(6) << std::left << toString(item.shaderModel);
        std::cout << " ";
        std::cout << std::setw(2) << std::left << toString(item.pipelineStage);
        std::cout << " ";
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2)
                  << std::right << (int) item.variant;
        std::cout << std::setfill(' ') << std::dec << std::endl;
    }
    std::cout << std::endl;
    return true;
}

static bool printVkInfo(ChunkContainer container) {
    std::vector<ShaderInfo> info;
    if (!getVkShaderInfo(container, &info)) {
        return false;
    }
    std::cout << "Vulkan shaders:" << std::endl;
    for (uint64_t i = 0; i < info.size(); ++i) {
        const auto& item = info[i];
        std::cout << "    #";
        std::cout << std::setw(4) << std::left << i;
        std::cout << std::setw(6) << std::left << toString(item.shaderModel);
        std::cout << " ";
        std::cout << std::setw(2) << std::left << toString(item.pipelineStage);
        std::cout << " ";
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2)
                  << std::right << (int) item.variant;
        std::cout << std::setfill(' ') << std::dec << std::endl;
    }
    std::cout << std::endl;
    return true;
}

static bool printMaterialInfo(const ChunkContainer& container) {
    if (!printMaterial(container)) {
        return false;
    }

    if (!printParametersInfo(container)) {
        return false;
    }

    if (!printGlslInfo(container)) {
        return false;
    }

    if (!printVkInfo(container)) {
        return false;
    }

    printChunks(container);

    std::cout << std::endl;

    return true;
}

static void transpileSpirv(const std::vector<uint32_t>& spirv) {
    using namespace spirv_cross;

    // We assume that users of the tool are interested in reading GLSL-ES, since our primary
    // target platform is Android.
    CompilerGLSL::Options emitOptions;
    emitOptions.es = true;
    emitOptions.vulkan_semantics = true;

    CompilerGLSL glslCompiler(move(spirv));
    glslCompiler.set_common_options(emitOptions);
    std::cout << glslCompiler.compile();
}

static void disassembleSpirv(const std::vector<uint32_t>& spirv) {
    // If desired feel free to locally replace this with the glslang disassembler (spv::Disassemble)
    // but please do not submit. We prefer to use the syntax that the standalone "spirv-dis" tool
    // uses, which lets us easily generate test cases for the spirv-cross project.
    auto context = spvContextCreate(SPV_ENV_UNIVERSAL_1_1);
    spv_text text = nullptr;
    const uint32_t options = SPV_BINARY_TO_TEXT_OPTION_INDENT;
    spvBinaryToText(context, spirv.data(), spirv.size(), options, &text, nullptr);
    std::cout << text->str << std::endl;
    spvTextDestroy(text);
    spvContextDestroy(context);
}

static void dumpSpirvBinary(const std::vector<uint32_t>& spirv, std::string filename) {
    std::ofstream out(filename, std::ofstream::binary);
    out.write((const char*) spirv.data(), spirv.size() * 4);
    std::cout << "Binary SPIR-V dumped to " << filename << std::endl;
}

static bool parseChunks(Config config, void* data, size_t size) {
    ChunkContainer container(data, size);
    if (!container.parse()) {
        return false;
    }
    if (config.printGLSL || config.printSPIRV) {
        filaflat::ShaderBuilder builder;
        std::vector<ShaderInfo> info;

        if (config.printGLSL) {
            MaterialParser parser(filament::driver::Backend::OPENGL, data, size);
            if (!parser.parse() ||
                    (!parser.isShadingMaterial() && !parser.isPostProcessMaterial())) {
                return false;
            }

            if (!getGlShaderInfo(container, &info)) {
                std::cerr << "Failed to parse GLSL chunk." << std::endl;
                return false;
            }

            if (config.shaderIndex >= info.size()) {
                std::cerr << "Shader index out of range." << std::endl;
                return false;
            }

            const auto& item = info[config.shaderIndex];
            parser.getShader(item.shaderModel, item.variant, item.pipelineStage, builder);
            std::cout << builder.c_str();

            return true;
        }

        if (config.printSPIRV) {
            MaterialParser parser(filament::driver::Backend::VULKAN, data, size);
            if (!parser.parse() ||
                    (!parser.isShadingMaterial() && !parser.isPostProcessMaterial())) {
                return false;
            }

            if (!getVkShaderInfo(container, &info)) {
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
            uint32_t const* words = reinterpret_cast<uint32_t const*>(builder.c_str());
            assert(0 == (builder.size() % 4));
            const std::vector<uint32_t> spirv(words, words + builder.size() / 4);

            if (config.transpile) {
                transpileSpirv(spirv);
            } else if (config.binary) {
                dumpSpirvBinary(spirv, "out.spv");
            } else {
                disassembleSpirv(spirv);
            }

            return true;
        }
    }

    if (!printMaterialInfo(container)) {
        std::cerr << "The source material is invalid." << std::endl;
        return false;
    }

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

    std::ifstream in(src.c_str(), std::ifstream::in);
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
