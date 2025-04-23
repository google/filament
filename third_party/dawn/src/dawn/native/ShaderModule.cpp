// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/ShaderModule.h"

#include <algorithm>
#include <limits>
#include <set>
#include <sstream>
#include <utility>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Constants.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CompilationMessages.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/TintUtils.h"

#ifdef DAWN_ENABLE_SPIRV_VALIDATION
#include "dawn/native/SpirvValidation.h"
#endif

#include "tint/tint.h"

namespace dawn::native {

namespace {

ResultOrError<SingleShaderStage> TintPipelineStageToShaderStage(
    tint::inspector::PipelineStage stage) {
    switch (stage) {
        case tint::inspector::PipelineStage::kVertex:
            return SingleShaderStage::Vertex;
        case tint::inspector::PipelineStage::kFragment:
            return SingleShaderStage::Fragment;
        case tint::inspector::PipelineStage::kCompute:
            return SingleShaderStage::Compute;
    }
    DAWN_UNREACHABLE();
}

BindingInfoType TintResourceTypeToBindingInfoType(
    tint::inspector::ResourceBinding::ResourceType type) {
    switch (type) {
        case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
        case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
            return BindingInfoType::Buffer;
        case tint::inspector::ResourceBinding::ResourceType::kSampler:
        case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
            return BindingInfoType::Sampler;
        case tint::inspector::ResourceBinding::ResourceType::kSampledTexture:
        case tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture:
        case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
        case tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture:
            return BindingInfoType::Texture;
        case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageTexture:
        case tint::inspector::ResourceBinding::ResourceType::kReadWriteStorageTexture:
            return BindingInfoType::StorageTexture;
        case tint::inspector::ResourceBinding::ResourceType::kExternalTexture:
            return BindingInfoType::ExternalTexture;
        case tint::inspector::ResourceBinding::ResourceType::kInputAttachment:
            return BindingInfoType::InputAttachment;

        default:
            DAWN_UNREACHABLE();
            return BindingInfoType::Buffer;
    }
}

wgpu::TextureFormat TintImageFormatToTextureFormat(
    tint::inspector::ResourceBinding::TexelFormat format) {
    switch (format) {
        case tint::inspector::ResourceBinding::TexelFormat::kR32Uint:
            return wgpu::TextureFormat::R32Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kR32Sint:
            return wgpu::TextureFormat::R32Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kR32Float:
            return wgpu::TextureFormat::R32Float;
        case tint::inspector::ResourceBinding::TexelFormat::kBgra8Unorm:
            return wgpu::TextureFormat::BGRA8Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Unorm:
            return wgpu::TextureFormat::RGBA8Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Snorm:
            return wgpu::TextureFormat::RGBA8Snorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Uint:
            return wgpu::TextureFormat::RGBA8Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Sint:
            return wgpu::TextureFormat::RGBA8Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Uint:
            return wgpu::TextureFormat::RG32Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Sint:
            return wgpu::TextureFormat::RG32Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Float:
            return wgpu::TextureFormat::RG32Float;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Uint:
            return wgpu::TextureFormat::RGBA16Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Sint:
            return wgpu::TextureFormat::RGBA16Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Float:
            return wgpu::TextureFormat::RGBA16Float;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Uint:
            return wgpu::TextureFormat::RGBA32Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Sint:
            return wgpu::TextureFormat::RGBA32Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Float:
            return wgpu::TextureFormat::RGBA32Float;
        case tint::inspector::ResourceBinding::TexelFormat::kR8Unorm:
            return wgpu::TextureFormat::R8Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kNone:
            return wgpu::TextureFormat::Undefined;

        default:
            DAWN_UNREACHABLE();
            return wgpu::TextureFormat::Undefined;
    }
}

wgpu::TextureViewDimension TintTextureDimensionToTextureViewDimension(
    tint::inspector::ResourceBinding::TextureDimension dim) {
    switch (dim) {
        case tint::inspector::ResourceBinding::TextureDimension::k1d:
            return wgpu::TextureViewDimension::e1D;
        case tint::inspector::ResourceBinding::TextureDimension::k2d:
            return wgpu::TextureViewDimension::e2D;
        case tint::inspector::ResourceBinding::TextureDimension::k2dArray:
            return wgpu::TextureViewDimension::e2DArray;
        case tint::inspector::ResourceBinding::TextureDimension::k3d:
            return wgpu::TextureViewDimension::e3D;
        case tint::inspector::ResourceBinding::TextureDimension::kCube:
            return wgpu::TextureViewDimension::Cube;
        case tint::inspector::ResourceBinding::TextureDimension::kCubeArray:
            return wgpu::TextureViewDimension::CubeArray;
        case tint::inspector::ResourceBinding::TextureDimension::kNone:
            return wgpu::TextureViewDimension::Undefined;
    }
    DAWN_UNREACHABLE();
}

wgpu::TextureSampleType TintSampledKindToSampleType(
    tint::inspector::ResourceBinding::SampledKind s) {
    switch (s) {
        case tint::inspector::ResourceBinding::SampledKind::kSInt:
            return wgpu::TextureSampleType::Sint;
        case tint::inspector::ResourceBinding::SampledKind::kUInt:
            return wgpu::TextureSampleType::Uint;
        case tint::inspector::ResourceBinding::SampledKind::kFloat:
            // Note that Float is compatible with both Float and UnfilterableFloat.
            return wgpu::TextureSampleType::Float;
        case tint::inspector::ResourceBinding::SampledKind::kUnknown:
            return wgpu::TextureSampleType::BindingNotUsed;
    }
    DAWN_UNREACHABLE();
}

ResultOrError<TextureComponentType> TintComponentTypeToTextureComponentType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kF32:
        case tint::inspector::ComponentType::kF16:
            return TextureComponentType::Float;
        case tint::inspector::ComponentType::kI32:
            return TextureComponentType::Sint;
        case tint::inspector::ComponentType::kU32:
            return TextureComponentType::Uint;
        case tint::inspector::ComponentType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempted to convert 'Unknown' component type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<VertexFormatBaseType> TintComponentTypeToVertexFormatBaseType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kF32:
        case tint::inspector::ComponentType::kF16:
            return VertexFormatBaseType::Float;
        case tint::inspector::ComponentType::kI32:
            return VertexFormatBaseType::Sint;
        case tint::inspector::ComponentType::kU32:
            return VertexFormatBaseType::Uint;
        case tint::inspector::ComponentType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempted to convert 'Unknown' component type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<wgpu::BufferBindingType> TintResourceTypeToBufferBindingType(
    tint::inspector::ResourceBinding::ResourceType resource_type) {
    switch (resource_type) {
        case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
            return wgpu::BufferBindingType::Uniform;
        case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
            return wgpu::BufferBindingType::Storage;
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
            return wgpu::BufferBindingType::ReadOnlyStorage;
        default:
            return DAWN_VALIDATION_ERROR("Attempted to convert non-buffer resource type");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<wgpu::StorageTextureAccess> TintResourceTypeToStorageTextureAccess(
    tint::inspector::ResourceBinding::ResourceType resource_type) {
    switch (resource_type) {
        case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
            return wgpu::StorageTextureAccess::WriteOnly;
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageTexture:
            return wgpu::StorageTextureAccess::ReadOnly;
        case tint::inspector::ResourceBinding::ResourceType::kReadWriteStorageTexture:
            return wgpu::StorageTextureAccess::ReadWrite;
        default:
            return DAWN_VALIDATION_ERROR("Attempted to convert non-storage texture resource type");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<InterStageComponentType> TintComponentTypeToInterStageComponentType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kF32:
            return InterStageComponentType::F32;
        case tint::inspector::ComponentType::kI32:
            return InterStageComponentType::I32;
        case tint::inspector::ComponentType::kU32:
            return InterStageComponentType::U32;
        case tint::inspector::ComponentType::kF16:
            return InterStageComponentType::F16;
        case tint::inspector::ComponentType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempted to convert 'Unknown' component type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<uint32_t> TintCompositionTypeToInterStageComponentCount(
    tint::inspector::CompositionType type) {
    switch (type) {
        case tint::inspector::CompositionType::kScalar:
            return 1u;
        case tint::inspector::CompositionType::kVec2:
            return 2u;
        case tint::inspector::CompositionType::kVec3:
            return 3u;
        case tint::inspector::CompositionType::kVec4:
            return 4u;
        case tint::inspector::CompositionType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempt to convert 'Unknown' composition type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<InterpolationType> TintInterpolationTypeToInterpolationType(
    tint::inspector::InterpolationType type) {
    switch (type) {
        case tint::inspector::InterpolationType::kPerspective:
            return InterpolationType::Perspective;
        case tint::inspector::InterpolationType::kLinear:
            return InterpolationType::Linear;
        case tint::inspector::InterpolationType::kFlat:
            return InterpolationType::Flat;
        case tint::inspector::InterpolationType::kUnknown:
            return DAWN_VALIDATION_ERROR(
                "Attempted to convert 'Unknown' interpolation type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<InterpolationSampling> TintInterpolationSamplingToInterpolationSamplingType(
    tint::inspector::InterpolationSampling type) {
    switch (type) {
        case tint::inspector::InterpolationSampling::kNone:
            return InterpolationSampling::None;
        case tint::inspector::InterpolationSampling::kCenter:
            return InterpolationSampling::Center;
        case tint::inspector::InterpolationSampling::kCentroid:
            return InterpolationSampling::Centroid;
        case tint::inspector::InterpolationSampling::kSample:
            return InterpolationSampling::Sample;
        case tint::inspector::InterpolationSampling::kFirst:
            return InterpolationSampling::First;
        case tint::inspector::InterpolationSampling::kEither:
            return InterpolationSampling::Either;
        case tint::inspector::InterpolationSampling::kUnknown:
            return DAWN_VALIDATION_ERROR(
                "Attempted to convert 'Unknown' interpolation sampling type from Tint");
    }
    DAWN_UNREACHABLE();
}

EntryPointMetadata::Override::Type FromTintOverrideType(tint::inspector::Override::Type type) {
    switch (type) {
        case tint::inspector::Override::Type::kBool:
            return EntryPointMetadata::Override::Type::Boolean;
        case tint::inspector::Override::Type::kFloat32:
            return EntryPointMetadata::Override::Type::Float32;
        case tint::inspector::Override::Type::kFloat16:
            return EntryPointMetadata::Override::Type::Float16;
        case tint::inspector::Override::Type::kInt32:
            return EntryPointMetadata::Override::Type::Int32;
        case tint::inspector::Override::Type::kUint32:
            return EntryPointMetadata::Override::Type::Uint32;
    }
    DAWN_UNREACHABLE();
}

ResultOrError<PixelLocalMemberType> FromTintPixelLocalMemberType(
    tint::inspector::PixelLocalMemberType type) {
    switch (type) {
        case tint::inspector::PixelLocalMemberType::kU32:
            return PixelLocalMemberType::U32;
        case tint::inspector::PixelLocalMemberType::kI32:
            return PixelLocalMemberType::I32;
        case tint::inspector::PixelLocalMemberType::kF32:
            return PixelLocalMemberType::F32;
        case tint::inspector::PixelLocalMemberType::kUnknown:
            return DAWN_VALIDATION_ERROR(
                "Attempted to convert 'Unknown' pixel local member type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<tint::Program> ParseWGSL(const tint::Source::File* file,
                                       const tint::wgsl::AllowedFeatures& allowedFeatures,
                                       const std::vector<tint::wgsl::Extension>& internalExtensions,
                                       OwnedCompilationMessages* outMessages) {
    tint::wgsl::reader::Options options;
    options.allowed_features = allowedFeatures;
    options.allowed_features.extensions.insert(internalExtensions.begin(),
                                               internalExtensions.end());
    tint::Program program = tint::wgsl::reader::Parse(file, options);
    if (outMessages != nullptr) {
        DAWN_TRY(outMessages->AddMessages(program.Diagnostics()));
    }
    if (!program.IsValid()) {
        return DAWN_VALIDATION_ERROR("Error while parsing WGSL: %s\n", program.Diagnostics().Str());
    }

    return std::move(program);
}

#if TINT_BUILD_SPV_READER
ResultOrError<tint::Program> ParseSPIRV(const std::vector<uint32_t>& spirv,
                                        const tint::wgsl::AllowedFeatures& allowedFeatures,
                                        OwnedCompilationMessages* outMessages,
                                        const DawnShaderModuleSPIRVOptionsDescriptor* optionsDesc) {
    tint::spirv::reader::Options options;
    if (optionsDesc) {
        options.allow_non_uniform_derivatives = optionsDesc->allowNonUniformDerivatives;
    }
    options.allowed_features = allowedFeatures;
    tint::Program program = tint::spirv::reader::Read(spirv, options);
    if (outMessages != nullptr) {
        DAWN_TRY(outMessages->AddMessages(program.Diagnostics()));
    }
    if (!program.IsValid()) {
        return DAWN_VALIDATION_ERROR("Error while parsing SPIR-V: %s\n",
                                     program.Diagnostics().Str());
    }

    return std::move(program);
}
#endif  // TINT_BUILD_SPV_READER

std::vector<uint64_t> GetBindGroupMinBufferSizes(const BindingGroupInfoMap& shaderBindings,
                                                 const BindGroupLayoutInternalBase* layout) {
    std::vector<uint64_t> requiredBufferSizes(layout->GetUnverifiedBufferCount());
    uint32_t packedIdx = 0;

    for (BindingIndex bindingIndex{0}; bindingIndex < layout->GetBufferCount(); ++bindingIndex) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);
        const auto* bufferBindingLayout =
            std::get_if<BufferBindingInfo>(&bindingInfo.bindingLayout);
        if (bufferBindingLayout == nullptr || bufferBindingLayout->minBindingSize > 0) {
            // Skip bindings that have minimum buffer size set in the layout
            continue;
        }

        DAWN_ASSERT(packedIdx < requiredBufferSizes.size());
        const auto& shaderInfo = shaderBindings.find(bindingInfo.binding);
        if (shaderInfo != shaderBindings.end()) {
            auto* shaderBufferInfo =
                std::get_if<BufferBindingInfo>(&shaderInfo->second.bindingInfo);
            if (shaderBufferInfo != nullptr) {
                requiredBufferSizes[packedIdx] = shaderBufferInfo->minBindingSize;
            } else {
                requiredBufferSizes[packedIdx] = 0;
            }
        } else {
            // We have to include buffers if they are included in the bind group's
            // packed vector. We don't actually need to check these at draw time, so
            // if this is a problem in the future we can optimize it further.
            requiredBufferSizes[packedIdx] = 0;
        }
        ++packedIdx;
    }

    return requiredBufferSizes;
}

bool IsShaderCompatibleWithPipelineLayoutOnStorageTextureAccess(
    const StorageTextureBindingInfo& pipelineBindingLayout,
    const StorageTextureBindingInfo& shaderBindingInfo) {
    return pipelineBindingLayout.access == shaderBindingInfo.access ||
           (pipelineBindingLayout.access == wgpu::StorageTextureAccess::ReadWrite &&
            shaderBindingInfo.access == wgpu::StorageTextureAccess::WriteOnly);
}

BindingInfoType GetShaderBindingType(const ShaderBindingInfo& shaderInfo) {
    return MatchVariant(
        shaderInfo.bindingInfo, [](const BufferBindingInfo&) { return BindingInfoType::Buffer; },
        [](const SamplerBindingInfo&) { return BindingInfoType::Sampler; },
        [](const TextureBindingInfo&) { return BindingInfoType::Texture; },
        [](const StorageTextureBindingInfo&) { return BindingInfoType::StorageTexture; },
        [](const ExternalTextureBindingInfo&) { return BindingInfoType::ExternalTexture; },
        [](const InputAttachmentBindingInfo&) { return BindingInfoType::InputAttachment; });
}

MaybeError ValidateCompatibilityOfSingleBindingWithLayout(const DeviceBase* device,
                                                          const BindGroupLayoutInternalBase* layout,
                                                          SingleShaderStage entryPointStage,
                                                          BindingNumber bindingNumber,
                                                          const ShaderBindingInfo& shaderInfo) {
    const BindGroupLayoutInternalBase::BindingMap& layoutBindings = layout->GetBindingMap();

    // An external texture binding found in the shader will later be expanded into multiple
    // bindings at compile time. This expansion will have already happened in the bgl - so
    // the shader and bgl will always mismatch at this point. Expansion info is contained in
    // the bgl object, so we can still verify the bgl used to have an external texture in
    // the slot corresponding to the shader reflection.
    if (std::holds_alternative<ExternalTextureBindingInfo>(shaderInfo.bindingInfo)) {
        // If an external texture binding used to exist in the bgl, it will be found as a
        // key in the ExternalTextureBindingExpansions map.
        // TODO(dawn:563): Provide info about the binding types.
        DAWN_INVALID_IF(!layout->GetExternalTextureBindingExpansionMap().contains(bindingNumber),
                        "Binding type in the shader (texture_external) doesn't match the "
                        "type in the layout.");

        return {};
    }

    const auto& bindingIt = layoutBindings.find(bindingNumber);
    DAWN_INVALID_IF(bindingIt == layoutBindings.end(), "Binding doesn't exist in %s.", layout);

    BindingIndex bindingIndex(bindingIt->second);
    const BindingInfo& layoutInfo = layout->GetBindingInfo(bindingIndex);

    BindingInfoType bindingLayoutType = GetBindingInfoType(layoutInfo);
    BindingInfoType shaderBindingType = GetShaderBindingType(shaderInfo);

    if (bindingLayoutType == BindingInfoType::StaticSampler) {
        DAWN_INVALID_IF(shaderBindingType != BindingInfoType::Sampler,
                        "Binding type in the shader (%s) doesn't match the required type of %s for "
                        "the %s type in the layout.",
                        shaderBindingType, BindingInfoType::Sampler, bindingLayoutType);
        return {};
    }

    DAWN_INVALID_IF(bindingLayoutType != shaderBindingType,
                    "Binding type in the shader (%s) doesn't match the type in the layout (%s).",
                    shaderBindingType, bindingLayoutType);

    ExternalTextureBindingExpansionMap expansions = layout->GetExternalTextureBindingExpansionMap();
    DAWN_INVALID_IF(expansions.find(bindingNumber) != expansions.end(),
                    "Binding type (buffer vs. texture vs. sampler vs. external) doesn't "
                    "match the type in the layout.");

    DAWN_INVALID_IF((layoutInfo.visibility & StageBit(entryPointStage)) == 0,
                    "Entry point's stage (%s) is not in the binding visibility in the layout (%s).",
                    StageBit(entryPointStage), layoutInfo.visibility);

    return MatchVariant(
        shaderInfo.bindingInfo,
        [&](const TextureBindingInfo& bindingInfo) -> MaybeError {
            const TextureBindingInfo& bindingLayout =
                std::get<TextureBindingInfo>(layoutInfo.bindingLayout);
            DAWN_INVALID_IF(
                bindingLayout.multisampled != bindingInfo.multisampled,
                "Binding multisampled flag (%u) doesn't match the layout's multisampled "
                "flag (%u)",
                bindingLayout.multisampled, bindingInfo.multisampled);

            wgpu::TextureSampleType requiredShaderType = bindingLayout.sampleType;
            // Both UnfilterableFloat and kInternalResolveAttachmentSampleType are compatible with
            // texture_Nd<f32> instead of having a specific WGSL type.
            if (requiredShaderType == kInternalResolveAttachmentSampleType ||
                requiredShaderType == wgpu::TextureSampleType::UnfilterableFloat) {
                requiredShaderType = wgpu::TextureSampleType::Float;
            }
            DAWN_INVALID_IF(bindingInfo.sampleType != requiredShaderType,
                            "The shader's texture sample type (%s) isn't compatible with the "
                            "layout's texture sample type (%s) (it is only compatible with %s for "
                            "the shader texture sample type).",
                            bindingInfo.sampleType, bindingLayout.sampleType, requiredShaderType);

            DAWN_INVALID_IF(
                bindingLayout.viewDimension != bindingInfo.viewDimension,
                "The shader's binding dimension (%s) doesn't match the layout's binding "
                "dimension (%s).",
                bindingLayout.viewDimension, bindingInfo.viewDimension);
            return {};
        },
        [&](const StorageTextureBindingInfo& bindingInfo) -> MaybeError {
            const StorageTextureBindingInfo& bindingLayout =
                std::get<StorageTextureBindingInfo>(layoutInfo.bindingLayout);
            DAWN_ASSERT(bindingLayout.format != wgpu::TextureFormat::Undefined);
            DAWN_ASSERT(bindingInfo.format != wgpu::TextureFormat::Undefined);

            DAWN_INVALID_IF(!IsShaderCompatibleWithPipelineLayoutOnStorageTextureAccess(
                                bindingLayout, bindingInfo),
                            "The layout's binding access (%s) isn't compatible with the shader's "
                            "binding access (%s).",
                            bindingLayout.access, bindingInfo.access);

            DAWN_INVALID_IF(bindingLayout.format != bindingInfo.format,
                            "The layout's binding format (%s) doesn't match the shader's binding "
                            "format (%s).",
                            bindingLayout.format, bindingInfo.format);

            DAWN_INVALID_IF(bindingLayout.viewDimension != bindingInfo.viewDimension,
                            "The layout's binding dimension (%s) doesn't match the "
                            "shader's binding dimension (%s).",
                            bindingLayout.viewDimension, bindingInfo.viewDimension);
            return {};
        },
        [&](const BufferBindingInfo& bindingInfo) -> MaybeError {
            const BufferBindingInfo& bindingLayout =
                std::get<BufferBindingInfo>(layoutInfo.bindingLayout);
            // Binding mismatch between shader and bind group is invalid. For example, a
            // writable binding in the shader with a readonly storage buffer in the bind
            // group layout is invalid. For internal usage with internal shaders, a storage
            // binding in the shader with an internal storage buffer in the bind group
            // layout is also valid.
            bool validBindingConversion =
                (bindingLayout.type == kInternalStorageBufferBinding &&
                 bindingInfo.type == wgpu::BufferBindingType::Storage) ||
                (bindingLayout.type == kInternalReadOnlyStorageBufferBinding &&
                 bindingInfo.type == wgpu::BufferBindingType::ReadOnlyStorage);

            DAWN_INVALID_IF(
                bindingLayout.type != bindingInfo.type && !validBindingConversion,
                "The buffer type in the shader (%s) is not compatible with the type in the "
                "layout (%s).",
                bindingInfo.type, bindingLayout.type);

            DAWN_INVALID_IF(bindingLayout.minBindingSize != 0 &&
                                bindingInfo.minBindingSize > bindingLayout.minBindingSize,
                            "The shader uses more bytes of the buffer (%u) than the layout's "
                            "minBindingSize (%u).",
                            bindingInfo.minBindingSize, bindingLayout.minBindingSize);
            return {};
        },
        [&](const SamplerBindingInfo& bindingInfo) -> MaybeError {
            const SamplerBindingInfo& bindingLayout =
                std::get<SamplerBindingInfo>(layoutInfo.bindingLayout);
            DAWN_INVALID_IF(
                (bindingLayout.type == wgpu::SamplerBindingType::Comparison) !=
                    (bindingInfo.type == wgpu::SamplerBindingType::Comparison),
                "The sampler type in the shader (comparison: %u) doesn't match the type in "
                "the layout (comparison: %u).",
                bindingInfo.type == wgpu::SamplerBindingType::Comparison,
                bindingLayout.type == wgpu::SamplerBindingType::Comparison);
            return {};
        },
        [](const ExternalTextureBindingInfo&) -> MaybeError {
            DAWN_UNREACHABLE();
            return {};
        },
        [&](const InputAttachmentBindingInfo& bindingInfo) -> MaybeError {
            // Internal use only, no validation, only assertions.
            const InputAttachmentBindingInfo& bindingLayout =
                std::get<InputAttachmentBindingInfo>(layoutInfo.bindingLayout);

            DAWN_ASSERT(bindingLayout.sampleType == bindingInfo.sampleType);

            return {};
        });
}

MaybeError ValidateCompatibilityWithBindGroupLayout(DeviceBase* device,
                                                    BindGroupIndex group,
                                                    const EntryPointMetadata& entryPoint,
                                                    const BindGroupLayoutInternalBase* layout) {
    // Iterate over all bindings used by this group in the shader, and find the
    // corresponding binding in the BindGroupLayout, if it exists.
    for (const auto& [bindingId, bindingInfo] : entryPoint.bindings[group]) {
        DAWN_TRY_CONTEXT(ValidateCompatibilityOfSingleBindingWithLayout(
                             device, layout, entryPoint.stage, bindingId, bindingInfo),
                         "validating that the entry-point's declaration for @group(%u) "
                         "@binding(%u) matches %s",
                         group, bindingId, layout);
    }

    return {};
}

ResultOrError<std::unique_ptr<EntryPointMetadata>> ReflectEntryPointUsingTint(
    const DeviceBase* device,
    tint::inspector::Inspector* inspector,
    const tint::inspector::EntryPoint& entryPoint) {
    std::unique_ptr<EntryPointMetadata> metadata = std::make_unique<EntryPointMetadata>();

    // Returns the invalid argument, and if it is true additionally store the formatted
    // error in metadata.infringedLimits. This is to delay the emission of these validation
    // errors until the entry point is used.
#define DelayedInvalidIf(invalid, ...)                                              \
    ([&] {                                                                          \
        if (invalid) {                                                              \
            metadata->infringedLimitErrors.push_back(absl::StrFormat(__VA_ARGS__)); \
        }                                                                           \
        return invalid;                                                             \
    })()

    const auto& name2Id = inspector->GetNamedOverrideIds();
    if (!entryPoint.overrides.empty()) {
        for (auto& c : entryPoint.overrides) {
            auto id = name2Id.at(c.name);
            EntryPointMetadata::Override override = {id, FromTintOverrideType(c.type),
                                                     c.is_initialized};

            std::string identifier = c.is_id_specified ? std::to_string(override.id.value) : c.name;
            metadata->overrides[identifier] = override;

            if (!c.is_initialized) {
                auto [_, inserted] =
                    metadata->uninitializedOverrides.emplace(std::move(identifier));
                // The insertion should have taken place
                DAWN_ASSERT(inserted);
            } else {
                auto [_, inserted] = metadata->initializedOverrides.emplace(std::move(identifier));
                // The insertion should have taken place
                DAWN_ASSERT(inserted);
            }
        }
    }

    // Add overrides which are not used by the entry point into the list so we
    // can validate set constants in the pipeline.
    for (auto& o : inspector->Overrides()) {
        std::string identifier = o.is_id_specified ? std::to_string(o.id.value) : o.name;
        if (metadata->overrides.count(identifier) != 0) {
            continue;
        }

        auto id = name2Id.at(o.name);
        EntryPointMetadata::Override override = {id, FromTintOverrideType(o.type), o.is_initialized,
                                                 /* isUsed */ false};
        metadata->overrides[identifier] = override;
    }

    DAWN_TRY_ASSIGN(metadata->stage, TintPipelineStageToShaderStage(entryPoint.stage));

    if (metadata->stage == SingleShaderStage::Compute) {
        metadata->usesNumWorkgroups = entryPoint.num_workgroups_used;
    }

    metadata->usesTextureLoadWithDepthTexture = entryPoint.has_texture_load_with_depth_texture;
    metadata->usesDepthTextureWithNonComparisonSampler =
        entryPoint.has_depth_texture_with_non_comparison_sampler;

    const CombinedLimits& limits = device->GetLimits();
    const uint32_t maxVertexAttributes = limits.v1.maxVertexAttributes;
    const uint32_t maxInterStageShaderVariables = limits.v1.maxInterStageShaderVariables;

    metadata->usedInterStageVariables.resize(maxInterStageShaderVariables);
    metadata->interStageVariables.resize(maxInterStageShaderVariables);

    // Immediate data byte size must be 4-byte aligned.
    if (entryPoint.push_constant_size) {
        DAWN_ASSERT(IsAligned(entryPoint.push_constant_size, 4u));
        metadata->immediateDataRangeByteSize = entryPoint.push_constant_size;
    }

    // Vertex shader specific reflection.
    if (metadata->stage == SingleShaderStage::Vertex) {
        // Vertex input reflection.
        for (const auto& inputVar : entryPoint.input_variables) {
            uint32_t unsanitizedLocation = inputVar.attributes.location.value();
            if (DelayedInvalidIf(unsanitizedLocation >= maxVertexAttributes,
                                 "Vertex input variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u)",
                                 inputVar.name, unsanitizedLocation, maxVertexAttributes)) {
                continue;
            }

            VertexAttributeLocation location(static_cast<uint8_t>(unsanitizedLocation));
            DAWN_TRY_ASSIGN(metadata->vertexInputBaseTypes[location],
                            TintComponentTypeToVertexFormatBaseType(inputVar.component_type));
            metadata->usedVertexInputs.set(location);
        }

        // Vertex output (inter-stage variables) reflection.
        uint32_t clipDistancesSlots = 0;
        if (entryPoint.clip_distances_size.has_value()) {
            clipDistancesSlots = RoundUp(*entryPoint.clip_distances_size, 4) / 4;
        }
        uint32_t minInvalidLocation = maxInterStageShaderVariables - clipDistancesSlots;
        for (const auto& outputVar : entryPoint.output_variables) {
            EntryPointMetadata::InterStageVariableInfo variable;
            variable.name = outputVar.variable_name;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToInterStageComponentType(outputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         outputVar.composition_type));
            DAWN_TRY_ASSIGN(variable.interpolationType,
                            TintInterpolationTypeToInterpolationType(outputVar.interpolation_type));
            DAWN_TRY_ASSIGN(variable.interpolationSampling,
                            TintInterpolationSamplingToInterpolationSamplingType(
                                outputVar.interpolation_sampling));

            uint32_t location = outputVar.attributes.location.value();
            if (location >= minInvalidLocation) {
                if (clipDistancesSlots > 0) {
                    metadata->infringedLimitErrors.push_back(absl::StrFormat(
                        "Vertex output variable \"%s\" has a location (%u) that "
                        "is too large. It should be less than (%u = %u - %u (clip_distances)).",
                        outputVar.name, location, minInvalidLocation, maxInterStageShaderVariables,
                        clipDistancesSlots));
                } else {
                    metadata->infringedLimitErrors.push_back(
                        absl::StrFormat("Vertex output variable \"%s\" has a location (%u) that "
                                        "is too large. It should be less than (%u).",
                                        outputVar.name, location, minInvalidLocation));
                }
                continue;
            }

            metadata->usedInterStageVariables[location] = true;
            metadata->interStageVariables[location] = variable;
        }

        // Other vertex metadata.
        metadata->totalInterStageShaderVariables =
            entryPoint.output_variables.size() + clipDistancesSlots;
        if (metadata->totalInterStageShaderVariables > maxInterStageShaderVariables) {
            size_t userDefinedOutputVariables = entryPoint.output_variables.size();

            std::ostringstream builtinInfo;
            if (entryPoint.clip_distances_size.has_value()) {
                builtinInfo << " + " << RoundUp(*entryPoint.clip_distances_size, 4) / 4
                            << " (clip_distances)";
            }

            metadata->infringedLimitErrors.push_back(absl::StrFormat(
                "Total vertex output variables count (%u = %u (user-defined)%s) exceeds the "
                "maximum (%u).",
                metadata->totalInterStageShaderVariables, userDefinedOutputVariables,
                builtinInfo.str(), maxInterStageShaderVariables));
        }

        metadata->usesVertexIndex = entryPoint.vertex_index_used;
        metadata->usesInstanceIndex = entryPoint.instance_index_used;
    }

    // Fragment shader specific reflection.
    if (metadata->stage == SingleShaderStage::Fragment) {
        // Fragment input (inter-stage variables) reflection.
        for (const auto& inputVar : entryPoint.input_variables) {
            // Skip over @color framebuffer fetch, it is handled below.
            if (!inputVar.attributes.location.has_value()) {
                DAWN_ASSERT(inputVar.attributes.color.has_value());
                continue;
            }

            uint32_t location = inputVar.attributes.location.value();
            EntryPointMetadata::InterStageVariableInfo variable;
            variable.name = inputVar.variable_name;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToInterStageComponentType(inputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         inputVar.composition_type));
            DAWN_TRY_ASSIGN(variable.interpolationType,
                            TintInterpolationTypeToInterpolationType(inputVar.interpolation_type));
            DAWN_TRY_ASSIGN(variable.interpolationSampling,
                            TintInterpolationSamplingToInterpolationSamplingType(
                                inputVar.interpolation_sampling));

            if (DelayedInvalidIf(location >= maxInterStageShaderVariables,
                                 "Fragment input variable \"%s\" has a location (%u) that "
                                 "is greater than or equal to (%u).",
                                 inputVar.name, location, maxInterStageShaderVariables)) {
                continue;
            }

            metadata->usedInterStageVariables[location] = true;
            metadata->interStageVariables[location] = variable;
        }

        uint32_t totalInterStageShaderVariables = entryPoint.input_variables.size();

        // Other fragment metadata
        metadata->usesSampleMaskOutput = entryPoint.output_sample_mask_used;
        metadata->usesSampleIndex = entryPoint.sample_index_used;
        if (entryPoint.front_facing_used || entryPoint.input_sample_mask_used ||
            entryPoint.sample_index_used) {
            ++totalInterStageShaderVariables;
        }
        metadata->usesFragDepth = entryPoint.frag_depth_used;

        metadata->totalInterStageShaderVariables = totalInterStageShaderVariables;
        if (metadata->totalInterStageShaderVariables > maxInterStageShaderVariables) {
            size_t userDefinedInputVariables = entryPoint.input_variables.size();

            std::ostringstream builtinInfo;
            if (metadata->totalInterStageShaderVariables > userDefinedInputVariables) {
                builtinInfo << " + 1 (";
                bool isFirst = true;
                if (entryPoint.front_facing_used) {
                    builtinInfo << "front_facing";
                    isFirst = false;
                }
                if (entryPoint.input_sample_mask_used) {
                    if (!isFirst) {
                        builtinInfo << "|";
                    }
                    builtinInfo << "sample_mask";
                    isFirst = false;
                }
                if (entryPoint.sample_index_used) {
                    if (!isFirst) {
                        builtinInfo << "|";
                    }
                    builtinInfo << "sample_index";
                    isFirst = false;
                }
            }

            metadata->infringedLimitErrors.push_back(absl::StrFormat(
                "Total fragment input variables count (%u = %u (user-defined)%s) exceeds the "
                "maximum (%u).",
                metadata->totalInterStageShaderVariables, userDefinedInputVariables,
                builtinInfo.str(), maxInterStageShaderVariables));
        }

        // Fragment output reflection.
        uint32_t maxColorAttachments = limits.v1.maxColorAttachments;
        for (const auto& outputVar : entryPoint.output_variables) {
            EntryPointMetadata::FragmentRenderAttachmentInfo variable;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToTextureComponentType(outputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         outputVar.composition_type));
            DAWN_ASSERT(variable.componentCount <= 4);

            uint32_t unsanitizedAttachment = outputVar.attributes.location.value();
            if (DelayedInvalidIf(unsanitizedAttachment >= maxColorAttachments,
                                 "Fragment output variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u).",
                                 outputVar.name, unsanitizedAttachment, maxColorAttachments)) {
                continue;
            }

            // Both `@blend_src(0)` and `@blend_src(1)` are related to color attachment 0 and must
            // have the same type, so we just need to save the type information of `@blend_src(1)`
            // in `metadata->fragmentOutputVariables[0]` so that when dual source blending is used
            // `metadata->fragmentOutputVariables[0].blendSrc` is always 1.
            bool isBlendSrc0 = false;
            if (outputVar.attributes.blend_src.has_value()) {
                variable.blendSrc = *outputVar.attributes.blend_src;
                isBlendSrc0 = variable.blendSrc == 0;
            } else {
                variable.blendSrc = 0;
            }

            if (!isBlendSrc0) {
                ColorAttachmentIndex attachment(static_cast<uint8_t>(unsanitizedAttachment));
                metadata->fragmentOutputVariables[attachment] = variable;
                metadata->fragmentOutputMask.set(attachment);
            }
        }

        // Fragment input reflection.
        for (const auto& inputVar : entryPoint.input_variables) {
            if (!inputVar.attributes.color.has_value()) {
                continue;
            }

            // Tint should disallow using @color(N) without the respective enable, which is gated
            // on the extension.
            DAWN_ASSERT(device->HasFeature(Feature::FramebufferFetch));

            EntryPointMetadata::FragmentRenderAttachmentInfo variable;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToTextureComponentType(inputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         inputVar.composition_type));
            DAWN_ASSERT(variable.componentCount <= 4);

            uint32_t unsanitizedAttachment = inputVar.attributes.color.value();
            if (DelayedInvalidIf(unsanitizedAttachment >= maxColorAttachments,
                                 "Fragment input variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u).",
                                 inputVar.name, unsanitizedAttachment, maxColorAttachments)) {
                continue;
            }

            ColorAttachmentIndex attachment(static_cast<uint8_t>(unsanitizedAttachment));
            metadata->fragmentInputVariables[attachment] = variable;
            metadata->fragmentInputMask.set(attachment);
        }

        // Fragment PLS reflection.
        if (!entryPoint.pixel_local_members.empty()) {
            metadata->usesPixelLocal = true;
            metadata->pixelLocalBlockSize =
                kPLSSlotByteSize * entryPoint.pixel_local_members.size();
            metadata->pixelLocalMembers.reserve(entryPoint.pixel_local_members.size());

            for (auto type : entryPoint.pixel_local_members) {
                PixelLocalMemberType metadataType;
                DAWN_TRY_ASSIGN(metadataType, FromTintPixelLocalMemberType(type));
                metadata->pixelLocalMembers.push_back(metadataType);
            }
        }
    }

    // Generic resource binding reflection.
    for (const tint::inspector::ResourceBinding& resource :
         inspector->GetResourceBindings(entryPoint.name)) {
        ShaderBindingInfo info;

        info.name = resource.variable_name;

        switch (TintResourceTypeToBindingInfoType(resource.resource_type)) {
            case BindingInfoType::Buffer: {
                BufferBindingInfo bindingInfo = {};
                bindingInfo.minBindingSize = resource.size;
                DAWN_TRY_ASSIGN(bindingInfo.type,
                                TintResourceTypeToBufferBindingType(resource.resource_type));
                info.bindingInfo = bindingInfo;
                break;
            }

            case BindingInfoType::Sampler: {
                SamplerBindingInfo bindingInfo = {};
                switch (resource.resource_type) {
                    case tint::inspector::ResourceBinding::ResourceType::kSampler:
                        bindingInfo.type = wgpu::SamplerBindingType::Filtering;
                        break;
                    case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
                        bindingInfo.type = wgpu::SamplerBindingType::Comparison;
                        break;
                    default:
                        DAWN_UNREACHABLE();
                }
                info.bindingInfo = bindingInfo;
                break;
            }

            case BindingInfoType::Texture: {
                TextureBindingInfo bindingInfo = {};
                bindingInfo.viewDimension =
                    TintTextureDimensionToTextureViewDimension(resource.dim);
                if (resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kDepthTexture ||
                    resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture) {
                    bindingInfo.sampleType = wgpu::TextureSampleType::Depth;
                } else {
                    bindingInfo.sampleType = TintSampledKindToSampleType(resource.sampled_kind);
                }
                bindingInfo.multisampled =
                    resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture ||
                    resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture;
                info.bindingInfo = bindingInfo;
                break;
            }

            case BindingInfoType::StorageTexture: {
                StorageTextureBindingInfo bindingInfo = {};
                DAWN_TRY_ASSIGN(bindingInfo.access,
                                TintResourceTypeToStorageTextureAccess(resource.resource_type));
                bindingInfo.format = TintImageFormatToTextureFormat(resource.image_format);
                bindingInfo.viewDimension =
                    TintTextureDimensionToTextureViewDimension(resource.dim);

                info.bindingInfo = bindingInfo;
                break;
            }

            case BindingInfoType::ExternalTexture: {
                info.bindingInfo.emplace<ExternalTextureBindingInfo>();
                break;
            }
            case BindingInfoType::StaticSampler: {
                return DAWN_VALIDATION_ERROR("Static samplers not supported in WGSL");
            }
            case BindingInfoType::InputAttachment: {
                InputAttachmentBindingInfo bindingInfo = {};
                bindingInfo.sampleType = TintSampledKindToSampleType(resource.sampled_kind);
                info.bindingInfo = bindingInfo;
                break;
            }
            default:
                return DAWN_VALIDATION_ERROR("Unknown binding type in Shader");
        }

        BindingNumber bindingNumber(resource.binding);
        BindGroupIndex bindGroupIndex(resource.bind_group);

        if (DelayedInvalidIf(bindGroupIndex >= kMaxBindGroupsTyped,
                             "The entry-point uses a binding with a group decoration (%u) "
                             "that exceeds maxBindGroups (%u) - 1.",
                             resource.bind_group, kMaxBindGroups) ||
            DelayedInvalidIf(bindingNumber >= kMaxBindingsPerBindGroupTyped,
                             "Binding number (%u) exceeds the maxBindingsPerBindGroup limit (%u).",
                             uint32_t(bindingNumber), kMaxBindingsPerBindGroup)) {
            continue;
        }

        const auto& [binding, inserted] =
            metadata->bindings[bindGroupIndex].emplace(bindingNumber, info);
        DAWN_INVALID_IF(!inserted,
                        "Entry-point has a duplicate binding for (group:%u, binding:%u).",
                        resource.binding, resource.bind_group);
    }

    // Compute the texture+sampler combination count.
    if (device->IsCompatibilityMode()) {
        tint::BindingPoint nonSamplerBindingPoint = {std::numeric_limits<uint32_t>::max()};
        auto samplerAndNonSamplerTextureUses =
            inspector->GetSamplerAndNonSamplerTextureUses(entryPoint.name, nonSamplerBindingPoint);

        // separate sampled from non-sampled and put sampled in set
        std::set<tint::BindingPoint> sampledTextures;
        std::set<tint::BindingPoint> sampledExternalTextures;
        std::vector<tint::BindingPoint> nonSampled;
        uint32_t numSamplerTexturePairs = 0;
        uint32_t numSamplerExternalTexturePairs = 0;

        for (const auto& pair : samplerAndNonSamplerTextureUses) {
            const auto& bindingGroupInfoMap =
                metadata->bindings[BindGroupIndex(pair.texture_binding_point.group)];
            const auto it =
                bindingGroupInfoMap.find(BindingNumber(pair.texture_binding_point.binding));
            auto isExternalTexture =
                std::holds_alternative<ExternalTextureBindingInfo>(it->second.bindingInfo);
            if (isExternalTexture) {
                ++numSamplerExternalTexturePairs;
                sampledExternalTextures.insert(pair.texture_binding_point);
            } else if (pair.sampler_binding_point == nonSamplerBindingPoint) {
                nonSampled.push_back(pair.texture_binding_point);
            } else {
                ++numSamplerTexturePairs;
                sampledTextures.insert(pair.texture_binding_point);
            }
        }

        // count the number of non-sampled that are not referenced by sampled pairs.
        auto numNonSampled = std::count_if(
            nonSampled.begin(), nonSampled.end(),
            [&](const tint::BindingPoint& nonSampledBindingPoint) {
                return sampledTextures.find(nonSampledBindingPoint) == sampledTextures.end();
            });
        metadata->numTextureSamplerCombinations = numSamplerTexturePairs + numNonSampled +
                                                  numSamplerExternalTexturePairs * 3 +
                                                  sampledExternalTextures.size();
    }

    // Reflection of combined sampler and texture uses.
    auto samplerTextureUses = inspector->GetSamplerTextureUses(entryPoint.name);

    metadata->samplerTexturePairs.reserve(samplerTextureUses.size());
    std::transform(samplerTextureUses.begin(), samplerTextureUses.end(),
                   std::back_inserter(metadata->samplerTexturePairs),
                   [](const tint::inspector::SamplerTexturePair& pair) {
                       EntryPointMetadata::SamplerTexturePair result;
                       result.sampler = {BindGroupIndex(pair.sampler_binding_point.group),
                                         BindingNumber(pair.sampler_binding_point.binding)};
                       result.texture = {BindGroupIndex(pair.texture_binding_point.group),
                                         BindingNumber(pair.texture_binding_point.binding)};
                       return result;
                   });

    metadata->usesSubgroupMatrix = entryPoint.uses_subgroup_matrix;

#undef DelayedInvalidIf
    return std::move(metadata);
}

MaybeError ReflectShaderUsingTint(DeviceBase* device,
                                  const tint::Program* program,
                                  OwnedCompilationMessages* compilationMessages,
                                  EntryPointMetadataTable* entryPointMetadataTable) {
    DAWN_ASSERT(program->IsValid());
    ScopedTintICEHandler scopedICEHandler(device);

    tint::inspector::Inspector inspector(*program);

    std::vector<tint::inspector::EntryPoint> entryPoints = inspector.GetEntryPoints();
    DAWN_INVALID_IF(inspector.has_error(), "Tint Reflection failure: Inspector: %s\n",
                    inspector.error());

    for (const tint::inspector::EntryPoint& entryPoint : entryPoints) {
        std::unique_ptr<EntryPointMetadata> metadata;
        DAWN_TRY_ASSIGN_CONTEXT(metadata,
                                ReflectEntryPointUsingTint(device, &inspector, entryPoint),
                                "processing entry point \"%s\".", entryPoint.name);

        DAWN_ASSERT(!entryPointMetadataTable->contains(entryPoint.name));
        entryPointMetadataTable->emplace(entryPoint.name, std::move(metadata));
    }
    return {};
}
}  // anonymous namespace

ResultOrError<Extent3D> ValidateComputeStageWorkgroupSize(
    const tint::Program& program,
    const char* entryPointName,
    bool usesSubgroupMatrix,
    uint32_t maxSubgroupSize,
    const LimitsForCompilationRequest& limits,
    const LimitsForCompilationRequest& adaterSupportedlimits) {
    tint::inspector::Inspector inspector(program);

    // At this point the entry point must exist and must have workgroup size values.
    tint::inspector::EntryPoint entryPoint = inspector.GetEntryPoint(entryPointName);
    DAWN_ASSERT(entryPoint.workgroup_size.has_value());
    const tint::inspector::WorkgroupSize& workgroup_size = entryPoint.workgroup_size.value();

    return ValidateComputeStageWorkgroupSize(workgroup_size.x, workgroup_size.y, workgroup_size.z,
                                             entryPoint.workgroup_storage_size, usesSubgroupMatrix,
                                             maxSubgroupSize, limits, adaterSupportedlimits);
}

ResultOrError<Extent3D> ValidateComputeStageWorkgroupSize(
    uint32_t x,
    uint32_t y,
    uint32_t z,
    size_t workgroupStorageSize,
    bool usesSubgroupMatrix,
    uint32_t maxSubgroupSize,
    const LimitsForCompilationRequest& limits,
    const LimitsForCompilationRequest& adaterSupportedlimits) {
    DAWN_INVALID_IF(x < 1 || y < 1 || z < 1,
                    "Entry-point uses workgroup_size(%u, %u, %u) that are below the "
                    "minimum allowed (1, 1, 1).",
                    x, y, z);

    if (DAWN_UNLIKELY(x > limits.maxComputeWorkgroupSizeX || y > limits.maxComputeWorkgroupSizeY ||
                      z > limits.maxComputeWorkgroupSizeZ)) {
        uint32_t maxComputeWorkgroupSizeXAdapterLimit =
            adaterSupportedlimits.maxComputeWorkgroupSizeX;
        uint32_t maxComputeWorkgroupSizeYAdapterLimit =
            adaterSupportedlimits.maxComputeWorkgroupSizeY;
        uint32_t maxComputeWorkgroupSizeZAdapterLimit =
            adaterSupportedlimits.maxComputeWorkgroupSizeZ;
        std::string increaseLimitAdvice =
            (x <= maxComputeWorkgroupSizeXAdapterLimit &&
             y <= maxComputeWorkgroupSizeYAdapterLimit && z <= maxComputeWorkgroupSizeZAdapterLimit)
                ? absl::StrFormat(
                      " This adapter supports higher maxComputeWorkgroupSizeX of %u, "
                      "maxComputeWorkgroupSizeY of %u, and maxComputeWorkgroupSizeZ of %u, which "
                      "can be specified in requiredLimits when calling requestDevice(). Limits "
                      "differ by hardware, so always check the adapter limits prior to requesting "
                      "a higher limit.",
                      maxComputeWorkgroupSizeXAdapterLimit, maxComputeWorkgroupSizeYAdapterLimit,
                      maxComputeWorkgroupSizeZAdapterLimit)
                : "";
        return DAWN_VALIDATION_ERROR(
            "Entry-point uses workgroup_size(%u, %u, %u) that exceeds the "
            "maximum allowed (%u, %u, %u).%s",
            x, y, z, limits.maxComputeWorkgroupSizeX, limits.maxComputeWorkgroupSizeY,
            limits.maxComputeWorkgroupSizeZ, increaseLimitAdvice);
    }

    uint64_t numInvocations = static_cast<uint64_t>(x) * y * z;
    uint32_t maxComputeInvocationsPerWorkgroup = limits.maxComputeInvocationsPerWorkgroup;
    DAWN_INVALID_IF(numInvocations > maxComputeInvocationsPerWorkgroup,
                    "The total number of workgroup invocations (%u) exceeds the "
                    "maximum allowed (%u).%s",
                    numInvocations, maxComputeInvocationsPerWorkgroup,
                    DAWN_INCREASE_LIMIT_MESSAGE(adaterSupportedlimits,
                                                maxComputeInvocationsPerWorkgroup, numInvocations));

    uint32_t maxComputeWorkgroupStorageSize = limits.maxComputeWorkgroupStorageSize;
    DAWN_INVALID_IF(
        workgroupStorageSize > maxComputeWorkgroupStorageSize,
        "The total use of workgroup storage (%u bytes) is larger than "
        "the maximum allowed (%u bytes).%s",
        workgroupStorageSize, maxComputeWorkgroupStorageSize,
        DAWN_INCREASE_LIMIT_MESSAGE(adaterSupportedlimits, maxComputeWorkgroupStorageSize,
                                    workgroupStorageSize));

    if (usesSubgroupMatrix) {
        // maxSubgroupSize must have a valid value if usesSubgroupMatrix is true and subgroups
        // feature is supported.
        DAWN_ASSERT(maxSubgroupSize > 0);
        DAWN_INVALID_IF((x % maxSubgroupSize) != 0,
                        "The x-dimension of workgroup_size (%u) must be a multiple of the device "
                        "maxSubgroupSize (%u) when the shader uses a subgroup matrix",
                        x, maxSubgroupSize);
    }

    return Extent3D{x, y, z};
}

ShaderModuleParseResult::ShaderModuleParseResult() = default;
ShaderModuleParseResult::~ShaderModuleParseResult() = default;

ShaderModuleParseResult::ShaderModuleParseResult(ShaderModuleParseResult&& rhs) = default;

ShaderModuleParseResult& ShaderModuleParseResult::operator=(ShaderModuleParseResult&& rhs) =
    default;

bool ShaderModuleParseResult::HasParsedShader() const {
    return tintProgram != nullptr;
}

MaybeError ValidateAndParseShaderModule(
    DeviceBase* device,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* outMessages) {
    DAWN_ASSERT(parseResult != nullptr);

    wgpu::SType moduleType;
    // A WGSL (or SPIR-V, if enabled) subdescriptor is required, and a Dawn-specific SPIR-V options
    // descriptor is allowed when using SPIR-V.
#if TINT_BUILD_SPV_READER
    DAWN_TRY_ASSIGN(
        moduleType,
        (descriptor
             .ValidateBranches<Branch<ShaderSourceWGSL, ShaderModuleCompilationOptions>,
                               Branch<ShaderSourceSPIRV, DawnShaderModuleSPIRVOptionsDescriptor,
                                      ShaderModuleCompilationOptions>>()));
#else
    DAWN_TRY_ASSIGN(
        moduleType,
        (descriptor.ValidateBranches<Branch<ShaderSourceWGSL, ShaderModuleCompilationOptions>>()));
#endif
    DAWN_ASSERT(moduleType != wgpu::SType(0u));

    ScopedTintICEHandler scopedICEHandler(device);

    const ShaderSourceWGSL* wgslDesc = nullptr;

    switch (moduleType) {
#if TINT_BUILD_SPV_READER
        case wgpu::SType::ShaderSourceSPIRV: {
            DAWN_INVALID_IF(device->IsToggleEnabled(Toggle::DisallowSpirv),
                            "SPIR-V is disallowed.");
            const auto* spirvDesc = descriptor.Get<ShaderSourceSPIRV>();
            const auto* spirvOptions = descriptor.Get<DawnShaderModuleSPIRVOptionsDescriptor>();

            // TODO(dawn:2033): Avoid unnecessary copies of the SPIR-V code.
            std::vector<uint32_t> spirv(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);

#ifdef DAWN_ENABLE_SPIRV_VALIDATION
            const bool dumpSpirv = device->IsToggleEnabled(Toggle::DumpShaders);
            DAWN_TRY(ValidateSpirv(device, spirv.data(), spirv.size(), dumpSpirv));
#endif  // DAWN_ENABLE_SPIRV_VALIDATION
            tint::Program program;
            DAWN_TRY_ASSIGN(program, ParseSPIRV(spirv, device->GetWGSLAllowedFeatures(),
                                                outMessages, spirvOptions));
            parseResult->tintProgram = AcquireRef(new TintProgram(std::move(program), nullptr));

            return {};
        }
#endif  // TINT_BUILD_SPV_READER
        case wgpu::SType::ShaderSourceWGSL: {
            wgslDesc = descriptor.Get<ShaderSourceWGSL>();
            break;
        }
        default:
            DAWN_UNREACHABLE();
    }
    DAWN_ASSERT(wgslDesc != nullptr);

    DAWN_INVALID_IF(descriptor.Get<ShaderModuleCompilationOptions>() != nullptr &&
                        !device->HasFeature(Feature::ShaderModuleCompilationOptions),
                    "Shader module compilation options used without %s enabled.",
                    wgpu::FeatureName::ShaderModuleCompilationOptions);

    auto tintFile = std::make_unique<tint::Source::File>("", wgslDesc->code);

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "// Dumped WGSL:\n" << std::string_view(wgslDesc->code) << "\n";
        device->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    tint::Program program;
    DAWN_TRY_ASSIGN(program, ParseWGSL(tintFile.get(), device->GetWGSLAllowedFeatures(),
                                       internalExtensions, outMessages));

    parseResult->tintProgram = AcquireRef(new TintProgram(std::move(program), std::move(tintFile)));

    return {};
}

RequiredBufferSizes ComputeRequiredBufferSizesForLayout(const EntryPointMetadata& entryPoint,
                                                        const PipelineLayoutBase* layout) {
    RequiredBufferSizes bufferSizes;
    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        bufferSizes[group] = GetBindGroupMinBufferSizes(entryPoint.bindings[group],
                                                        layout->GetBindGroupLayout(group));
    }

    return bufferSizes;
}

ResultOrError<tint::Program> RunTransforms(tint::ast::transform::Manager* transformManager,
                                           const tint::Program* program,
                                           const tint::ast::transform::DataMap& inputs,
                                           tint::ast::transform::DataMap* outputs,
                                           OwnedCompilationMessages* outMessages) {
    DAWN_ASSERT(program != nullptr);
    tint::ast::transform::DataMap transform_outputs;
    tint::Program result = transformManager->Run(*program, inputs, transform_outputs);
    if (outMessages != nullptr) {
        DAWN_TRY(outMessages->AddMessages(result.Diagnostics()));
    }
    DAWN_INVALID_IF(!result.IsValid(), "Tint program failure: %s\n", result.Diagnostics().Str());
    if (outputs != nullptr) {
        *outputs = std::move(transform_outputs);
    }
    return std::move(result);
}

MaybeError ValidateCompatibilityWithPipelineLayout(DeviceBase* device,
                                                   const EntryPointMetadata& entryPoint,
                                                   const PipelineLayoutBase* layout) {
    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        DAWN_TRY_CONTEXT(ValidateCompatibilityWithBindGroupLayout(
                             device, group, entryPoint, layout->GetBindGroupLayout(group)),
                         "validating the entry-point's compatibility for group %u with %s", group,
                         layout->GetBindGroupLayout(group));
    }

    for (BindGroupIndex group : IterateBitSet(~layout->GetBindGroupLayoutsMask())) {
        DAWN_INVALID_IF(entryPoint.bindings[group].size() > 0,
                        "The entry-point uses bindings in group %u but %s doesn't have a "
                        "BindGroupLayout for this index",
                        group, layout);
    }

    // Validate that filtering samplers are not used with unfilterable textures.
    for (const auto& pair : entryPoint.samplerTexturePairs) {
        const BindGroupLayoutInternalBase* samplerBGL =
            layout->GetBindGroupLayout(pair.sampler.group);
        const BindingInfo& samplerInfo =
            samplerBGL->GetBindingInfo(samplerBGL->GetBindingIndex(pair.sampler.binding));
        bool samplerIsFiltering = false;
        if (std::holds_alternative<StaticSamplerBindingInfo>(samplerInfo.bindingLayout)) {
            const StaticSamplerBindingInfo& samplerLayout =
                std::get<StaticSamplerBindingInfo>(samplerInfo.bindingLayout);
            samplerIsFiltering = samplerLayout.sampler->IsFiltering();
        } else {
            const SamplerBindingInfo& samplerLayout =
                std::get<SamplerBindingInfo>(samplerInfo.bindingLayout);
            samplerIsFiltering = (samplerLayout.type == wgpu::SamplerBindingType::Filtering);
        }
        if (!samplerIsFiltering) {
            continue;
        }
        const BindGroupLayoutInternalBase* textureBGL =
            layout->GetBindGroupLayout(pair.texture.group);
        const BindingInfo& textureInfo =
            textureBGL->GetBindingInfo(textureBGL->GetBindingIndex(pair.texture.binding));
        const TextureBindingInfo& sampledTextureBindingInfo =
            std::get<TextureBindingInfo>(textureInfo.bindingLayout);

        DAWN_INVALID_IF(
            sampledTextureBindingInfo.sampleType != wgpu::TextureSampleType::Float &&
                sampledTextureBindingInfo.sampleType != kInternalResolveAttachmentSampleType,
            "Texture binding (group:%u, binding:%u) is %s but used statically with a sampler "
            "(group:%u, binding:%u) that's %s",
            pair.texture.group, pair.texture.binding, sampledTextureBindingInfo.sampleType,
            pair.sampler.group, pair.sampler.binding, wgpu::SamplerBindingType::Filtering);
    }

    // Validate compatibility of the pixel local storage.
    if (entryPoint.usesPixelLocal) {
        DAWN_INVALID_IF(!layout->HasPixelLocalStorage(),
                        "The entry-point uses `pixel_local` block but the pipeline layout doesn't "
                        "contain a pixel local storage.");

        // TODO(dawn:1704): Allow entryPoint.pixelLocalBlockSize < layoutPixelLocalSize.
        auto layoutStorageAttachments = layout->GetStorageAttachmentSlots();
        size_t layoutPixelLocalSize = layoutStorageAttachments.size() * kPLSSlotByteSize;
        DAWN_INVALID_IF(entryPoint.pixelLocalBlockSize != layoutPixelLocalSize,
                        "The entry-point's pixel local block size (%u) is different from the "
                        "layout's total pixel local size (%u).",
                        entryPoint.pixelLocalBlockSize, layoutPixelLocalSize);

        for (size_t i = 0; i < entryPoint.pixelLocalMembers.size(); i++) {
            wgpu::TextureFormat layoutFormat = layoutStorageAttachments[i];

            // TODO(dawn:1704): Allow format conversions by injecting them in the shader
            // automatically.
            PixelLocalMemberType expectedType;
            switch (layoutFormat) {
                case wgpu::TextureFormat::R32Sint:
                    expectedType = PixelLocalMemberType::I32;
                    break;
                case wgpu::TextureFormat::R32Float:
                    expectedType = PixelLocalMemberType::F32;
                    break;
                case wgpu::TextureFormat::R32Uint:
                case wgpu::TextureFormat::Undefined:
                    expectedType = PixelLocalMemberType::U32;
                    break;

                default:
                    DAWN_UNREACHABLE();
            }

            PixelLocalMemberType entryPointType = entryPoint.pixelLocalMembers[i];
            DAWN_INVALID_IF(
                expectedType != entryPointType,
                "The `pixel_local` block's member at index %u has a type (%s) that's not "
                "compatible with the layout's storage format (%s), the expected type is %s.",
                i, entryPointType, layoutFormat, expectedType);
        }
    } else {
        // TODO(dawn:1704): Allow a fragment entry-point without PLS to be used with a layout that
        // has PLS.
        DAWN_INVALID_IF(entryPoint.stage == SingleShaderStage::Fragment &&
                            !layout->GetStorageAttachmentSlots().empty(),
                        "The layout contains a (non-empty) pixel local storage but the entry-point "
                        "doesn't use a `pixel local` block.");
    }

    // Validate that immediate data used by programmable state are smaller than pipelineLayout
    // immediate data range bytes.
    DAWN_INVALID_IF(entryPoint.immediateDataRangeByteSize > layout->GetImmediateDataRangeByteSize(),
                    "The entry-point uses more bytes of immediate data (%u) than the reserved "
                    "amount (%u) in %s.",
                    entryPoint.immediateDataRangeByteSize, layout->GetImmediateDataRangeByteSize(),
                    layout);

    return {};
}

// ShaderModuleBase

ShaderModuleBase::ShaderModuleBase(DeviceBase* device,
                                   const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                                   std::vector<tint::wgsl::Extension> internalExtensions,
                                   ApiObjectBase::UntrackedByDeviceTag tag)
    : Base(device, descriptor->label),
      mType(Type::Undefined),
      mInternalExtensions(std::move(internalExtensions)) {
    if (auto* spirvDesc = descriptor.Get<ShaderSourceSPIRV>()) {
        mType = Type::Spirv;
        mOriginalSpirv.assign(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);
    } else if (auto* wgslDesc = descriptor.Get<ShaderSourceWGSL>()) {
        mType = Type::Wgsl;
        mWgsl = std::string(wgslDesc->code);
    } else {
        DAWN_ASSERT(false);
    }

    if (const auto* compileOptions = descriptor.Get<ShaderModuleCompilationOptions>()) {
        mStrictMath = compileOptions->strictMath;
    }
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device,
                                   const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                                   std::vector<tint::wgsl::Extension> internalExtensions)
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions), kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : Base(device, tag, label), mType(Type::Undefined) {}

ShaderModuleBase::~ShaderModuleBase() = default;

void ShaderModuleBase::DestroyImpl() {
    Uncache();
}

// static
Ref<ShaderModuleBase> ShaderModuleBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new ShaderModuleBase(device, ObjectBase::kError, label));
}

ObjectType ShaderModuleBase::GetType() const {
    return ObjectType::ShaderModule;
}

bool ShaderModuleBase::HasEntryPoint(absl::string_view entryPoint) const {
    return mEntryPoints.contains(entryPoint);
}

ShaderModuleEntryPoint ShaderModuleBase::ReifyEntryPointName(StringView entryPointName,
                                                             SingleShaderStage stage) const {
    ShaderModuleEntryPoint entryPoint;
    if (entryPointName.IsUndefined()) {
        entryPoint.defaulted = true;
        entryPoint.name = mDefaultEntryPointNames[stage];
    } else {
        entryPoint.defaulted = false;
        entryPoint.name = entryPointName;
    }
    return entryPoint;
}

std::optional<bool> ShaderModuleBase::GetStrictMath() const {
    return mStrictMath;
}

const EntryPointMetadata& ShaderModuleBase::GetEntryPoint(absl::string_view entryPoint) const {
    DAWN_ASSERT(HasEntryPoint(entryPoint));
    return *mEntryPoints.at(entryPoint);
}

size_t ShaderModuleBase::ComputeContentHash() {
    ObjectContentHasher recorder;
    recorder.Record(mType);
    recorder.Record(mOriginalSpirv);
    recorder.Record(mWgsl);
    recorder.Record(mStrictMath);
    return recorder.GetContentHash();
}

bool ShaderModuleBase::EqualityFunc::operator()(const ShaderModuleBase* a,
                                                const ShaderModuleBase* b) const {
    return a->mType == b->mType && a->mOriginalSpirv == b->mOriginalSpirv && a->mWgsl == b->mWgsl &&
           a->mStrictMath == b->mStrictMath;
}

ShaderModuleBase::ScopedUseTintProgram ShaderModuleBase::UseTintProgram() {
    // Directly return ScopedUseTintProgram to add ref count. If the mTintProgram is valid,
    // this will prevent it from being released before using. If it is already released,
    // it will be recreated in the GetTintProgram, right before actually using it.
    return ScopedUseTintProgram(this);
}

Ref<TintProgram> ShaderModuleBase::GetTintProgram() {
    return mTintData.Use([&](auto tintData) {
        // If the tintProgram is valid, just return it.
        if (tintData->tintProgram) {
            return tintData->tintProgram;
        }
        // Otherwise, recreate the tintProgram. When the ShaderModuleBase is not referenced
        // externally, and not used for initializing any pipeline, the mTintProgram will be
        // released. However the ShaderModuleBase itself may still alive due to being referenced by
        // some pipelines. In this case, when DeviceBase::APICreateShaderModule() with the same
        // shader source code, Dawn will look up from the cache and return the same
        // ShaderModuleBase. In this case, we have to recreate the released mTintProgram for
        // initializing new pipelines.
        ShaderModuleDescriptor descriptor;
        ShaderSourceWGSL wgslDescriptor;
        ShaderSourceSPIRV sprivDescriptor;

        switch (mType) {
            case Type::Spirv:
                sprivDescriptor.codeSize = mOriginalSpirv.size();
                sprivDescriptor.code = mOriginalSpirv.data();
                descriptor.nextInChain = &sprivDescriptor;
                break;
            case Type::Wgsl:
                wgslDescriptor.code = mWgsl.c_str();
                descriptor.nextInChain = &wgslDescriptor;
                break;
            default:
                DAWN_ASSERT(false);
        }

        ShaderModuleParseResult parseResult;
        ValidateAndParseShaderModule(GetDevice(), Unpack(&descriptor), mInternalExtensions,
                                     &parseResult,
                                     /*compilationMessages*/ nullptr)
            .AcquireSuccess();
        DAWN_ASSERT(parseResult.tintProgram != nullptr);

        tintData->tintProgram = std::move(parseResult.tintProgram);
        tintData->tintProgramRecreateCount++;

        return tintData->tintProgram;
    });
}

Ref<TintProgram> ShaderModuleBase::GetNullableTintProgramForTesting() const {
    return mTintData.Use([&](auto tintData) { return tintData->tintProgram; });
}

int ShaderModuleBase::GetTintProgramRecreateCountForTesting() const {
    return mTintData.Use([&](auto tintData) { return tintData->tintProgramRecreateCount; });
}

Future ShaderModuleBase::APIGetCompilationInfo(
    const WGPUCompilationInfoCallbackInfo& callbackInfo) {
    struct CompilationInfoEvent final : public EventManager::TrackedEvent {
        WGPUCompilationInfoCallback mCallback;
        raw_ptr<void> mUserdata1;
        raw_ptr<void> mUserdata2;
        // Need to keep a Ref of the compilation messages in case the ShaderModule goes away before
        // the callback happens.
        Ref<ShaderModuleBase> mShaderModule;

        CompilationInfoEvent(const WGPUCompilationInfoCallbackInfo& callbackInfo,
                             Ref<ShaderModuleBase> shaderModule)
            : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                           TrackedEvent::Completed{}),
              mCallback(callbackInfo.callback),
              mUserdata1(callbackInfo.userdata1),
              mUserdata2(callbackInfo.userdata2),
              mShaderModule(std::move(shaderModule)) {}

        ~CompilationInfoEvent() override { EnsureComplete(EventCompletionType::Shutdown); }

        void Complete(EventCompletionType completionType) override {
            WGPUCompilationInfoRequestStatus status =
                WGPUCompilationInfoRequestStatus_CallbackCancelled;
            const CompilationInfo* compilationInfo = nullptr;
            if (completionType == EventCompletionType::Ready) {
                status = WGPUCompilationInfoRequestStatus_Success;
                compilationInfo = mShaderModule->mCompilationMessages->GetCompilationInfo();
            }

            mCallback(status, ToAPI(compilationInfo), mUserdata1.ExtractAsDangling(),
                      mUserdata2.ExtractAsDangling());
        }
    };
    FutureID futureID = GetDevice()->GetInstance()->GetEventManager()->TrackEvent(
        AcquireRef(new CompilationInfoEvent(callbackInfo, this)));
    return {futureID};
}

void ShaderModuleBase::InjectCompilationMessages(
    std::unique_ptr<OwnedCompilationMessages> compilationMessages) {
    // TODO(dawn:944): ensure the InjectCompilationMessages is properly handled for shader
    // module returned from cache.
    // InjectCompilationMessages should be called only once for a shader module, after it is
    // created. However currently InjectCompilationMessages may be called on a shader module
    // returned from cache rather than newly created, and violate the rule. We just skip the
    // injection in this case for now, but a proper solution including ensure the cache goes
    // before the validation is required.
    if (mCompilationMessages != nullptr) {
        return;
    }
    // Move the compilationMessages into the shader module and emit the tint errors and warnings
    mCompilationMessages = std::move(compilationMessages);
}

OwnedCompilationMessages* ShaderModuleBase::GetCompilationMessages() const {
    return mCompilationMessages.get();
}

MaybeError ShaderModuleBase::InitializeBase(ShaderModuleParseResult* parseResult,
                                            OwnedCompilationMessages* compilationMessages) {
    DAWN_TRY(mTintData.Use([&](auto tintData) -> MaybeError {
        tintData->tintProgram = std::move(parseResult->tintProgram);

        DAWN_TRY(ReflectShaderUsingTint(GetDevice(), &(tintData->tintProgram->program),
                                        compilationMessages, &mEntryPoints));
        return {};
    }));

    for (auto stage : IterateStages(kAllStages)) {
        mEntryPointCounts[stage] = 0;
    }
    for (auto& [name, metadata] : mEntryPoints) {
        SingleShaderStage stage = metadata->stage;
        if (mEntryPointCounts[stage] == 0) {
            mDefaultEntryPointNames[stage] = name;
        }
        mEntryPointCounts[stage]++;
    }

    return {};
}

void ShaderModuleBase::WillDropLastExternalRef() {
    // The last external ref being dropped indicates that the application is not currently using,
    // and no pending task will use the shader module. In this case we can free the memory for the
    // parsed module.
    mTintData.Use([&](auto tintData) { tintData->tintProgram = nullptr; });
}

}  // namespace dawn::native
