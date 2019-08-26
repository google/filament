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

#include <matdbg/JsonWriter.h>
#include <matdbg/ShaderInfo.h>

#include "CommonWriter.h"

#include <private/filament/Variant.h>

#include <iomanip>
#include <sstream>

using namespace filament;
using namespace backend;
using namespace filaflat;
using namespace filamat;
using namespace std;
using namespace utils;

namespace filament {
namespace matdbg {

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
static void printChunk(ostream& json, const ChunkContainer& container, ChunkType type,
        const char* title) {
    T value;
    if (read(container, type, reinterpret_cast<V*>(&value))) {
        json << "\"" << title << "\": \"" << toString(value) << "\",\n";
    }
}

static void printFloatChunk(ostream& json, const ChunkContainer& container, ChunkType type,
        const char* title) {
    float value;
    if (read(container, type, &value)) {
        json << "\"" << title << "\": " << setprecision(2) << value << ",\n";
    }
}

static void printUint32Chunk(ostream& json, const ChunkContainer& container,
        filamat::ChunkType type, const char* title) {
    uint32_t value;
    if (read(container, type, &value)) {
        json << "\"" << title << "\": " << value << ",\n";
    }
}

static void printStringChunk(ostream& json, const ChunkContainer& container,
        filamat::ChunkType type, const char* title) {
    CString value;
    if (read(container, type, &value)) {
        json << "\"" << title << "\": \"" << value.c_str() << "\",\n";
    }
}

static bool printMaterial(ostream& json, const ChunkContainer& container) {
    printStringChunk(json, container, filamat::MaterialName, "name");
    printUint32Chunk(json, container, filamat::MaterialVersion, "version");
    printUint32Chunk(json, container, filamat::PostProcessVersion, "pp_version");
    json << "\"shading\": {\n";
    printChunk<Shading, uint8_t>(json, container, MaterialShading, "model");
    printChunk<VertexDomain, uint8_t>(json, container, MaterialVertexDomain, "vertex_domain");
    printChunk<Interpolation, uint8_t>(json, container, MaterialInterpolation, "interpolation");
    printChunk<bool, bool>(json, container, MaterialShadowMultiplier, "shadow_multiply");
    printChunk<bool, bool>(json, container, MaterialSpecularAntiAliasing, "specular_antialiasing");
    printFloatChunk(json, container, MaterialSpecularAntiAliasingVariance, "variance");
    printFloatChunk(json, container, MaterialSpecularAntiAliasingThreshold, "threshold");
    printChunk<bool, bool>(json, container, MaterialClearCoatIorChange, "clear_coat_IOR_change");
    json << "\"_\": 0 },\n";
    json << "\"raster\": {\n";
    printChunk<BlendingMode, uint8_t>(json, container, MaterialBlendingMode, "blending");
    printFloatChunk(json, container, MaterialMaskThreshold, "mask_threshold");
    printChunk<bool, bool>(json, container, MaterialColorWrite, "color_write");
    printChunk<bool, bool>(json, container, MaterialDepthWrite, "depth_write");
    printChunk<bool, bool>(json, container, MaterialDepthTest, "depth_test");
    printChunk<bool, bool>(json, container, MaterialDoubleSided, "double_sided");
    printChunk<CullingMode, uint8_t>(json, container, MaterialCullingMode, "culling");
    printChunk<TransparencyMode, uint8_t>(json, container, MaterialTransparencyMode, "transparency");
    json << "\"_\": 0 },\n";
    return true;
}

static bool printParametersInfo(ostream& json, const ChunkContainer& container) {
    // TODO
    return true;
}

static void printShaderInfo(ostream& json, const std::vector<ShaderInfo>& info) {
    for (uint64_t i = 0; i < info.size(); ++i) {
        const auto& item = info[i];
        string variantString = "";

        // NOTE: The 3-character nomenclature used here is consistent with the ASCII art seen in the
        // Variant header file and allows the information to fit in a reasonable amount of space on
        // the page. The HTML file has a legend.
        if (item.variant) {
            if (item.variant & filament::Variant::DIRECTIONAL_LIGHTING)  variantString += "DIR|";
            if (item.variant & filament::Variant::DYNAMIC_LIGHTING)      variantString += "DYN|";
            if (item.variant & filament::Variant::SHADOW_RECEIVER)       variantString += "SRE|";
            if (item.variant & filament::Variant::SKINNING_OR_MORPHING)  variantString += "SKN|";
            variantString = variantString.substr(0, variantString.length() - 1);
        }

        string ps = (item.pipelineStage == backend::ShaderType::VERTEX) ? "vertex  " : "fragment";
        json
            << "    {"
            << "\"index\": \"" << std::setw(2) << i << "\", "
            << "\"shaderModel\": \"" << toString(item.shaderModel) << "\", "
            << "\"pipelineStage\": \"" << ps << "\", "
            << "\"variantString\": \"" << variantString << "\", "
            << "\"variant\": \"" << std::hex << int(item.variant) << std::dec << "\" }"
            << ((i == info.size() - 1) ? "\n" : ",\n");
    }
}

static bool printGlslInfo(ostream& json, const ChunkContainer& container) {
    std::vector<ShaderInfo> info;
    info.resize(getShaderCount(container, ChunkType::MaterialGlsl));
    if (!getGlShaderInfo(container, info.data())) {
        return false;
    }
    json << "\"opengl\": [\n";
    printShaderInfo(json, info);
    json << "],\n";
    return true;
}

static bool printVkInfo(ostream& json, const ChunkContainer& container) {
    std::vector<ShaderInfo> info;
    info.resize(getShaderCount(container, ChunkType::MaterialSpirv));
    if (!getVkShaderInfo(container, info.data())) {
        return false;
    }
    json << "\"vulkan\": [\n";
    printShaderInfo(json, info);
    json << "],\n";
    return true;
}

static bool printMetalInfo(ostream& json, const ChunkContainer& container) {
    std::vector<ShaderInfo> info;
    info.resize(getShaderCount(container, ChunkType::MaterialMetal));
    if (!getMetalShaderInfo(container, info.data())) {
        return false;
    }
    json << "\"metal\": [\n";
    printShaderInfo(json, info);
    json << "],\n";
    return true;
}

bool JsonWriter::writeMaterialInfo(const filaflat::ChunkContainer& container) {
    ostringstream json;
    if (!printMaterial(json, container)) {
        return false;
    }
    if (!printParametersInfo(json, container)) {
        return false;
    }
    if (!printGlslInfo(json, container)) {
        return false;
    }
    if (!printVkInfo(json, container)) {
        return false;
    }
    if (!printMetalInfo(json, container)) {
        return false;
    }

    json << "\"required_attributes\": [\n";
    uint32_t requiredAttributes;
    if (read(container, MaterialRequiredAttributes, &requiredAttributes)) {
        string comma;
        AttributeBitset bitset;
        bitset.setValue(requiredAttributes);
        if (bitset.count() > 0) {
            for (size_t i = 0; i < bitset.size(); i++) {
                if (bitset.test(i)) {
                    json << comma << "\"" << toString(static_cast<VertexAttribute>(i)) << "\"";
                    comma = ",";
                }
            }
        }
    }
    json << "]\n";

    mJsonString = CString(json.str().c_str());
    return true;
}

const char* JsonWriter::getJsonString() const {
    return mJsonString.c_str();
}

size_t JsonWriter::getJsonSize() const {
    return mJsonString.size();
}

} // namespace matdbg
} // namespace filament
