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

#include "dawn/native/opengl/ShaderModuleGL.h"

#include <sstream>
#include <unordered_map>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/CacheRequest.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/opengl/BindGroupLayoutGL.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/PipelineGL.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/native/opengl/UtilsGL.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "tint/api/common/binding_point.h"

namespace dawn::native::opengl {
namespace {
using InterstageLocationAndName = std::pair<uint32_t, std::string>;
using SubstituteOverrideConfig = std::unordered_map<tint::OverrideId, double>;

#define GLSL_COMPILATION_REQUEST_MEMBERS(X)                                          \
    X(ShaderModuleBase::ShaderModuleHash, shaderModuleHash)                          \
    X(UnsafeUnserializedValue<ShaderModuleBase::ScopedUseTintProgram>, inputProgram) \
    X(std::string, entryPointName)                                                   \
    X(SingleShaderStage, stage)                                                      \
    X(SubstituteOverrideConfig, substituteOverrideConfig)                            \
    X(LimitsForCompilationRequest, limits)                                           \
    X(UnsafeUnserializedValue<LimitsForCompilationRequest>, adapterSupportedLimits)  \
    X(bool, disableSymbolRenaming)                                                   \
    X(std::vector<InterstageLocationAndName>, interstageVariables)                   \
    X(tint::glsl::writer::Options, tintOptions)                                      \
    X(UnsafeUnserializedValue<dawn::platform::Platform*>, platform)
DAWN_MAKE_CACHE_REQUEST(GLSLCompilationRequest, GLSL_COMPILATION_REQUEST_MEMBERS);
#undef GLSL_COMPILATION_REQUEST_MEMBERS

#define GLSL_COMPILATION_MEMBERS(X) X(std::string, glsl)
DAWN_SERIALIZABLE(struct, GLSLCompilation, GLSL_COMPILATION_MEMBERS) {
    static ResultOrError<GLSLCompilation> FromValidatedBlob(Blob blob);
};
#undef GLSL_COMPILATION_MEMBERS

// Separated from the class definition above to fix a clang-format over-indentation.
// static
ResultOrError<GLSLCompilation> GLSLCompilation::FromValidatedBlob(Blob blob) {
    GLSLCompilation result;
    DAWN_TRY_ASSIGN(result, FromBlob(std::move(blob)));
    DAWN_INVALID_IF(result.glsl.empty(), "Cached GLSLCompilation result has no GLSL");
    return result;
}

GLenum GLShaderType(SingleShaderStage stage) {
    switch (stage) {
        case SingleShaderStage::Vertex:
            return GL_VERTEX_SHADER;
        case SingleShaderStage::Fragment:
            return GL_FRAGMENT_SHADER;
        case SingleShaderStage::Compute:
            return GL_COMPUTE_SHADER;
    }
    DAWN_UNREACHABLE();
}

tint::glsl::writer::Version::Standard ToTintGLStandard(opengl::OpenGLVersion::Standard standard) {
    switch (standard) {
        case OpenGLVersion::Standard::Desktop:
            return tint::glsl::writer::Version::Standard::kDesktop;
        case OpenGLVersion::Standard::ES:
            return tint::glsl::writer::Version::Standard::kES;
    }
    DAWN_UNREACHABLE();
}

using BindingMap = absl::flat_hash_map<tint::BindingPoint, tint::BindingPoint>;

// Returns information about the texture/sampler pairs used by the entry point. This is necessary
// because GL uses combined texture/sampler bindings while WGSL allows mixing and matching textures
// and samplers in the shader. GL also uses a placeholder sampler to use with textures when they
// aren't combined with a sampler in the WGSL.
//
// Another subtlety is that Dawn uses pre-remapping BindingPoints when referring to bindings while
// the Tint GLSL writer uses post-remapping BindingPoints.
void GenerateCombinedSamplerInfo(
    const EntryPointMetadata& metadata,
    const tint::glsl::writer::Bindings& bindings,
    const BindingMap& externalTextureExpansionMap,
    std::vector<CombinedSampler>* combinedSamplers,
    tint::glsl::writer::CombinedTextureSamplerInfo* samplerTextureToName) {
    // Helper to avoid duplicated logic for when a CombinedSampler is determined.
    auto AddCombinedSampler = [&](tint::BindingPoint textureWGSL,
                                  tint::BindingPoint textureRemapped,
                                  std::optional<tint::BindingPoint> samplerWGSL,
                                  BindingIndex textureArraySize, bool isPlane1 = false) {
        // Dawn needs pre-remapping WGSL bind points.
        CombinedSampler combinedSampler = {{
            .samplerLocation = std::nullopt,
            .textureLocation = {{
                .group = BindGroupIndex(textureWGSL.group),
                .binding = BindingNumber(textureWGSL.binding),
                .arraySize = textureArraySize,
            }},
        }};
        if (samplerWGSL.has_value()) {
            combinedSampler.samplerLocation = {{{
                .group = BindGroupIndex(samplerWGSL->group),
                .binding = BindingNumber(samplerWGSL->binding),
            }}};
        }
        combinedSamplers->push_back(combinedSampler);

        // Tint uses post-remapping bind points.
        tint::BindingPoint samplerRemapped = bindings.placeholder_sampler_bind_point;
        if (samplerWGSL.has_value()) {
            samplerRemapped = {.group = 0,
                               .binding = bindings.sampler.at(samplerWGSL.value()).binding};
        }

        samplerTextureToName->emplace(
            tint::glsl::writer::CombinedTextureSamplerPair{textureRemapped, samplerRemapped,
                                                           isPlane1},
            combinedSampler.GetName());
    };

    for (const auto& use : metadata.samplerAndNonSamplerTexturePairs) {
        // Replace uses of the placeholder sampler with its actual binding point.
        std::optional<tint::BindingPoint> sampler = std::nullopt;
        if (use.sampler != EntryPointMetadata::nonSamplerBindingPoint) {
            sampler = ToTint(use.sampler);
        }

        // Tint reflection returns information about uses of both regular textures and sampled
        // textures so we need to differentiate both cases here.

        // The easy case is when a regular texture is being handled.
        if (!externalTextureExpansionMap.contains(ToTint(use.texture))) {
            tint::BindingPoint textureWGSL = ToTint(use.texture);
            tint::BindingPoint textureRemapped = {0, bindings.texture.at(textureWGSL).binding};
            BindingIndex arraySizeInShader = metadata.bindings.at(BindGroupIndex(textureWGSL.group))
                                                 .at(BindingNumber(textureWGSL.binding))
                                                 .arraySize;
            AddCombinedSampler(textureWGSL, textureRemapped, sampler, arraySizeInShader);
            continue;
        }

        // Add plane 0 of the external texture (this happen to be the same code as for regular
        // textures because plane0 uses the original WGSL bind point).
        tint::BindingPoint plane0WGSL = ToTint(use.texture);
        tint::BindingPoint plane0Remapped = {
            0, bindings.external_texture.at(plane0WGSL).plane0.binding};
        AddCombinedSampler(plane0WGSL, plane0Remapped, sampler, BindingIndex(1));

        // Plane 1 needs its pre-remapping bind point queried from the expansion map.
        tint::BindingPoint plane1WGSL = externalTextureExpansionMap.at(plane0WGSL);
        tint::BindingPoint plane1Remapped = {
            0, bindings.external_texture.at(plane0WGSL).plane1.binding};
        AddCombinedSampler(plane1WGSL, plane1Remapped, sampler, BindingIndex(1), true);
    }
}

// Returns whether the stage uses any texture builtin metadata.
void GenerateTextureBuiltinFromUniformData(
    const EntryPointMetadata& metadata,
    const PipelineLayout* layout,
    const tint::glsl::writer::Bindings& bindings,
    EmulatedTextureBuiltinRegistrar* emulatedTextureBuiltins,
    tint::glsl::writer::TextureBuiltinsFromUniformOptions* textureBuiltinsFromUniform) {
    // Tell Tint where the uniform containing the builtin data will be (in post-remapping space),
    // only when this shader stage uses some builtin metadata.
    if (!metadata.textureQueries.empty()) {
        textureBuiltinsFromUniform->ubo_binding = {
            uint32_t(layout->GetInternalTextureBuiltinsUniformBinding())};
    }

    for (auto [i, query] : Enumerate(metadata.textureQueries)) {
        BindGroupIndex group = BindGroupIndex(query.group);
        const auto* bgl = layout->GetBindGroupLayout(group);
        BindingIndex binding = bgl->GetBindingIndex(BindingNumber{query.binding});

        // Register that the query needs to be emulated and get the offset in the UBO where the data
        // will be passed.
        TextureQuery textureQuery;
        switch (query.type) {
            case EntryPointMetadata::TextureMetadataQuery::TextureQueryType::TextureNumLevels:
                textureQuery = TextureQuery::NumLevels;
                break;
            case EntryPointMetadata::TextureMetadataQuery::TextureQueryType::TextureNumSamples:
                textureQuery = TextureQuery::NumSamples;
                break;
        }
        uint32_t offset = emulatedTextureBuiltins->Register(group, binding, textureQuery);

        // Tint uses post-remapping binding points for textureBuiltinFromUniform options.
        tint::BindingPoint wgslBindPoint = {.group = query.group, .binding = query.binding};

        tint::glsl::writer::BindingInfo remappedBinding;
        if (bindings.texture.contains(wgslBindPoint)) {
            remappedBinding = bindings.texture.at(wgslBindPoint);
        } else {
            remappedBinding = bindings.storage_texture.at(wgslBindPoint);
        }
        textureBuiltinsFromUniform->ubo_contents.push_back(
            {.offset = offset, .count = 1, .binding = remappedBinding});
    }
}

bool GenerateArrayLengthFromuniformData(const BindingInfoArray& moduleBindingInfo,
                                        const PipelineLayout* layout,
                                        tint::glsl::writer::Bindings& bindings) {
    const PipelineLayout::BindingIndexInfo& indexInfo = layout->GetBindingIndexInfo();

    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayoutInternalBase* bgl = layout->GetBindGroupLayout(group);
        for (const auto& [binding, shaderBindingInfo] : moduleBindingInfo[group]) {
            BindingIndex bindingIndex = bgl->GetBindingIndex(binding);
            const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);

            // TODO(crbug.com/408010433): capturing binding directly in lambda is C++20
            // extension in cmake
            uint32_t capturedBindingNumber = static_cast<uint32_t>(binding);

            MatchVariant(
                bindingInfo.bindingLayout,
                [&](const BufferBindingInfo& bufferBinding) {
                    switch (bufferBinding.type) {
                        case wgpu::BufferBindingType::Storage:
                        case kInternalStorageBufferBinding:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                        case kInternalReadOnlyStorageBufferBinding: {
                            // Use ssbo index as the indices for the buffer size lookups
                            // in the array length from uniform transform.
                            tint::BindingPoint srcBindingPoint = {static_cast<uint32_t>(group),
                                                                  capturedBindingNumber};
                            FlatBindingIndex ssboIndex = indexInfo[group][bindingIndex];
                            bindings.array_length_from_uniform.bindpoint_to_size_index.emplace(
                                srcBindingPoint, uint32_t(ssboIndex));
                            break;
                        }
                        default:
                            break;
                    }
                },
                [](const StaticSamplerBindingInfo&) {}, [](const SamplerBindingInfo&) {},
                [](const TextureBindingInfo&) {}, [](const StorageTextureBindingInfo&) {},
                [](const InputAttachmentBindingInfo&) {});
        }
    }

    return bindings.array_length_from_uniform.bindpoint_to_size_index.size() > 0;
}

}  // namespace

std::string GetBindingName(BindGroupIndex group, BindingNumber bindingNumber) {
    std::ostringstream o;
    o << "dawn_binding_" << static_cast<uint32_t>(group) << "_"
      << static_cast<uint32_t>(bindingNumber);
    return o.str();
}

bool operator<(const CombinedSamplerElement& a, const CombinedSamplerElement& b) {
    return std::tie(a.group, a.binding, a.arraySize) < std::tie(b.group, b.binding, b.arraySize);
}

bool operator<(const CombinedSampler& a, const CombinedSampler& b) {
    return std::tie(a.samplerLocation, a.textureLocation) <
           std::tie(b.samplerLocation, b.textureLocation);
}

std::string CombinedSampler::GetName() const {
    std::ostringstream o;
    o << "dawn_combined";
    if (!samplerLocation) {
        o << "_placeholder_sampler";
    } else {
        o << "_" << static_cast<uint32_t>(samplerLocation->group) << "_"
          << static_cast<uint32_t>(samplerLocation->binding);
    }
    o << "_with_" << static_cast<uint32_t>(textureLocation.group) << "_"
      << static_cast<uint32_t>(textureLocation.binding);
    return o.str();
}

// static
ResultOrError<Ref<ShaderModule>> ShaderModule::Create(
    Device* device,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    ShaderModuleParseResult* parseResult) {
    Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor, internalExtensions));
    DAWN_TRY(module->Initialize(parseResult));
    return module;
}

ShaderModule::ShaderModule(Device* device,
                           const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                           std::vector<tint::wgsl::Extension> internalExtensions)
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions)) {}

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult) {
    DAWN_TRY(InitializeBase(parseResult));

    return {};
}

std::pair<tint::glsl::writer::Bindings, BindingMap> GenerateBindingInfo(
    SingleShaderStage stage,
    const PipelineLayout* layout,
    const BindingInfoArray& moduleBindingInfo,
    GLSLCompilationRequest& req) {
    // Because of the way the rest of the backend uses the binding information, we need to pass
    // through the original WGSL values in the combined shader map. That means, we need to store
    // that data for the external texture, otherwise it ends up getting lost.
    BindingMap externalTextureExpansionMap;

    tint::glsl::writer::Bindings bindings;

    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));

        for (const auto& [binding, shaderBindingInfo] : moduleBindingInfo[group]) {
            tint::BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                               static_cast<uint32_t>(binding)};

            BindingIndex bindingIndex = bgl->GetBindingIndex(binding);
            const auto& bindingIndexInfo = layout->GetBindingIndexInfo()[group];
            FlatBindingIndex shaderIndex = bindingIndexInfo[bindingIndex];
            tint::BindingPoint dstBindingPoint{0, uint32_t(shaderIndex)};

            auto* const bufferBindingInfo =
                std::get_if<BufferBindingInfo>(&shaderBindingInfo.bindingInfo);

            if (bufferBindingInfo) {
                switch (bufferBindingInfo->type) {
                    case wgpu::BufferBindingType::Uniform:
                        bindings.uniform.emplace(srcBindingPoint, tint::glsl::writer::BindingInfo{
                                                                      dstBindingPoint.binding});
                        break;
                    case kInternalStorageBufferBinding:
                    case wgpu::BufferBindingType::Storage:
                    case wgpu::BufferBindingType::ReadOnlyStorage:
                    case kInternalReadOnlyStorageBufferBinding:
                        bindings.storage.emplace(srcBindingPoint, tint::glsl::writer::BindingInfo{
                                                                      dstBindingPoint.binding});
                        break;
                    case wgpu::BufferBindingType::BindingNotUsed:
                    case wgpu::BufferBindingType::Undefined:
                        DAWN_UNREACHABLE();
                        break;
                }
            } else if (std::holds_alternative<SamplerBindingInfo>(shaderBindingInfo.bindingInfo)) {
                bindings.sampler.emplace(srcBindingPoint,
                                         tint::glsl::writer::BindingInfo{dstBindingPoint.binding});
            } else if (std::holds_alternative<TextureBindingInfo>(shaderBindingInfo.bindingInfo)) {
                bindings.texture.emplace(srcBindingPoint,
                                         tint::glsl::writer::BindingInfo{dstBindingPoint.binding});
            } else if (std::holds_alternative<StorageTextureBindingInfo>(
                           shaderBindingInfo.bindingInfo)) {
                bindings.storage_texture.emplace(
                    srcBindingPoint, tint::glsl::writer::BindingInfo{dstBindingPoint.binding});
            } else if (std::holds_alternative<ExternalTextureBindingInfo>(
                           shaderBindingInfo.bindingInfo)) {
                const auto& etBindingMap = bgl->GetExternalTextureBindingExpansionMap();
                const auto& expansion = etBindingMap.find(binding);
                DAWN_ASSERT(expansion != etBindingMap.end());

                using BindingInfo = tint::glsl::writer::BindingInfo;

                const auto& bindingExpansion = expansion->second;
                const BindingInfo plane0{
                    uint32_t(bindingIndexInfo[bgl->GetBindingIndex(bindingExpansion.plane0)])};
                const BindingInfo plane1{
                    uint32_t(bindingIndexInfo[bgl->GetBindingIndex(bindingExpansion.plane1)])};
                const BindingInfo metadata{
                    uint32_t(bindingIndexInfo[bgl->GetBindingIndex(bindingExpansion.params)])};

                tint::BindingPoint plane1WGSLBindingPoint{
                    static_cast<uint32_t>(group), static_cast<uint32_t>(bindingExpansion.plane1)};
                externalTextureExpansionMap[srcBindingPoint] = plane1WGSLBindingPoint;

                bindings.external_texture.emplace(
                    srcBindingPoint, tint::glsl::writer::ExternalTexture{metadata, plane0, plane1});
            }
        }
    }
    return {bindings, externalTextureExpansionMap};
}

ResultOrError<GLuint> ShaderModule::CompileShader(
    const OpenGLFunctions& gl,
    const ProgrammableStage& programmableStage,
    SingleShaderStage stage,
    bool usesVertexIndex,
    bool usesInstanceIndex,
    bool usesFragDepth,
    VertexAttributeMask bgraSwizzleAttributes,
    std::vector<CombinedSampler>* combinedSamplersOut,
    const PipelineLayout* layout,
    EmulatedTextureBuiltinRegistrar* emulatedTextureBuiltins,
    bool* needsSSBOLengthUniformBuffer) {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "TranslateToGLSL");

    const OpenGLVersion& version = ToBackend(GetDevice())->GetGL().GetVersion();

    GLSLCompilationRequest req = {};

    req.shaderModuleHash = GetHash();
    req.inputProgram = UnsafeUnserializedValue(UseTintProgram());

    // Since (non-Vulkan) GLSL does not support descriptor sets, generate a mapping from the
    // original group/binding pair to a binding-only value. This mapping will be used by Tint to
    // remap all global variables to the 1D space.
    const EntryPointMetadata& entryPointMetaData = GetEntryPoint(programmableStage.entryPoint);
    const BindingInfoArray& moduleBindingInfo = entryPointMetaData.bindings;

    auto [bindings, externalTextureExpansionMap] =
        GenerateBindingInfo(stage, layout, moduleBindingInfo, req);

    // When textures are accessed without a sampler (e.g., textureLoad()), returned
    // CombinedSamplerInfo should use this sentinel value as sampler binding point.
    bindings.placeholder_sampler_bind_point = {static_cast<uint32_t>(kMaxBindGroupsTyped), 0};

    // Compute the metadata necessary for translating to GL's combined textures and samplers, both
    // for Dawn and for the Tint translation to GLSL.
    {
        std::vector<CombinedSampler> combinedSamplers;
        tint::glsl::writer::CombinedTextureSamplerInfo samplerTextureToName;
        GenerateCombinedSamplerInfo(entryPointMetaData, bindings, externalTextureExpansionMap,
                                    &combinedSamplers, &samplerTextureToName);

        bindings.sampler_texture_to_name = std::move(samplerTextureToName);
        *combinedSamplersOut = std::move(combinedSamplers);
    }

    // Compute the metadata necessary to emulate some of the texture "getter" builtins not present
    // in GLSL, both for Dawn and for the Tint translation to GLSL.
    {
        tint::glsl::writer::TextureBuiltinsFromUniformOptions textureBuiltinsFromUniform;
        GenerateTextureBuiltinFromUniformData(entryPointMetaData, layout, bindings,
                                              emulatedTextureBuiltins, &textureBuiltinsFromUniform);
        bindings.texture_builtins_from_uniform = std::move(textureBuiltinsFromUniform);
    }

    req.stage = stage;
    req.entryPointName = programmableStage.entryPoint;
    req.substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    req.limits = LimitsForCompilationRequest::Create(GetDevice()->GetLimits().v1);
    req.adapterSupportedLimits = UnsafeUnserializedValue(
        LimitsForCompilationRequest::Create(GetDevice()->GetAdapter()->GetLimits().v1));

    if (GetDevice()->IsToggleEnabled(Toggle::GLUseArrayLengthFromUniform)) {
        *needsSSBOLengthUniformBuffer =
            GenerateArrayLengthFromuniformData(moduleBindingInfo, layout, bindings);
        if (*needsSSBOLengthUniformBuffer) {
            req.tintOptions.use_array_length_from_uniform = true;
            bindings.array_length_from_uniform.ubo_binding = {kMaxBindGroups + 2, 0};
            bindings.uniform.emplace(bindings.array_length_from_uniform.ubo_binding,
                                     tint::glsl::writer::BindingInfo{
                                         uint32_t(layout->GetInternalArrayLengthUniformBinding())});
        }
    }

    req.platform = UnsafeUnserializedValue(GetDevice()->GetPlatform());

    req.tintOptions.version = tint::glsl::writer::Version(ToTintGLStandard(version.GetStandard()),
                                                          version.GetMajor(), version.GetMinor());

    req.tintOptions.disable_robustness = false;

    if (usesVertexIndex) {
        req.tintOptions.first_vertex_offset = 4 * PipelineLayout::ImmediateLocation::FirstVertex;
    }

    if (usesInstanceIndex) {
        req.tintOptions.first_instance_offset =
            4 * PipelineLayout::ImmediateLocation::FirstInstance;
    }

    if (usesFragDepth) {
        req.tintOptions.depth_range_offsets = {4 * PipelineLayout::ImmediateLocation::MinDepth,
                                               4 * PipelineLayout::ImmediateLocation::MaxDepth};
    }

    if (stage == SingleShaderStage::Vertex) {
        for (VertexAttributeLocation i : bgraSwizzleAttributes) {
            req.tintOptions.bgra_swizzle_locations.insert(static_cast<uint8_t>(i));
        }
    }

    req.tintOptions.strip_all_names = !GetDevice()->IsToggleEnabled(Toggle::DisableSymbolRenaming);

    req.interstageVariables = {};
    for (size_t i = 0; i < entryPointMetaData.interStageVariables.size(); i++) {
        if (entryPointMetaData.usedInterStageVariables[i]) {
            req.interstageVariables.emplace_back(static_cast<uint32_t>(i),
                                                 entryPointMetaData.interStageVariables[i].name);
        }
    }

    req.tintOptions.bindings = std::move(bindings);
    req.tintOptions.disable_polyfill_integer_div_mod =
        GetDevice()->IsToggleEnabled(Toggle::DisablePolyfillsOnIntegerDivisonAndModulo);

    req.tintOptions.enable_integer_range_analysis =
        GetDevice()->IsToggleEnabled(Toggle::EnableIntegerRangeAnalysisInRobustness);

    CacheResult<GLSLCompilation> compilationResult;
    DAWN_TRY_LOAD_OR_RUN(
        compilationResult, GetDevice(), std::move(req), GLSLCompilation::FromValidatedBlob,
        [](GLSLCompilationRequest r) -> ResultOrError<GLSLCompilation> {
            // Requires Tint Program here right before actual using.
            auto inputProgram = r.inputProgram.UnsafeGetValue()->GetTintProgram();
            const tint::Program* tintInputProgram = &(inputProgram->program);
            // Convert the AST program to an IR module.
            tint::Result<tint::core::ir::Module> ir;
            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleProgramToIR");
                ir = tint::wgsl::reader::ProgramToLoweredIR(*tintInputProgram);
                DAWN_INVALID_IF(ir != tint::Success,
                                "An error occurred while generating Tint IR\n%s",
                                ir.Failure().reason);
            }

            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleSingleEntryPoint");
                auto singleEntryPointResult =
                    tint::core::ir::transform::SingleEntryPoint(ir.Get(), r.entryPointName);
                DAWN_INVALID_IF(singleEntryPointResult != tint::Success,
                                "Pipeline single entry point (IR) failed:\n%s",
                                singleEntryPointResult.Failure().reason);
            }

            // this needs to run after SingleEntryPoint transform which removes unused
            // overrides for the current entry point.

            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleSubstituteOverrides");
                tint::core::ir::transform::SubstituteOverridesConfig cfg;
                cfg.map = std::move(r.substituteOverrideConfig);
                auto substituteOverridesResult =
                    tint::core::ir::transform::SubstituteOverrides(ir.Get(), cfg);
                DAWN_INVALID_IF(substituteOverridesResult != tint::Success,
                                "Pipeline override substitution (IR) failed:\n%s",
                                substituteOverridesResult.Failure().reason);
            }

            tint::Result<tint::glsl::writer::Output> result;
            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleGenerateGLSL");
                // Generate GLSL from Tint IR.
                result = tint::glsl::writer::Generate(ir.Get(), r.tintOptions);
                DAWN_INVALID_IF(result != tint::Success,
                                "An error occurred while generating GLSL:\n%s",
                                result.Failure().reason);
            }

            // Workgroup validation has to come after `Generate` because it may require
            // overrides to have been substituted.
            if (r.stage == SingleShaderStage::Compute) {
                // Validate workgroup size after program runs transforms.
                Extent3D _;
                DAWN_TRY_ASSIGN(_,
                                ValidateComputeStageWorkgroupSize(
                                    result->workgroup_info.x, result->workgroup_info.y,
                                    result->workgroup_info.z, result->workgroup_info.storage_size,
                                    /* usesSubgroupMatrix */ false,
                                    /* maxSubgroupSize, GL backend not support */ 0, r.limits,
                                    r.adapterSupportedLimits.UnsafeGetValue()));
            }

            return GLSLCompilation{{std::move(result->glsl)}};
        },
        "OpenGL.CompileShaderToGLSL");

    if (GetDevice()->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated GLSL */\n" << compilationResult->glsl;

        GetDevice()->EmitLog(wgpu::LoggingType::Info, dumpedMsg.str().c_str());
    }

    GLuint shader = DAWN_GL_TRY(gl, CreateShader(GLShaderType(stage)));
    const char* source = compilationResult->glsl.c_str();
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetDevice()->GetPlatform(), "GLSL.CompileShader");

        DAWN_GL_TRY(gl, ShaderSource(shader, 1, &source, nullptr));
        DAWN_GL_TRY(gl, CompileShader(shader));
    }

    GLint compileStatus = GL_FALSE;
    DAWN_GL_TRY(gl, GetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus));
    if (compileStatus == GL_FALSE) {
        GLint infoLogLength = 0;
        DAWN_GL_TRY(gl, GetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength));

        if (infoLogLength > 1) {
            std::vector<char> buffer(infoLogLength);
            DAWN_GL_TRY(gl, GetShaderInfoLog(shader, infoLogLength, nullptr, &buffer[0]));
            DAWN_GL_TRY(gl, DeleteShader(shader));
            return DAWN_VALIDATION_ERROR("%s\nProgram compilation failed:\n%s", source,
                                         buffer.data());
        }
    }

    GetDevice()->GetBlobCache()->EnsureStored(compilationResult);

    return shader;
}

}  // namespace dawn::native::opengl
