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
#include "dawn/native/ImmediateConstantsLayout.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/opengl/BindGroupLayoutGL.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/PipelineGL.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/native/opengl/UtilsGL.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "tint/tint.h"

namespace dawn::native::opengl {
namespace {
using InterstageLocationAndName = std::pair<uint32_t, std::string>;

#define GLSL_COMPILATION_REQUEST_MEMBERS(X)                                          \
    X(ShaderModuleBase::ShaderModuleHash, shaderModuleHash)                          \
    X(UnsafeUnserializedValue<ShaderModuleBase::ScopedUseTintProgram>, inputProgram) \
    X(SingleShaderStage, stage)                                                      \
    X(LimitsForCompilationRequest, limits)                                           \
    X(UnsafeUnserializedValue<LimitsForCompilationRequest>, adapterSupportedLimits)  \
    X(bool, disableSymbolRenaming)                                                   \
    X(std::vector<InterstageLocationAndName>, interstageVariables)                   \
    X(tint::glsl::writer::Options, tintOptions)                                      \
    X(UnsafeUnserializedValue<dawn::platform::Platform*>, platform)
DAWN_MAKE_CACHE_REQUEST(GLSLCompilationRequest, GLSL_COMPILATION_REQUEST_MEMBERS);
#undef GLSL_COMPILATION_REQUEST_MEMBERS

#define GLSL_COMPILATION_MEMBERS(X) \
    X(std::string, glsl)            \
    X(Extent3D, workgroupSize)
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

// Returns information about the texture/sampler pairs used by the entry point. This is necessary
// because GL uses combined texture/sampler bindings while WGSL allows mixing and matching textures
// and samplers in the shader. GL also uses a placeholder sampler to use with textures when they
// aren't combined with a sampler in the WGSL.
//
// Another subtlety is that Dawn uses pre-remapping BindingPoints when referring to bindings while
// the Tint GLSL writer uses post-remapping BindingPoints.
void GenerateCombinedSamplerInfo(
    const EntryPointMetadata& metadata,
    const tint::Bindings& bindings,
    const PipelineLayout* layout,
    std::vector<CombinedSampler>* combinedSamplers,
    tint::glsl::writer::CombinedTextureSamplerInfo* samplerTextureToName,
    tint::BindingPoint* placeholder_sampler_bind_point) {
    // Helper to avoid duplicated logic for when a CombinedSampler is determined. It takes a bunch
    // of information for both the texture and the sampler and translate to what Dawn/Tint need.
    struct CombinedBindingInfo {
        // Dawn takes BindGroupIndex + BindingIndex.
        BindGroupIndex group;
        BindingIndex index;
        BindingIndex shaderArraySize = BindingIndex(1);

        // Tint takes the post-remapping binding point.
        tint::BindingPoint remappedBinding;
    };
    auto AddCombinedSampler = [&](CombinedBindingInfo texture,
                                  std::optional<CombinedBindingInfo> sampler,
                                  bool isPlane1 = false) {
        // Reflect to the pipeline the combination with BindGroupIndex + BindingIndex in that BGL.
        CombinedSampler combinedSampler = {{
            .samplerLocation = std::nullopt,
            .textureLocation = {{
                .group = texture.group,
                .index = texture.index,
                .shaderArraySize = texture.shaderArraySize,
            }},
        }};
        if (sampler.has_value()) {
            combinedSampler.samplerLocation = {{{
                .group = sampler->group,
                .index = sampler->index,
                .shaderArraySize = sampler->shaderArraySize,
            }}};
        }
        combinedSamplers->push_back(combinedSampler);

        // Let Tint know to generate a new GLSL sampler for this combination.
        tint::BindingPoint samplerRemapped = *placeholder_sampler_bind_point;
        if (sampler.has_value()) {
            samplerRemapped = {0, sampler->remappedBinding.binding};
        }
        samplerTextureToName->emplace(
            tint::glsl::writer::CombinedTextureSamplerPair{
                {0, texture.remappedBinding.binding}, samplerRemapped, isPlane1},
            combinedSampler.GetName());
    };

    for (const auto& use : metadata.samplerAndNonSamplerTexturePairs) {
        // Replace uses of the placeholder sampler with its actual binding point.
        std::optional<CombinedBindingInfo> sampler = std::nullopt;
        if (use.sampler != EntryPointMetadata::nonSamplerBindingPoint) {
            const BindGroupLayoutInternalBase* bgl = layout->GetBindGroupLayout(use.sampler.group);
            sampler = {
                .group = use.sampler.group,
                .index = bgl->AsBindingIndex(bgl->GetBindingMap().at(use.sampler.binding)),
                .remappedBinding = bindings.sampler.at(ToTint(use.sampler)),
            };
        }

        // Tint reflection returns information about uses of both regular textures and sampled
        // textures so we need to differentiate both cases here.
        const BindGroupLayoutInternalBase* bgl = layout->GetBindGroupLayout(use.texture.group);
        APIBindingIndex textureAPIIndex = bgl->GetBindingMap().at(use.texture.binding);
        const auto& bindingInfo = bgl->GetAPIBindingInfo(textureAPIIndex);

        // The easy case is when a regular texture is being handled.
        if (std::holds_alternative<TextureBindingInfo>(bindingInfo.bindingLayout)) {
            CombinedBindingInfo texture = {
                .group = use.texture.group,
                .index = bgl->AsBindingIndex(textureAPIIndex),
                .shaderArraySize =
                    metadata.bindings.at(use.texture.group).at(use.texture.binding).arraySize,
                .remappedBinding = bindings.texture.at(ToTint(use.texture)),
            };
            AddCombinedSampler(texture, sampler);
            continue;
        }

        // This is an external texture, add planes individually.
        const auto& bindingLayout = std::get<ExternalTextureBindingInfo>(bindingInfo.bindingLayout);

        auto& tint_data = bindings.external_texture.at(ToTint(use.texture));
        DAWN_ASSERT(std::holds_alternative<tint::ExternalMultiplanarTexture>(tint_data));

        tint::ExternalMultiplanarTexture mp_data =
            std::get<tint::ExternalMultiplanarTexture>(tint_data);

        CombinedBindingInfo plane0 = {
            .group = use.texture.group,
            .index = bindingLayout.plane0,
            .remappedBinding = mp_data.plane0,
        };
        AddCombinedSampler(plane0, sampler, false);

        CombinedBindingInfo plane1 = {
            .group = use.texture.group,
            .index = bindingLayout.plane1,
            .remappedBinding = mp_data.plane1,
        };
        AddCombinedSampler(plane1, sampler, true);
    }
}

// Returns whether the stage uses any texture builtin metadata.
void GenerateTextureBuiltinFromUniformData(
    const EntryPointMetadata& metadata,
    const PipelineLayout* layout,
    const tint::Bindings& bindings,
    EmulatedTextureBuiltinRegistrar* emulatedTextureBuiltins,
    tint::glsl::writer::TextureBuiltinsFromUniformOptions* textureBuiltinsFromUniform) {
    // Tell Tint where the uniform containing the builtin data will be (in post-remapping space),
    // only when this shader stage uses some builtin metadata.
    if (!metadata.textureQueries.empty()) {
        textureBuiltinsFromUniform->ubo_binding = {
            .group = 0,
            .binding = uint32_t(layout->GetInternalTextureBuiltinsUniformBinding()),
        };
    }

    for (auto [i, query] : Enumerate(metadata.textureQueries)) {
        BindGroupIndex group = BindGroupIndex(query.group);
        const auto* bgl = layout->GetBindGroupLayout(group);
        BindingIndex binding =
            bgl->AsBindingIndex(bgl->GetAPIBindingIndex(BindingNumber{query.binding}));

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

        tint::BindingPoint remappedBinding;
        if (bindings.texture.contains(wgslBindPoint)) {
            remappedBinding = bindings.texture.at(wgslBindPoint);
        } else {
            remappedBinding = bindings.storage_texture.at(wgslBindPoint);
        }
        textureBuiltinsFromUniform->ubo_contents.push_back({
            .offset = offset,
            .count = 1,
            .binding = remappedBinding,
        });
    }
}

bool GenerateArrayLengthFromuniformData(
    const BindingInfoArray& moduleBindingInfo,
    const PipelineLayout* layout,
    tint::glsl::writer::ArrayLengthFromUniformOptions& options) {
    const PipelineLayout::BindingIndexInfo& indexInfo = layout->GetBindingIndexInfo();

    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayoutInternalBase* bgl = layout->GetBindGroupLayout(group);

        for (BindingIndex binding : bgl->GetBufferIndices()) {
            const BindingInfo& bindingInfo = bgl->GetBindingInfo(binding);

            switch (std::get<BufferBindingInfo>(bindingInfo.bindingLayout).type) {
                case wgpu::BufferBindingType::Storage:
                case kInternalStorageBufferBinding:
                case wgpu::BufferBindingType::ReadOnlyStorage:
                case kInternalReadOnlyStorageBufferBinding: {
                    // Use ssbo index as the indices for the buffer size lookups
                    // in the array length from uniform transform.
                    tint::BindingPoint srcBindingPoint = {uint32_t(group),
                                                          uint32_t(bindingInfo.binding)};
                    FlatBindingIndex ssboIndex = indexInfo[group][binding];
                    options.bindpoint_to_size_index.emplace(srcBindingPoint, uint32_t(ssboIndex));
                    break;
                }
                default:
                    break;
            }
        }
    }

    return options.bindpoint_to_size_index.size() > 0;
}

}  // namespace

bool operator<(const CombinedSamplerElement& a, const CombinedSamplerElement& b) {
    return std::tie(a.group, a.index, a.shaderArraySize) <
           std::tie(b.group, b.index, b.shaderArraySize);
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
          << static_cast<uint32_t>(samplerLocation->index);
    }
    o << "_with_" << static_cast<uint32_t>(textureLocation.group) << "_"
      << static_cast<uint32_t>(textureLocation.index);
    return o.str();
}

// static
ResultOrError<Ref<ShaderModule>> ShaderModule::Create(
    Device* device,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions) {
    Ref<ShaderModule> shader = AcquireRef(new ShaderModule(device, descriptor, internalExtensions));
    shader->Initialize();
    return shader;
}

ShaderModule::ShaderModule(Device* device,
                           const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                           std::vector<tint::wgsl::Extension> internalExtensions)
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions)) {}

ResultOrError<GLuint> ShaderModule::CompileShader(
    const OpenGLFunctions& gl,
    const ProgrammableStage& programmableStage,
    SingleShaderStage stage,
    const ImmediateConstantMask& pipelineImmediateMask,
    VertexAttributeMask bgraSwizzleAttributes,
    std::vector<CombinedSampler>* combinedSamplersOut,
    const PipelineLayout* layout,
    EmulatedTextureBuiltinRegistrar* emulatedTextureBuiltins,
    bool* needsSSBOLengthUniformBuffer,
    Extent3D* workgroupSize) {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "TranslateToGLSL");

    const OpenGLVersion& version = gl.GetVersion();

    GLSLCompilationRequest req = {};

    req.shaderModuleHash = GetHash();
    req.inputProgram = UnsafeUnserializedValue(UseTintProgram());

    // Since (non-Vulkan) GLSL does not support descriptor sets, generate a mapping from the
    // original group/binding pair to a binding-only value. This mapping will be used by Tint to
    // remap all global variables to the 1D space.
    const EntryPointMetadata& entryPointMetaData = GetEntryPoint(programmableStage.entryPoint);
    const BindingInfoArray& moduleBindingInfo = entryPointMetaData.bindings;

    tint::Bindings bindings =
        GenerateBindingRemapping(layout, stage, [&](BindGroupIndex group, BindingIndex index) {
            return tint::BindingPoint{
                .group = 0,
                .binding = uint32_t(layout->GetBindingIndexInfo()[group][index]),
            };
        });

    // When textures are accessed without a sampler (e.g., textureLoad()), returned
    // CombinedSamplerInfo should use this sentinel value as sampler binding point.
    req.tintOptions.placeholder_sampler_bind_point = {
        .group = static_cast<uint32_t>(kMaxBindGroupsTyped),
        .binding = 0,
    };

    // Compute the metadata necessary for translating to GL's combined textures and samplers, both
    // for Dawn and for the Tint translation to GLSL.
    {
        std::vector<CombinedSampler> combinedSamplers;
        GenerateCombinedSamplerInfo(entryPointMetaData, bindings, layout, &combinedSamplers,
                                    &(req.tintOptions.sampler_texture_to_name),
                                    &(req.tintOptions.placeholder_sampler_bind_point));
        *combinedSamplersOut = std::move(combinedSamplers);
    }

    // Compute the metadata necessary to emulate some of the texture "getter" builtins not present
    // in GLSL, both for Dawn and for the Tint translation to GLSL.
    GenerateTextureBuiltinFromUniformData(entryPointMetaData, layout, bindings,
                                          emulatedTextureBuiltins,
                                          &(req.tintOptions.texture_builtins_from_uniform));

    req.stage = stage;
    req.limits = LimitsForCompilationRequest::Create(GetDevice()->GetLimits().v1);
    req.adapterSupportedLimits = UnsafeUnserializedValue(
        LimitsForCompilationRequest::Create(GetDevice()->GetAdapter()->GetLimits().v1));

    if (GetDevice()->IsToggleEnabled(Toggle::GLUseArrayLengthFromUniform)) {
        *needsSSBOLengthUniformBuffer = GenerateArrayLengthFromuniformData(
            moduleBindingInfo, layout, req.tintOptions.array_length_from_uniform);
        if (*needsSSBOLengthUniformBuffer) {
            req.tintOptions.use_array_length_from_uniform = true;
            req.tintOptions.array_length_from_uniform.ubo_binding = {
                .group = kMaxBindGroups + 2,
                .binding = 0,
            };
            bindings.uniform.emplace(
                req.tintOptions.array_length_from_uniform.ubo_binding,
                tint::BindingPoint{
                    .group = 0,
                    .binding = uint32_t(layout->GetInternalArrayLengthUniformBinding()),
                });
        }
    }

    req.platform = UnsafeUnserializedValue(GetDevice()->GetPlatform());

    req.tintOptions.entry_point_name = programmableStage.entryPoint;
    req.tintOptions.version = tint::glsl::writer::Version(ToTintGLStandard(version.GetStandard()),
                                                          version.GetMajor(), version.GetMinor());

    req.tintOptions.substitute_overrides_config = {
        .map = BuildSubstituteOverridesTransformConfig(programmableStage),
    };

    req.tintOptions.disable_robustness = !GetDevice()->IsRobustnessEnabled();
    req.tintOptions.disable_workgroup_init =
        GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);

    // If the size or alignment of the vertex and fragment stage immediate variables differ (e.g.,
    // the vertex shader immediates contain a vec4 and the fragment shader do not), the generated
    // structs may have differing alignment or size, and GLSL will give an error at link time. Count
    // the actual used slots, round up to the widest possible alignment (4 u32s), multiply by the
    // element byte size and pass that to Tint.
    auto immediateCount = RoundUp(pipelineImmediateMask.count(), 4u);

    req.tintOptions.minimum_immediate_size = immediateCount * kImmediateConstantElementByteSize;
    if (HasImmediateConstants(&RenderImmediateConstants::firstVertex, pipelineImmediateMask)) {
        req.tintOptions.first_vertex_offset = GetImmediateByteOffsetInPipelineIfAny(
            &RenderImmediateConstants::firstVertex, pipelineImmediateMask);
    }

    if (HasImmediateConstants(&RenderImmediateConstants::firstInstance, pipelineImmediateMask)) {
        req.tintOptions.first_instance_offset = GetImmediateByteOffsetInPipelineIfAny(
            &RenderImmediateConstants::firstInstance, pipelineImmediateMask);
    }

    if (HasImmediateConstants(&RenderImmediateConstants::clampFragDepth, pipelineImmediateMask)) {
        uint32_t offsetStartBytes = GetImmediateByteOffsetInPipeline(
            &RenderImmediateConstants::clampFragDepth, pipelineImmediateMask);
        req.tintOptions.depth_range_offsets = {
            offsetStartBytes, offsetStartBytes + kImmediateConstantElementByteSize};
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

    req.tintOptions.disable_integer_range_analysis =
        !GetDevice()->IsToggleEnabled(Toggle::EnableIntegerRangeAnalysisInRobustness);

    req.tintOptions.use_uniform_buffers =
        !GetDevice()->IsToggleEnabled(Toggle::DecomposeUniformBuffers);

    CacheResult<GLSLCompilation> compilationResult;
    DAWN_TRY_LOAD_OR_RUN(
        compilationResult, GetDevice(), std::move(req), GLSLCompilation::FromValidatedBlob,
        [](GLSLCompilationRequest r) -> ResultOrError<GLSLCompilation> {
            // Requires Tint Program here right before actual using.
            auto shaderModule = r.inputProgram.UnsafeGetValue();
            auto inputProgram = shaderModule->GetTintProgram();
            auto device = shaderModule->GetDevice();
            const tint::Program* tintInputProgram = &(inputProgram->program);
            // Convert the AST program to an IR module.
            tint::Result<tint::core::ir::Module> ir;
            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleProgramToIR");
                tint::wgsl::reader::IROptions irOptions{
                    .dump_ir_when_validating = device->IsToggleEnabled(Toggle::DumpTintIR),
                    .enable_validation_asserts =
                        device->IsToggleEnabled(Toggle::EnableTintIRValidationAsserts),
                };
                ir = tint::wgsl::reader::ProgramToLoweredIR(*tintInputProgram, irOptions);
                DAWN_INVALID_IF(ir != tint::Success,
                                "An error occurred while generating Tint IR\n%s",
                                ir.Failure().reason);
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

            GLSLCompilation compResult{{.glsl = std::move(result->glsl)}};
            // Workgroup validation has to come after `Generate` because it may require
            // overrides to have been substituted.
            if (r.stage == SingleShaderStage::Compute) {
                // Validate workgroup size after program runs transforms.
                // Subgroups are not supported on OpenGL backend.
                Extent3D workgroupSize;
                DAWN_TRY_ASSIGN(workgroupSize, ValidateComputeStageWorkgroupSize(
                                                   result->workgroup_info,
                                                   /*usesSubgroupMatrix=*/false,
                                                   /*maxSubgroupSize=*/0, r.limits,
                                                   r.adapterSupportedLimits.UnsafeGetValue()));
                compResult.workgroupSize = workgroupSize;
            }

            return compResult;
        },
        "OpenGL.CompileShaderToGLSL");

    *workgroupSize = compilationResult->workgroupSize;

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
