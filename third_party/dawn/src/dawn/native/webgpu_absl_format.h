// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_WEBGPU_ABSL_FORMAT_H_
#define SRC_DAWN_NATIVE_WEBGPU_ABSL_FORMAT_H_

#include <optional>

#include "absl/strings/str_format.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/native/webgpu_absl_format_autogen.h"

namespace dawn::detail {
template <typename Tag, typename T>
class TypedIntegerImpl;
}  // namespace dawn::detail

namespace dawn::ityp {
template <typename Index, typename Value>
class span;
}  // namespace dawn::ityp

namespace dawn::native {

//
// Structs
//

struct Color;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(const Color* value, const absl::FormatConversionSpec& spec, absl::FormatSink* s);

struct Extent2D;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Extent2D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct Extent3D;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Extent3D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct Origin2D;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Origin2D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct Origin3D;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Origin3D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct BindingInfo;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const BindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct BufferBindingInfo;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const BufferBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct BufferBindingLayout;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const BufferBindingLayout& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct TextureBindingInfo;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const TextureBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct TextureBindingLayout;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const TextureBindingLayout& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct StorageTextureBindingInfo;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const StorageTextureBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct StorageTextureBindingLayout;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const StorageTextureBindingLayout& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct SamplerBindingInfo;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const SamplerBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct SamplerBindingLayout;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const SamplerBindingLayout& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct InputAttachmentBindingInfo;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const InputAttachmentBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct StaticSamplerBindingInfo;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const StaticSamplerBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct TexelCopyTextureInfo;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const TexelCopyTextureInfo* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct TexelCopyBufferLayout;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const TexelCopyBufferLayout* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

struct ShaderModuleEntryPoint;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const ShaderModuleEntryPoint* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

//
// Objects
//

class DeviceBase;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const AdapterBase* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

class DeviceBase;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const DeviceBase* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

class ApiObjectBase;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const ApiObjectBase* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

class AttachmentState;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const AttachmentState* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

class Surface;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Surface* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

//
// Enums
//

enum class Aspect : uint8_t;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(Aspect value, const absl::FormatConversionSpec& spec, absl::FormatSink* s);

enum class BindingInfoType;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    BindingInfoType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

enum class SampleTypeBit : uint8_t;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(SampleTypeBit value, const absl::FormatConversionSpec& spec, absl::FormatSink* s);

enum class SingleShaderStage;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    SingleShaderStage value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

enum class VertexFormatBaseType;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    VertexFormatBaseType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

enum class InterStageComponentType;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    InterStageComponentType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

enum class InterpolationType;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    InterpolationType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

enum class InterpolationSampling;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    InterpolationSampling value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

enum class TextureComponentType;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    TextureComponentType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

enum class PixelLocalMemberType;
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    PixelLocalMemberType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

//
// Others
//

template <typename I, typename T>
absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const ityp::span<I, T>& values,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    s->Append("[");
    bool first = true;
    for (const auto& v : values) {
        if (!first) {
            s->Append(absl::StrFormat(", %s", v));
        } else {
            s->Append(absl::StrFormat("%s", v));
        }
        first = false;
    }
    s->Append("]");
    return {true};
}

template <typename Tag, typename T>
absl::FormatConvertResult<absl::FormatConversionCharSet::kNumeric> AbslFormatConvert(
    const dawn::detail::TypedIntegerImpl<Tag, T>& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    s->Append(absl::StrFormat("%u", static_cast<T>(value)));
    return {true};
}

template <typename T>
struct UndefinedWrapper {
    std::optional<T> value;
};

template <typename T>
UndefinedWrapper<T> WrapUndefined(T value, T undefinedValue) {
    return value == undefinedValue ? UndefinedWrapper<T>() : UndefinedWrapper<T>{value};
}

template <typename T>
absl::FormatConvertResult<absl::FormatConversionCharSet::kNumeric> AbslFormatConvert(
    const UndefinedWrapper<T>& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (!value.value) {
        s->Append("undefined");
    } else {
        s->Append(absl::StrFormat("%u", static_cast<T>(*value.value)));
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(StringView value, const absl::FormatConversionSpec& spec, absl::FormatSink* s);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_WEBGPU_ABSL_FORMAT_H_
