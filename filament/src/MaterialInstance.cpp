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
#include "details/MaterialInstance.h"

#include <filament/Color.h>
#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/debug.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat3.h>

#include <algorithm>
#include <string_view>

#include <stddef.h>
#include <stdint.h>

namespace filament {

using namespace math;
using namespace backend;

// ------------------------------------------------------------------------------------------------

// This is the untyped/sized version of the setParameter: we end up here for e.g. vec4<int> and
// vec4<float>. This must not be inlined (this is the whole point).
template<size_t Size>
UTILS_NOINLINE
void FMaterialInstance::setParameterUntypedImpl(std::string_view const name, const void* value) {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getFieldOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        mUniforms.setUniformUntyped<Size>(size_t(offset), value);  // handles specialization for mat3f
    }
}

// ------------------------------------------------------------------------------------------------

// this converts typed calls into the untyped-sized call.
template<typename T>
UTILS_ALWAYS_INLINE
inline void FMaterialInstance::setParameterImpl(std::string_view const name, T const& value) {
    static_assert(!std::is_same_v<T, mat3f>);
    setParameterUntypedImpl<sizeof(T)>(name, &value);
}

// specialization for mat3f
template<>
inline void FMaterialInstance::setParameterImpl(std::string_view const name, mat3f const& value) {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getFieldOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        mUniforms.setUniform(size_t(offset), value);
    }
}

// ------------------------------------------------------------------------------------------------

template<size_t Size>
UTILS_NOINLINE
void FMaterialInstance::setParameterUntypedImpl(std::string_view const name,
        const void* value, size_t const count) {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getFieldOffset(name, 0);
    if (UTILS_LIKELY(offset >= 0)) {
        mUniforms.setUniformArrayUntyped<Size>(size_t(offset), value, count);
    }
}

template<typename T>
UTILS_ALWAYS_INLINE
inline void FMaterialInstance::setParameterImpl(std::string_view const name,
        const T* value, size_t const count) {
    static_assert(!std::is_same_v<T, mat3f>);
    setParameterUntypedImpl<sizeof(T)>(name, value, count);
}

// ------------------------------------------------------------------------------------------------

template<typename T, typename>
void MaterialInstance::setParameter(const char* name, size_t nameLength, T const& value) {
    downcast(this)->setParameterImpl({ name, nameLength }, value);
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t const nameLength, bool const& v) {
    // this kills tail-call optimization
    setParameter(name, nameLength, uint32_t(v));
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t const nameLength, bool2 const& v) {
    // this kills tail-call optimization
    setParameter(name, nameLength, uint2(v));
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t const nameLength, bool3 const& v) {
    // this kills tail-call optimization
    setParameter(name, nameLength, uint3(v));
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t const nameLength, bool4 const& v) {
    // this kills tail-call optimization
    setParameter(name, nameLength, uint4(v));
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
void MaterialInstance::setParameter(const char* name, size_t nameLength, const T* values, size_t count) {
    downcast(this)->setParameterImpl({ name, nameLength }, values, count);
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t const nameLength, const bool* v, size_t const c) {
    auto* p = new uint32_t[c];
    std::copy_n(v, c, p);
    setParameter(name, nameLength, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t const nameLength, const bool2* v, size_t const c) {
    auto* p = new uint2[c];
    std::copy_n(v, c, p);
    setParameter(name, nameLength, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t const nameLength, const bool3* v, size_t const c) {
    auto* p = new uint3[c];
    std::copy_n(v, c, p);
    setParameter(name, nameLength, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter(const char* name, size_t const nameLength, const bool4* v, size_t const c) {
    auto* p = new uint4[c];
    std::copy_n(v, c, p);
    setParameter(name, nameLength, p, c);
    delete [] p;
}

template<>
UTILS_PUBLIC void MaterialInstance::setParameter<mat3f>(const char* name, size_t const nameLength, const mat3f* v, size_t const c) {
    // pretend each mat3 is an array of 3 float3
    setParameter(name, nameLength, reinterpret_cast<float3 const*>(v), c * 3);
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
T FMaterialInstance::getParameterImpl(std::string_view const name) const {
    ssize_t offset = mMaterial->getUniformInterfaceBlock().getFieldOffset(name, 0);
    assert_invariant(offset>=0);
    return downcast(this)->getUniformBuffer().getUniform<T>(offset);
}

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

template<>
inline void FMaterialInstance::setConstantImpl<bool>(std::string_view const name, bool const& value) {
    std::optional<uint32_t> id = mMaterial->getMutableConstantId(name);
    FILAMENT_CHECK_PRECONDITION(id.has_value()) << "No mutable constant with name " << name;
    mConstants.set(*id, value);
}

template<typename T, typename>
void MaterialInstance::setConstant(const char* name, size_t nameLength, T const& value) {
    downcast(this)->setConstantImpl({ name, nameLength }, value);
}

// Explicit template instantiation of our supported types.
//
// Mutable spec constants will probably only ever allow bools, but it's nice to keep the API
// forwards-compatible.
template UTILS_PUBLIC void MaterialInstance::setConstant<bool>(const char* name, size_t nameLength, bool const& v);

// ------------------------------------------------------------------------------------------------

template<>
inline bool FMaterialInstance::getConstantImpl<bool>(std::string_view const name) const {
    std::optional<uint32_t> id = mMaterial->getMutableConstantId(name);
    FILAMENT_CHECK_PRECONDITION(id.has_value()) << "No mutable constant with name " << name;
    return mConstants[*id];
}

template<typename T, typename>
T MaterialInstance::getConstant(const char* name, size_t nameLength) const {
    return downcast(this)->getConstantImpl<T>({ name, nameLength });
}

// Explicit template instantiation of our supported types.
//
// Mutable spec constants will probably only ever allow bools, but it's nice to keep the API
// forwards-compatible.
template UTILS_PUBLIC bool MaterialInstance::getConstant<bool>(const char* name, size_t nameLength) const;

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
        const char* name, size_t nameLength, RgbType const type, float3 const color) {
    downcast(this)->setParameterImpl<float3>({ name, nameLength }, Color::toLinear(type, color));
}

void MaterialInstance::setParameter(
        const char* name, size_t nameLength, RgbaType const type, float4 const color) {
    downcast(this)->setParameterImpl<float4>({ name, nameLength }, Color::toLinear(type, color));
}

void MaterialInstance::setScissor(
        uint32_t const left, uint32_t const bottom, uint32_t const width, uint32_t const height) noexcept {
    downcast(this)->setScissor(left, bottom, width, height);
}

void MaterialInstance::unsetScissor() noexcept {
    downcast(this)->unsetScissor();
}

void MaterialInstance::setPolygonOffset(float const scale, float const constant) noexcept {
    downcast(this)->setPolygonOffset(scale, constant);
}

void MaterialInstance::setMaskThreshold(float const threshold) noexcept {
    downcast(this)->setMaskThreshold(threshold);
}

void MaterialInstance::setSpecularAntiAliasingVariance(float const variance) noexcept {
    downcast(this)->setSpecularAntiAliasingVariance(variance);
}

void MaterialInstance::setSpecularAntiAliasingThreshold(float const threshold) noexcept {
    downcast(this)->setSpecularAntiAliasingThreshold(threshold);
}

void MaterialInstance::setDoubleSided(bool const doubleSided) noexcept {
    downcast(this)->setDoubleSided(doubleSided);
}

void MaterialInstance::setTransparencyMode(TransparencyMode const mode) noexcept {
    downcast(this)->setTransparencyMode(mode);
}

void MaterialInstance::setCullingMode(CullingMode const culling) noexcept {
    downcast(this)->setCullingMode(culling);
}

void MaterialInstance::setCullingMode(CullingMode const colorPassCullingMode,
    CullingMode const shadowPassCullingMode) noexcept {
    downcast(this)->setCullingMode(colorPassCullingMode, shadowPassCullingMode);
}

void MaterialInstance::setColorWrite(bool const enable) noexcept {
    downcast(this)->setColorWrite(enable);
}

void MaterialInstance::setDepthWrite(bool const enable) noexcept {
    downcast(this)->setDepthWrite(enable);
}

void MaterialInstance::setDepthCulling(bool const enable) noexcept {
    downcast(this)->setDepthCulling(enable);
}

void MaterialInstance::setDepthFunc(DepthFunc const depthFunc) noexcept {
    downcast(this)->setDepthFunc(depthFunc);
}

MaterialInstance::DepthFunc MaterialInstance::getDepthFunc() const noexcept {
    return downcast(this)->getDepthFunc();
}

void MaterialInstance::setStencilWrite(bool const enable) noexcept {
    downcast(this)->setStencilWrite(enable);
}

void MaterialInstance::setStencilCompareFunction(StencilCompareFunc const func, StencilFace const face) noexcept {
    downcast(this)->setStencilCompareFunction(func, face);
}

void MaterialInstance::setStencilOpStencilFail(StencilOperation const op, StencilFace const face) noexcept {
    downcast(this)->setStencilOpStencilFail(op, face);
}

void MaterialInstance::setStencilOpDepthFail(StencilOperation const op, StencilFace const face) noexcept {
    downcast(this)->setStencilOpDepthFail(op, face);
}

void MaterialInstance::setStencilOpDepthStencilPass(StencilOperation const op, StencilFace const face) noexcept {
    downcast(this)->setStencilOpDepthStencilPass(op, face);
}

void MaterialInstance::setStencilReferenceValue(uint8_t const value, StencilFace const face) noexcept {
    downcast(this)->setStencilReferenceValue(value, face);
}

void MaterialInstance::setStencilReadMask(uint8_t const readMask, StencilFace const face) noexcept {
    downcast(this)->setStencilReadMask(readMask, face);
}

void MaterialInstance::setStencilWriteMask(uint8_t const writeMask, StencilFace const face) noexcept {
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

CullingMode MaterialInstance::getShadowCullingMode() const noexcept {
    return downcast(this)->getShadowCullingMode();
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

void MaterialInstance::commit(Engine& engine) const {
    downcast(this)->commit(downcast(engine));
}

} // namespace filament
