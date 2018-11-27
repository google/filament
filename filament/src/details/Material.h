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

#include <private/filament/Variant.h>

#include <filaflat/ShaderBuilder.h>

#include <utils/compiler.h>


namespace filaflat {
    class MaterialParser;
}

namespace filament {
namespace details {

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
    FMaterialInstance* createInstance() const noexcept;

    bool hasParameter(const char* name) const noexcept;

    FMaterialInstance const* getDefaultInstance() const noexcept { return &mDefaultInstance; }
    FMaterialInstance* getDefaultInstance() noexcept { return &mDefaultInstance; }

    FEngine& getEngine() const noexcept  { return mEngine; }

    Handle<HwProgram> getProgramSlow(uint8_t variantKey) const noexcept;
    Handle<HwProgram> getProgram(uint8_t variantKey) const noexcept {

        // filterVariant() has already been applied in generateCommands(), shouldn't be needed here
        assert( variantKey == Variant::filterVariant(variantKey, isVariantLit()) );

        Handle<HwProgram> const entry = mCachedPrograms[variantKey];
        return UTILS_LIKELY(entry) ? entry : getProgramSlow(variantKey);
    }

    bool isVariantLit() const noexcept { return mIsVariantLit; }

    const utils::CString& getName() const noexcept { return mName; }
    Driver::RasterState getRasterState() const noexcept  { return mRasterState; }
    uint32_t getId() const noexcept { return mMaterialId; }

    Shading getShading() const noexcept { return mShading; }
    Interpolation getInterpolation() const noexcept { return mInterpolation; }
    BlendingMode getBlendingMode() const noexcept { return mBlendingMode; }
    BlendingMode getRenderBlendingMode() const noexcept { return mRenderBlendingMode; }
    VertexDomain getVertexDomain() const noexcept { return mVertexDomain; }
    CullingMode getCullingMode() const noexcept { return mCullingMode; }
    TransparencyMode getTransparencyMode() const noexcept { return mTransparencyMode; }
    bool isColorWriteEnabled() const noexcept { return mRasterState.colorWrite; }
    bool isDepthWriteEnabled() const noexcept { return mRasterState.depthWrite; }
    bool isDepthCullingEnabled() const noexcept {
        return mRasterState.depthFunc != Driver::RasterState::DepthFunc::A;
    }
    bool isDoubleSided() const noexcept { return mDoubleSided; }
    float getMaskThreshold() const noexcept { return mMaskThreshold; }
    bool hasShadowMultiplier() const noexcept { return mHasShadowMultiplier; }
    AttributeBitset getRequiredAttributes() const noexcept { return mRequiredAttributes; }

    size_t getParameterCount() const noexcept {
        return mUniformInterfaceBlock.getUniformInfoList().size() +
                mSamplerInterfaceBlock.getSamplerInfoList().size();
    }
    size_t getParameters(ParameterInfo* parameters, size_t count) const noexcept;

    uint32_t generateMaterialInstanceId() const noexcept { return mMaterialInstanceId++; }

private:
    // try to order by frequency of use
    mutable std::array<Handle<HwProgram>, VARIANT_COUNT> mCachedPrograms;

    Driver::RasterState mRasterState;
    BlendingMode mRenderBlendingMode;
    TransparencyMode mTransparencyMode;
    bool mIsVariantLit;
    Shading mShading;

    BlendingMode mBlendingMode;
    Interpolation mInterpolation;
    VertexDomain mVertexDomain;
    CullingMode mCullingMode;
    AttributeBitset mRequiredAttributes;
    float mMaskThreshold;
    bool mDoubleSided;
    bool mHasShadowMultiplier = false;
    bool mHasCustomDepthShader = false;
    bool mIsDefaultMaterial = false;

    FMaterialInstance mDefaultInstance;
    SamplerInterfaceBlock mSamplerInterfaceBlock;
    UniformInterfaceBlock mUniformInterfaceBlock;
    SamplerBindingMap mSamplerBindings;

    utils::CString mName;
    FEngine& mEngine;
    const uint32_t mMaterialId;
    mutable uint32_t mMaterialInstanceId = 0;
    filaflat::MaterialParser* mMaterialParser = nullptr;
};


FILAMENT_UPCAST(Material)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_MATERIAL_H
