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

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

//getParameter() functions, using non-inlined untyped calls for code size optimization, see setParameter() functions
UTILS_NOINLINE
const void* FMaterialInstance::getParameterUntypedImpl(const char* name) noexcept {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getUniformOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        return mUniforms.getUniformUntyped(size_t(offset));
    }
    return nullptr;
}

template<typename T>
UTILS_ALWAYS_INLINE
inline T FMaterialInstance::getParameterImpl(const char* name) noexcept {
    static_assert(!std::is_same_v<T, math::mat3f>);
    return *reinterpret_cast<T const*>(getParameterUntypedImpl(name));
}

//specialization for mat3f
template<>
inline mat3f FMaterialInstance::getParameterImpl(const char* name) noexcept {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getUniformOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        return mUniforms.getUniform<mat3f>(size_t(offset));
    }
}

template <typename T, typename>
T MaterialInstance::getParameter(const char* name) noexcept {
    return upcast(this)->getParameterImpl<T>(name);
}

// ------------------------------------------------------------------------------------------------

// This is the untyped/sized version of the setParameter: we end up here for e.g. vec4<int> and
// vec4<float>. This must not be inlined (this is the whole point).
template<size_t Size>
UTILS_NOINLINE
void FMaterialInstance::setParameterUntypedImpl(const char* name, const void* value) noexcept {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getUniformOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        mUniforms.setUniformUntyped<Size>(size_t(offset), value);  // handles specialization for mat3f
    }
}

// ------------------------------------------------------------------------------------------------

// this converts typed calls into the untyped-sized call.
template<typename T>
UTILS_ALWAYS_INLINE
inline void FMaterialInstance::setParameterImpl(const char* name, T const& value) noexcept {
    static_assert(!std::is_same_v<T, math::mat3f>);
    setParameterUntypedImpl<sizeof(T)>(name, &value);
}

// specialization for mat3f
template<>
inline void FMaterialInstance::setParameterImpl(const char* name, mat3f const& value) noexcept {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getUniformOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        mUniforms.setUniform(size_t(offset), value);
    }
}

// ------------------------------------------------------------------------------------------------

template <typename T, typename>
void MaterialInstance::setParameter(const char* name, T const& value) noexcept {
    upcast(this)->setParameterImpl(name, value);
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, bool const& v) noexcept {
    // this kills tail-call optimization
    MaterialInstance::setParameter(name, (uint32_t)v);
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, bool2 const& v) noexcept {
    // this kills tail-call optimization
    MaterialInstance::setParameter(name, uint2(v));
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, bool3 const& v) noexcept {
    // this kills tail-call optimization
    MaterialInstance::setParameter(name, uint3(v));
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, bool4 const& v) noexcept {
    // this kills tail-call optimization
    MaterialInstance::setParameter(name, uint4(v));
}

// explicit template instantiation of our supported types
template UTILS_PUBLIC void MaterialInstance::setParameter<float>   (const char* name, float const&    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int32_t> (const char* name, int32_t const&  v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint32_t>(const char* name, uint32_t const& v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int2>    (const char* name, int2 const&     v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int3>    (const char* name, int3 const&     v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int4>    (const char* name, int4 const&     v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint2>   (const char* name, uint2 const&    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint3>   (const char* name, uint3 const&    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint4>   (const char* name, uint4 const&    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float2>  (const char* name, float2 const&   v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float3>  (const char* name, float3 const&   v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float4>  (const char* name, float4 const&   v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<mat3f>   (const char* name, mat3f const&    v) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<mat4f>   (const char* name, mat4f const&    v) noexcept;

template UTILS_PUBLIC bool MaterialInstance::getParameter<bool>(const char *name) noexcept;
template UTILS_PUBLIC bool2 MaterialInstance::getParameter<bool2>(const char *name) noexcept;
template UTILS_PUBLIC bool3 MaterialInstance::getParameter<bool3>(const char *name) noexcept;
template UTILS_PUBLIC bool4 MaterialInstance::getParameter<bool4>(const char *name) noexcept;

template UTILS_PUBLIC float MaterialInstance::getParameter<float>(const char *name) noexcept;
template UTILS_PUBLIC float2 MaterialInstance::getParameter<float2>(const char *name) noexcept;
template UTILS_PUBLIC float3 MaterialInstance::getParameter<float3>(const char *name) noexcept;
template UTILS_PUBLIC float4 MaterialInstance::getParameter<float4>(const char *name) noexcept;

template UTILS_PUBLIC int32_t MaterialInstance::getParameter<int32_t>(const char *name) noexcept;
template UTILS_PUBLIC int2 MaterialInstance::getParameter<int2>(const char *name) noexcept;
template UTILS_PUBLIC int3 MaterialInstance::getParameter<int3>(const char *name) noexcept;
template UTILS_PUBLIC int4 MaterialInstance::getParameter<int4>(const char *name) noexcept;

template UTILS_PUBLIC uint32_t MaterialInstance::getParameter<uint32_t>(const char *name) noexcept;
template UTILS_PUBLIC uint2 MaterialInstance::getParameter<uint2>(const char *name) noexcept;
template UTILS_PUBLIC uint3 MaterialInstance::getParameter<uint3>(const char *name) noexcept;
template UTILS_PUBLIC uint4 MaterialInstance::getParameter<uint4>(const char *name) noexcept;
template UTILS_PUBLIC mat3f MaterialInstance::getParameter<mat3f>(const char *name) noexcept;
template UTILS_PUBLIC mat4f MaterialInstance::getParameter<mat4f>(const char *name) noexcept;

// ------------------------------------------------------------------------------------------------

template <typename T, typename>
void MaterialInstance::setParameter(const char* name, const T* value, size_t count) noexcept {
    upcast(this)->setParameterImpl(name, value, count);
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, const bool* v, size_t c) noexcept {
    auto* p = new uint32_t[c];
    std::copy_n(v, c, p);
    MaterialInstance::setParameter(name, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, const bool2* v, size_t c) noexcept {
    auto* p = new uint2[c];
    std::copy_n(v, c, p);
    MaterialInstance::setParameter(name, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, const bool3* v, size_t c) noexcept {
    auto* p = new uint3[c];
    std::copy_n(v, c, p);
    MaterialInstance::setParameter(name, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, const bool4* v, size_t c) noexcept {
    auto* p = new uint4[c];
    std::copy_n(v, c, p);
    MaterialInstance::setParameter(name, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter<mat3f>(const char* name, const mat3f* v, size_t c) noexcept {
    // pretend each mat3 is an array of 3 float3
    MaterialInstance::setParameter(name, reinterpret_cast<math::float3 const*>(v), c * 3);
}

template UTILS_PUBLIC void MaterialInstance::setParameter<float>   (const char* name, const float    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int32_t> (const char* name, const int32_t  *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint32_t>(const char* name, const uint32_t *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int2>    (const char* name, const int2     *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int3>    (const char* name, const int3     *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<int4>    (const char* name, const int4     *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint2>   (const char* name, const uint2    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint3>   (const char* name, const uint3    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<uint4>   (const char* name, const uint4    *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float2>  (const char* name, const float2   *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float3>  (const char* name, const float3   *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<float4>  (const char* name, const float4   *v, size_t c) noexcept;
template UTILS_PUBLIC void MaterialInstance::setParameter<mat4f>   (const char* name, const mat4f    *v, size_t c) noexcept;

// ------------------------------------------------------------------------------------------------

FMaterialInstance::FMaterialInstance() noexcept = default;

FMaterialInstance::FMaterialInstance(FEngine& engine, FMaterial const* material, const char* name) :
        mName(name) {
    FEngine::DriverApi& driver = engine.getDriverApi();

    if (!material->getUniformInterfaceBlock().isEmpty()) {
        mUniforms.setUniforms(material->getDefaultInstance()->getUniformBuffer());
        mUbHandle = driver.createUniformBuffer(mUniforms.getSize(), backend::BufferUsage::DYNAMIC);
    }

    if (!material->getSamplerInterfaceBlock().isEmpty()) {
        mSamplers.setSamplers(material->getDefaultInstance()->getSamplerGroup());
        mSbHandle = driver.createSamplerGroup(mSamplers.getSize());
    }

    initialize(material);
}

// This version is used to initialize the default material instance
void FMaterialInstance::initDefaultInstance(FEngine& engine, FMaterial const* material) {
    FEngine::DriverApi& driver = engine.getDriverApi();

    if (!material->getUniformInterfaceBlock().isEmpty()) {
        mUniforms = UniformBuffer(material->getUniformInterfaceBlock().getSize());
        mUbHandle = driver.createUniformBuffer(mUniforms.getSize(), backend::BufferUsage::STATIC);
    }

    if (!material->getSamplerInterfaceBlock().isEmpty()) {
        mSamplers = SamplerGroup(material->getSamplerInterfaceBlock().getSize());
        mSbHandle = driver.createSamplerGroup(mSamplers.getSize());
    }

    initialize(material);
}

FMaterialInstance::~FMaterialInstance() noexcept = default;

void FMaterialInstance::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyUniformBuffer(mUbHandle);
    driver.destroySamplerGroup(mSbHandle);
}

UTILS_NOINLINE
void FMaterialInstance::initialize(FMaterial const* material) {
    mMaterial = material;

    const RasterState& rasterState = mMaterial->getRasterState();

    // We inherit the resolved culling mode rather than the builder-set culling mode.
    // This preserves the property whereby double-sidedness automatically disables culling.
    mCulling = rasterState.culling;

    mColorWrite = rasterState.colorWrite;
    mDepthWrite = rasterState.depthWrite;
    mDepthFunc = rasterState.depthFunc;

    mMaterialSortingKey = RenderPass::makeMaterialSortingKey(
            material->getId(), material->generateMaterialInstanceId());

    if (material->getBlendingMode() == BlendingMode::MASKED) {
        setMaskThreshold(material->getMaskThreshold());
    }

    if (material->hasDoubleSidedCapability()) {
        setDoubleSided(material->isDoubleSided());
    }

    if (material->hasSpecularAntiAliasing()) {
        setSpecularAntiAliasingVariance(material->getSpecularAntiAliasingVariance());
        setSpecularAntiAliasingThreshold(material->getSpecularAntiAliasingThreshold());
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

// ------------------------------------------------------------------------------------------------

template<size_t Size>
UTILS_NOINLINE
void FMaterialInstance::setParameterUntypedImpl(const char* name, const void* value, size_t count) noexcept {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getUniformOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        mUniforms.setUniformArrayUntyped<Size>(size_t(offset), value, count);
    }
}

template<typename T>
UTILS_ALWAYS_INLINE
inline void FMaterialInstance::setParameterImpl(const char* name, const T* value, size_t count) noexcept {
    static_assert(!std::is_same_v<T, math::mat3f>);
    setParameterUntypedImpl<sizeof(T)>(name, value, count);
}

// ------------------------------------------------------------------------------------------------

void FMaterialInstance::setParameter(const char* name,
        backend::Handle<backend::HwTexture> texture, backend::SamplerParams params) noexcept {
    const auto cachedSamplerParameters = mSamplerParametersCache.find(name);
    if (cachedSamplerParameters != mSamplerParametersCache.cend()) {
        mSamplerParametersCache.erase(cachedSamplerParameters);
    }
    size_t index = mMaterial->getSamplerInterfaceBlock().getSamplerInfo(name)->offset;
    mSamplers.setSampler(index, { texture, params });
}

void FMaterialInstance::setParameterImpl(const char* name,
        Texture const* texture, TextureSampler const& sampler) noexcept {
    setParameter(name, upcast(texture)->getHwHandle(), sampler.getSamplerParams());
    mSamplerParametersCache[name] = SamplerParameters { texture, sampler };
}

void FMaterialInstance::setMaskThreshold(float threshold) noexcept {
    setParameter("_maskThreshold", math::saturate(threshold));
}

void FMaterialInstance::setSpecularAntiAliasingVariance(float variance) noexcept {
    setParameter("_specularAntiAliasingVariance", math::saturate(variance));
}

void FMaterialInstance::setSpecularAntiAliasingThreshold(float threshold) noexcept {
    setParameter("_specularAntiAliasingThreshold", math::saturate(threshold * threshold));
}

bool FMaterialInstance::getParameter(const char *name,
        const Texture *&outTexture, TextureSampler &outSampler) noexcept {
    const auto cachedSamplerParameters = mSamplerParametersCache.find(name);
    if (cachedSamplerParameters == mSamplerParametersCache.cend()) {
        return false;
    }
    outTexture = cachedSamplerParameters->second.texture;
    outSampler = cachedSamplerParameters->second.sampler;
    return true;
}

void FMaterialInstance::setDoubleSided(bool doubleSided) noexcept {
    if (UTILS_UNLIKELY(!mMaterial->hasDoubleSidedCapability())) {
        slog.w << "Parent material does not have double-sided capability." << io::endl;
        return;
    }
    setParameter("_doubleSided", doubleSided);
    if (doubleSided) {
        setCullingMode(CullingMode::NONE);
    }
}

void FMaterialInstance::setDepthCulling(bool enable) noexcept {
    mDepthFunc = enable ? RasterState::DepthFunc::GE : RasterState::DepthFunc::A;
}

const char* FMaterialInstance::getName() const noexcept {
    // To decide whether to use the parent material name as a fallback, we check for the nullness of
    // the instance's CString rather than calling empty(). This allows instances to override the
    // parent material's name with a blank string.
    if (mName.data() == nullptr) {
        return mMaterial->getName().c_str();
    }
    return mName.c_str();
}

Material const* MaterialInstance::getMaterial() const noexcept {
    return upcast(this)->getMaterial();
}

const char* MaterialInstance::getName() const noexcept {
    return upcast(this)->getName();
}

// ------------------------------------------------------------------------------------------------


void MaterialInstance::setParameter(const char* name, Texture const* texture,
        TextureSampler const& sampler) noexcept {
    return upcast(this)->setParameterImpl(name, texture, sampler);
}

bool MaterialInstance::getParameter(const char *name, const Texture *&outTexture, TextureSampler &outSampler) noexcept {
    return upcast(this)->getParameter(name, outTexture, outSampler);
}

void MaterialInstance::setParameter(const char* name, RgbType type, float3 color) noexcept {
    upcast(this)->setParameterImpl<float3>(name, Color::toLinear(type, color));
}

void MaterialInstance::setParameter(const char* name, RgbaType type, float4 color) noexcept {
    upcast(this)->setParameterImpl<float4>(name, Color::toLinear(type, color));
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

void MaterialInstance::setColorWrite(bool enable) noexcept {
    upcast(this)->setColorWrite(enable);
}

void MaterialInstance::setDepthWrite(bool enable) noexcept {
    upcast(this)->setDepthWrite(enable);
}

void MaterialInstance::setDepthCulling(bool enable) noexcept {
    upcast(this)->setDepthCulling(enable);
}


} // namespace filament
