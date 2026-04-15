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
#include <sstream>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Sha3.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CompilationMessages.h"
#include "dawn/native/Device.h"
#include "dawn/native/Error.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/ShaderModuleParseRequest.h"
#include "dawn/native/TintUtils.h"

#ifdef DAWN_ENABLE_SPIRV_VALIDATION
#include "dawn/native/SpirvValidation.h"
#endif

#include "tint/tint.h"

namespace dawn::native {

namespace {

SingleShaderStage TintPipelineStageToShaderStage(tint::inspector::PipelineStage stage) {
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
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyTexelBuffer:
        case tint::inspector::ResourceBinding::ResourceType::kReadWriteTexelBuffer:
            return BindingInfoType::TexelBuffer;
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
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Unorm:
            return wgpu::TextureFormat::RGBA16Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Snorm:
            return wgpu::TextureFormat::RGBA16Snorm;
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
        case tint::inspector::ResourceBinding::TexelFormat::kR8Snorm:
            return wgpu::TextureFormat::R8Snorm;
        case tint::inspector::ResourceBinding::TexelFormat::kR8Uint:
            return wgpu::TextureFormat::R8Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kR8Sint:
            return wgpu::TextureFormat::R8Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg8Unorm:
            return wgpu::TextureFormat::RG8Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRg8Snorm:
            return wgpu::TextureFormat::RG8Snorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRg8Uint:
            return wgpu::TextureFormat::RG8Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg8Sint:
            return wgpu::TextureFormat::RG8Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kR16Unorm:
            return wgpu::TextureFormat::R16Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kR16Snorm:
            return wgpu::TextureFormat::R16Snorm;
        case tint::inspector::ResourceBinding::TexelFormat::kR16Uint:
            return wgpu::TextureFormat::R16Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kR16Sint:
            return wgpu::TextureFormat::R16Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kR16Float:
            return wgpu::TextureFormat::R16Float;
        case tint::inspector::ResourceBinding::TexelFormat::kRg16Unorm:
            return wgpu::TextureFormat::RG16Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRg16Snorm:
            return wgpu::TextureFormat::RG16Snorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRg16Uint:
            return wgpu::TextureFormat::RG16Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg16Sint:
            return wgpu::TextureFormat::RG16Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg16Float:
            return wgpu::TextureFormat::RG16Float;
        case tint::inspector::ResourceBinding::TexelFormat::kRgb10A2Uint:
            return wgpu::TextureFormat::RGB10A2Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgb10A2Unorm:
            return wgpu::TextureFormat::RGB10A2Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRg11B10Ufloat:
            return wgpu::TextureFormat::RG11B10Ufloat;
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
        case tint::inspector::ResourceBinding::SampledKind::kFilterable:
            return wgpu::TextureSampleType::Float;
        case tint::inspector::ResourceBinding::SampledKind::kUnfilterable:
            return wgpu::TextureSampleType::UnfilterableFloat;
        case tint::inspector::ResourceBinding::SampledKind::kUnknownFilterable:
            return kUnknownFilterableFloatSampleType;
    }
    DAWN_UNREACHABLE();
}

wgpu::SamplerBindingType TintSamplerTypeToSamplerBindingType(
    tint::inspector::ResourceBinding::SamplerType type) {
    switch (type) {
        case tint::inspector::ResourceBinding::SamplerType::kComparison:
            return wgpu::SamplerBindingType::Comparison;
        case tint::inspector::ResourceBinding::SamplerType::kFiltering:
            return wgpu::SamplerBindingType::Filtering;
        case tint::inspector::ResourceBinding::SamplerType::kNonFiltering:
            return wgpu::SamplerBindingType::NonFiltering;
        case tint::inspector::ResourceBinding::SamplerType::kUnknownFiltering:
            return kUnknownFilteringSamplerBindingType;
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

ResultOrError<wgpu::TexelBufferAccess> TintResourceTypeToTexelBufferAccess(
    tint::inspector::ResourceBinding::ResourceType resource_type) {
    switch (resource_type) {
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyTexelBuffer:
            return wgpu::TexelBufferAccess::ReadOnly;
        case tint::inspector::ResourceBinding::ResourceType::kReadWriteTexelBuffer:
            return wgpu::TexelBufferAccess::ReadWrite;
        default:
            return DAWN_VALIDATION_ERROR("Attempted to convert non-texel buffer resource type");
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

EntryPointMetadata::OverrideId FromTintOverrideId(tint::OverrideId id) {
    return EntryPointMetadata::OverrideId{{id.value}};
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

EntryPointMetadata::TextureMetadataQuery FromTintLevelSampleInfo(
    tint::inspector::Inspector::LevelSampleInfo info) {
    EntryPointMetadata::TextureMetadataQuery result;
    switch (info.type) {
        case tint::inspector::Inspector::TextureQueryType::kTextureNumLevels:
            result.type =
                EntryPointMetadata::TextureMetadataQuery::TextureQueryType::TextureNumLevels;
            break;
        case tint::inspector::Inspector::TextureQueryType::kTextureNumSamples:
            result.type =
                EntryPointMetadata::TextureMetadataQuery::TextureQueryType::TextureNumSamples;
            break;
        default:
            DAWN_UNREACHABLE();
    }
    result.group = info.group;
    result.binding = info.binding;
    return result;
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

// Validation errors, if any, are stored within outputParseResult instead of get returned as
// ErrorData.
MaybeError ParseWGSL(std::unique_ptr<tint::Source::File> file,
                     const WGSLAllowedFeatures& allowedFeatures,
                     const std::vector<tint::wgsl::Extension>& internalExtensions,
                     ShaderModuleParseResult* outputParseResult) {
    tint::wgsl::reader::Options options;
    options.allowed_features = allowedFeatures.ToTint();
    options.allowed_features.extensions.insert(internalExtensions.begin(),
                                               internalExtensions.end());
    tint::Program program = tint::wgsl::reader::Parse(file.get(), options);

    // Store the compilation messages into outputParseResult.
    outputParseResult->compilationMessages.AddMessages(program.Diagnostics());

    // If WGSL parsing succeed, store the generated Tint program with no validation error.
    if (program.IsValid()) {
        outputParseResult->tintProgram = UnsafeUnserializedValue<std::optional<Ref<TintProgram>>>(
            AcquireRef(new TintProgram(std::move(program), std::move(file))));
        DAWN_ASSERT(outputParseResult->HasTintProgram() && !outputParseResult->HasError());
    } else {
        // Otherwise, store the validation error messages to outputParseResult.
        outputParseResult->SetValidationError(
            DAWN_VALIDATION_ERROR("Error while parsing WGSL: %s\n", program.Diagnostics().Str()));
        DAWN_ASSERT(!outputParseResult->HasTintProgram() && outputParseResult->HasError());
    }

    return {};
}

#if TINT_BUILD_SPV_READER
// Validation errors, if any, are stored within outputParseResult instead of get returned as
// ErrorData
MaybeError ParseSPIRV(const std::vector<uint32_t>& spirv,
                      const WGSLAllowedFeatures& allowedFeatures,
                      ShaderModuleParseResult* outputParseResult,
                      bool allowNonUniformDerivatives) {
    tint::Result<tint::core::ir::Module> irResult = tint::spirv::reader::ReadIR(spirv);
    if (irResult != tint::Success) {
        outputParseResult->SetValidationError(
            DAWN_VALIDATION_ERROR("Error while parsing SPIR-V: %s\n", irResult.Failure().reason));
        DAWN_ASSERT(!outputParseResult->HasTintProgram() && outputParseResult->HasError());
        return {};
    }

    tint::wgsl::writer::Options options;
    options.allow_non_uniform_derivatives = allowNonUniformDerivatives;
    options.disable_unreachable_code_warning = true;
    options.allowed_features = allowedFeatures.ToTint();
    auto wgslResult = tint::wgsl::writer::ProgramFromIR(irResult.Get(), options);

    // If WGSL generation succeeded, store the generated Tint program with no validation error.
    if (wgslResult == tint::Success) {
        tint::Program program = wgslResult.Move();

        // Store the compilation messages into outputParseResult.
        outputParseResult->compilationMessages.AddMessages(program.Diagnostics());

        outputParseResult->tintProgram = UnsafeUnserializedValue<std::optional<Ref<TintProgram>>>(
            AcquireRef(new TintProgram(std::move(program), nullptr)));
        DAWN_ASSERT(outputParseResult->HasTintProgram() && !outputParseResult->HasError());
    } else {
        // Otherwise, store the validation error messages to outputParseResult.
        outputParseResult->SetValidationError(DAWN_VALIDATION_ERROR(
            "Error while generating WGSL: %s\n", wgslResult.Failure().reason));
        DAWN_ASSERT(!outputParseResult->HasTintProgram() && outputParseResult->HasError());
    }

    return {};
}
#endif  // TINT_BUILD_SPV_READER

std::vector<uint64_t> GetBindGroupMinBufferSizes(const BindingGroupInfoMap& shaderBindings,
                                                 const BindGroupLayoutInternalBase* layout) {
    std::vector<uint64_t> requiredBufferSizes(layout->GetUnverifiedBufferCount());
    uint32_t packedIdx = 0;

    for (BindingIndex bindingIndex : layout->GetBufferIndices()) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);
        const auto& bufferBindingLayout = std::get<BufferBindingInfo>(bindingInfo.bindingLayout);

        if (bufferBindingLayout.minBindingSize > 0) {
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
        [](const TexelBufferBindingInfo&) { return BindingInfoType::TexelBuffer; },
        [](const ExternalTextureBindingInfo&) { return BindingInfoType::ExternalTexture; },
        [](const InputAttachmentBindingInfo&) { return BindingInfoType::InputAttachment; });
}

MaybeError ValidateCompatibilityOfSingleBindingWithLayout(const DeviceBase* device,
                                                          const BindGroupLayoutInternalBase* layout,
                                                          SingleShaderStage entryPointStage,
                                                          BindingNumber bindingNumber,
                                                          const ShaderBindingInfo& shaderInfo) {
    // Check that the binding exists.
    const BindGroupLayoutInternalBase::BindingMap& layoutBindings = layout->GetBindingMap();

    const auto& bindingIt = layoutBindings.find(bindingNumber);
    DAWN_INVALID_IF(bindingIt == layoutBindings.end(), "Binding doesn't exist in %s.", layout);

    APIBindingIndex bindingIndex(bindingIt->second);
    const BindingInfo& layoutInfo = layout->GetAPIBindingInfo(bindingIndex);

    // Check that it is of the same type as in the layout.
    BindingInfoType shaderBindingType = GetShaderBindingType(shaderInfo);
    BindingInfoType bglBindingType = GetBindingInfoType(layoutInfo);
    if (bglBindingType == BindingInfoType::StaticSampler) {
        bglBindingType = BindingInfoType::Sampler;
    }
    DAWN_INVALID_IF(bglBindingType != shaderBindingType,
                    "Binding type in the shader (%s) doesn't match the type in the layout (%s).",
                    shaderBindingType, bglBindingType);

    DAWN_INVALID_IF((layoutInfo.visibility & StageBit(entryPointStage)) == 0,
                    "Entry point's stage (%s) is not in the binding visibility in the layout (%s).",
                    StageBit(entryPointStage), layoutInfo.visibility);

    // Check the arraySize matches the layout and that the shader has the start of the array.
    DAWN_INVALID_IF(layoutInfo.arraySize < shaderInfo.arraySize,
                    "Binding type in the shader is a binding_array with %u elements but the "
                    "layout only provides %u elements",
                    shaderInfo.arraySize, layoutInfo.arraySize);
    DAWN_INVALID_IF(layoutInfo.indexInArray != BindingIndex(0),
                    "@binding(%u) in the shader is element %u of the layout's binding which is an "
                    "array starting at binding %u.",
                    shaderInfo.binding, layoutInfo.indexInArray,
                    uint32_t(layoutInfo.binding) - uint32_t(layoutInfo.indexInArray));

    // Validation specific to each type of binding.
    return MatchVariant(
        shaderInfo.bindingInfo,
        [&](const TextureBindingInfo& shaderBindingInfo) -> MaybeError {
            const TextureBindingInfo& bindingLayout =
                std::get<TextureBindingInfo>(layoutInfo.bindingLayout);
            DAWN_INVALID_IF(
                bindingLayout.multisampled != shaderBindingInfo.multisampled,
                "Binding multisampled flag (%u) doesn't match the layout's multisampled "
                "flag (%u)",
                bindingLayout.multisampled, shaderBindingInfo.multisampled);

            wgpu::TextureSampleType bglSampleType = bindingLayout.sampleType;
            // `kInternalResolveAttachmentSampleType` is compatible with texture_Nd<f32> instead of
            // having a specific WGSL type.
            if (bglSampleType == kInternalResolveAttachmentSampleType) {
                bglSampleType = wgpu::TextureSampleType::Float;
            }

            wgpu::TextureSampleType shaderSampleType = shaderBindingInfo.sampleType;

            bool isSameSampleType = shaderSampleType == bglSampleType;
            bool unknownFloatSampleTypeInShader =
                shaderSampleType == kUnknownFilterableFloatSampleType &&
                (bglSampleType == wgpu::TextureSampleType::Float ||
                 bglSampleType == wgpu::TextureSampleType::UnfilterableFloat);
            bool shaderSampleTypeConvertsFromRequiredFloat =
                shaderSampleType == wgpu::TextureSampleType::UnfilterableFloat &&
                bglSampleType == wgpu::TextureSampleType::Float;

            bool bglConvertsToShaderSampleType = isSameSampleType ||
                                                 unknownFloatSampleTypeInShader ||
                                                 shaderSampleTypeConvertsFromRequiredFloat;
            DAWN_INVALID_IF(!bglConvertsToShaderSampleType,
                            "The shader's texture sample type (%s) isn't compatible with the "
                            "layout's texture sample type (%s) (it is only compatible with %s for "
                            "the shader texture sample type).",
                            shaderSampleType, bindingLayout.sampleType, bglSampleType);

            DAWN_INVALID_IF(
                bindingLayout.viewDimension != shaderBindingInfo.viewDimension,
                "The shader's binding dimension (%s) doesn't match the layout's binding "
                "dimension (%s).",
                bindingLayout.viewDimension, shaderBindingInfo.viewDimension);
            return {};
        },
        [&](const StorageTextureBindingInfo& shaderBindingInfo) -> MaybeError {
            const StorageTextureBindingInfo& bindingLayout =
                std::get<StorageTextureBindingInfo>(layoutInfo.bindingLayout);
            DAWN_ASSERT(bindingLayout.format != wgpu::TextureFormat::Undefined);
            DAWN_ASSERT(shaderBindingInfo.format != wgpu::TextureFormat::Undefined);

            DAWN_INVALID_IF(!IsShaderCompatibleWithPipelineLayoutOnStorageTextureAccess(
                                bindingLayout, shaderBindingInfo),
                            "The layout's binding access (%s) isn't compatible with the shader's "
                            "binding access (%s).",
                            bindingLayout.access, shaderBindingInfo.access);

            DAWN_INVALID_IF(bindingLayout.format != shaderBindingInfo.format,
                            "The layout's binding format (%s) doesn't match the shader's binding "
                            "format (%s).",
                            bindingLayout.format, shaderBindingInfo.format);

            DAWN_INVALID_IF(bindingLayout.viewDimension != shaderBindingInfo.viewDimension,
                            "The layout's binding dimension (%s) doesn't match the "
                            "shader's binding dimension (%s).",
                            bindingLayout.viewDimension, shaderBindingInfo.viewDimension);
            return {};
        },
        [&](const TexelBufferBindingInfo& shaderBindingInfo) -> MaybeError {
            const TexelBufferBindingInfo& bindingLayout =
                std::get<TexelBufferBindingInfo>(layoutInfo.bindingLayout);
            DAWN_ASSERT(bindingLayout.format != wgpu::TextureFormat::Undefined);
            DAWN_ASSERT(shaderBindingInfo.format != wgpu::TextureFormat::Undefined);

            DAWN_INVALID_IF(bindingLayout.access != shaderBindingInfo.access,
                            "The layout's binding access (%s) doesn't match the shader's binding "
                            "access (%s).",
                            bindingLayout.access, shaderBindingInfo.access);

            DAWN_INVALID_IF(bindingLayout.format != shaderBindingInfo.format,
                            "The layout's binding format (%s) doesn't match the shader's binding "
                            "format (%s).",
                            bindingLayout.format, shaderBindingInfo.format);
            return {};
        },
        [&](const BufferBindingInfo& shaderBindingInfo) -> MaybeError {
            const BufferBindingInfo& bindingLayout =
                std::get<BufferBindingInfo>(layoutInfo.bindingLayout);
            // Binding mismatch between shader and bind group is invalid. For example, a
            // writable binding in the shader with a readonly storage buffer in the bind
            // group layout is invalid. For internal usage with internal shaders, a storage
            // binding in the shader with an internal storage buffer in the bind group
            // layout is also valid.
            bool validBindingConversion =
                (bindingLayout.type == kInternalStorageBufferBinding &&
                 shaderBindingInfo.type == wgpu::BufferBindingType::Storage) ||
                (bindingLayout.type == kInternalReadOnlyStorageBufferBinding &&
                 shaderBindingInfo.type == wgpu::BufferBindingType::ReadOnlyStorage);

            DAWN_INVALID_IF(
                bindingLayout.type != shaderBindingInfo.type && !validBindingConversion,
                "The buffer type in the shader (%s) is not compatible with the type in the "
                "layout (%s).",
                shaderBindingInfo.type, bindingLayout.type);

            DAWN_INVALID_IF(bindingLayout.minBindingSize != 0 &&
                                shaderBindingInfo.minBindingSize > bindingLayout.minBindingSize,
                            "The shader uses more bytes of the buffer (%u) than the layout's "
                            "minBindingSize (%u).",
                            shaderBindingInfo.minBindingSize, bindingLayout.minBindingSize);
            return {};
        },
        [&](const SamplerBindingInfo& shaderBindingInfo) -> MaybeError {
            wgpu::SamplerBindingType shaderSamplerType = shaderBindingInfo.type;

            wgpu::SamplerBindingType bglSamplerType;
            if (auto* staticBindingLayout =
                    std::get_if<StaticSamplerBindingInfo>(&layoutInfo.bindingLayout)) {
                bglSamplerType = staticBindingLayout->sampler->GetBindingType();
            } else {
                bglSamplerType = std::get<SamplerBindingInfo>(layoutInfo.bindingLayout).type;
            }

            bool isSameSamplerType = shaderSamplerType == bglSamplerType;
            bool unknownFilteringTypeInShader =
                shaderSamplerType == kUnknownFilteringSamplerBindingType &&
                (bglSamplerType == wgpu::SamplerBindingType::Filtering ||
                 bglSamplerType == wgpu::SamplerBindingType::NonFiltering);
            bool shaderSamplerTypeConvertsFromFiltering =
                shaderSamplerType == wgpu::SamplerBindingType::NonFiltering &&
                bglSamplerType == wgpu::SamplerBindingType::Filtering;

            bool bglConvertsToShaderSamplerType = isSameSamplerType ||
                                                  unknownFilteringTypeInShader ||
                                                  shaderSamplerTypeConvertsFromFiltering;
            DAWN_INVALID_IF(!bglConvertsToShaderSamplerType,
                            "The sampler type in the shader (%s) doesn't match the type in "
                            "the layout (%s).",
                            shaderSamplerType, bglSamplerType);

            return {};
        },
        [](const ExternalTextureBindingInfo&) -> MaybeError {
            // There are no other things to validate for the external textures.
            return {};
        },
        [&](const InputAttachmentBindingInfo& shaderBindingInfo) -> MaybeError {
            // Internal use only, no validation, only assertions.
            const InputAttachmentBindingInfo& bindingLayout =
                std::get<InputAttachmentBindingInfo>(layoutInfo.bindingLayout);

            DAWN_ASSERT(bindingLayout.sampleType == shaderBindingInfo.sampleType);

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
    const ShaderModuleParseDeviceInfo& deviceInfo,
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
            EntryPointMetadata::Override override = {
                {FromTintOverrideId(id), FromTintOverrideType(c.type), c.is_initialized}};

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
        if (metadata->overrides.contains(identifier)) {
            continue;
        }

        auto id = name2Id.at(o.name);
        EntryPointMetadata::Override override = {{FromTintOverrideId(id),
                                                  FromTintOverrideType(o.type), o.is_initialized,
                                                  /* isUsed */ false}};
        metadata->overrides[identifier] = override;
    }

    metadata->stage = TintPipelineStageToShaderStage(entryPoint.stage);

    if (metadata->stage == SingleShaderStage::Compute) {
        metadata->usesNumWorkgroups = entryPoint.num_workgroups_used;
        metadata->usesGlobalInvocationIndex = entryPoint.global_invocation_index_used;
        metadata->usesWorkgroupIndex = entryPoint.workgroup_index_used;
    }

    metadata->usesTextureLoadWithDepthTexture = entryPoint.has_texture_load_with_depth_texture;
    metadata->usesDepthTextureWithNonComparisonSampler =
        entryPoint.has_depth_texture_with_non_comparison_sampler;

    const LimitsForShaderModuleParseRequest& limits = deviceInfo.limits;
    const uint32_t maxVertexAttributes = limits.maxVertexAttributes;
    const uint32_t maxInterStageShaderVariables = limits.maxInterStageShaderVariables;

    metadata->usedInterStageVariables.resize(maxInterStageShaderVariables);
    metadata->interStageVariables.resize(maxInterStageShaderVariables);

    // Immediate data byte size must be 4-byte aligned.
    if (entryPoint.immediate_data_size) {
        DAWN_ASSERT(IsAligned(entryPoint.immediate_data_size, 4u));
        metadata->immediateDataRangeByteSize = entryPoint.immediate_data_size;

        // Avoid calling GetImmediateBlockInfo if the size exceeds the limit,
        // as it might cause an assertion in Tint. The error is recorded
        // to be caught at pipeline creation time.
        if (!DelayedInvalidIf(
                entryPoint.immediate_data_size > kMaxExternalImmediateConstantsPerPipeline * 4,
                "Immediate data size (%u) exceeds the maximum allowed size (%u).",
                entryPoint.immediate_data_size, kMaxExternalImmediateConstantsPerPipeline * 4)) {
            auto immediateBlockInfo = inspector->GetImmediateBlockInfo(entryPoint.name);
            metadata->immediateDataUsedSlots =
                ImmediateConstantMask(immediateBlockInfo.to_ullong());
        }
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
            clipDistancesSlots = uint32_t(RoundUp(*entryPoint.clip_distances_size, 4) / 4);
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
            uint32_t(entryPoint.output_variables.size()) + clipDistancesSlots;
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
            if (inputVar.interpolation_sampling ==
                tint::inspector::InterpolationSampling::kSample) {
                metadata->usesSampleInterpolants = true;
            }
        }

        uint32_t totalInterStageShaderVariables = uint32_t(entryPoint.input_variables.size());

        // Other fragment metadata
        metadata->usesSampleMaskOutput = entryPoint.output_sample_mask_used;
        metadata->usesSampleIndex = entryPoint.sample_index_used;

        struct BoolName {
            raw_ptr<const bool> value;
            const char* name;
        };
        BoolName boolNames[] = {
            {&entryPoint.front_facing_used, "front_facing"},
            {&entryPoint.input_sample_mask_used, "sample_mask"},
            {&entryPoint.sample_index_used, "sample_index_used"},
            {&entryPoint.primitive_index_used, "primitive_index_used"},
            {&entryPoint.subgroup_invocation_id_used, "subgroup_invocation_id"},
            {&entryPoint.subgroup_size_used, "subgroup_size"},
        };
        for (const auto& boolName : boolNames) {
            if (*boolName.value) {
                ++totalInterStageShaderVariables;
            }
        }

        metadata->usesFragDepth = entryPoint.frag_depth_used;
        metadata->usesFragPosition = entryPoint.frag_position_used;
        metadata->usesFineDerivativeBuiltin = entryPoint.fine_derivative_builtin_used;

        metadata->totalInterStageShaderVariables = totalInterStageShaderVariables;
        if (metadata->totalInterStageShaderVariables > maxInterStageShaderVariables) {
            size_t userDefinedInputVariables = entryPoint.input_variables.size();

            std::ostringstream builtinInfo;
            if (metadata->totalInterStageShaderVariables > userDefinedInputVariables) {
                builtinInfo << " + 1 (";

                const char* separator = "";
                for (const auto& boolName : boolNames) {
                    if (*boolName.value) {
                        builtinInfo << separator << boolName.name;
                        separator = "|";
                    }
                }
            }

            metadata->infringedLimitErrors.push_back(absl::StrFormat(
                "Total fragment input variables count (%u = %u (user-defined)%s) exceeds the "
                "maximum (%u).",
                metadata->totalInterStageShaderVariables, userDefinedInputVariables,
                builtinInfo.str(), maxInterStageShaderVariables));
        }

        // Fragment output reflection.
        uint32_t maxColorAttachments = limits.maxColorAttachments;
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
            DAWN_ASSERT(deviceInfo.features.IsEnabled(Feature::FramebufferFetch));

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

        info.arraySize = BindingIndex(resource.array_size.value_or(1));
        DAWN_INVALID_IF(resource.array_size.has_value() &&
                            deviceInfo.toggles.Has(Toggle::DisableBindGroupLayoutEntryArraySize),
                        "Use of binding_array is disabled.");
        DAWN_INVALID_IF(
            resource.array_size.has_value() && !deviceInfo.toggles.Has(Toggle::AllowUnsafeAPIs),
            "Use of binding_array is disabled as an unsafe API.");
        DAWN_INVALID_IF(info.arraySize == BindingIndex(0), "binding_array size is 0.");
        if (DelayedInvalidIf(
                info.arraySize >= BindingIndex(kMaxBindingsPerBindGroup),
                "binding_array size (%u) exceeds the maxBindingsPerBindGroup (%u) - 1.",
                info.arraySize, kMaxBindingsPerBindGroup)) {
            continue;
        }

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
                DAWN_ASSERT(resource.resource_type ==
                            tint::inspector::ResourceBinding::ResourceType::kSampler);

                bindingInfo.type = TintSamplerTypeToSamplerBindingType(resource.sampler_type);
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

            case BindingInfoType::TexelBuffer: {
                TexelBufferBindingInfo bindingInfo = {};
                DAWN_TRY_ASSIGN(bindingInfo.access,
                                TintResourceTypeToTexelBufferAccess(resource.resource_type));
                bindingInfo.format = TintImageFormatToTextureFormat(resource.image_format);

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

        BindGroupIndex bindGroupIndex(resource.bind_group);
        if (DelayedInvalidIf(bindGroupIndex >= kMaxBindGroupsTyped,
                             "The entry-point uses a binding with a group decoration (%u) "
                             "that exceeds maxBindGroups (%u) - 1.",
                             resource.bind_group, kMaxBindGroups)) {
            continue;
        }

        BindingNumber bindingNumber(resource.binding);
        if (DelayedInvalidIf(
                bindingNumber >= kMaxBindingsPerBindGroupTyped,
                "Binding number (%u) exceeds the maxBindingsPerBindGroup limit (%u) - 1.",
                uint32_t(bindingNumber), kMaxBindingsPerBindGroup)) {
            continue;
        }

        const auto& [binding, inserted] =
            metadata->bindings[bindGroupIndex].emplace(bindingNumber, info);
        DAWN_INVALID_IF(!inserted,
                        "Entry-point has a duplicate binding for (group:%u, binding:%u).",
                        resource.binding, resource.bind_group);
    }

    // Resource table reflection.
    // TODO(https://issues.chromium.org/473354064): Check that the list of uses of the resource
    // table are:
    // 1) compatible with the bindless feature enabled (sampling vs. full)
    // 2) potentially that each type used is enabled by whatever feature enables it (for example
    // texel buffers might require an extension)
    auto resourceTableInfo = inspector->GetResourceTableInfo(entryPoint.name);
    metadata->usesResourceTable = !resourceTableInfo.empty();

    // Sampler binding point placeholder for non-sampler texture usage. Make it
    // ToTint(EntryPointMetadata::nonSamplerBindingPoint), so that we have
    // FromTint(tintNonSamplerBindingPoint) == EntryPointMetadata::nonSamplerBindingPoint, and we
    // don't need to explicitly check if a tint BindingPoint is tintNonSamplerBindingPoint when
    // converting them to WGSLBindPoint.
    constexpr tint::BindingPoint tintNonSamplerBindingPoint =
        ToTint(EntryPointMetadata::nonSamplerBindingPoint);
    static_assert(FromTint(tintNonSamplerBindingPoint) ==
                  EntryPointMetadata::nonSamplerBindingPoint);

    // Reflection of combined sampler and texture uses.
    const auto samplerAndNonSamplerTextureUses =
        inspector->GetSamplerAndNonSamplerTextureUses(entryPoint.name, tintNonSamplerBindingPoint);

    metadata->samplerAndNonSamplerTexturePairs.reserve(samplerAndNonSamplerTextureUses.size());
    std::transform(samplerAndNonSamplerTextureUses.cbegin(), samplerAndNonSamplerTextureUses.cend(),
                   std::back_inserter(metadata->samplerAndNonSamplerTexturePairs),
                   [](const tint::inspector::SamplerTexturePair& pair) {
                       EntryPointMetadata::SamplerTexturePair result;
                       // The sampler binding point might be tintNonSamplerBindingPoint for
                       // non-sampler texture usages, and FromTint maps it to
                       // EntryPointMetadata::nonSamplerBindingPoint according to the definition of
                       // tintNonSamplerBindingPoint.
                       result.sampler = FromTint(pair.sampler_binding_point);
                       result.texture = FromTint(pair.texture_binding_point);
                       return result;
                   });

    auto textureQueries = inspector->GetTextureQueries(entryPoint.name);
    metadata->textureQueries.reserve(textureQueries.size());
    std::transform(textureQueries.begin(), textureQueries.end(),
                   std::back_inserter(metadata->textureQueries), FromTintLevelSampleInfo);

    metadata->usesSubgroupMatrix = entryPoint.uses_subgroup_matrix;

#undef DelayedInvalidIf
    return std::move(metadata);
}

void ReflectShaderUsingTint(const ShaderModuleParseDeviceInfo& deviceInfo,
                            ShaderModuleParseResult* outputParseResult) {
    DAWN_ASSERT(outputParseResult->HasTintProgram());
    const tint::Program* program =
        &outputParseResult->tintProgram.UnsafeGetValue().value().Get()->program;
    DAWN_ASSERT(program && program->IsValid());

    tint::inspector::Inspector inspector(*program);

    std::vector<tint::inspector::EntryPoint> entryPoints = inspector.GetEntryPoints();
    if (inspector.has_error()) {
        outputParseResult->SetValidationError(
            DAWN_VALIDATION_ERROR("Tint Reflection failure: Inspector: %s\n", inspector.error()));
        return;
    }

    // A ShaderModuleParseResult should get reflected at most once.
    DAWN_ASSERT(!outputParseResult->metadataTable.has_value());
    EntryPointMetadataTable& metadataTable = outputParseResult->metadataTable.emplace();

    for (const tint::inspector::EntryPoint& entryPoint : entryPoints) {
        auto entryPointReflectionResult =
            ReflectEntryPointUsingTint(deviceInfo, &inspector, entryPoint);
        // If validation error occurs, store the error into output parse result, drop the incomplete
        // metadate table, and stop reflection.
        if (entryPointReflectionResult.IsError()) {
            auto error = entryPointReflectionResult.AcquireError();
            error->AppendContext(
                absl::StrFormat("processing entry point \"%s\".", entryPoint.name));
            // The incomplete metadate table is also dropped in SetValidationError.
            outputParseResult->SetValidationError(std::move(error));
            return;
        }
        // Otherwise add the reflection to metadata table.
        auto reflection = entryPointReflectionResult.AcquireSuccess();
        DAWN_ASSERT(!metadataTable.contains(entryPoint.name));
        metadataTable.emplace(entryPoint.name, std::move(reflection));
    }
}
}  // anonymous namespace

ResultOrError<Extent3D> ValidateComputeStageWorkgroupSize(
    const tint::WorkgroupInfo& workgroupInfo,
    bool usesSubgroupMatrix,
    uint32_t maxSubgroupSize,
    const LimitsForCompilationRequest& limits,
    const LimitsForCompilationRequest& adapterSupportedlimits) {
    DAWN_INVALID_IF(workgroupInfo.x < 1 || workgroupInfo.y < 1 || workgroupInfo.z < 1,
                    "Entry-point uses workgroup_size(%u, %u, %u) that are below the "
                    "minimum allowed (1, 1, 1).",
                    workgroupInfo.x, workgroupInfo.y, workgroupInfo.z);

    if (workgroupInfo.x > limits.maxComputeWorkgroupSizeX ||
        workgroupInfo.y > limits.maxComputeWorkgroupSizeY ||
        workgroupInfo.z > limits.maxComputeWorkgroupSizeZ) [[unlikely]] {
        uint32_t maxComputeWorkgroupSizeXAdapterLimit =
            adapterSupportedlimits.maxComputeWorkgroupSizeX;
        uint32_t maxComputeWorkgroupSizeYAdapterLimit =
            adapterSupportedlimits.maxComputeWorkgroupSizeY;
        uint32_t maxComputeWorkgroupSizeZAdapterLimit =
            adapterSupportedlimits.maxComputeWorkgroupSizeZ;
        std::string increaseLimitAdvice =
            (workgroupInfo.x <= maxComputeWorkgroupSizeXAdapterLimit &&
             workgroupInfo.y <= maxComputeWorkgroupSizeYAdapterLimit &&
             workgroupInfo.z <= maxComputeWorkgroupSizeZAdapterLimit)
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
            workgroupInfo.x, workgroupInfo.y, workgroupInfo.z, limits.maxComputeWorkgroupSizeX,
            limits.maxComputeWorkgroupSizeY, limits.maxComputeWorkgroupSizeZ, increaseLimitAdvice);
    }

    uint64_t numInvocations =
        static_cast<uint64_t>(workgroupInfo.x) * workgroupInfo.y * workgroupInfo.z;
    uint32_t maxComputeInvocationsPerWorkgroup = limits.maxComputeInvocationsPerWorkgroup;
    DAWN_INVALID_IF(numInvocations > maxComputeInvocationsPerWorkgroup,
                    "The total number of workgroup invocations (%u) exceeds the "
                    "maximum allowed (%u).%s",
                    numInvocations, maxComputeInvocationsPerWorkgroup,
                    DAWN_INCREASE_LIMIT_MESSAGE(adapterSupportedlimits,
                                                maxComputeInvocationsPerWorkgroup, numInvocations));

    uint32_t maxComputeWorkgroupStorageSize = limits.maxComputeWorkgroupStorageSize;
    DAWN_INVALID_IF(
        workgroupInfo.storage_size > maxComputeWorkgroupStorageSize,
        "The total use of workgroup storage (%u bytes) is larger than "
        "the maximum allowed (%u bytes).%s",
        workgroupInfo.storage_size, maxComputeWorkgroupStorageSize,
        DAWN_INCREASE_LIMIT_MESSAGE(adapterSupportedlimits, maxComputeWorkgroupStorageSize,
                                    workgroupInfo.storage_size));

    if (usesSubgroupMatrix) {
        // maxSubgroupSize must have a valid value if usesSubgroupMatrix is true and subgroups
        // feature is supported.
        DAWN_ASSERT(maxSubgroupSize > 0);
        DAWN_INVALID_IF((workgroupInfo.x % maxSubgroupSize) != 0,
                        "The x-dimension of workgroup_size (%u) must be a multiple of the device "
                        "maxSubgroupSize (%u) when the shader uses a subgroup matrix",
                        workgroupInfo.x, maxSubgroupSize);
    }

    if (workgroupInfo.subgroup_size.has_value()) {
        const uint32_t explicitSubgroupSize = workgroupInfo.subgroup_size.value();
        DAWN_ASSERT(explicitSubgroupSize > 0);
        DAWN_INVALID_IF((workgroupInfo.x % explicitSubgroupSize != 0),
                        "The x-dimension of workgroup invocations (%u) is not a multiple of the "
                        "subgroup_size attribute (%u)",
                        workgroupInfo.x, explicitSubgroupSize);
    }

    return Extent3D{workgroupInfo.x, workgroupInfo.y, workgroupInfo.z};
}

MaybeError ValidateExplicitComputeSubgroupSize(const tint::WorkgroupInfo& workgroupInfo,
                                               uint32_t minExplicitSubgroupSize,
                                               uint32_t maxExplicitSubgroupSize,
                                               uint32_t maxComputeWorkgroupSubgroups) {
    if (workgroupInfo.subgroup_size.has_value()) {
        DAWN_ASSERT(minExplicitSubgroupSize > 0 && maxExplicitSubgroupSize > 0);
        const uint32_t explicitSubgroupSize = workgroupInfo.subgroup_size.value();
        DAWN_INVALID_IF(
            explicitSubgroupSize < minExplicitSubgroupSize ||
                explicitSubgroupSize > maxExplicitSubgroupSize,
            "The subgroup_size attribute (%u) is not in the allowed range "
            "[minExplicitComputeSubgroupSize, maxExplicitComputeSubgroupSize] ([%u, %u]).",
            explicitSubgroupSize, minExplicitSubgroupSize, maxExplicitSubgroupSize);
        uint64_t numInvocations =
            static_cast<uint64_t>(workgroupInfo.x) * workgroupInfo.y * workgroupInfo.z;
        DAWN_INVALID_IF(
            numInvocations > maxComputeWorkgroupSubgroups * explicitSubgroupSize,
            "The total number of workgroup invocations (%u) exceeds the product of"
            "maxComputeWorkgroupSubgroups and the subgroup_size attribute (%u * %u = %u).",
            numInvocations, maxComputeWorkgroupSubgroups, explicitSubgroupSize, numInvocations);
    }

    return {};
}

CachedValidationError::CachedValidationError(std::unique_ptr<ErrorData>&& errorData) {
    DAWN_ASSERT(errorData->GetType() == InternalErrorType::Validation);
    message = errorData->GetMessage();
    contexts = errorData->GetContexts();
    DAWN_ASSERT(!message.empty());
}

std::unique_ptr<ErrorData> CachedValidationError::ToErrorData() const {
    DAWN_ASSERT(!message.empty());
    auto error = std::make_unique<ErrorData>(InternalErrorType::Validation, message);
    std::for_each(contexts.begin(), contexts.end(), [&error](auto c) { error->AppendContext(c); });
    return error;
}

bool ShaderModuleParseResult::HasTintProgram() const {
    return tintProgram.UnsafeGetValue().has_value() &&
           tintProgram.UnsafeGetValue().value() != nullptr;
}

bool ShaderModuleParseResult::HasError() const {
    // If cachedValidationError holds error, it must have non-empty error message string.
    DAWN_ASSERT(!cachedValidationError.has_value() || !cachedValidationError->message.empty());
    return cachedValidationError.has_value();
}

std::unique_ptr<ErrorData> ShaderModuleParseResult::ToErrorData() const {
    DAWN_ASSERT(HasError());
    return cachedValidationError->ToErrorData();
}

void ShaderModuleParseResult::SetValidationError(std::unique_ptr<ErrorData>&& errorData) {
    DAWN_ASSERT(errorData->GetType() == InternalErrorType::Validation);
    cachedValidationError = CachedValidationError(std::move(errorData));
    // If validation error occurs, clear the Tint program and metadata table.
    tintProgram.UnsafeGetValue().reset();
    metadataTable.reset();
    DAWN_ASSERT(HasError());
}

void DumpShaderFromDescriptor(LogEmitter* logEmitter,
                              const UnpackedPtr<ShaderModuleDescriptor>& shaderModuleDesc) {
#if TINT_BUILD_SPV_READER
    if ([[maybe_unused]] const auto* spirvDesc = shaderModuleDesc.Get<ShaderSourceSPIRV>()) {
        // Dump SPIR-V if enabled.
#ifdef DAWN_ENABLE_SPIRV_VALIDATION
        DumpSpirv(logEmitter, spirvDesc->code, spirvDesc->codeSize);
#endif  // DAWN_ENABLE_SPIRV_VALIDATION
        return;
    }
#else   // TINT_BUILD_SPV_READER
    // SPIR-V is not enabled, so the descriptor should not contain it.
    DAWN_ASSERT(!shaderModuleDesc.Has<ShaderSourceSPIRV>());
#endif  // TINT_BUILD_SPV_READER

    // Dump WGSL.
    const ShaderSourceWGSL* wgslDesc = shaderModuleDesc.Get<ShaderSourceWGSL>();
    DAWN_ASSERT(wgslDesc != nullptr);
    std::ostringstream dumpedMsg;
    dumpedMsg << "// Dumped WGSL:\n" << std::string_view(wgslDesc->code) << "\n";
    logEmitter->EmitLog(wgpu::LoggingType::Info, dumpedMsg.str().c_str());
}

ResultOrError<ShaderModuleParseResult> ParseShaderModule(ShaderModuleParseRequest req) {
    ShaderModuleParseResult outputParseResult;

    const ShaderModuleParseDeviceInfo& deviceInfo = req.deviceInfo;

#if TINT_BUILD_SPV_READER
    // Handling SPIR-V if enabled.
    if (std::holds_alternative<ShaderModuleParseSpirvDescription>(req.shaderDescription)) {
        // SPIRV toggle and instance feature should have been validated before checking cache.
        DAWN_ASSERT(!deviceInfo.toggles.Has(Toggle::DisallowSpirv));
        // (No assert for wgpu::InstanceFeatureName::ShaderSourceSPIRV since there's no instance.)

        ShaderModuleParseSpirvDescription& spirvDesc =
            std::get<ShaderModuleParseSpirvDescription>(req.shaderDescription);
        const std::vector<uint32_t>& spirvCode = spirvDesc.spirvCode.UnsafeGetValue();

#ifdef DAWN_ENABLE_SPIRV_VALIDATION
        MaybeError validationResult =
            ValidateSpirv(req.logEmitter.UnsafeGetValue(), spirvCode.data(), spirvCode.size(),
                          deviceInfo.toggles.Has(Toggle::UseSpirv14));
        // If SpirV validation error occurs, store it into outputParseResult and return.
        if (validationResult.IsError()) {
            outputParseResult.SetValidationError(validationResult.AcquireError());
        }
#endif  // DAWN_ENABLE_SPIRV_VALIDATION
        // Try parsing SpirV if no validation error.
        if (!outputParseResult.HasError()) {
            DAWN_TRY(ParseSPIRV(spirvCode, deviceInfo.wgslAllowedFeatures, &outputParseResult,
                                spirvDesc.allowNonUniformDerivatives));
        }
    }
#else   // TINT_BUILD_SPV_READER
        // SPIR-V is not enabled, so the descriptor should not contain it.
    DAWN_ASSERT(!std::holds_alternative<ShaderModuleParseSpirvDescription>(req.shaderDescription));
#endif  // TINT_BUILD_SPV_READER

    // Handling WGSL.
    if (std::holds_alternative<ShaderModuleParseWGSLDescription>(req.shaderDescription)) {
        ShaderModuleParseWGSLDescription wgslDesc =
            std::get<ShaderModuleParseWGSLDescription>(req.shaderDescription);
        const std::vector<tint::wgsl::Extension>& internalExtensions =
            wgslDesc.internalExtensions.UnsafeGetValue();
        const StringView& wgsl = wgslDesc.wgsl.UnsafeGetValue();
        auto tintFile = std::make_unique<tint::Source::File>("", wgsl);

        DAWN_TRY(ParseWGSL(std::move(tintFile), deviceInfo.wgslAllowedFeatures, internalExtensions,
                           &outputParseResult));
    }

    // Generate reflection information if required and parsed succeed.
    if (outputParseResult.HasTintProgram() && req.needReflection) {
        ReflectShaderUsingTint(deviceInfo, &outputParseResult);
    }

    // Assert everything succeed and we have a Tint program, xor validation error occurs and we only
    // get error with no Tint program (not generated or get removed).
    DAWN_ASSERT(outputParseResult.HasTintProgram() != outputParseResult.HasError());
    return outputParseResult;
}

RequiredBufferSizes ComputeRequiredBufferSizesForLayout(const EntryPointMetadata& entryPoint,
                                                        const PipelineLayoutBase* layout) {
    RequiredBufferSizes bufferSizes;
    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        bufferSizes[group] = GetBindGroupMinBufferSizes(entryPoint.bindings[group],
                                                        layout->GetBindGroupLayout(group));
    }

    return bufferSizes;
}

MaybeError ValidateCompatibilityWithPipelineLayout(DeviceBase* device,
                                                   const EntryPointMetadata& entryPoint,
                                                   const PipelineLayoutBase* layout) {
    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        DAWN_TRY_CONTEXT(ValidateCompatibilityWithBindGroupLayout(
                             device, group, entryPoint, layout->GetBindGroupLayout(group)),
                         "validating the entry-point's compatibility for group %u with %s", group,
                         layout->GetBindGroupLayout(group));
    }

    for (BindGroupIndex group : ~layout->GetBindGroupLayoutsMask()) {
        DAWN_INVALID_IF(entryPoint.bindings[group].size() > 0,
                        "The entry-point uses bindings in group %u but %s doesn't have a "
                        "BindGroupLayout for this index.",
                        group, layout);
    }

    // Validate that filtering samplers are not used with unfilterable textures.
    for (const auto& pair : entryPoint.samplerAndNonSamplerTexturePairs) {
        // Skip non-sampler textures.
        if (pair.sampler == EntryPointMetadata::nonSamplerBindingPoint) {
            continue;
        }

        bool samplerIsFiltering = false;
        {
            const BindGroupLayoutInternalBase* samplerBGL =
                layout->GetBindGroupLayout(pair.sampler.group);
            const BindingInfo& samplerInfo =
                samplerBGL->GetAPIBindingInfo(samplerBGL->GetAPIBindingIndex(pair.sampler.binding));
            if (std::holds_alternative<StaticSamplerBindingInfo>(samplerInfo.bindingLayout)) {
                const StaticSamplerBindingInfo& samplerLayout =
                    std::get<StaticSamplerBindingInfo>(samplerInfo.bindingLayout);
                samplerIsFiltering = samplerLayout.sampler->IsFiltering();
            } else {
                const SamplerBindingInfo& samplerLayout =
                    std::get<SamplerBindingInfo>(samplerInfo.bindingLayout);
                samplerIsFiltering = (samplerLayout.type == wgpu::SamplerBindingType::Filtering);
            }
        }

        wgpu::TextureSampleType sampleType = wgpu::TextureSampleType::Undefined;
        {
            const BindGroupLayoutInternalBase* textureBGL =
                layout->GetBindGroupLayout(pair.texture.group);
            const BindingInfo& textureInfo =
                textureBGL->GetAPIBindingInfo(textureBGL->GetAPIBindingIndex(pair.texture.binding));

            if (std::holds_alternative<ExternalTextureBindingInfo>(textureInfo.bindingLayout)) {
                sampleType = wgpu::TextureSampleType::Float;
            } else {
                const TextureBindingInfo& sampledTextureBindingInfo =
                    std::get<TextureBindingInfo>(textureInfo.bindingLayout);
                sampleType = sampledTextureBindingInfo.sampleType;
            }
        }

        DAWN_INVALID_IF(
            samplerIsFiltering && sampleType != wgpu::TextureSampleType::Float &&
                sampleType != kInternalResolveAttachmentSampleType,
            "Texture binding (group:%u, binding:%u) is %s but used statically with a sampler "
            "(group:%u, binding:%u) that's %s",
            pair.texture.group, pair.texture.binding, sampleType, pair.sampler.group,
            pair.sampler.binding, wgpu::SamplerBindingType::Filtering);
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

    // Validate resource table usage
    DAWN_INVALID_IF(entryPoint.usesResourceTable && !layout->UsesResourceTable(),
                    "The entry-point uses a resource table but the pipeline layout doesn't enable "
                    "`usesResourceTable`.");

    // Validate that immediate data used by programmable state are smaller than pipelineLayout
    // immediate data range bytes.
    DAWN_INVALID_IF(entryPoint.immediateDataRangeByteSize > layout->GetImmediateDataRangeByteSize(),
                    "The entry-point uses more bytes of immediate data (%u) than the reserved "
                    "amount (%u) in %s.",
                    entryPoint.immediateDataRangeByteSize, layout->GetImmediateDataRangeByteSize(),
                    layout);

    return {};
}

MaybeError ValidateSubgroupMatrixConfiguration(const tint::SubgroupMatrixInfo& smInfo,
                                               const std::vector<SubgroupMatrixConfig>& cfg) {
    if (cfg.empty()) {
        DAWN_INVALID_IF(!smInfo.configs.empty(),
                        "Shader uses a subgroup matrix, but no subgroup matrix configuration "
                        "found for the device");
    }

    auto compare = [](wgpu::SubgroupMatrixComponentType wgpu_type,
                      tint::SubgroupMatrixType tint_type) -> bool {
        return (wgpu_type == wgpu::SubgroupMatrixComponentType::F16 &&
                tint_type == tint::SubgroupMatrixType::kF16) ||
               (wgpu_type == wgpu::SubgroupMatrixComponentType::F32 &&
                tint_type == tint::SubgroupMatrixType::kF32) ||
               (wgpu_type == wgpu::SubgroupMatrixComponentType::I8 &&
                tint_type == tint::SubgroupMatrixType::kI8) ||
               (wgpu_type == wgpu::SubgroupMatrixComponentType::U8 &&
                tint_type == tint::SubgroupMatrixType::kU8) ||
               (wgpu_type == wgpu::SubgroupMatrixComponentType::I32 &&
                tint_type == tint::SubgroupMatrixType::kI32) ||
               (wgpu_type == wgpu::SubgroupMatrixComponentType::U32 &&
                tint_type == tint::SubgroupMatrixType::kU32);
    };

    auto SubgroupTypeToString = [](tint::SubgroupMatrixType type) -> std::string {
        switch (type) {
            case tint::SubgroupMatrixType::kF16:
                return "f16";
            case tint::SubgroupMatrixType::kF32:
                return "f32";
            case tint::SubgroupMatrixType::kI8:
                return "i8";
            case tint::SubgroupMatrixType::kU8:
                return "u8";
            case tint::SubgroupMatrixType::kI32:
                return "i32";
            case tint::SubgroupMatrixType::kU32:
                return "u32";
            default:
                DAWN_UNREACHABLE();
        }
    };

    for (auto& info : smInfo.configs) {
        bool found = false;
        for (const auto& config : cfg) {
            if (info.direction == tint::SubgroupMatrixDirection::kResult &&
                compare(config.resultComponentType, info.type) && config.M == info.M &&
                config.N == info.N) {
                found = true;
            } else if (compare(config.componentType, info.type)) {
                if (info.direction == tint::SubgroupMatrixDirection::kLeft && info.M == config.M &&
                    info.K == config.K) {
                    found = true;
                } else if (info.direction == tint::SubgroupMatrixDirection::kRight &&
                           info.N == config.N && info.K == config.K) {
                    found = true;
                }
            }
        }
        DAWN_INVALID_IF(!found,
                        "Subgroup matrix usage found which is not supported by the device.\n"
                        "Unknown configuration is M(%zu), N(%zu), K(%zu), %s",
                        info.M, info.N, info.K, SubgroupTypeToString(info.type));
    }

    for (auto& info : smInfo.multiplies) {
        bool found = false;
        for (const auto& config : cfg) {
            if (config.M == info.M && config.N == info.N && config.K == info.K &&
                compare(config.resultComponentType, info.output_type) &&
                compare(config.componentType, info.input_type)) {
                found = true;
                break;
            }
        }
        DAWN_INVALID_IF(!found,
                        "Subgroup matrix multiplication found which is not "
                        "supported by the device.\n"
                        "Unknown configuration is M(%zu) N(%zu) K(%zu) "
                        "InputType(%s) OutputType(%s).",
                        info.M, info.N, info.K, SubgroupTypeToString(info.input_type),
                        SubgroupTypeToString(info.output_type));
    }

    return {};
}

// ShaderModuleBase
ShaderModuleBase::ShaderModuleBase(DeviceBase* device,
                                   const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                                   std::vector<tint::wgsl::Extension> internalExtensions,
                                   ApiObjectBase::UntrackedByDeviceTag tag)
    : Base(device, ObjectBase::kDelayedInitialization, descriptor->label),
      mType(Type::Undefined),
      mInternalExtensions(std::move(internalExtensions)) {
    size_t shaderCodeByteSize = 0;
    uint8_t* shaderCode = nullptr;

    if (auto* spirvDesc = descriptor.Get<ShaderSourceSPIRV>()) {
        mType = Type::Spirv;
        mOriginalSpirv.assign(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);
        shaderCodeByteSize = mOriginalSpirv.size() * sizeof(decltype(mOriginalSpirv)::value_type);
        shaderCode = reinterpret_cast<uint8_t*>(mOriginalSpirv.data());
        if (auto* spirvOptions = descriptor.Get<DawnShaderModuleSPIRVOptionsDescriptor>()) {
            mAllowSpirvNonUniformDerivitives =
                static_cast<bool>(spirvOptions->allowNonUniformDerivatives);
        }
    } else if (auto* wgslDesc = descriptor.Get<ShaderSourceWGSL>()) {
        mType = Type::Wgsl;
        mWgsl = std::string(wgslDesc->code);
        shaderCodeByteSize = mWgsl.size() * sizeof(decltype(mWgsl)::value_type);
        shaderCode = reinterpret_cast<uint8_t*>(mWgsl.data());
    } else {
        DAWN_ASSERT(false);
    }

    if (const auto* compileOptions = descriptor.Get<ShaderModuleCompilationOptions>()) {
        mStrictMath = compileOptions->strictMath;
    }

    ShaderModuleHasher hasher;
    // Hash the metadata.
    hasher.Update(mType);
    hasher.Update(mAllowSpirvNonUniformDerivitives);
    // mStrictMath is a std::optional<bool>, and the bool value might not get initialized by default
    // constructor and thus contains dirty data.
    bool strictMathAssigned = mStrictMath.has_value();
    bool strictMathValue = mStrictMath.value_or(false);
    hasher.Update(strictMathAssigned);
    hasher.Update(strictMathValue);
    // mInternalExtensions is a length-variable vector, so we need to hash its size and its content
    // if any.
    hasher.Update(mInternalExtensions.size());
    hasher.Update(mInternalExtensions.data(),
                  mInternalExtensions.size() * sizeof(decltype(mInternalExtensions)::value_type));
    // Hash the shader code and its size.
    hasher.Update(shaderCodeByteSize);
    hasher.Update(shaderCode, shaderCodeByteSize);

    mHash = hasher.Finalize();
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device,
                                   const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                                   std::vector<tint::wgsl::Extension> internalExtensions)
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions), kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device,
                                   ObjectBase::ErrorTag tag,
                                   StringView label,
                                   ParsedCompilationMessages&& compilationMessages)
    : Base(device, tag, label), mType(Type::Undefined) {
    mCompiledState.compilationMessages =
        std::make_unique<OwnedCompilationMessages>(std::move(compilationMessages));
}

ShaderModuleBase::~ShaderModuleBase() = default;

void ShaderModuleBase::DestroyImpl(DestroyReason reason) {
    Uncache();
}

// static
Ref<ShaderModuleBase> ShaderModuleBase::MakeError(DeviceBase* device,
                                                  StringView label,
                                                  ParsedCompilationMessages&& compilationMessages) {
    return AcquireRef(
        new ShaderModuleBase(device, ObjectBase::kError, label, std::move(compilationMessages)));
}

void ShaderModuleBase::Initialize() {
    auto task = [&, shaderModuleRef = Ref<ShaderModuleBase>(this)]() {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetDevice()->GetPlatform(), "CreateShaderModuleUS");

        CompiledState resultState;
        auto taskMaybeError = [&resultState, shaderModule = static_cast<const ShaderModuleBase*>(
                                                 this)]() -> MaybeError {
            // Check blob cache first before calling ParseShaderModule. ShaderModuleParseResult
            // returned from blob cache or ParseShaderModule will hold compilation messages and
            // validation errors if any. ShaderModuleParseResult from ParseShaderModule also
            // holds tint program.
            CacheResult<ShaderModuleParseResult> cacheResult;
            DAWN_TRY_LOAD_OR_RUN(cacheResult, shaderModule->GetDevice(),
                                 shaderModule->GenerateShaderModuleParseRequest(true),
                                 ShaderModuleParseResult::FromBlob, ParseShaderModule,
                                 "ShaderModuleParsing");
            shaderModule->GetDevice()->GetBlobCache()->EnsureStored(cacheResult);

            ShaderModuleParseResult parseResult = cacheResult.Acquire();

            // Move the compilation messages regardless of compilation success. Compilation messages
            // should be inject only once for each shader module.
            DAWN_ASSERT(resultState.compilationMessages == nullptr);
            // Move the compilationMessages into the shader module and emit the tint errors and
            // warnings
            resultState.compilationMessages = std::make_unique<OwnedCompilationMessages>(
                std::move(parseResult.compilationMessages));

            // If ShaderModuleParseResult has validation error, notify the caller that compilation
            // failed. The compilation messages have already been stored.
            if (parseResult.HasError()) {
                return parseResult.cachedValidationError->ToErrorData();
            }

            DAWN_ASSERT(!parseResult.HasError());
            if (parseResult.HasTintProgram()) {
                resultState.tintData.Use([&](auto tintData) {
                    tintData->tintProgram =
                        std::move(parseResult.tintProgram.UnsafeGetValue().value());
                });
            }

            // Gather the metadata and default entry point names
            DAWN_ASSERT(parseResult.metadataTable.has_value());
            resultState.entryPoints = std::move(parseResult.metadataTable.value());

            for (auto stage : IterateStages(kAllStages)) {
                resultState.entryPointCounts[stage] = 0;
            }
            for (auto& [name, metadata] : resultState.entryPoints) {
                SingleShaderStage stage = metadata->stage;
                if (resultState.entryPointCounts[stage] == 0) {
                    resultState.defaultEntryPointNames[stage] = name;
                }
                resultState.entryPointCounts[stage]++;
            }

            return {};
        }();

        // Always set mCompiledState, even if the compilation failed. It contains error messages
        // that are used regardless of success.
        mCompiledState = std::move(resultState);

        DAWN_HISTOGRAM_BOOLEAN(GetDevice()->GetPlatform(), "CreateShaderModuleSuccess",
                               taskMaybeError.IsSuccess());
        if (taskMaybeError.IsError()) {
            SetInitializedError();
            mInitializationError = CachedValidationError(taskMaybeError.AcquireError());
        } else {
            SetInitializedNoError();

            // On successful compilation, emit the compilation log
            GetDevice()->EmitCompilationLog(this);
        }
    };

    task();
    DAWN_ASSERT(IsInitialized());
}

std::unique_ptr<ErrorData> ShaderModuleBase::GetInitializationError() {
    DAWN_ASSERT(mInitializationError.has_value());
    return mInitializationError->ToErrorData();
}

ObjectType ShaderModuleBase::GetType() const {
    return ObjectType::ShaderModule;
}

bool ShaderModuleBase::HasEntryPoint(absl::string_view entryPoint) const {
    return mCompiledState.entryPoints.contains(entryPoint);
}

ShaderModuleEntryPoint ShaderModuleBase::ReifyEntryPointName(StringView entryPointName,
                                                             SingleShaderStage stage) const {
    ShaderModuleEntryPoint entryPoint;
    if (entryPointName.IsUndefined()) {
        entryPoint.defaulted = true;
        entryPoint.name = mCompiledState.defaultEntryPointNames[stage];
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
    return *mCompiledState.entryPoints.at(entryPoint);
}

size_t ShaderModuleBase::ComputeContentHash() {
    ObjectContentHasher recorder;
    // Use mHash to represent the source content, which includes shader source and metadata.
    recorder.Record(mHash);
    return recorder.GetContentHash();
}

bool ShaderModuleBase::EqualityFunc::operator()(const ShaderModuleBase* a,
                                                const ShaderModuleBase* b) const {
    bool membersEq = a->mType == b->mType && a->mOriginalSpirv == b->mOriginalSpirv &&
                     a->mWgsl == b->mWgsl && a->mStrictMath == b->mStrictMath;
    // Assert that the hash is equal if and only if the members are equal.
    DAWN_ASSERT(membersEq == (a->mHash == b->mHash));
    return membersEq;
}

const ShaderModuleBase::ShaderModuleHash& ShaderModuleBase::GetHash() const {
    return mHash;
}

ShaderModuleBase::ScopedUseTintProgram ShaderModuleBase::UseTintProgram() {
    // Directly return ScopedUseTintProgram to add ref count. If the mTintProgram is valid,
    // this will prevent it from being released before using. If it is already released,
    // it will be recreated in the GetTintProgram, right before actually using it.
    return ScopedUseTintProgram(this);
}

Ref<TintProgram> ShaderModuleBase::GetTintProgram() {
    return mCompiledState.tintData.Use([&](auto tintData) {
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
        // Assuming ParseShaderModule will not throw error for regenerating.
        ShaderModuleParseResult regeneratedParseResult =
            ParseShaderModule(GenerateShaderModuleParseRequest(/* needReflection */ false))
                .AcquireSuccess();
        DAWN_ASSERT(regeneratedParseResult.HasTintProgram() && !regeneratedParseResult.HasError());

        tintData->tintProgram =
            std::move(regeneratedParseResult.tintProgram.UnsafeGetValue().value());
        tintData->tintProgramRecreateCount++;

        return tintData->tintProgram;
    });
}

Ref<TintProgram> ShaderModuleBase::GetNullableTintProgramForTesting() const {
    return mCompiledState.tintData.Use([&](auto tintData) { return tintData->tintProgram; });
}

int ShaderModuleBase::GetTintProgramRecreateCountForTesting() const {
    return mCompiledState.tintData.Use(
        [&](auto tintData) { return tintData->tintProgramRecreateCount; });
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
                compilationInfo = mShaderModule->GetCompilationMessages()->GetCompilationInfo();
            }

            mCallback(status, ToAPI(compilationInfo), mUserdata1.ExtractAsDangling(),
                      mUserdata2.ExtractAsDangling());
        }
    };
    FutureID futureID = GetDevice()->GetInstance()->GetEventManager()->TrackEvent(
        AcquireRef(new CompilationInfoEvent(callbackInfo, this)));
    return {futureID};
}

const OwnedCompilationMessages* ShaderModuleBase::GetCompilationMessages() const {
    return mCompiledState.compilationMessages.get();
}

std::string ShaderModuleBase::GetCompilationLog() const {
    DAWN_ASSERT(mCompiledState.compilationMessages);
    if (!mCompiledState.compilationMessages->HasWarningsOrErrors()) {
        return "";
    }

    // Emit the formatted Tint errors and warnings.
    std::ostringstream t;
    t << absl::StrFormat("Compilation log for %s:\n", this);
    for (const auto& pMessage : mCompiledState.compilationMessages->GetFormattedTintMessages()) {
        t << "\n" << pMessage;
    }

    return t.str();
}

void ShaderModuleBase::SetCompilationMessagesForTesting(
    std::unique_ptr<OwnedCompilationMessages>* compilationMessages) {
    mCompiledState.compilationMessages = std::move(*compilationMessages);
}

void ShaderModuleBase::WillDropLastExternalRef() {
    // The last external ref being dropped indicates that the application is not currently using,
    // and no pending task will use the shader module. In this case we can free the memory for the
    // parsed module.
    mCompiledState.tintData.Use([&](auto tintData) { tintData->tintProgram = nullptr; });
}

ShaderModuleParseRequest ShaderModuleBase::GenerateShaderModuleParseRequest(
    bool needReflection) const {
    ShaderModuleDescriptor descriptor;
    ShaderSourceWGSL wgslDescriptor;
    ShaderSourceSPIRV spirvDescriptor;
    DawnShaderModuleSPIRVOptionsDescriptor spirvOptionsDescriptor;

    switch (mType) {
        case Type::Spirv:
            spirvOptionsDescriptor.allowNonUniformDerivatives = mAllowSpirvNonUniformDerivitives;
            spirvDescriptor.nextInChain = &spirvOptionsDescriptor;

            spirvDescriptor.codeSize = uint32_t(mOriginalSpirv.size());
            spirvDescriptor.code = mOriginalSpirv.data();
            descriptor.nextInChain = &spirvDescriptor;
            break;
        case Type::Wgsl:
            wgslDescriptor.code = std::string_view(mWgsl);
            descriptor.nextInChain = &wgslDescriptor;
            break;
        default:
            DAWN_UNREACHABLE();
    }

    return BuildShaderModuleParseRequest(GetDevice(), mHash, Unpack(&descriptor),
                                         mInternalExtensions, needReflection);
}

ShaderModuleBase::CompiledState& ShaderModuleBase::CompiledState::operator=(
    ShaderModuleBase::CompiledState&& source) {
    entryPoints = std::move(source.entryPoints);
    defaultEntryPointNames = std::move(source.defaultEntryPointNames);
    entryPointCounts = std::move(source.entryPointCounts);

    TintData sourceTintData;
    source.tintData.Use(
        [&sourceTintData](auto tintData) { sourceTintData = std::move(*tintData); });
    tintData.Use(
        [&sourceTintData](auto destTintData) { *destTintData = std::move(sourceTintData); });

    compilationMessages = std::move(source.compilationMessages);

    return *this;
}
}  // namespace dawn::native
