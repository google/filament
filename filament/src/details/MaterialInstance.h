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

#include "upcast.h"
#include "UniformBuffer.h"
#include "details/Engine.h"

#include "private/backend/DriverApi.h"

#include <backend/Handle.h>

#include <math/scalar.h>

#include <utils/compiler.h>

#include <filament/MaterialInstance.h>

namespace filament {
namespace details {

class FMaterial;

class FMaterialInstance : public MaterialInstance {
public:
    FMaterialInstance(FEngine& engine, FMaterial const* material, const char* name);
    FMaterialInstance(const FMaterialInstance& rhs) = delete;
    FMaterialInstance& operator=(const FMaterialInstance& rhs) = delete;

    ~FMaterialInstance() noexcept;

    void terminate(FEngine& engine);

    void commit(FEngine::DriverApi& driver) const {
        if (UTILS_UNLIKELY(mUniforms.isDirty() || mSamplers.isDirty())) {
            commitSlow(driver);
        }
    }

    void use(FEngine::DriverApi& driver) const {
        if (mUbHandle) {
            driver.bindUniformBuffer(BindingPoints::PER_MATERIAL_INSTANCE, mUbHandle);
        }
        if (mSbHandle) {
            driver.bindSamplers(BindingPoints::PER_MATERIAL_INSTANCE, mSbHandle);
        }
    }

    template <typename T, typename = is_supported_parameter_t<T>>
    void setParameter(const char* name, T value) noexcept;

    template <typename T, typename = is_supported_parameter_t<T>>
    void setParameter(const char* name, const T* value, size_t count) noexcept;

    void setParameter(const char* name,
            Texture const* texture, TextureSampler const& sampler) noexcept;

    void setParameter(const char* name,
            backend::Handle<backend::HwTexture> texture, backend::SamplerParams params) noexcept;

    FMaterial const* getMaterial() const noexcept { return mMaterial; }

    uint64_t getSortingKey() const noexcept { return mMaterialSortingKey; }

    UniformBuffer const& getUniformBuffer() const noexcept { return mUniforms; }
    backend::SamplerGroup const& getSamplerGroup() const noexcept { return mSamplers; }

    void setScissor(int32_t left, int32_t bottom, uint32_t width, uint32_t height) noexcept {
        mScissorRect = { left, bottom,
                std::min(width, (uint32_t)std::numeric_limits<int32_t>::max()),
                std::min(height, (uint32_t)std::numeric_limits<int32_t>::max())
        };
    }

    void unsetScissor() noexcept {
        mScissorRect = { 0, 0,
                (uint32_t)std::numeric_limits<int32_t>::max(),
                (uint32_t)std::numeric_limits<int32_t>::max()
        };
    }

    backend::Viewport const& getScissor() const noexcept { return mScissorRect; }

    backend::CullingMode getCullingMode() const noexcept { return mCulling; }

    bool getColorWrite() const noexcept { return mColorWrite; }

    bool getDepthWrite() const noexcept { return mDepthWrite; }

    backend::RasterState::DepthFunc getDepthFunc() const noexcept { return mDepthFunc; }

    void setPolygonOffset(float scale, float constant) noexcept {
        mPolygonOffset = { scale, constant };
    }

    backend::PolygonOffset getPolygonOffset() const noexcept { return mPolygonOffset; }

    void setMaskThreshold(float threshold) noexcept {
        setParameter("_maskThreshold", math::saturate(threshold));
    }

    void setSpecularAntiAliasingVariance(float variance) noexcept {
        setParameter("_specularAntiAliasingVariance", math::saturate(variance));
    }

    void setSpecularAntiAliasingThreshold(float threshold) noexcept {
        setParameter("_specularAntiAliasingThreshold", math::saturate(threshold * threshold));
    }

    void setDoubleSided(bool doubleSided) noexcept;

    void setCullingMode(CullingMode culling) noexcept;

    void setColorWrite(bool enable) noexcept;

    void setDepthWrite(bool enable) noexcept;

    void setDepthCulling(bool enable) noexcept;

    const char* getName() const noexcept;

private:
    friend class FMaterial;
    friend class MaterialInstance;

    FMaterialInstance() noexcept;
    void initDefaultInstance(FEngine& engine, FMaterial const* material);
    void initialize(FMaterial const* material);

    void commitSlow(FEngine::DriverApi& driver) const;

    // keep these grouped, they're accessed together in the render-loop
    FMaterial const* mMaterial = nullptr;
    backend::Handle<backend::HwUniformBuffer> mUbHandle;
    backend::Handle<backend::HwSamplerGroup> mSbHandle;

    UniformBuffer mUniforms;
    backend::SamplerGroup mSamplers;
    backend::PolygonOffset mPolygonOffset;
    backend::CullingMode mCulling;
    bool mColorWrite;
    bool mDepthWrite;
    backend::RasterState::DepthFunc mDepthFunc;

    uint64_t mMaterialSortingKey = 0;

    // Scissor rectangle is specified as: Left Bottom Width Height.
    backend::Viewport mScissorRect = { 0, 0,
            (uint32_t)std::numeric_limits<int32_t>::max(),
            (uint32_t)std::numeric_limits<int32_t>::max()
    };

    utils::CString mName;
};

FILAMENT_UPCAST(MaterialInstance)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_MATERIALINSTANCE_H
