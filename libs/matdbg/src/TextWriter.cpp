/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <filaflat/ChunkContainer.h>

#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

#include <matdbg/TextWriter.h>
#include <matdbg/ShaderInfo.h>

#include <sstream>
#include <iomanip>

#include "CommonWriter.h"

using namespace filament;
using namespace backend;
using namespace filaflat;
using namespace filamat;
using namespace std;
using namespace utils;

namespace filament {
namespace matdbg {

constexpr int alignment = 32;
constexpr int shortAlignment = 15;

static string arraySizeToString(uint64_t size) {
    if (size > 1) {
        string s = "[";
        s += size;
        s += "]";
        return s;
    }
    return "";
}

template<typename T, typename V>
static void printChunk(ostream& text, const ChunkContainer& container, ChunkType type,
        const char* title) {
    T value;
    if (read(container, type, reinterpret_cast<V*>(&value))) {
        text << "    " << setw(alignment) << left << title;
        text << toString(value) << endl;
    }
}

static void printFloatChunk(ostream& text, const ChunkContainer& container, ChunkType type,
        const char* title) {
    float value;
    if (read(container, type, &value)) {
        text << "    " << setw(alignment) << left << title;
        text << setprecision(2) << value << endl;
    }
}

static void printUint32Chunk(ostream& text, const ChunkContainer& container,
        ChunkType type, const char* title) {
    uint32_t value;
    if (read(container, type, &value)) {
        text << "\"" << title << "\": " << value << ",\n";
    }
}

static void printStringChunk(ostream& text, const ChunkContainer& container,
        ChunkType type, const char* title) {
    CString value;
    if (read(container, type, &value)) {
        text << "\"" << title << "\": \"" << value.c_str() << "\",\n";
    }
}

static bool printMaterial(ostream& text, const ChunkContainer& container) {
    text << "Material:" << endl;

    uint32_t version;
    if (read(container, MaterialVersion, &version)) {
        text << "    " << setw(alignment) << left << "Version: ";
        text << version << endl;
    }

    uint8_t featureLevel;
    if (read(container, MaterialFeatureLevel, &featureLevel)) {
        text << "    " << setw(alignment) << left << "Feature level: ";
        text << +featureLevel << endl;
    }

    CString name;
    if (read(container, MaterialName, &name)) {
        text << "    " << setw(alignment) << left << "Name: ";
        text << name.c_str() << endl;
    }

    text << endl;

    text << "Shading:" << endl;
    printChunk<Shading, uint8_t>(text, container, MaterialShading, "Model: ");
    printChunk<MaterialDomain, uint8_t>(text, container, ChunkType::MaterialDomain, "Material domain: ");
    printChunk<UserVariantFilterMask, uint32_t>(text, container, ChunkType::MaterialVariantFilterMask, "Material Variant Filter: ");
    printChunk<VertexDomain, uint8_t>(text, container, MaterialVertexDomain, "Vertex domain: ");
    printChunk<Interpolation, uint8_t>(text, container, MaterialInterpolation, "Interpolation: ");
    printChunk<bool, bool>(text, container, MaterialShadowMultiplier, "Shadow multiply: ");
    printChunk<bool, bool>(text, container, MaterialSpecularAntiAliasing, "Specular anti-aliasing: ");
    printFloatChunk(text, container, MaterialSpecularAntiAliasingVariance, "    Variance: ");
    printFloatChunk(text, container, MaterialSpecularAntiAliasingThreshold, "    Threshold: ");
    printChunk<bool, bool>(text, container, MaterialClearCoatIorChange, "Clear coat IOR change: ");
    printChunk<bool, bool>(text, container, MaterialHasCustomDepthShader, "Has custom depth: ");

    text << endl;

    text << "Raster state:" << endl;
    printChunk<BlendingMode, uint8_t>(text, container, MaterialBlendingMode, "Blending: ");
    printFloatChunk(text, container, MaterialMaskThreshold, "Mask threshold: ");
    printChunk<bool, bool>(text, container, MaterialColorWrite, "Color write: ");
    printChunk<bool, bool>(text, container, MaterialDepthWrite, "Depth write: ");
    printChunk<bool, bool>(text, container, MaterialDepthTest, "Depth test: ");
    printChunk<bool, bool>(text, container, MaterialInstanced, "Instanced: ");
    printChunk<bool, bool>(text, container, MaterialDoubleSided, "Double sided: ");
    printChunk<CullingMode, uint8_t>(text, container, MaterialCullingMode, "Culling: ");
    printChunk<TransparencyMode, uint8_t>(text, container, MaterialTransparencyMode, "Transparency: ");

    text << endl;

    uint32_t requiredAttributes;
    if (read(container, MaterialRequiredAttributes, &requiredAttributes)) {
        AttributeBitset bitset;
        bitset.setValue(requiredAttributes);

        if (bitset.count() > 0) {
            text << "Required attributes:" << endl;
            for (size_t i = 0; i < bitset.size(); i++) {
                if (bitset.test(i)) {
                    text << "    " << toString(static_cast<VertexAttribute>(i)) << endl;
                }
            }
            text << endl;
        }
    }

    return true;
}

static bool printParametersInfo(ostream& text, const ChunkContainer& container) {
    if (!container.hasChunk(ChunkType::MaterialUib)) {
        return true;
    }

    auto [startUib, endUib] = container.getChunkRange(ChunkType::MaterialUib);
    Unflattener uib(startUib, endUib);

    CString name;
    if (!uib.read(&name)) {
        return false;
    }

    uint64_t uibCount;
    if (!uib.read(&uibCount)) {
        return false;
    }

    auto [startSib, endSib] = container.getChunkRange(ChunkType::MaterialSib);
    Unflattener sib(startSib, endSib);

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

    text << "Parameters:" << endl;

    for (uint64_t i = 0; i < uibCount; i++) {
        CString fieldName;
        uint64_t fieldSize;
        uint8_t fieldType;
        uint8_t fieldPrecision;
        uint8_t fieldAssociatedSampler;

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

        if (!uib.read(&fieldAssociatedSampler)) {
            return false;
        }

        text << "    "
                  << setw(alignment) << fieldName.c_str()
                  << setw(shortAlignment) << toString(UniformType(fieldType))
                  << arraySizeToString(fieldSize)
                  << setw(shortAlignment) << toString(Precision(fieldPrecision))
                  << endl;
    }

    for (uint64_t i = 0; i < sibCount; i++) {
        CString fieldName;
        uint8_t fieldBinding;
        uint8_t fieldType;
        uint8_t fieldFormat;
        uint8_t fieldPrecision;
        bool fieldUnfilterable;
        bool fieldMultisample;

        if (!sib.read(&fieldName)) {
            return false;
        }

        if (!sib.read(&fieldBinding)) {
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

        if (!sib.read(&fieldUnfilterable)) {
            return false;
        }

        if (!sib.read(&fieldMultisample)) {
            return false;
        }

        text << "    "
                << setw(alignment) << fieldName.c_str()
                << setw(shortAlignment) << +fieldBinding
                << setw(shortAlignment) << toString(SamplerType(fieldType))
                << setw(shortAlignment) << toString(Precision(fieldPrecision))
                << toString(SamplerFormat(fieldFormat))
                << endl;
    }

    text << endl;

    return true;
}

static bool printConstantInfo(ostream& text, const ChunkContainer& container) {
    if (!container.hasChunk(ChunkType::MaterialConstants)) {
        return true;
    }

    auto [startConstants, endConstants] = container.getChunkRange(ChunkType::MaterialConstants);
    Unflattener constants(startConstants, endConstants);

    uint64_t constantsCount;
    constants.read(&constantsCount);

    text << "Constants:" << endl;

    for (uint64_t i = 0; i < constantsCount; i++) {
        CString fieldName;
        uint8_t fieldType;

        if (!constants.read(&fieldName)) {
            return false;
        }

        if (!constants.read(&fieldType)) {
            return false;
        }

         text << "    "
         << setw(alignment) << fieldName.c_str()
         << setw(shortAlignment) << toString(ConstantType(fieldType))
         << endl;
    }

    text << endl;

    return true;
}

static bool printSubpassesInfo(ostream& text, const ChunkContainer& container) {

    // Subpasses are optional.
    if (container.hasChunk(ChunkType::MaterialSubpass)) {
        text << "Sub-passes:" << endl;
        auto [start, end] = container.getChunkRange(ChunkType::MaterialSubpass);
        Unflattener subpasses(start, end);

        CString name;
        if (!subpasses.read(&name)) {
            return false;
        }

        uint64_t subpassCount;
        subpasses.read(&subpassCount);

        for (uint64_t i = 0; i < subpassCount; i++) {
            CString fieldName;
            uint8_t fieldType;
            uint8_t fieldFormat;
            uint8_t fieldPrecision;
            uint8_t attachmentIndex;
            uint8_t binding;

            if (!subpasses.read(&fieldName)) {
                return false;
            }

            if (!subpasses.read(&fieldType)) {
                return false;
            }

            if (!subpasses.read(&fieldFormat))
                return false;

            if (!subpasses.read(&fieldPrecision)) {
                return false;
            }

            if (!subpasses.read(&attachmentIndex)) {
                return false;
            }

            if (!subpasses.read(&binding)) {
                return false;
            }

            text << "    "
                    << setw(alignment) << fieldName.c_str()
                    << setw(shortAlignment) << toString(SubpassType(fieldType))
                    << setw(shortAlignment) << toString(Precision(fieldPrecision))
                    << toString(SamplerFormat(fieldFormat))
                    << endl;
        }
        text << endl;
    }
    return true;
}

// Unpack a 64 bit integer into a string
inline utils::CString typeToString(uint64_t v) {
    uint8_t* raw = (uint8_t*) &v;
    char str[9];
    for (size_t i = 0; i < 8; i++) {
        str[7 - i] = raw[i];
    }
    str[8] = '\0';
    return utils::CString(str, strnlen(str, 8));
}

static void printChunks(ostream& text, const ChunkContainer& container) {
    text << "Chunks:" << endl;

    text << "    " << setw(9) << left << "Name ";
    text << setw(7) << right << "Size" << endl;

    size_t count = container.getChunkCount();
    for (size_t i = 0; i < count; i++) {
        auto chunk = container.getChunk(i);
        text << "    " << typeToString(chunk.type).c_str() << " ";
        text << setw(7) << right << chunk.desc.size << endl;
    }
}

static void printShaderInfo(ostream& text, const vector<ShaderInfo>& info,
        const ChunkContainer& container) {
    MaterialDomain domain = MaterialDomain::SURFACE;
    read(container, ChunkType::MaterialDomain, reinterpret_cast<uint8_t*>(&domain));
    for (uint64_t i = 0; i < info.size(); ++i) {
        const auto& item = info[i];
        text << "    #";
        text << setw(4) << left << i;
        text << setw(6) << left << toString(item.shaderModel);
        text << " ";
        text << setw(2) << left << toString(item.pipelineStage);
        text << " ";
        text << "0x" << hex << setfill('0') << setw(2)
             << right << +item.variant.key;
        text << setfill(' ') << dec;
        text << "   ";
        text << formatVariantString(item.variant, domain);
        text << endl;
    }
    text << endl;
}

static bool printShaderInfo(ostream& text, const ChunkContainer& container, ChunkType chunkType) {
    vector<ShaderInfo> info;
    info.resize(getShaderCount(container, chunkType));
    if (!getShaderInfo(container, info.data(), chunkType)) {
        return false;
    }
    switch (chunkType) {
        case ChunkType::MaterialGlsl:
            text << "GLSL shaders:" << endl;
            break;
        case ChunkType::MaterialEssl1:
            text << "ESSL1 shaders:" << endl;
            break;
        case ChunkType::MaterialSpirv:
            text << "Vulkan shaders:" << endl;
            break;
        case ChunkType::MaterialMetal:
            text << "Metal shaders:" << endl;
            break;
        case ChunkType::MaterialMetalLibrary:
            text << "Metal precompiled shader libraries:" << endl;
        break;
        case ChunkType::MaterialWgsl:
            text << "WGSL precompiled shader libraries:" << endl;
        break;
        default:
            assert(false && "Invalid shader ChunkType");
            break;
    }
    printShaderInfo(text, info, container);
    return true;
}

bool TextWriter::writeMaterialInfo(const filaflat::ChunkContainer& container) {
    ostringstream text;
    if (!printMaterial(text, container)) {
        return false;
    }
    if (!printParametersInfo(text, container)) {
        return false;
    }
    if (!printConstantInfo(text, container)) {
        return false;
    }
    if (!printSubpassesInfo(text, container)) {
        return false;
    }
    if (!printShaderInfo(text, container, ChunkType::MaterialGlsl)) {
        return false;
    }
    if (!printShaderInfo(text, container, ChunkType::MaterialEssl1)) {
        return false;
    }
    if (!printShaderInfo(text, container, ChunkType::MaterialSpirv)) {
        return false;
    }
    if (!printShaderInfo(text, container, ChunkType::MaterialMetal)) {
        return false;
    }
    if (!printShaderInfo(text, container, ChunkType::MaterialMetalLibrary)) {
        return false;
    }
    if (!printShaderInfo(text, container, ChunkType::MaterialWgsl)) {
        return false;
    }

    printChunks(text, container);

    text << endl;

    mTextString = CString(text.str().c_str());
    return true;
}

const char* TextWriter::getString() const {
    return mTextString.c_str();
}

size_t TextWriter::getSize() const {
    return mTextString.size();
}

} // namespace matdbg
} // namespace filament
