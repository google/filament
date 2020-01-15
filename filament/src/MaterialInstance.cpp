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

#include <filament/MaterialInstance.h>

#include <filament/TextureSampler.h>

#include "details/MaterialInstance.h"

#include "RenderPass.h"

#include "details/Engine.h"
#include "details/Material.h"
#include "details/Texture.h"

#include <utils/Log.h>

#include <string.h>

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

namespace details {

FMaterialInstance::FMaterialInstance() noexcept = default;

FMaterialInstance::FMaterialInstance(FEngine& engine, FMaterial const* material) {
    mMaterial = material;

    // We inherit the resolved culling mode rather than the builder-set culling mode.
    // This preserves the property whereby double-sidedness automatically disables culling.
    mCulling = mMaterial->getRasterState().culling;

    mMaterialSortingKey = RenderPass::makeMaterialSortingKey(
            material->getId(), material->generateMaterialInstanceId());

    FEngine::DriverApi& driver = engine.getDriverApi();

    if (!material->getUniformInterfaceBlock().isEmpty()) {
        mUniforms.setUniforms(material->getDefaultInstance()->getUniformBuffer());
        mUbHandle = driver.createUniformBuffer(mUniforms.getSize(), backend::BufferUsage::DYNAMIC);
    }

    if (!material->getSamplerInterfaceBlock().isEmpty()) {
        mSamplers.setSamplers(material->getDefaultInstance()->getSamplerGroup());
        mSbHandle = driver.createSamplerGroup(mSamplers.getSize());
    }

    initParameters(material);
}

// This version is used to initialize the default material instance
void FMaterialInstance::initDefaultInstance(FEngine& engine, FMaterial const* material) {
    mMaterial = material;

    // We inherit the resolved culling mode rather than the builder-set culling mode.
    // This preserves the property whereby double-sidedness automatically disables culling.
    mCulling = mMaterial->getRasterState().culling;

    mMaterialSortingKey = RenderPass::makeMaterialSortingKey(
            material->getId(), material->generateMaterialInstanceId());

    FEngine::DriverApi& driver = engine.getDriverApi();

    if (!material->getUniformInterfaceBlock().isEmpty()) {
        mUniforms = UniformBuffer(material->getUniformInterfaceBlock().getSize());
        mUbHandle = driver.createUniformBuffer(mUniforms.getSize(), backend::BufferUsage::STATIC);
    }

    if (!material->getSamplerInterfaceBlock().isEmpty()) {
        mSamplers = SamplerGroup(material->getSamplerInterfaceBlock().getSize());
        mSbHandle = driver.createSamplerGroup(mSamplers.getSize());
    }

    initParameters(material);
}

FMaterialInstance::~FMaterialInstance() noexcept = default;

void FMaterialInstance::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyUniformBuffer(mUbHandle);
    driver.destroySamplerGroup(mSbHandle);
}

void FMaterialInstance::initParameters(FMaterial const* material) {

    if (material->getBlendingMode() == BlendingMode::MASKED) {
        static_cast<MaterialInstance*>(this)->setParameter(
                "_maskThreshold", material->getMaskThreshold());
    }

    if (material->hasDoubleSidedCapability()) {
        static_cast<MaterialInstance*>(this)->setParameter(
                "_doubleSided", material->isDoubleSided());
    }

    if (material->hasSpecularAntiAliasing()) {
        static_cast<MaterialInstance*>(this)->setParameter(
                "_specularAntiAliasingVariance", material->getSpecularAntiAliasingVariance());
        static_cast<MaterialInstance*>(this)->setParameter(
                "_specularAntiAliasingThreshold", material->getSpecularAntiAliasingThreshold());
    }
}

void FMaterialInstance::commitSlow(DriverApi& driver) const {
    // update uniforms if needed
    if (mUniforms.isDirty()) {
        driver.loadUniformBuffer(mUbHandle, mUniforms.toBufferDescriptor(driver));
    }
    if (mSamplers.isDirty()) {
        driver.updateSamplerGroup(mSbHandle, std::move(mSamplers.toCommandStream()));
    }
}

template<typename T>
inline void FMaterialInstance::setParameter(const char* name, T value) noexcept {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getUniformOffset(name, 0);
    if (offset >= 0) {
        mUniforms.setUniform<T>(size_t(offset), value);  // handles specialization for mat3f
    }
}

template <typename T>
inline void FMaterialInstance::setParameter(const char* name, const T* value, size_t count) noexcept {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getUniformOffset(name, 0);
    if (offset >= 0) {
        mUniforms.setUniformArray<T>(size_t(offset), value, count);
    }
}

void FMaterialInstance::setParameter(const char* name,
        Texture const* texture, TextureSampler const& sampler) noexcept {
    setParameter(name, upcast(texture)->getHwHandle(), sampler.getSamplerParams());
}

void FMaterialInstance::setParameter(const char* name,
        backend::Handle<backend::HwTexture> texture, backend::SamplerParams params) noexcept {
    size_t index = mMaterial->getSamplerInterfaceBlock().getSamplerInfo(name)->offset;
    mSamplers.setSampler(index, { texture, params });
}

void FMaterialInstance::setDoubleSided(bool doubleSided) noexcept {
    if (!mMaterial->hasDoubleSidedCapability()) {
        slog.w << "Parent material does not have double-sided capability." << io::endl;
        return;
    }
    setParameter("_doubleSided", doubleSided);
    if (doubleSided) {
        setCullingMode(CullingMode::NONE);
    }
}

void FMaterialInstance::setCullingMode(CullingMode culling) noexcept {
    mCulling = culling;
}

// explicit template instantiation of our supported types
template UTILS_NOINLINE void FMaterialInstance::setParameter<bool>    (const char* name, bool     v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<float>   (const char* name, float    v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<int32_t> (const char* name, int32_t  v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<uint32_t>(const char* name, uint32_t v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<bool2>   (const char* name, bool2    v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<bool3>   (const char* name, bool3    v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<bool4>   (const char* name, bool4    v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<int2>    (const char* name, int2     v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<int3>    (const char* name, int3     v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<int4>    (const char* name, int4     v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<uint2>   (const char* name, uint2    v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<uint3>   (const char* name, uint3    v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<uint4>   (const char* name, uint4    v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<float2>  (const char* name, float2   v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<float3>  (const char* name, float3   v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<float4>  (const char* name, float4   v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<mat3f>   (const char* name, mat3f    v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<mat4f>   (const char* name, mat4f    v) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<bool>    (const char* name, const bool     *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<float>   (const char* name, const float    *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<int32_t> (const char* name, const int32_t  *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<uint32_t>(const char* name, const uint32_t *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<bool2>   (const char* name, const bool2    *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<bool3>   (const char* name, const bool3    *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<bool4>   (const char* name, const bool4    *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<int2>    (const char* name, const int2     *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<int3>    (const char* name, const int3     *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<int4>    (const char* name, const int4     *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<uint2>   (const char* name, const uint2    *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<uint3>   (const char* name, const uint3    *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<uint4>   (const char* name, const uint4    *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<float2>  (const char* name, const float2   *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<float3>  (const char* name, const float3   *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<float4>  (const char* name, const float4   *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<mat3f>   (const char* name, const mat3f    *v, size_t c) noexcept;
template UTILS_NOINLINE void FMaterialInstance::setParameter<mat4f>   (const char* name, const mat4f    *v, size_t c) noexcept;

} // namespace details

using namespace details;

Material const* MaterialInstance::getMaterial() const noexcept {
    return upcast(this)->mMaterial;
}

template <typename T>
void MaterialInstance::setParameter(const char* name, T value) noexcept {
    upcast(this)->setParameter<T>(name, value);
}

template <typename T>
void MaterialInstance::setParameter(const char* name, const T* value, size_t count) noexcept {
    upcast(this)->setParameter<T>(name, value, count);
}

// explicit template instantiation of our supported types
template UTILS_PUBLIC void MaterialInstance::setParameter<bool>    (const char* name, bool     v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float>   (const char* name, float    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int32_t> (const char* name, int32_t  v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint32_t>(const char* name, uint32_t v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<bool2>   (const char* name, bool2    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<bool3>   (const char* name, bool3    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<bool4>   (const char* name, bool4    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int2>    (const char* name, int2     v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int3>    (const char* name, int3     v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int4>    (const char* name, int4     v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint2>   (const char* name, uint2    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint3>   (const char* name, uint3    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint4>   (const char* name, uint4    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float2>  (const char* name, float2   v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float3>  (const char* name, float3   v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float4>  (const char* name, float4   v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<mat3f>   (const char* name, mat3f    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<mat4f>   (const char* name, mat4f    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<bool>    (const char* name, const bool     *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float>   (const char* name, const float    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int32_t> (const char* name, const int32_t  *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint32_t>(const char* name, const uint32_t *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<bool2>   (const char* name, const bool2    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<bool3>   (const char* name, const bool3    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<bool4>   (const char* name, const bool4    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int2>    (const char* name, const int2     *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int3>    (const char* name, const int3     *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int4>    (const char* name, const int4     *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint2>   (const char* name, const uint2    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint3>   (const char* name, const uint3    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint4>   (const char* name, const uint4    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float2>  (const char* name, const float2   *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float3>  (const char* name, const float3   *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float4>  (const char* name, const float4   *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<mat3f>   (const char* name, const mat3f    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<mat4f>   (const char* name, const mat4f    *v, size_t c) noexcept;

void MaterialInstance::setParameter(const char* name, Texture const* texture,
        TextureSampler const& sampler) noexcept {
    return upcast(this)->setParameter(name, texture, sampler);
}

void MaterialInstance::setParameter(const char* name, RgbType type, float3 color) noexcept {
    upcast(this)->setParameter<float3>(name, Color::toLinear(type, color));
}

void MaterialInstance::setParameter(const char* name, RgbaType type, float4 color) noexcept {
    upcast(this)->setParameter<float4>(name, Color::toLinear(type, color));
}

void MaterialInstance::setScissor(uint32_t left, uint32_t bottom, uint32_t width,
        uint32_t height) noexcept {
    upcast(this)->setScissor(left, bottom, width, height);
}

void MaterialInstance::unsetScissor() noexcept {
    upcast(this)->unsetScissor();
}

void MaterialInstance::setPolygonOffset(float scale, float constant) noexcept {
    upcast(this)->setPolygonOffset(scale, constant);
}

void MaterialInstance::setMaskThreshold(float threshold) noexcept {
    upcast(this)->setMaskThreshold(threshold);
}

void MaterialInstance::setSpecularAntiAliasingVariance(float variance) noexcept {
    upcast(this)->setSpecularAntiAliasingVariance(variance);
}

void MaterialInstance::setSpecularAntiAliasingThreshold(float threshold) noexcept {
    upcast(this)->setSpecularAntiAliasingThreshold(threshold);
}

void MaterialInstance::setDoubleSided(bool doubleSided) noexcept {
    upcast(this)->setDoubleSided(doubleSided);
}

void MaterialInstance::setCullingMode(CullingMode culling) noexcept {
    upcast(this)->setCullingMode(culling);
}

} // namespace filament
