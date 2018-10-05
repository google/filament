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
#include "details/Engine.h"
#include "driver/DriverApi.h"
#include "driver/Handle.h"
#include "driver/UniformBuffer.h"

#include <utils/compiler.h>

#include <filament/MaterialInstance.h>

namespace filament {
namespace details {

class FMaterial;

class FMaterialInstance : public MaterialInstance {
public:
    FMaterialInstance(FEngine& engine, FMaterial const* material);
    FMaterialInstance(const FMaterialInstance& rhs) = delete;
    FMaterialInstance& operator=(const FMaterialInstance& rhs) = delete;

    ~FMaterialInstance() noexcept;

    void terminate(FEngine& engine);

    void commit(FEngine& engine) const {
        if (UTILS_UNLIKELY(mUniforms.isDirty() || mSamplers.isDirty())) {
            commitSlow(engine);
        }
    }

    void use(FEngine::DriverApi& driver) const {
        if (mUbHandle) {
            driver.bindUniformBuffer(BindingPoints::PER_MATERIAL_INSTANCE, mUbHandle);
        }
        if (mSbHandle) {
            driver.bindSamplers(BindingPoints::PER_MATERIAL_INSTANCE, mSbHandle);
        }
        driver.setViewportScissor(
                mScissorRect[0], mScissorRect[1],
                uint32_t(mScissorRect[2]), uint32_t(mScissorRect[3]));
    }

    template <typename T>
    void setParameter(const char* name, T value) noexcept;

    template <typename T>
    void setParameter(const char* name, const T* value, size_t count) noexcept;

    void setParameter(const char* name,
            Texture const* texture, TextureSampler const& sampler) noexcept;

    FMaterial const* getMaterial() const noexcept { return mMaterial; }

    uint64_t getSortingKey() const noexcept { return mMaterialSortingKey; }

    SamplerBuffer const& getSamplerBuffer() const noexcept { return mSamplers; }

    void setScissor(int32_t left, int32_t bottom, uint32_t width, uint32_t height) noexcept {
        mScissorRect[0] = left;
        mScissorRect[1] = bottom;
        mScissorRect[2] = (int32_t)std::min(width,  (uint32_t)std::numeric_limits<int32_t>::max());
        mScissorRect[3] = (int32_t)std::min(height, (uint32_t)std::numeric_limits<int32_t>::max());
    }

    void unsetScissor() noexcept {
        mScissorRect[0] = mScissorRect[1] = 0;
        mScissorRect[2] = mScissorRect[3] = std::numeric_limits<int32_t>::max();
    }

private:
    friend class FMaterial;
    friend class MaterialInstance;

    FMaterialInstance() noexcept;
    void initDefaultInstance(FEngine& engine, FMaterial const* material);

    void commitSlow(FEngine& engine) const;

    // keep these grouped, they're accessed together in the render-loop
    FMaterial const* mMaterial = nullptr;
    Handle<HwUniformBuffer> mUbHandle;
    Handle<HwSamplerBuffer> mSbHandle;

    UniformBuffer mUniforms;
    SamplerBuffer mSamplers;

    uint64_t mMaterialSortingKey = 0;

    // Scissor rectangle is specified as: Left Bottom Width Height.
    int32_t mScissorRect[4] = {
        0, 0, std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()
    };
};

FILAMENT_UPCAST(MaterialInstance)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_MATERIALINSTANCE_H
