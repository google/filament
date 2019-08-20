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
    // TODO
    json << "},\n";
    json << "\"raster\": {\n";
    // TODO
    json << "},\n";
    return true;
}

static bool printParametersInfo(ostream& json, const ChunkContainer& container) {
    // TODO
    return true;
}

static void printShaderInfo(ostream& json, const std::vector<ShaderInfo>& info) {
    for (uint64_t i = 0; i < info.size(); ++i) {
        const auto& item = info[i];
        json
            << "    {"
            << "\"index\": " << i << ", "
            << "\"shaderModel\": \"" << toString(item.shaderModel) << "\", "
            << "\"pipelineStage\": \"" << toString(item.pipelineStage) << "\", "
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
    json << "{\n";
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
    // TODO
    json << "]\n";

    json << "}\n";
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
