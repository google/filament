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

#ifndef TNT_FILAMENT_DETAILS_MATERIALINSTANCE_H
#define TNT_FILAMENT_DETAILS_MATERIALINSTANCE_H

#include "downcast.h"

#include "UniformBuffer.h"

#include "ds/DescriptorSet.h"

#include "details/Engine.h"

#include "private/backend/DriverApi.h"

#include <filament/MaterialInstance.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/BitmaskEnum.h>
#include <utils/bitset.h>
#include <utils/CString.h>

#include <tsl/robin_map.h>

#include <algorithm>
#include <limits>
#include <mutex>
#include <string_view>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class FMaterial;
class FTexture;

class FMaterialInstance : public MaterialInstance {
public:
    FMaterialInstance(FEngine& engine, FMaterial const* material,
                      const char* name) noexcept;
    FMaterialInstance(FEngine& engine, FMaterialInstance const* other, const char* name);
    FMaterialInstance(const FMaterialInstance& rhs) = delete;
    FMaterialInstance& operator=(const FMaterialInstance& rhs) = delete;

    static FMaterialInstance* duplicate(FMaterialInstance const* other, const char* name) noexcept;

    ~FMaterialInstance() noexcept;

    void terminate(FEngine& engine);

    void commitStreamUniformAssociations(FEngine::DriverApi& driver);
    
    void commit(FEngine::DriverApi& driver) const;

    void use(FEngine::DriverApi& driver) const;

    FMaterial const* getMaterial() const noexcept { return mMaterial; }

    uint64_t getSortingKey() const noexcept { return mMaterialSortingKey; }

    UniformBuffer const& getUniformBuffer() const noexcept { return mUniforms; }

    void setScissor(uint32_t const left, uint32_t const bottom, uint32_t const width, uint32_t const height) noexcept {
        constexpr uint32_t maxvalu = std::numeric_limits<int32_t>::max();
        mScissorRect = { int32_t(left), int32_t(bottom),
                std::min(width, maxvalu), std::min(height, maxvalu) };
        mHasScissor = true;
    }

    void unsetScissor() noexcept {
        constexpr uint32_t maxvalu = std::numeric_limits<int32_t>::max();
        mScissorRect = { 0, 0, maxvalu, maxvalu };
        mHasScissor = false;
    }

    backend::Viewport const& getScissor() const noexcept { return mScissorRect; }

    bool hasScissor() const noexcept { return mHasScissor; }

    backend::CullingMode getCullingMode() const noexcept { return mCulling; }

    backend::CullingMode getShadowCullingMode() const noexcept { return mShadowCulling; }

    bool isColorWriteEnabled() const noexcept { return mColorWrite; }

    bool isDepthWriteEnabled() const noexcept { return mDepthWrite; }

    bool isStencilWriteEnabled() const noexcept { return mStencilState.stencilWrite; }

    backend::StencilState getStencilState() const noexcept { return mStencilState; }

    TransparencyMode getTransparencyMode() const noexcept { return mTransparencyMode; }

    backend::RasterState::DepthFunc getDepthFunc() const noexcept { return mDepthFunc; }

    void setDepthFunc(backend::RasterState::DepthFunc const depthFunc) noexcept {
        mDepthFunc = depthFunc;
    }

    void setPolygonOffset(float const scale, float const constant) noexcept {
        // handle reversed Z
        mPolygonOffset = { -scale, -constant };
    }

    backend::PolygonOffset getPolygonOffset() const noexcept { return mPolygonOffset; }

    void setMaskThreshold(float threshold) noexcept;

    float getMaskThreshold() const noexcept;

    void setSpecularAntiAliasingVariance(float variance) noexcept;

    float getSpecularAntiAliasingVariance() const noexcept;

    void setSpecularAntiAliasingThreshold(float threshold) noexcept;

    float getSpecularAntiAliasingThreshold() const noexcept;

    void setDoubleSided(bool doubleSided) noexcept;

    bool isDoubleSided() const noexcept;

    void setTransparencyMode(TransparencyMode mode) noexcept;

    void setCullingMode(CullingMode const culling) noexcept {
        mCulling = culling;
        mShadowCulling = culling;
    }

    void setCullingMode(CullingMode const color, CullingMode const shadow) noexcept {
        mCulling = color;
        mShadowCulling = shadow;
    }

    void setColorWrite(bool const enable) noexcept { mColorWrite = enable; }

    void setDepthWrite(bool const enable) noexcept { mDepthWrite = enable; }

    void setStencilWrite(bool const enable) noexcept { mStencilState.stencilWrite = enable; }

    void setDepthCulling(bool enable) noexcept;

    bool isDepthCullingEnabled() const noexcept;

    void setStencilCompareFunction(StencilCompareFunc const func, StencilFace const face) noexcept {
        if (any(face & StencilFace::FRONT)) {
            mStencilState.front.stencilFunc = func;
        }
        if (any(face & StencilFace::BACK)) {
            mStencilState.back.stencilFunc = func;
        }
    }

    void setStencilOpStencilFail(StencilOperation const op, StencilFace const face) noexcept {
        if (any(face & StencilFace::FRONT)) {
            mStencilState.front.stencilOpStencilFail = op;
        }
        if (any(face & StencilFace::BACK)) {
            mStencilState.back.stencilOpStencilFail = op;
        }
    }

    void setStencilOpDepthFail(StencilOperation const op, StencilFace const face) noexcept {
        if (any(face & StencilFace::FRONT)) {
            mStencilState.front.stencilOpDepthFail = op;
        }
        if (any(face & StencilFace::BACK)) {
            mStencilState.back.stencilOpDepthFail = op;
        }
    }

    void setStencilOpDepthStencilPass(StencilOperation const op, StencilFace const face) noexcept {
        if (any(face & StencilFace::FRONT)) {
            mStencilState.front.stencilOpDepthStencilPass = op;
        }
        if (any(face & StencilFace::BACK)) {
            mStencilState.back.stencilOpDepthStencilPass = op;
        }
    }

    void setStencilReferenceValue(uint8_t const value, StencilFace const face) noexcept {
        if (any(face & StencilFace::FRONT)) {
            mStencilState.front.ref = value;
        }
        if (any(face & StencilFace::BACK)) {
            mStencilState.back.ref = value;
        }
    }

    void setStencilReadMask(uint8_t const readMask, StencilFace const face) noexcept {
        if (any(face & StencilFace::FRONT)) {
            mStencilState.front.readMask = readMask;
        }
        if (any(face & StencilFace::BACK)) {
            mStencilState.back.readMask = readMask;
        }
    }

    void setStencilWriteMask(uint8_t const writeMask, StencilFace const face) noexcept {
        if (any(face & StencilFace::FRONT)) {
            mStencilState.front.writeMask = writeMask;
        }
        if (any(face & StencilFace::BACK)) {
            mStencilState.back.writeMask = writeMask;
        }
    }

    void setDefaultInstance(bool const value) noexcept {
        mIsDefaultInstance = value;
    }

    bool isDefaultInstance() const noexcept {
        return mIsDefaultInstance;
    }

    // Called by the engine to ensure that unset samplers are initialized with placedholders.
    void fixMissingSamplers() const;

    const char* getName() const noexcept;

    void setParameter(std::string_view name,
            backend::Handle<backend::HwTexture> texture, backend::SamplerParams params);

    using MaterialInstance::setParameter;

private:
    friend class FMaterial;
    friend class MaterialInstance;

    template<size_t Size>
    void setParameterUntypedImpl(std::string_view name, const void* value);

    template<size_t Size>
    void setParameterUntypedImpl(std::string_view name, const void* value, size_t count);

    template<typename T>
    void setParameterImpl(std::string_view name, T const& value);

    template<typename T>
    void setParameterImpl(std::string_view name, const T* value, size_t count);

    void setParameterImpl(std::string_view name,
            FTexture const* texture, TextureSampler const& sampler);

    template<typename T>
    T getParameterImpl(std::string_view name) const;

    // keep these grouped, they're accessed together in the render-loop
    FMaterial const* mMaterial = nullptr;

    struct TextureParameter {
        FTexture const* texture;
        backend::SamplerParams params;
    };

    backend::Handle<backend::HwBufferObject> mUbHandle;
    tsl::robin_map<backend::descriptor_binding_t, TextureParameter> mTextureParameters;
    mutable DescriptorSet mDescriptorSet;
    UniformBuffer mUniforms;

    backend::PolygonOffset mPolygonOffset{};
    backend::StencilState mStencilState{};

    float mMaskThreshold = 0.0f;
    float mSpecularAntiAliasingVariance = 0.0f;
    float mSpecularAntiAliasingThreshold = 0.0f;

    backend::CullingMode mCulling : 2;
    backend::CullingMode mShadowCulling : 2;
    backend::RasterState::DepthFunc mDepthFunc : 3;

    bool mColorWrite : 1;
    bool mDepthWrite : 1;
    bool mHasScissor : 1;
    bool mIsDoubleSided : 1;
    bool mIsDefaultInstance : 1;
    TransparencyMode mTransparencyMode : 2;

    uint64_t mMaterialSortingKey = 0;

    // Scissor rectangle is specified as: Left Bottom Width Height.
    backend::Viewport mScissorRect = { 0, 0,
            uint32_t(std::numeric_limits<int32_t>::max()),
            uint32_t(std::numeric_limits<int32_t>::max())
    };

    utils::CString mName;
    mutable utils::bitset64 mMissingSamplerDescriptors{};
    mutable std::once_flag mMissingSamplersFlag;
};

FILAMENT_DOWNCAST(MaterialInstance)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_MATERIALINSTANCE_H
