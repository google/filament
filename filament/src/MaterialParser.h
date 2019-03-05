/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAFLAT_MATERIAL_PARSER_H
#define TNT_FILAFLAT_MATERIAL_PARSER_H

#include <private/filament/EngineEnums.h>

#include <filament/MaterialEnums.h>
#include <filament/driver/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/CString.h>

#include <inttypes.h>

namespace filaflat {
class ChunkContainer;
class ShaderBuilder;
class Unflattener;
}

namespace filament {

class UniformInterfaceBlock;
class SamplerInterfaceBlock;
struct MaterialParserDetails;

class UTILS_PUBLIC MaterialParser {
public:
    MaterialParser(driver::Backend backend, const void* data, size_t size);
    ~MaterialParser();

    MaterialParser(MaterialParser const& rhs) noexcept = delete;
    MaterialParser& operator=(MaterialParser const& rhs) noexcept = delete;

    bool parse() noexcept;
    bool isShadingMaterial() const noexcept;
    bool isPostProcessMaterial() const noexcept;

    // Accessors
    bool getMaterialVersion(uint32_t* value) const noexcept;
    bool getPostProcessVersion(uint32_t* value) const noexcept;
    bool getName(utils::CString*) const noexcept;
    bool getUIB(UniformInterfaceBlock* uib) const noexcept;
    bool getSIB(SamplerInterfaceBlock* sib) const noexcept;
    bool getShaderModels(uint32_t* value) const noexcept;

    bool getDepthWriteSet(bool* value) const noexcept;
    bool getDepthWrite(bool* value) const noexcept;
    bool getDoubleSidedSet(bool* value) const noexcept;
    bool getDoubleSided(bool* value) const noexcept;
    bool getCullingMode(driver::CullingMode* value) const noexcept;
    bool getTransparencyMode(TransparencyMode* value) const noexcept;
    bool getColorWrite(bool* value) const noexcept;
    bool getDepthTest(bool* value) const noexcept;
    bool getInterpolation(Interpolation* value) const noexcept;
    bool getVertexDomain(VertexDomain* value) const noexcept;

    bool getShading(Shading*) const noexcept;
    bool getBlendingMode(BlendingMode*) const noexcept;
    bool getMaskThreshold(float*) const noexcept;
    bool hasShadowMultiplier(bool*) const noexcept;
    bool getRequiredAttributes(AttributeBitset*) const noexcept;
    bool hasCustomDepthShader(bool* value) const noexcept;

    bool getShader(filaflat::ShaderBuilder& shader, driver::ShaderModel shaderModel,
            uint8_t variant, driver::ShaderType stage) noexcept;

protected:
    filaflat::ChunkContainer& getChunkContainer() noexcept;
    filaflat::ChunkContainer const& getChunkContainer() const noexcept;
    MaterialParserDetails* mImpl = nullptr;
};

struct ChunkUniformInterfaceBlock {
    static bool unflatten(filaflat::Unflattener& unflattener, UniformInterfaceBlock* uib);
};

struct ChunkSamplerInterfaceBlock {
    static bool unflatten(filaflat::Unflattener& unflattener, SamplerInterfaceBlock* sib);
};

} // namespace filamat
#endif
