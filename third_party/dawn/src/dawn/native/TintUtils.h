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

#ifndef SRC_DAWN_NATIVE_TINTUTILS_H_
#define SRC_DAWN_NATIVE_TINTUTILS_H_

#include <functional>
#include <unordered_map>

#include "dawn/common/NonCopyable.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/native/stream/Stream.h"
#include "tint/tint.h"

namespace dawn::native {

class DeviceBase;
class PipelineLayoutBase;
struct ProgrammableStage;
class RenderPipelineBase;

tint::VertexPullingConfig BuildVertexPullingTransformConfig(
    const RenderPipelineBase& renderPipeline,
    BindGroupIndex pullingBufferBindingSet);

std::unordered_map<tint::OverrideId, double> BuildSubstituteOverridesTransformConfig(
    const ProgrammableStage& stage);

std::unordered_map<tint::OverrideId, double> BuildSubstituteOverridesTransformConfig(
    const EntryPointMetadata& metadata,
    const PipelineConstantEntries& constants);

namespace stream {
// Uses tint::ForeachField when available to implement the stream::Stream trait for types.
template <typename T>
    requires(tint::HasReflection<T>)
class Stream<T> {
  public:
    static void Write(Sink* s, const T& v) {
        tint::ForeachField(v, [&](const auto& f) { StreamIn(s, f); });
    }
    static MaybeError Read(Source* s, T* v) {
        MaybeError error = {};
        tint::ForeachField(*v, [&](auto& f) {
            if (!error.IsError()) {
                error = StreamOut(s, &f);
            }
        });
        return error;
    }
};
}  // namespace stream

constexpr tint::BindingPoint ToTint(const WGSLBindPoint& b) {
    return {static_cast<uint32_t>(b.group), static_cast<uint32_t>(b.binding)};
}

constexpr WGSLBindPoint FromTint(const tint::BindingPoint& tintBindingPoint) {
    return {BindGroupIndex(tintBindingPoint.group), BindingNumber(tintBindingPoint.binding)};
}

// Helper function to generate the binding remapping information for Tint compilation. Each backend
// remaps the group + BindingIndex to a BindingPoint differently using the `BindingPointFor`
// function passed as argument.
template <typename F>
concept ConvertsBindingIndexToBindingPoint = requires(F f, BindGroupIndex group, BindingIndex i) {
    { f(group, i) } -> std::same_as<tint::BindingPoint>;
};
template <ConvertsBindingIndexToBindingPoint F>
tint::Bindings GenerateBindingRemapping(const PipelineLayoutBase* layout,
                                        SingleShaderStage stage,
                                        F&& BindingPointFor) {
    tint::Bindings bindings;

    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayoutInternalBase* bgl = layout->GetBindGroupLayout(group);

        for (const auto& [bindingNumber, apiBindingIndex] : bgl->GetBindingMap()) {
            if (!(bgl->GetAPIBindingInfo(apiBindingIndex).visibility & StageBit(stage))) {
                continue;
            }

            tint::BindingPoint srcBindingPoint{
                .group = uint32_t(group),
                .binding = uint32_t(bindingNumber),
            };

            MatchVariant(
                bgl->GetAPIBindingInfo(apiBindingIndex).bindingLayout,
                [&](const BufferBindingInfo& bindingInfo) {
                    tint::BindingPoint dstBindingPoint =
                        BindingPointFor(group, bgl->AsBindingIndex(apiBindingIndex));
                    switch (bindingInfo.type) {
                        case wgpu::BufferBindingType::Uniform:
                            bindings.uniform.emplace(srcBindingPoint, dstBindingPoint);
                            break;
                        case kInternalStorageBufferBinding:
                        case wgpu::BufferBindingType::Storage:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                        case kInternalReadOnlyStorageBufferBinding:
                            bindings.storage.emplace(srcBindingPoint, dstBindingPoint);
                            break;
                        case wgpu::BufferBindingType::BindingNotUsed:
                        case wgpu::BufferBindingType::Undefined:
                            DAWN_UNREACHABLE();
                            break;
                    }
                },
                [&](const SamplerBindingInfo& bindingInfo) {
                    bindings.sampler.emplace(
                        srcBindingPoint,
                        BindingPointFor(group, bgl->AsBindingIndex(apiBindingIndex)));
                },
                [&](const StaticSamplerBindingInfo& bindingInfo) {
                    bindings.sampler.emplace(
                        srcBindingPoint,
                        BindingPointFor(group, bgl->AsBindingIndex(apiBindingIndex)));
                },
                [&](const TextureBindingInfo& bindingInfo) {
                    bindings.texture.emplace(
                        srcBindingPoint,
                        BindingPointFor(group, bgl->AsBindingIndex(apiBindingIndex)));
                },
                [&](const StorageTextureBindingInfo& bindingInfo) {
                    bindings.storage_texture.emplace(
                        srcBindingPoint,
                        BindingPointFor(group, bgl->AsBindingIndex(apiBindingIndex)));
                },
                [&](const InputAttachmentBindingInfo&) {
                    bindings.input_attachment.emplace(
                        srcBindingPoint,
                        BindingPointFor(group, bgl->AsBindingIndex(apiBindingIndex)));
                },
                [&](const TexelBufferBindingInfo&) {
                    bindings.texel_buffer.emplace(
                        srcBindingPoint,
                        BindingPointFor(group, bgl->AsBindingIndex(apiBindingIndex)));
                },
                [&](const ExternalTextureBindingInfo& bindingInfo) {
                    // TODO(491363837): Add YCBCR texture information as tint::ExternalYCBCRTexture
                    // data

                    bindings.external_texture.emplace(
                        srcBindingPoint,
                        tint::ExternalMultiplanarTexture{
                            .metadata = BindingPointFor(group, bindingInfo.metadata),
                            .plane0 = BindingPointFor(group, bindingInfo.plane0),
                            .plane1 = BindingPointFor(group, bindingInfo.plane1)});
                });
        }
    }

    return bindings;
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_TINTUTILS_H_
