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

#include "details/Material.h"

namespace filament {

using namespace math;
using namespace backend;

// ------------------------------------------------------------------------------------------------

// This is the untyped/sized version of the setParameter: we end up here for e.g. vec4<int> and
// vec4<float>. This must not be inlined (this is the whole point).
template<size_t Size>
UTILS_NOINLINE
void FMaterialInstance::setParameterUntypedImpl(std::string_view name, const void* value) {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getFieldOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        mUniforms.setUniformUntyped<Size>(size_t(offset), value);  // handles specialization for mat3f
    }
}

// ------------------------------------------------------------------------------------------------

// this converts typed calls into the untyped-sized call.
template<typename T>
UTILS_ALWAYS_INLINE
inline void FMaterialInstance::setParameterImpl(std::string_view name, T const& value) {
    static_assert(!std::is_same_v<T, math::mat3f>);
    setParameterUntypedImpl<sizeof(T)>(name, &value);
}

// specialization for mat3f
template<>
inline void FMaterialInstance::setParameterImpl(std::string_view name, mat3f const& value) {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getFieldOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        mUniforms.setUniform(size_t(offset), value);
    }
}

// ------------------------------------------------------------------------------------------------

template<size_t Size>
UTILS_NOINLINE
void FMaterialInstance::setParameterUntypedImpl(std::string_view name,
        const void* value, size_t count) {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getFieldOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        mUniforms.setUniformArrayUntyped<Size>(size_t(offset), value, count);
    }
}

template<typename T>
UTILS_ALWAYS_INLINE
inline void FMaterialInstance::setParameterImpl(std::string_view name,
        const T* value, size_t count) {
    static_assert(!std::is_same_v<T, math::mat3f>);
    setParameterUntypedImpl<sizeof(T)>(name, value, count);
}

// ------------------------------------------------------------------------------------------------

template<typename T, typename>
void MaterialInstance::setParameter(const char* name, size_t nameLength, T const& value) {
    downcast(this)->setParameterImpl({ name, nameLength }, value);
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t nameLength, bool const& v) {
    // this kills tail-call optimization
    MaterialInstance::setParameter(name, nameLength, (uint32_t)v);
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t nameLength, bool2 const& v) {
    // this kills tail-call optimization
    MaterialInstance::setParameter(name, nameLength, uint2(v));
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t nameLength, bool3 const& v) {
    // this kills tail-call optimization
    MaterialInstance::setParameter(name, nameLength, uint3(v));
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t nameLength, bool4 const& v) {
    // this kills tail-call optimization
    MaterialInstance::setParameter(name, nameLength, uint4(v));
}

// explicit template instantiation of our supported types
template UTILS_PUBLIC void MaterialInstance::setParameter<float>   (const char* name, size_t nameLength, float const&    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<int32_t> (const char* name, size_t nameLength, int32_t const&  v);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint32_t>(const char* name, size_t nameLength, uint32_t const& v);
template UTILS_PUBLIC void MaterialInstance::setParameter<int2>    (const char* name, size_t nameLength, int2 const&     v);
template UTILS_PUBLIC void MaterialInstance::setParameter<int3>    (const char* name, size_t nameLength, int3 const&     v);
template UTILS_PUBLIC void MaterialInstance::setParameter<int4>    (const char* name, size_t nameLength, int4 const&     v);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint2>   (const char* name, size_t nameLength, uint2 const&    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint3>   (const char* name, size_t nameLength, uint3 const&    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint4>   (const char* name, size_t nameLength, uint4 const&    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<float2>  (const char* name, size_t nameLength, float2 const&   v);
template UTILS_PUBLIC void MaterialInstance::setParameter<float3>  (const char* name, size_t nameLength, float3 const&   v);
template UTILS_PUBLIC void MaterialInstance::setParameter<float4>  (const char* name, size_t nameLength, float4 const&   v);
template UTILS_PUBLIC void MaterialInstance::setParameter<mat3f>   (const char* name, size_t nameLength, mat3f const&    v);
template UTILS_PUBLIC void MaterialInstance::setParameter<mat4f>   (const char* name, size_t nameLength, mat4f const&    v);

// ------------------------------------------------------------------------------------------------

template <typename T, typename>
void MaterialInstance::setParameter(const char* name, size_t nameLength, const T* value, size_t count) {
    downcast(this)->setParameterImpl({ name, nameLength }, value, count);
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t nameLength, const bool* v, size_t c) {
    auto* p = new uint32_t[c];
    std::copy_n(v, c, p);
    MaterialInstance::setParameter(name, nameLength, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t nameLength, const bool2* v, size_t c) {
    auto* p = new uint2[c];
    std::copy_n(v, c, p);
    MaterialInstance::setParameter(name, nameLength, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t nameLength, const bool3* v, size_t c) {
    auto* p = new uint3[c];
    std::copy_n(v, c, p);
    MaterialInstance::setParameter(name, nameLength, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t nameLength, const bool4* v, size_t c) {
    auto* p = new uint4[c];
    std::copy_n(v, c, p);
    MaterialInstance::setParameter(name, nameLength, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter<mat3f>(const char* name, size_t nameLength, const mat3f* v, size_t c) {
    // pretend each mat3 is an array of 3 float3
    MaterialInstance::setParameter(name, nameLength, reinterpret_cast<math::float3 const*>(v), c * 3);
}

template UTILS_PUBLIC void MaterialInstance::setParameter<float>   (const char* name, size_t nameLength, const float    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<int32_t> (const char* name, size_t nameLength, const int32_t  *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint32_t>(const char* name, size_t nameLength, const uint32_t *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<int2>    (const char* name, size_t nameLength, const int2     *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<int3>    (const char* name, size_t nameLength, const int3     *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<int4>    (const char* name, size_t nameLength, const int4     *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint2>   (const char* name, size_t nameLength, const uint2    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint3>   (const char* name, size_t nameLength, const uint3    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<uint4>   (const char* name, size_t nameLength, const uint4    *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<float2>  (const char* name, size_t nameLength, const float2   *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<float3>  (const char* name, size_t nameLength, const float3   *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<float4>  (const char* name, size_t nameLength, const float4   *v, size_t c);
template UTILS_PUBLIC void MaterialInstance::setParameter<mat4f>   (const char* name, size_t nameLength, const mat4f    *v, size_t c);

// ------------------------------------------------------------------------------------------------

template<typename T>
T MaterialInstance::getParameter(const char* name, size_t nameLength) const {
    return downcast(this)->getParameterImpl<T>({ name, nameLength });
}

// explicit template instantiation of our supported types
template UTILS_PUBLIC float MaterialInstance::getParameter<float>        (const char* name, size_t nameLength) const;
template UTILS_PUBLIC int32_t MaterialInstance::getParameter<int32_t>  (const char* name, size_t nameLength) const;
template UTILS_PUBLIC uint32_t MaterialInstance::getParameter<uint32_t>(const char* name, size_t nameLength) const;
template UTILS_PUBLIC int2 MaterialInstance::getParameter<int2>        (const char* name, size_t nameLength) const;
template UTILS_PUBLIC int3 MaterialInstance::getParameter<int3>        (const char* name, size_t nameLength) const;
template UTILS_PUBLIC int4 MaterialInstance::getParameter<int4>        (const char* name, size_t nameLength) const;
template UTILS_PUBLIC uint2 MaterialInstance::getParameter<uint2>      (const char* name, size_t nameLength) const;
template UTILS_PUBLIC uint3 MaterialInstance::getParameter<uint3>      (const char* name, size_t nameLength) const;
template UTILS_PUBLIC uint4 MaterialInstance::getParameter<uint4>      (const char* name, size_t nameLength) const;
template UTILS_PUBLIC float2 MaterialInstance::getParameter<float2>      (const char* name, size_t nameLength) const;
template UTILS_PUBLIC float3 MaterialInstance::getParameter<float3>      (const char* name, size_t nameLength) const;
template UTILS_PUBLIC float4 MaterialInstance::getParameter<float4>      (const char* name, size_t nameLength) const;
template UTILS_PUBLIC mat3f MaterialInstance::getParameter<mat3f>      (const char* name, size_t nameLength) const;

// ------------------------------------------------------------------------------------------------

Material const* MaterialInstance::getMaterial() const noexcept {
    return downcast(this)->getMaterial();
}

const char* MaterialInstance::getName() const noexcept {
    return downcast(this)->getName();
}

void MaterialInstance::setParameter(const char* name, size_t nameLength, Texture const* texture,
        TextureSampler const& sampler) {
    return downcast(this)->setParameterImpl({ name, nameLength }, downcast(texture), sampler);
}

void MaterialInstance::setParameter(
        const char* name, size_t nameLength, RgbType type, float3 color) {
    downcast(this)->setParameterImpl<float3>({ name, nameLength }, Color::toLinear(type, color));
}

void MaterialInstance::setParameter(
        const char* name, size_t nameLength, RgbaType type, float4 color) {
    downcast(this)->setParameterImpl<float4>({ name, nameLength }, Color::toLinear(type, color));
}

void MaterialInstance::setScissor(
        uint32_t left, uint32_t bottom, uint32_t width, uint32_t height) noexcept {
    downcast(this)->setScissor(left, bottom, width, height);
}

void MaterialInstance::unsetScissor() noexcept {
    downcast(this)->unsetScissor();
}

void MaterialInstance::setPolygonOffset(float scale, float constant) noexcept {
    downcast(this)->setPolygonOffset(scale, constant);
}

void MaterialInstance::setMaskThreshold(float threshold) noexcept {
    downcast(this)->setMaskThreshold(threshold);
}

void MaterialInstance::setSpecularAntiAliasingVariance(float variance) noexcept {
    downcast(this)->setSpecularAntiAliasingVariance(variance);
}

void MaterialInstance::setSpecularAntiAliasingThreshold(float threshold) noexcept {
    downcast(this)->setSpecularAntiAliasingThreshold(threshold);
}

void MaterialInstance::setDoubleSided(bool doubleSided) noexcept {
    downcast(this)->setDoubleSided(doubleSided);
}

void MaterialInstance::setTransparencyMode(TransparencyMode mode) noexcept {
    downcast(this)->setTransparencyMode(mode);
}

void MaterialInstance::setCullingMode(CullingMode culling) noexcept {
    downcast(this)->setCullingMode(culling);
}

void MaterialInstance::setColorWrite(bool enable) noexcept {
    downcast(this)->setColorWrite(enable);
}

void MaterialInstance::setDepthWrite(bool enable) noexcept {
    downcast(this)->setDepthWrite(enable);
}

void MaterialInstance::setDepthCulling(bool enable) noexcept {
    downcast(this)->setDepthCulling(enable);
}

void MaterialInstance::setDepthFunc(DepthFunc depthFunc) noexcept {
    downcast(this)->setDepthFunc(depthFunc);
}

MaterialInstance::DepthFunc MaterialInstance::getDepthFunc() const noexcept {
    return downcast(this)->getDepthFunc();
}

void MaterialInstance::setStencilWrite(bool enable) noexcept {
    downcast(this)->setStencilWrite(enable);
}

void MaterialInstance::setStencilCompareFunction(StencilCompareFunc func, StencilFace face) noexcept {
    downcast(this)->setStencilCompareFunction(func, face);
}

void MaterialInstance::setStencilOpStencilFail(StencilOperation op, StencilFace face) noexcept {
    downcast(this)->setStencilOpStencilFail(op, face);
}

void MaterialInstance::setStencilOpDepthFail(StencilOperation op, StencilFace face) noexcept {
    downcast(this)->setStencilOpDepthFail(op, face);
}

void MaterialInstance::setStencilOpDepthStencilPass(StencilOperation op, StencilFace face) noexcept {
    downcast(this)->setStencilOpDepthStencilPass(op, face);
}

void MaterialInstance::setStencilReferenceValue(uint8_t value, StencilFace face) noexcept {
    downcast(this)->setStencilReferenceValue(value, face);
}

void MaterialInstance::setStencilReadMask(uint8_t readMask, StencilFace face) noexcept {
    downcast(this)->setStencilReadMask(readMask, face);
}

void MaterialInstance::setStencilWriteMask(uint8_t writeMask, StencilFace face) noexcept {
    downcast(this)->setStencilWriteMask(writeMask, face);
}

MaterialInstance* MaterialInstance::duplicate(MaterialInstance const* other, const char* name) noexcept {
    return FMaterialInstance::duplicate(downcast(other), name);
}


float MaterialInstance::getMaskThreshold() const noexcept {
    return downcast(this)->getMaskThreshold();
}

float MaterialInstance::getSpecularAntiAliasingVariance() const noexcept {
    return downcast(this)->getSpecularAntiAliasingVariance();
}

float MaterialInstance::getSpecularAntiAliasingThreshold() const noexcept {
    return downcast(this)->getSpecularAntiAliasingThreshold();
}

bool MaterialInstance::isDoubleSided() const noexcept {
    return downcast(this)->isDoubleSided();
}

TransparencyMode MaterialInstance::getTransparencyMode() const noexcept {
    return downcast(this)->getTransparencyMode();
}

CullingMode MaterialInstance::getCullingMode() const noexcept {
    return downcast(this)->getCullingMode();
}

bool MaterialInstance::isColorWriteEnabled() const noexcept {
    return downcast(this)->isColorWriteEnabled();
}

bool MaterialInstance::isDepthWriteEnabled() const noexcept {
    return downcast(this)->isDepthWriteEnabled();
}

bool MaterialInstance::isStencilWriteEnabled() const noexcept {
    return downcast(this)->isStencilWriteEnabled();
}

bool MaterialInstance::isDepthCullingEnabled() const noexcept {
    return downcast(this)->isDepthCullingEnabled();
}

} // namespace filament
