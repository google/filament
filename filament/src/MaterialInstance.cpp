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

#include "details/MaterialInstance.h"

#include "RenderPass.h"

#include "details/Engine.h"
#include "details/Material.h"
#include "details/Texture.h"

#include <string.h>

using namespace math;

namespace filament {

using namespace driver;

namespace details {

FMaterialInstance::FMaterialInstance() noexcept = default;

FMaterialInstance::FMaterialInstance(FEngine& engine, FMaterial const* material) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    mMaterial = material;
    mMaterialSortingKey = RenderPass::makeMaterialSortingKey(
            material->getId(), material->generateMaterialInstanceId());

    if (!material->getUniformInterfaceBlock().isEmpty()) {
        const UniformBuffer& defaultUniforms = upcast(material)->getDefaultInstance()->mUniforms;
        mUniforms = UniformBuffer(upcast(material)->getUniformInterfaceBlock());
        ::memcpy(const_cast<void*>(mUniforms.getBuffer()), defaultUniforms.getBuffer(), mUniforms.getSize());
        mUbHandle = driver.createUniformBuffer(mUniforms.getSize(), driver::BufferUsage::DYNAMIC);
    }

    if (!material->getSamplerInterfaceBlock().isEmpty()) {
        mSamplers = SamplerBuffer(material->getDefaultInstance()->getSamplerBuffer());
        mSbHandle = driver.createSamplerBuffer(mSamplers.getSize());
    }

    if (material->getBlendingMode() == BlendingMode::MASKED) {
        static_cast<MaterialInstance*>(this)->setParameter(
                "maskThreshold", material->getMaskThreshold());
    }
}

// This version is used to initialize the default material instance
void FMaterialInstance::initDefaultInstance(FEngine& engine, FMaterial const* material) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    mMaterial = material;
    mMaterialSortingKey = RenderPass::makeMaterialSortingKey(
            material->getId(), material->generateMaterialInstanceId());

    if (!material->getUniformInterfaceBlock().isEmpty()) {
        mUniforms = UniformBuffer(material->getUniformInterfaceBlock());
        mUbHandle = driver.createUniformBuffer(mUniforms.getSize(), driver::BufferUsage::STATIC);
    }

    if (!material->getSamplerInterfaceBlock().isEmpty()) {
        mSamplers = SamplerBuffer(material->getSamplerInterfaceBlock());
        mSbHandle = driver.createSamplerBuffer(mSamplers.getSize());
    }

    if (material->getBlendingMode() == BlendingMode::MASKED) {
        static_cast<MaterialInstance*>(this)->setParameter(
                "maskThreshold", material->getMaskThreshold());
    }
}

FMaterialInstance::~FMaterialInstance() noexcept = default;

void FMaterialInstance::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyUniformBuffer(mUbHandle);
    driver.destroySamplerBuffer(mSbHandle);
}

void FMaterialInstance::commitSlow(FEngine& engine) const {
    // update uniforms if needed
    FEngine::DriverApi& driver = engine.getDriverApi();
    if (mUniforms.isDirty()) {
        driver.updateUniformBuffer(mUbHandle, mUniforms.toBufferDescriptor(driver));
        mUniforms.clean();
    }
    if (mSamplers.isDirty()) {
        driver.updateSamplerBuffer(mSbHandle, SamplerBuffer(mSamplers));
        mSamplers.clean();
    }
}

template <typename T>
inline void FMaterialInstance::setParameter(const char* name, T value) noexcept {
    mUniforms.setUniform<T>(mMaterial->getUniformInterfaceBlock(), name, 0, value);
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
    mSamplers.setSampler(mMaterial->getSamplerInterfaceBlock(), name, 0,
            { upcast(texture)->getHwHandle(), sampler.getSamplerParams() });
}

} // namespace details

using namespace details;

Material const* MaterialInstance::getMaterial() const noexcept {
    return upcast(this)->mMaterial;
}

template <typename T>
void MaterialInstance::setParameter(const char* name, T value) noexcept {
    upcast(this)->setParameter<T>(name, value);
}

// explicit template instantiation of our supported types
template UTILS_PUBLIC void MaterialInstance::setParameter<bool>    (const char* name, bool     v);
template UTILS_PUBLIC void MaterialInstance::setParameter<float>   (const char* name, float    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<int32_t> (const char* name, int32_t  v);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint32_t>(const char* name, uint32_t v);
template UTILS_PUBLIC void MaterialInstance::setParameter<bool2>   (const char* name, bool2    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<bool3>   (const char* name, bool3    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<bool4>   (const char* name, bool4    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<int2>    (const char* name, int2     v);
template UTILS_PUBLIC void MaterialInstance::setParameter<int3>    (const char* name, int3     v);
template UTILS_PUBLIC void MaterialInstance::setParameter<int4>    (const char* name, int4     v);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint2>   (const char* name, uint2    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint3>   (const char* name, uint3    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint4>   (const char* name, uint4    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<float2>  (const char* name, float2   v);
template UTILS_PUBLIC void MaterialInstance::setParameter<float3>  (const char* name, float3   v);
template UTILS_PUBLIC void MaterialInstance::setParameter<float4>  (const char* name, float4   v);
template UTILS_PUBLIC void MaterialInstance::setParameter<mat3f>   (const char* name, mat3f    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<mat4f>   (const char* name, mat4f    v);

template <typename T>
void MaterialInstance::setParameter(const char* name, const T* value, size_t count) noexcept {
    upcast(this)->setParameter<T>(name, value, count);
}

// explicit template instantiation of our supported types
template UTILS_PUBLIC void MaterialInstance::setParameter<bool>    (const char* name, const bool     *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<float>   (const char* name, const float    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<int32_t> (const char* name, const int32_t  *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint32_t>(const char* name, const uint32_t *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<bool2>   (const char* name, const bool2    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<bool3>   (const char* name, const bool3    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<bool4>   (const char* name, const bool4    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<int2>    (const char* name, const int2     *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<int3>    (const char* name, const int3     *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<int4>    (const char* name, const int4     *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint2>   (const char* name, const uint2    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint3>   (const char* name, const uint3    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint4>   (const char* name, const uint4    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<float2>  (const char* name, const float2   *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<float3>  (const char* name, const float3   *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<float4>  (const char* name, const float4   *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<mat3f>   (const char* name, const mat3f    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<mat4f>   (const char* name, const mat4f    *v, size_t c);

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

} // namespace filament
