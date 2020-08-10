/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_MATERIAL_H
#define TNT_FILAMENT_DETAILS_MATERIAL_H

#include "upcast.h"

#include "details/MaterialInstance.h"

#include <filament/Material.h>

#include <private/filament/SamplerBindingMap.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/Variant.h>

#include <filaflat/ShaderBuilder.h>

#include <utils/compiler.h>

#include <atomic>

namespace filament {

class MaterialParser;

class  FEngine;

class FMaterial : public Material {
public:
    FMaterial(FEngine& engine, const Material::Builder& builder);
    ~FMaterial() noexcept;

    class DefaultMaterialBuilder : public Material::Builder {
    public:
        DefaultMaterialBuilder();
    };


    void terminate(FEngine& engine);

    // return the uniform interface block for this material
    const UniformInterfaceBlock& getUniformInterfaceBlock() const noexcept {
        return mUniformInterfaceBlock;
    }

    // return the uniform interface block for this material
    const SamplerInterfaceBlock& getSamplerInterfaceBlock() const noexcept {
        return mSamplerInterfaceBlock;
    }

    // Create an instance of this material
    FMaterialInstance* createInstance(const char* name) const noexcept;

    bool hasParameter(const char* name) const noexcept;

    bool isSampler(const char* name) const noexcept;

    UniformInterfaceBlock::UniformInfo const* reflect(utils::StaticString const& name) const noexcept;

    FMaterialInstance const* getDefaultInstance() const noexcept { return &mDefaultInstance; }
    FMaterialInstance* getDefaultInstance() noexcept { return &mDefaultInstance; }

    FEngine& getEngine() const noexcept  { return mEngine; }

    backend::Handle<backend::HwProgram> getProgram(uint8_t variantKey) const noexcept {
#if FILAMENT_ENABLE_MATDBG
        if (UTILS_UNLIKELY(mPendingEdits.load())) {
            const_cast<FMaterial*>(this)->applyPendingEdits();
        }
#endif
        backend::Handle<backend::HwProgram> const entry = mCachedPrograms[variantKey];
        return UTILS_LIKELY(entry) ? entry : getProgramSlow(variantKey);
    }
    backend::Program getProgramBuilderWithVariants(uint8_t variantKey, uint8_t vertexVariantKey,
            uint8_t fragmentVariantKey) const noexcept;
    backend::Handle<backend::HwProgram> createAndCacheProgram(backend::Program&& p,
            uint8_t variantKey) const noexcept;

    bool isVariantLit() const noexcept { return mIsVariantLit; }

    const utils::CString& getName() const noexcept { return mName; }
    backend::RasterState getRasterState() const noexcept  { return mRasterState; }
    uint32_t getId() const noexcept { return mMaterialId; }

    Shading getShading() const noexcept { return mShading; }
    Interpolation getInterpolation() const noexcept { return mInterpolation; }
    BlendingMode getBlendingMode() const noexcept { return mBlendingMode; }
    BlendingMode getRenderBlendingMode() const noexcept { return mRenderBlendingMode; }
    VertexDomain getVertexDomain() const noexcept { return mVertexDomain; }
    MaterialDomain getMaterialDomain() const noexcept { return mMaterialDomain; }
    CullingMode getCullingMode() const noexcept { return mCullingMode; }
    TransparencyMode getTransparencyMode() const noexcept { return mTransparencyMode; }
    bool isColorWriteEnabled() const noexcept { return mRasterState.colorWrite; }
    bool isDepthWriteEnabled() const noexcept { return mRasterState.depthWrite; }
    bool isDepthCullingEnabled() const noexcept {
        return mRasterState.depthFunc != backend::RasterState::DepthFunc::A;
    }
    bool isDoubleSided() const noexcept { return mDoubleSided; }
    bool hasDoubleSidedCapability() const noexcept { return mDoubleSidedCapability; }
    float getMaskThreshold() const noexcept { return mMaskThreshold; }
    bool hasShadowMultiplier() const noexcept { return mHasShadowMultiplier; }
    AttributeBitset getRequiredAttributes() const noexcept { return mRequiredAttributes; }
    RefractionMode getRefractionMode() const noexcept { return mRefractionMode; }
    RefractionType getRefractionType() const noexcept { return mRefractionType; }

    bool hasSpecularAntiAliasing() const noexcept { return mSpecularAntiAliasing; }
    float getSpecularAntiAliasingVariance() const noexcept { return mSpecularAntiAliasingVariance; }
    float getSpecularAntiAliasingThreshold() const noexcept { return mSpecularAntiAliasingThreshold; }

    bool hasMaterialProperty(Property property) const noexcept {
        return bool(mMaterialProperties & uint64_t(property));
    }

    size_t getParameterCount() const noexcept {
        return mUniformInterfaceBlock.getUniformInfoList().size() +
                mSamplerInterfaceBlock.getSamplerInfoList().size();
    }
    size_t getParameters(ParameterInfo* parameters, size_t count) const noexcept;

    uint32_t generateMaterialInstanceId() const noexcept { return mMaterialInstanceId++; }

    void applyPendingEdits() noexcept;

    void destroyPrograms(FEngine& engine);

    /**
     * Callback handlers for the debug server, potentially called from any thread. The userdata
     * argument has the same value that was passed to DebugServer::addMaterial(), which should
     * be an instance of the public-facing Material.
     * @{
     */

    /** Replaces the material package. */
    static void onEditCallback(void* userdata, const utils::CString& name, const void* packageData,
            size_t packageSize);

    /** Queries the program cache to check which variants are resident. */
    static void onQueryCallback(void* userdata, uint64_t* variants);

    /** @}*/

    static MaterialParser* createParser(backend::Backend backend, const void* data, size_t size);

private:
    backend::Handle<backend::HwProgram> getProgramSlow(uint8_t variantKey) const noexcept;
    backend::Handle<backend::HwProgram> getSurfaceProgramSlow(uint8_t variantKey) const noexcept;
    backend::Handle<backend::HwProgram> getPostProcessProgramSlow(uint8_t variantKey) const noexcept;

    // try to order by frequency of use
    mutable std::array<backend::Handle<backend::HwProgram>, VARIANT_COUNT> mCachedPrograms;

    backend::RasterState mRasterState;
    BlendingMode mRenderBlendingMode = BlendingMode::OPAQUE;
    TransparencyMode mTransparencyMode = TransparencyMode::DEFAULT;
    bool mIsVariantLit = false;
    Shading mShading = Shading::UNLIT;

    BlendingMode mBlendingMode = BlendingMode::OPAQUE;
    Interpolation mInterpolation = Interpolation::SMOOTH;
    VertexDomain mVertexDomain = VertexDomain::OBJECT;
    MaterialDomain mMaterialDomain = MaterialDomain::SURFACE;
    CullingMode mCullingMode = CullingMode::NONE;
    AttributeBitset mRequiredAttributes;
    RefractionMode mRefractionMode = RefractionMode::NONE;
    RefractionType mRefractionType = RefractionType::SOLID;
    uint64_t mMaterialProperties = 0;

    float mMaskThreshold = 0.4f;
    float mSpecularAntiAliasingVariance = 0.0f;
    float mSpecularAntiAliasingThreshold = 0.0f;

    bool mDoubleSided = false;
    bool mDoubleSidedCapability = false;
    bool mHasShadowMultiplier = false;
    bool mHasCustomDepthShader = false;
    bool mIsDefaultMaterial = false;
    bool mSpecularAntiAliasing = false;

    FMaterialInstance mDefaultInstance;
    SamplerInterfaceBlock mSamplerInterfaceBlock;
    UniformInterfaceBlock mUniformInterfaceBlock;
    SamplerBindingMap mSamplerBindings;

    utils::CString mName;
    FEngine& mEngine;
    const uint32_t mMaterialId;
    mutable uint32_t mMaterialInstanceId = 0;
    MaterialParser* mMaterialParser = nullptr;
    std::atomic<MaterialParser*> mPendingEdits = {};
};


FILAMENT_UPCAST(Material)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_MATERIAL_H
