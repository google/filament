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

#include <filament/EngineEnums.h>
#include <filament/MaterialEnums.h>

#include <filament/driver/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/CString.h>

#include <inttypes.h>

namespace filament {
class UniformInterfaceBlock;
class SamplerInterfaceBlock;
class SamplerBindingMap;
}

namespace filaflat {

class ChunkContainer;
class ShaderBuilder;

struct MaterialParserDetails;

class UTILS_PUBLIC MaterialParser {
public:
    MaterialParser(filament::driver::Backend backend, const void* data, size_t size);
    ~MaterialParser();

    MaterialParser(MaterialParser const& rhs) noexcept = delete;
    MaterialParser& operator = (MaterialParser const& rhs) noexcept  = delete;

    bool parse() noexcept;
    bool isShadingMaterial() const noexcept;
    bool isPostProcessMaterial() const noexcept;

    // Accessors
    bool getName(utils::CString*) const noexcept;
    bool getUIB(filament::UniformInterfaceBlock* uib) const noexcept;
    bool getSIB(filament::SamplerInterfaceBlock* sib) const noexcept;
    bool getSamplerBindingMap(filament::SamplerBindingMap*) const noexcept;
    bool getShaderModels(uint32_t* value) const noexcept;

    bool getDepthWriteSet(bool* value) const noexcept;
    bool getDepthWrite(bool* value) const noexcept;
    bool getDoubleSidedSet(bool* value) const noexcept;
    bool getDoubleSided(bool* value) const noexcept;
    bool getCullingMode(filament::driver::CullingMode* value) const noexcept;
    bool getTransparencyMode(filament::TransparencyMode* value) const noexcept;
    bool getColorWrite(bool* value) const noexcept;
    bool getDepthTest(bool* value) const noexcept;
    bool getInterpolation(filament::Interpolation* value) const noexcept;
    bool getVertexDomain(filament::VertexDomain* value) const noexcept;

    bool getShading(filament::Shading*) const noexcept;
    bool getBlendingMode(filament::BlendingMode*) const noexcept;
    bool getMaskThreshold(float*) const noexcept;
    bool hasShadowMultiplier(bool*) const noexcept;
    bool getRequiredAttributes(filament::AttributeBitset*) const noexcept;
    bool hasCustomDepthShader(bool* value) const noexcept;

    bool getShader(
            filament::driver::ShaderModel shaderModel, uint8_t variant,
            filament::driver::ShaderType st,
            ShaderBuilder& shader) noexcept;

protected:
    ChunkContainer& getChunkContainer() noexcept;
    ChunkContainer const& getChunkContainer() const noexcept;
    MaterialParserDetails* mImpl = nullptr;
};

} // namespace filamat
#endif
