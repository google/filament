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

#include "dawn/native/Pipeline.h"

#include <algorithm>
#include <utility>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/string_view.h"
#include "dawn/common/Enumerator.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Device.h"
#include "dawn/native/ImmediateConstantsLayout.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/ShaderModule.h"
#include "src/utils/numeric.h"

namespace dawn::native {
ResultOrError<ShaderModuleEntryPoint> ValidateProgrammableStage(DeviceBase* device,
                                                                const ShaderModuleBase* module,
                                                                StringView entryPointName,
                                                                uint32_t constantCount,
                                                                const ConstantEntry* constants,
                                                                const PipelineLayoutBase* layout,
                                                                SingleShaderStage stage) {
    DAWN_TRY(device->ValidateObject(module));

    if (!entryPointName.IsUndefined()) {
        DAWN_INVALID_IF(!module->HasEntryPoint(entryPointName),
                        "Entry point \"%s\" doesn't exist in the shader module %s.", entryPointName,
                        module);
    } else {
        size_t entryPointCount = module->GetEntryPointCount(stage);
        if (entryPointCount == 0) {
            return DAWN_VALIDATION_ERROR(
                "Compatible entry point for stage (%s) doesn't exist in the shader module %s.",
                stage, module);
        } else if (entryPointCount > 1) {
            return DAWN_VALIDATION_ERROR(
                "The entry-point is defaulted but multiple entry points for stage (%s) exist in "
                "the shader module %s.",
                stage, module);
        }
    }

    ShaderModuleEntryPoint entryPoint = module->ReifyEntryPointName(entryPointName, stage);
    const EntryPointMetadata& metadata = module->GetEntryPoint(entryPoint.name);

    if (!metadata.infringedLimitErrors.empty()) {
        std::ostringstream limitList;
        for (const std::string& limit : metadata.infringedLimitErrors) {
            limitList << " - " << limit << "\n";
        }
        return DAWN_VALIDATION_ERROR("%s infringes limits:\n%s", &entryPoint, limitList.str());
    }

    DAWN_INVALID_IF(metadata.stage != stage,
                    "The stage (%s) of the entry point \"%s\" isn't the expected one (%s).",
                    metadata.stage, entryPoint.name, stage);

    if (layout != nullptr) {
        DAWN_TRY(ValidateCompatibilityWithPipelineLayout(device, metadata, layout));
    }

    DAWN_INVALID_IF(device->IsCompatibilityMode() && metadata.usesTextureLoadWithDepthTexture,
                    "textureLoad can not be used with depth textures in compatibility mode in "
                    "stage (%s), entry point \"%s\"",
                    metadata.stage, entryPoint.name);

    DAWN_INVALID_IF(
        device->IsCompatibilityMode() && metadata.usesDepthTextureWithNonComparisonSampler,
        "texture_depth_xx can not be used with non-comparison samplers in compatibility mode in "
        "stage (%s), entry point \"%s\"",
        metadata.stage, entryPoint.name);

    const CombinedLimits& limits = device->GetLimits();
    uint32_t maxCombos =
        std::min(limits.v1.maxSampledTexturesPerShaderStage, limits.v1.maxSamplersPerShaderStage);
    DAWN_INVALID_IF(
        device->IsCompatibilityMode() && metadata.numTextureSamplerCombinations > maxCombos,
        "Entry-point uses %u texture+sampler combinations which is more than the maximum of %u "
        "combinations in compatibility mode",
        metadata.numTextureSamplerCombinations, maxCombos);

    // Validate if overridable constants exist in shader module
    // pipelineBase is not yet constructed at this moment so iterate constants from descriptor
    size_t numUninitializedConstants = metadata.uninitializedOverrides.size();
    // Keep an initialized constants sets to handle duplicate initialization cases
    absl::flat_hash_set<std::string_view> stageInitializedConstantIdentifiers;
    for (uint32_t i = 0; i < constantCount; i++) {
        absl::string_view key = {constants[i].key};
        double value = constants[i].value;

        DAWN_INVALID_IF(metadata.overrides.count(key) == 0,
                        "Pipeline overridable constant \"%s\" not found in %s.", constants[i].key,
                        module);
        DAWN_INVALID_IF(!std::isfinite(value),
                        "Pipeline overridable constant \"%s\" with value (%f) is not finite in %s",
                        key, value, module);

        // Validate if constant value can be represented by the given scalar type in shader
        auto type = metadata.overrides.at(key).type;
        switch (type) {
            case EntryPointMetadata::Override::Type::Float32:
                DAWN_INVALID_IF(!IsDoubleValueRepresentable<float>(value),
                                "Pipeline overridable constant \"%s\" with value (%f) is not "
                                "representable in type (%s)",
                                key, value, "f32");
                break;
            case EntryPointMetadata::Override::Type::Float16:
                DAWN_INVALID_IF(!IsDoubleValueRepresentableAsF16(value),
                                "Pipeline overridable constant \"%s\" with value (%f) is not "
                                "representable in type (%s)",
                                key, value, "f16");
                break;
            case EntryPointMetadata::Override::Type::Int32:
                DAWN_INVALID_IF(!IsDoubleValueRepresentable<int32_t>(value),
                                "Pipeline overridable constant \"%s\" with value (%f) is not "
                                "representable in type (%s)",
                                key, value,
                                type == EntryPointMetadata::Override::Type::Int32 ? "i32" : "b");
                break;
            case EntryPointMetadata::Override::Type::Uint32:
                DAWN_INVALID_IF(!IsDoubleValueRepresentable<uint32_t>(value),
                                "Pipeline overridable constant \"%s\" with value (%f) is not "
                                "representable in type (%s)",
                                key, value, "u32");
                break;
            case EntryPointMetadata::Override::Type::Boolean:
                // Conversion to boolean can't fail
                // https://webidl.spec.whatwg.org/#es-boolean
                break;
            default:
                DAWN_UNREACHABLE();
        }

        if (!stageInitializedConstantIdentifiers.contains(key)) {
            if (metadata.uninitializedOverrides.contains(key)) {
                numUninitializedConstants--;
            }
            stageInitializedConstantIdentifiers.insert(key);
        } else {
            // There are duplicate initializations
            return DAWN_VALIDATION_ERROR(
                "Pipeline overridable constants \"%s\" is set more than once", key);
        }
    }

    // Validate if any overridable constant is left uninitialized
    if (DAWN_UNLIKELY(numUninitializedConstants > 0)) {
        std::string uninitializedConstantsArray;
        bool isFirst = true;
        for (std::string identifier : metadata.uninitializedOverrides) {
            if (stageInitializedConstantIdentifiers.contains(identifier)) {
                continue;
            }

            if (isFirst) {
                isFirst = false;
            } else {
                uninitializedConstantsArray.append(", ");
            }
            uninitializedConstantsArray.append(identifier);
        }

        return DAWN_VALIDATION_ERROR(
            "There are uninitialized pipeline overridable constants, their "
            "identifiers:[%s]",
            uninitializedConstantsArray);
    }

    return entryPoint;
}

uint32_t GetRawBits(ImmediateConstantMask bits) {
    return static_cast<uint32_t>(bits.to_ulong());
}

// PipelineBase

PipelineBase::PipelineBase(DeviceBase* device,
                           PipelineLayoutBase* layout,
                           StringView label,
                           std::vector<StageAndDescriptor> stages)
    : ApiObjectBase(device, label), mLayout(layout) {
    DAWN_ASSERT(!stages.empty());

    for (const StageAndDescriptor& stage : stages) {
        // Extract argument for this stage.
        SingleShaderStage shaderStage = stage.shaderStage;
        ShaderModuleBase* module = stage.module;
        const char* entryPointName = stage.entryPoint.c_str();

        const EntryPointMetadata& metadata = module->GetEntryPoint(entryPointName);
        DAWN_ASSERT(metadata.stage == shaderStage);

        // Record them internally.
        bool isFirstStage = mStageMask == wgpu::ShaderStage::None;
        mStageMask |= StageBit(shaderStage);
        mStages[shaderStage] = {module, entryPointName, &metadata, {}};
        auto& constants = mStages[shaderStage].constants;
        for (uint32_t i = 0; i < stage.constantCount; i++) {
            constants.emplace(stage.constants[i].key, stage.constants[i].value);
        }

        // Compute the max() of all minBufferSizes across all stages.
        RequiredBufferSizes stageMinBufferSizes =
            ComputeRequiredBufferSizesForLayout(metadata, layout);

        if (isFirstStage) {
            mMinBufferSizes = std::move(stageMinBufferSizes);
        } else {
            for (auto [group, minBufferSize] : Enumerate(mMinBufferSizes)) {
                DAWN_ASSERT(stageMinBufferSizes[group].size() == minBufferSize.size());

                for (size_t i = 0; i < stageMinBufferSizes[group].size(); ++i) {
                    minBufferSize[i] = std::max(minBufferSize[i], stageMinBufferSizes[group][i]);
                }
            }
        }
    }
}

PipelineBase::PipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : ApiObjectBase(device, tag, label) {}

PipelineBase::~PipelineBase() = default;

PipelineLayoutBase* PipelineBase::GetLayout() {
    DAWN_ASSERT(!IsError());
    return mLayout.Get();
}

const PipelineLayoutBase* PipelineBase::GetLayout() const {
    DAWN_ASSERT(!IsError());
    return mLayout.Get();
}

const RequiredBufferSizes& PipelineBase::GetMinBufferSizes() const {
    DAWN_ASSERT(!IsError());
    return mMinBufferSizes;
}

const ProgrammableStage& PipelineBase::GetStage(SingleShaderStage stage) const {
    DAWN_ASSERT(!IsError());
    return mStages[stage];
}

const PerStage<ProgrammableStage>& PipelineBase::GetAllStages() const {
    return mStages;
}

bool PipelineBase::HasStage(SingleShaderStage stage) const {
    return mStageMask & StageBit(stage);
}

wgpu::ShaderStage PipelineBase::GetStageMask() const {
    return mStageMask;
}

const ImmediateConstantMask& PipelineBase::GetImmediateMask() const {
    return mImmediateMask;
}

MaybeError PipelineBase::ValidateGetBindGroupLayout(BindGroupIndex groupIndex) {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_TRY(GetDevice()->ValidateObject(mLayout.Get()));
    DAWN_INVALID_IF(groupIndex >= kMaxBindGroupsTyped,
                    "Bind group layout index (%u) exceeds the maximum number of bind groups (%u).",
                    groupIndex, kMaxBindGroups);
    return {};
}

ResultOrError<Ref<BindGroupLayoutBase>> PipelineBase::GetBindGroupLayout(uint32_t groupIndexIn) {
    BindGroupIndex groupIndex(groupIndexIn);

    DAWN_TRY(ValidateGetBindGroupLayout(groupIndex));
    return Ref<BindGroupLayoutBase>(mLayout->GetFrontendBindGroupLayout(groupIndex));
}

BindGroupLayoutBase* PipelineBase::APIGetBindGroupLayout(uint32_t groupIndexIn) {
    Ref<BindGroupLayoutBase> result;
    if (GetDevice()->ConsumedError(GetBindGroupLayout(groupIndexIn), &result,
                                   "Validating GetBindGroupLayout (%u) on %s", groupIndexIn,
                                   this)) {
        result = BindGroupLayoutBase::MakeError(GetDevice());
    }
    return ReturnToAPI(std::move(result));
}

size_t PipelineBase::ComputeContentHash() {
    ObjectContentHasher recorder;
    recorder.Record(mLayout->GetContentHash());

    recorder.Record(mStageMask);
    for (SingleShaderStage stage : IterateStages(mStageMask)) {
        recorder.Record(mStages[stage].module->GetContentHash());
        recorder.Record(mStages[stage].entryPoint);
        recorder.Record(mStages[stage].constants);
    }

    return recorder.GetContentHash();
}

// static
bool PipelineBase::EqualForCache(const PipelineBase* a, const PipelineBase* b) {
    // The layout is deduplicated so it can be compared by pointer.
    if (a->mLayout.Get() != b->mLayout.Get() || a->mStageMask != b->mStageMask) {
        return false;
    }

    for (SingleShaderStage stage : IterateStages(a->mStageMask)) {
        // The module is deduplicated so it can be compared by pointer.
        if (a->mStages[stage].module.Get() != b->mStages[stage].module.Get() ||
            a->mStages[stage].entryPoint != b->mStages[stage].entryPoint ||
            a->mStages[stage].constants.size() != b->mStages[stage].constants.size()) {
            return false;
        }

        // If the constants.size are the same, we still need to compare the key and value.
        if (!std::equal(a->mStages[stage].constants.begin(), a->mStages[stage].constants.end(),
                        b->mStages[stage].constants.begin())) {
            return false;
        }
    }

    return true;
}

PipelineBase::ScopedUseShaderPrograms PipelineBase::UseShaderPrograms() {
    ScopedUseShaderPrograms programs;
    for (SingleShaderStage shaderStage :
         {SingleShaderStage::Vertex, SingleShaderStage::Fragment, SingleShaderStage::Compute}) {
        auto& module = mStages[shaderStage].module;
        if (module.Get()) {
            // Hold an external API reference of ShaderModuleBase to keep mTintProgram in
            // ShaderModuleBase alive.
            programs[shaderStage] = module->UseTintProgram();
        }
    }
    return programs;
}

MaybeError PipelineBase::Initialize(std::optional<ScopedUseShaderPrograms> scopedUsePrograms) {
    if (!scopedUsePrograms) {
        scopedUsePrograms = UseShaderPrograms();
    }

    // Set immediate constant status. userConstants is the first element in both
    // RenderImmediateConstants and ComputeImmediateConstants.
    ImmediateConstantMask userConstantsBits =
        GetImmediateConstantBlockBits(0, GetLayout()->GetImmediateDataRangeByteSize());
    mImmediateMask |= userConstantsBits;

    DAWN_TRY_CONTEXT(InitializeImpl(), "initializing %s", this);
    return {};
}

void PipelineBase::SetImmediateMaskForTesting(ImmediateConstantMask immediateConstantMask) {
    mImmediateMask = immediateConstantMask;
}

uint32_t PipelineBase::GetImmediateConstantSize() const {
    return static_cast<uint32_t>(mImmediateMask.count());
}

}  // namespace dawn::native
