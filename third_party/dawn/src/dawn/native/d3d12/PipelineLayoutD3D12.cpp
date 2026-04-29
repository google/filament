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

#include "dawn/native/d3d12/PipelineLayoutD3D12.h"

#include <limits>
#include <sstream>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/ResourceTableD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"

using Microsoft::WRL::ComPtr;

namespace dawn::native::d3d12 {
namespace {

// Reserve register names for internal use. This registers map to bindings in the shader,
// but are not directly related to allocation of the root signature.
// In the root signature, it the index of the root parameter where these registers are
// used that determines the layout of the root signature.
// TODO(crbug.com/366291600): Use Immediates to support internal constants.
static constexpr uint32_t kRenderOrComputeInternalRegisterSpace = kMaxBindGroups + 1;
static constexpr uint32_t kRenderOrComputeInternalBaseRegister = 0;

static constexpr uint32_t kDynamicStorageBufferLengthsRegisterSpace = kMaxBindGroups + 2;
static constexpr uint32_t kDynamicStorageBufferLengthsBaseRegister = 0;

static constexpr uint32_t kDynamicStorageBufferOffsetsRegisterSpace = kMaxBindGroups + 3;
static constexpr uint32_t kDynamicStorageBufferOffsetsBaseRegister = 0;

static constexpr uint32_t kImmediatesRegisterSpace = kMaxBindGroups + 4;
static constexpr uint32_t kImmediatesBaseRegister = 0;

// Keep this last as we need a large range of register spaces for resource table descriptors,
// one per resource type. For example, for all texture types, we need 26 spaces.
static constexpr uint32_t kBaseResourceTableRegisterSpace = kMaxBindGroups + 5;

static constexpr uint32_t kInvalidResourceTableRootParameterIndex =
    std::numeric_limits<uint32_t>::max();
static constexpr uint32_t kInvalidDynamicStorageBufferLengthsParameterIndex =
    std::numeric_limits<uint32_t>::max();
static constexpr uint32_t kInvalidDynamicStorageBufferOffsetsParameterIndex =
    std::numeric_limits<uint32_t>::max();
static constexpr uint32_t kInvalidDynamicUniformBufferParameterIndex =
    std::numeric_limits<uint32_t>::max();
static constexpr uint32_t kInvalidImmediatesParameterIndex = std::numeric_limits<uint32_t>::max();

D3D12_ROOT_PARAMETER_TYPE RootParameterType(wgpu::BufferBindingType type) {
    switch (type) {
        case wgpu::BufferBindingType::Uniform:
            return D3D12_ROOT_PARAMETER_TYPE_CBV;
        case wgpu::BufferBindingType::Storage:
        case kInternalStorageBufferBinding:
            return D3D12_ROOT_PARAMETER_TYPE_UAV;
        case wgpu::BufferBindingType::ReadOnlyStorage:
        case kInternalReadOnlyStorageBufferBinding:
            return D3D12_ROOT_PARAMETER_TYPE_SRV;
        case wgpu::BufferBindingType::BindingNotUsed:
        case wgpu::BufferBindingType::Undefined:
        default:
            DAWN_UNREACHABLE();
    }
}

HRESULT SerializeRootParameter1_0(Device* device,
                                  const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& rootSignature1_1,
                                  ID3DBlob** ppBlob,
                                  ID3DBlob** ppErrorBlob) {
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> allDescriptorRanges1_0;
    std::vector<D3D12_ROOT_PARAMETER> rootParameters1_0(rootSignature1_1.Desc_1_1.NumParameters);
    for (size_t i = 0; i < rootParameters1_0.size(); ++i) {
        const D3D12_ROOT_PARAMETER1& rootParameter1_1 = rootSignature1_1.Desc_1_1.pParameters[i];

        rootParameters1_0[i].ParameterType = rootParameter1_1.ParameterType;
        rootParameters1_0[i].ShaderVisibility = rootParameter1_1.ShaderVisibility;

        switch (rootParameters1_0[i].ParameterType) {
            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
                rootParameters1_0[i].Constants = rootParameter1_1.Constants;
                break;

            case D3D12_ROOT_PARAMETER_TYPE_CBV:
            case D3D12_ROOT_PARAMETER_TYPE_SRV:
            case D3D12_ROOT_PARAMETER_TYPE_UAV:
                rootParameters1_0[i].Descriptor.RegisterSpace =
                    rootParameter1_1.Descriptor.RegisterSpace;
                rootParameters1_0[i].Descriptor.ShaderRegister =
                    rootParameter1_1.Descriptor.ShaderRegister;
                break;

            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                rootParameters1_0[i].DescriptorTable.NumDescriptorRanges =
                    rootParameter1_1.DescriptorTable.NumDescriptorRanges;
                if (rootParameters1_0[i].DescriptorTable.NumDescriptorRanges > 0) {
                    std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges1_0(
                        rootParameters1_0[i].DescriptorTable.NumDescriptorRanges);
                    for (uint32_t index = 0;
                         index < rootParameter1_1.DescriptorTable.NumDescriptorRanges; ++index) {
                        const D3D12_DESCRIPTOR_RANGE1& descriptorRange1_1 =
                            rootParameter1_1.DescriptorTable.pDescriptorRanges[index];
                        descriptorRanges1_0[index].BaseShaderRegister =
                            descriptorRange1_1.BaseShaderRegister;
                        descriptorRanges1_0[index].NumDescriptors =
                            descriptorRange1_1.NumDescriptors;
                        descriptorRanges1_0[index].OffsetInDescriptorsFromTableStart =
                            descriptorRange1_1.OffsetInDescriptorsFromTableStart;
                        descriptorRanges1_0[index].RangeType = descriptorRange1_1.RangeType;
                        descriptorRanges1_0[index].RegisterSpace = descriptorRange1_1.RegisterSpace;
                    }
                    allDescriptorRanges1_0.push_back(descriptorRanges1_0);
                    rootParameters1_0[i].DescriptorTable.pDescriptorRanges =
                        allDescriptorRanges1_0.back().data();
                }
                break;

            default:
                DAWN_UNREACHABLE();
                break;
        }
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor;
    rootSignatureDescriptor.NumParameters = static_cast<uint32_t>(rootParameters1_0.size());
    rootSignatureDescriptor.pParameters = rootParameters1_0.data();
    rootSignatureDescriptor.NumStaticSamplers = 0;
    rootSignatureDescriptor.pStaticSamplers = nullptr;
    rootSignatureDescriptor.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    return device->GetFunctions()->d3d12SerializeRootSignature(
        &rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);
}

}  // anonymous namespace

ResultOrError<Ref<PipelineLayout>> PipelineLayout::Create(
    Device* device,
    const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) {
    Ref<PipelineLayout> layout = AcquireRef(new PipelineLayout(device, descriptor));
    DAWN_TRY(layout->Initialize());
    return layout;
}

MaybeError PipelineLayout::Initialize() {
    Device* device = ToBackend(GetDevice());
    // Parameters are D3D12_ROOT_PARAMETER_TYPE which is either a root table, constant, or
    // descriptor.
    std::vector<D3D12_ROOT_PARAMETER1> rootParameters;
    std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;

    size_t rangesCount = 0;
    size_t staticSamplerCount = 0;
    for (BindGroupIndex group : GetBindGroupLayoutsMask()) {
        const BindGroupLayout* bindGroupLayout = ToBackend(GetBindGroupLayout(group));
        rangesCount += bindGroupLayout->GetCbvUavSrvDescriptorRanges().size() +
                       bindGroupLayout->GetSamplerDescriptorRanges().size();
        staticSamplerCount += bindGroupLayout->GetStaticSamplerCount();
    }

    std::vector<D3D12_DESCRIPTOR_RANGE1> resourceTableCbvUavSrvDescriptorRanges;
    if (UsesResourceTable()) {
        resourceTableCbvUavSrvDescriptorRanges = ResourceTable::GetCbvUavSrvDescriptorRanges(*this);
        rangesCount += resourceTableCbvUavSrvDescriptorRanges.size();
    }

    // We are taking pointers to `ranges`, so we cannot let it resize while we're pushing to it.
    std::vector<D3D12_DESCRIPTOR_RANGE1> ranges(rangesCount);
    staticSamplers.reserve(staticSamplerCount);

    uint32_t rangeIndex = 0;

    // Set the root descriptor table parameter and copy ranges. An optional registerSpace can be
    // passed in and is set only on ranges with kRegisterSpacePlaceholder. Returns whether or not
    // the parameter was set. A root parameter is not set if the number of ranges is 0.
    auto SetRootDescriptorTable = [&](std::span<const D3D12_DESCRIPTOR_RANGE1> descriptorRanges,
                                      uint32_t registerSpace =
                                          kRegisterSpacePlaceholder) -> std::optional<uint32_t> {
        auto rangeCount = descriptorRanges.size();
        if (rangeCount == 0) {
            return {};
        }

        D3D12_ROOT_PARAMETER1 rootParameter = {};
        rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rootParameter.DescriptorTable.NumDescriptorRanges = static_cast<uint32_t>(rangeCount);
        rootParameter.DescriptorTable.pDescriptorRanges = &ranges[rangeIndex];

        for (auto& range : descriptorRanges) {
            ranges[rangeIndex] = range;
            if (range.RegisterSpace == kRegisterSpacePlaceholder) {
                ranges[rangeIndex].RegisterSpace = registerSpace;
            }
            rangeIndex++;
        }

        rootParameters.emplace_back(rootParameter);
        return static_cast<uint32_t>(rootParameters.size() - 1);
    };

    mResourceTableRootParameterIndex = kInvalidResourceTableRootParameterIndex;
    if (UsesResourceTable()) {
        if (auto paramIndex = SetRootDescriptorTable(resourceTableCbvUavSrvDescriptorRanges)) {
            mResourceTableRootParameterIndex = *paramIndex;
        }
    }

    for (BindGroupIndex group : GetBindGroupLayoutsMask()) {
        const BindGroupLayout* bindGroupLayout = ToBackend(GetBindGroupLayout(group));

        // Note that CbvUavSrvDescriptorRanges includes dynamic storage buffers.
        if (auto paramIndex = SetRootDescriptorTable(
                bindGroupLayout->GetCbvUavSrvDescriptorRanges(), static_cast<uint32_t>(group))) {
            mCbvUavSrvRootParameterIndices[group] = *paramIndex;
        }
        if (auto paramIndex = SetRootDescriptorTable(bindGroupLayout->GetSamplerDescriptorRanges(),
                                                     static_cast<uint32_t>(group))) {
            mSamplerRootParameterIndices[group] = *paramIndex;
        }

        // Combine the static samplers from the all of the bind group layouts to one vector.
        for (auto& samplerDesc : bindGroupLayout->GetStaticSamplers()) {
            auto& newSampler = staticSamplers.emplace_back(samplerDesc);
            newSampler.RegisterSpace = static_cast<uint32_t>(group);
        }

        // Init root descriptors in root signatures for dynamic uniform buffer bindings.
        // Dynamic buffer bindings are packed at the beginning of the layout binding info.
        // Resize the vector to include entries for both dynamic uniform and storage buffers, even
        // though we only write to the uniform entries, so that we can index them correctly.
        mDynamicUniformRootParameterIndices[group].resize(
            bindGroupLayout->GetDynamicBufferCount(), kInvalidDynamicUniformBufferParameterIndex);
        for (BindingIndex dynamicBindingIndex : bindGroupLayout->GetDynamicBufferIndices()) {
            if (GetBindGroupLayout(group)->IsStorageBufferBinding(dynamicBindingIndex)) {
                // Dynamic storage buffers are stored in the root descriptor table
                continue;
            }

            const BindingInfo& bindingInfo = bindGroupLayout->GetBindingInfo(dynamicBindingIndex);

            if (bindingInfo.visibility == wgpu::ShaderStage::None) {
                // Skip dynamic buffers that are not visible. D3D12 does not have None
                // visibility.
                continue;
            }

            D3D12_ROOT_PARAMETER1 rootParameter = {};

            // Setup root descriptor.
            D3D12_ROOT_DESCRIPTOR1 rootDescriptor;
            rootDescriptor.ShaderRegister = bindGroupLayout->GetShaderRegister(dynamicBindingIndex);
            rootDescriptor.RegisterSpace = static_cast<uint32_t>(group);
            // Using D3D12_ROOT_DESCRIPTOR_FLAG_NONE means using DATA_STATIC_WHILE_SET_AT_EXECUTE
            // for CBV/SRV and using DATA_VOLATILE for UAV, which is allowed because currently in
            // Dawn the views with dynamic offsets are always re-applied every time before a draw or
            // a dispatch call.
            rootDescriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;

            // Set root descriptors in root signatures.
            rootParameter.Descriptor = rootDescriptor;

            // Set parameter types according to bind group layout descriptor.
            rootParameter.ParameterType =
                RootParameterType(std::get<BufferBindingInfo>(bindingInfo.bindingLayout).type);

            // Set visibilities according to bind group layout descriptor.
            rootParameter.ShaderVisibility = ShaderVisibilityType(bindingInfo.visibility);

            mDynamicUniformRootParameterIndices[group][dynamicBindingIndex] = rootParameters.size();
            rootParameters.emplace_back(rootParameter);
        }
    }

    // Make sure that we added exactly the number of elements we expected. If we added more,
    // |ranges| will have resized and the pointers in the |rootParameter|s will be invalid.
    DAWN_ASSERT(rangeIndex == rangesCount);

    D3D12_ROOT_PARAMETER1 renderOrComputeInternalConstants{};
    renderOrComputeInternalConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    renderOrComputeInternalConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    // Always allocate 3 constants for either:
    //  - vertex_index and instance_index
    //  - num_workgroups_x, num_workgroups_y and num_workgroups_z
    // NOTE: We should consider delaying root signature creation until we know how many values
    // we need
    renderOrComputeInternalConstants.Constants.Num32BitValues = 3;
    renderOrComputeInternalConstants.Constants.RegisterSpace =
        kRenderOrComputeInternalRegisterSpace;
    renderOrComputeInternalConstants.Constants.ShaderRegister =
        kRenderOrComputeInternalBaseRegister;
    mFirstIndexOffsetParameterIndex = static_cast<uint32_t>(rootParameters.size());
    mNumWorkgroupsParameterIndex = static_cast<uint32_t>(rootParameters.size());
    // NOTE: We should consider moving this entry to earlier in the root signature since offsets
    // would need to be updated often
    rootParameters.emplace_back(renderOrComputeInternalConstants);

    // For dynamic storage buffers, we store the length and offset of each binding as root
    // constants. Lengths and offsets are bound to separate groups, but share the same binding value
    // (aka register offset). Here we populate mDynamicStorageBufferInfo with this mapping of
    // dynamic storage buffer bind group to register offset, which will be used to update the root
    // constant values, as well as to tell Tint to emit loads from these root constant values for
    // lengths and offsets. Each bind group's length/offset data is stored contiguously in the root
    // constant, so we also compute and store the first register offset for each group where the
    // data should start.
    uint32_t dynamicStorageBufferInfoShaderRegisterOffset = 0;
    for (BindGroupIndex group : GetBindGroupLayoutsMask()) {
        const BindGroupLayoutInternalBase* bgl = GetBindGroupLayout(group);
        const size_t dynamicStorageBufferCount =
            static_cast<size_t>(bgl->GetDynamicStorageBufferCount());

        BindGroupDynamicStorageBufferInfo info;
        info.firstRegisterOffset = dynamicStorageBufferInfoShaderRegisterOffset;
        info.bindingAndRegisterOffsets.reserve(dynamicStorageBufferCount);

        for (BindingIndex bindingIndex : bgl->GetDynamicBufferIndices()) {
            if (bgl->IsStorageBufferBinding(bindingIndex)) {
                info.bindingAndRegisterOffsets.push_back(
                    {bgl->GetBindingInfo(bindingIndex).binding,
                     dynamicStorageBufferInfoShaderRegisterOffset++});
            }
        }
        DAWN_ASSERT(info.bindingAndRegisterOffsets.size() == dynamicStorageBufferCount);
        mDynamicStorageBufferInfo[group] = std::move(info);
    }

    if (dynamicStorageBufferInfoShaderRegisterOffset > 0) {
        auto createRootConstants = [&](uint32_t num32BitValues, uint32_t registerSpace,
                                       uint32_t shaderRegister) -> uint32_t {
            D3D12_ROOT_PARAMETER1 rootParam{};
            rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            rootParam.Constants.Num32BitValues = num32BitValues;
            rootParam.Constants.RegisterSpace = registerSpace;
            rootParam.Constants.ShaderRegister = shaderRegister;
            rootParameters.emplace_back(rootParam);
            return static_cast<uint32_t>(rootParameters.size() - 1);
        };

        // Create the same number of root constants for both the lengths and the offsets of each
        // dynamic storage buffer
        mDynamicStorageBufferLengthsParameterIndex = createRootConstants(
            dynamicStorageBufferInfoShaderRegisterOffset, kDynamicStorageBufferLengthsRegisterSpace,
            kDynamicStorageBufferLengthsBaseRegister);
        mDynamicStorageBufferOffsetsParameterIndex = createRootConstants(
            dynamicStorageBufferInfoShaderRegisterOffset, kDynamicStorageBufferOffsetsRegisterSpace,
            kDynamicStorageBufferOffsetsBaseRegister);
    } else {
        mDynamicStorageBufferLengthsParameterIndex =
            kInvalidDynamicStorageBufferLengthsParameterIndex;
        mDynamicStorageBufferOffsetsParameterIndex =
            kInvalidDynamicStorageBufferOffsetsParameterIndex;
    }

    if (GetImmediateDataRangeByteSize() > 0) {
        D3D12_ROOT_PARAMETER1 immediateConstants{};
        immediateConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        immediateConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        immediateConstants.Constants.Num32BitValues =
            GetImmediateDataRangeByteSize() / sizeof(uint32_t);
        immediateConstants.Constants.RegisterSpace = kImmediatesRegisterSpace;
        immediateConstants.Constants.ShaderRegister = kImmediatesBaseRegister;
        mImmediatesParameterIndex = rootParameters.size();
        rootParameters.emplace_back(immediateConstants);
    } else {
        mImmediatesParameterIndex = kInvalidImmediatesParameterIndex;
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDescriptor = {};
    versionedRootSignatureDescriptor.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    versionedRootSignatureDescriptor.Desc_1_1.NumParameters =
        static_cast<uint32_t>(rootParameters.size());
    versionedRootSignatureDescriptor.Desc_1_1.pParameters = rootParameters.data();
    versionedRootSignatureDescriptor.Desc_1_1.NumStaticSamplers =
        static_cast<uint32_t>(staticSamplers.size());
    versionedRootSignatureDescriptor.Desc_1_1.pStaticSamplers = staticSamplers.data();
    versionedRootSignatureDescriptor.Desc_1_1.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    DAWN_TRY([&]() -> MaybeError {
        ComPtr<ID3DBlob> error;
        if (device->IsToggleEnabled(Toggle::D3D12UseRootSignatureVersion1_1) &&
            SUCCEEDED(device->GetFunctions()->d3d12SerializeVersionedRootSignature(
                &versionedRootSignatureDescriptor, &mRootSignatureBlob, &error))) [[likely]] {
            return {};
        }
        // If using root signature version 1.1 failed, try again with root signature version 1.0.
        // Some drivers appear to run an outdated version of the DXIL validator and can't support
        // 1.1.
        // Note that retrying again is OK because whether we use version 1.0 or 1.1 doesn't
        // affect anything else except pipeline layout creation. Nothing else in Dawn cares
        // what decision we made here.
        // TODO(crbug.com/1512318): Add some telemetry so we log how often/when this happens.
        std::ostringstream messageStream;
        if (error) {
            messageStream << static_cast<const char*>(error->GetBufferPointer()) << "\n";
        }
        HRESULT hr = SerializeRootParameter1_0(device, versionedRootSignatureDescriptor,
                                               &mRootSignatureBlob, &error);
        if (SUCCEEDED(hr)) [[likely]] {
            return {};
        }
        if (error) {
            messageStream << static_cast<const char*>(error->GetBufferPointer()) << "\n";
        }
        messageStream << "D3D12 serialize root signature";
        DAWN_TRY(CheckHRESULT(hr, messageStream.str().c_str()));
        return {};
    }());
    DAWN_TRY(CheckHRESULT(device->GetD3D12Device()->CreateRootSignature(
                              0, mRootSignatureBlob->GetBufferPointer(),
                              mRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)),
                          "D3D12 create root signature"));
    StreamIn(&mCacheKey, mRootSignatureBlob.Get());
    return {};
}

void PipelineLayout::DestroyImpl(DestroyReason reason) {
    PipelineLayoutBase::DestroyImpl(reason);

    Device* device = ToBackend(GetDevice());
    device->ReferenceUntilUnused(mRootSignature);

    // The ID3D12CommandSignature object should not be referenced by GPU operations in-flight on
    // Command Queue when it is being deleted. According to D3D12 debug layer, "it is not safe to
    // final-release objects that may have GPU operations pending. This can result in application
    // instability (921)".
    if (mDispatchIndirectCommandSignatureWithNumWorkgroups.Get()) {
        device->ReferenceUntilUnused(mDispatchIndirectCommandSignatureWithNumWorkgroups);
    }
    if (mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get()) {
        device->ReferenceUntilUnused(mDrawIndirectCommandSignatureWithInstanceVertexOffsets);
    }
    if (mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get()) {
        device->ReferenceUntilUnused(mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets);
    }
}

uint32_t PipelineLayout::GetResourceTableRootParameterIndex() const {
    DAWN_ASSERT(mResourceTableRootParameterIndex != kInvalidResourceTableRootParameterIndex);
    return mResourceTableRootParameterIndex;
}

uint32_t PipelineLayout::GetBaseResourceTableRegisterSpace() const {
    return kBaseResourceTableRegisterSpace;
}

uint32_t PipelineLayout::GetCbvUavSrvRootParameterIndex(BindGroupIndex group) const {
    DAWN_ASSERT(group < kMaxBindGroupsTyped);
    return mCbvUavSrvRootParameterIndices[group];
}

uint32_t PipelineLayout::GetSamplerRootParameterIndex(BindGroupIndex group) const {
    DAWN_ASSERT(group < kMaxBindGroupsTyped);
    return mSamplerRootParameterIndices[group];
}

ID3D12RootSignature* PipelineLayout::GetRootSignature() const {
    return mRootSignature.Get();
}

ID3DBlob* PipelineLayout::GetRootSignatureBlob() const {
    return mRootSignatureBlob.Get();
}

const PipelineLayout::DynamicStorageBufferInfo& PipelineLayout::GetDynamicStorageBufferInfo()
    const {
    return mDynamicStorageBufferInfo;
}

uint32_t PipelineLayout::GetDynamicUniformRootParameterIndex(BindGroupIndex group,
                                                             BindingIndex bindingIndex) const {
    DAWN_ASSERT(group < kMaxBindGroupsTyped);
    DAWN_ASSERT(std::get<BufferBindingInfo>(
                    GetBindGroupLayout(group)->GetBindingInfo(bindingIndex).bindingLayout)
                    .hasDynamicOffset);
    DAWN_ASSERT(GetBindGroupLayout(group)->GetBindingInfo(bindingIndex).visibility !=
                wgpu::ShaderStage::None);
    DAWN_ASSERT(std::get<BufferBindingInfo>(
                    GetBindGroupLayout(group)->GetBindingInfo(bindingIndex).bindingLayout)
                    .type == wgpu::BufferBindingType::Uniform);

    return mDynamicUniformRootParameterIndices[group][bindingIndex];
}

uint32_t PipelineLayout::GetFirstIndexOffsetRegisterSpace() const {
    return kRenderOrComputeInternalRegisterSpace;
}

uint32_t PipelineLayout::GetFirstIndexOffsetShaderRegister() const {
    return kRenderOrComputeInternalBaseRegister;
}

uint32_t PipelineLayout::GetFirstIndexOffsetParameterIndex() const {
    return mFirstIndexOffsetParameterIndex;
}

uint32_t PipelineLayout::GetNumWorkgroupsRegisterSpace() const {
    return kRenderOrComputeInternalRegisterSpace;
}

uint32_t PipelineLayout::GetNumWorkgroupsShaderRegister() const {
    return kRenderOrComputeInternalBaseRegister;
}

uint32_t PipelineLayout::GetNumWorkgroupsParameterIndex() const {
    return mNumWorkgroupsParameterIndex;
}

uint32_t PipelineLayout::GetDynamicStorageBufferLengthsRegisterSpace() const {
    return kDynamicStorageBufferLengthsRegisterSpace;
}

uint32_t PipelineLayout::GetDynamicStorageBufferLengthsShaderRegister() const {
    return kDynamicStorageBufferLengthsBaseRegister;
}

uint32_t PipelineLayout::GetDynamicStorageBufferLengthsParameterIndex() const {
    DAWN_ASSERT(mDynamicStorageBufferLengthsParameterIndex !=
                kInvalidDynamicStorageBufferLengthsParameterIndex);
    return mDynamicStorageBufferLengthsParameterIndex;
}

uint32_t PipelineLayout::GetDynamicStorageBufferOffsetsRegisterSpace() const {
    return kDynamicStorageBufferOffsetsRegisterSpace;
}

uint32_t PipelineLayout::GetDynamicStorageBufferOffsetsShaderRegister() const {
    return kDynamicStorageBufferOffsetsBaseRegister;
}

uint32_t PipelineLayout::GetDynamicStorageBufferOffsetsParameterIndex() const {
    DAWN_ASSERT(mDynamicStorageBufferOffsetsParameterIndex !=
                kInvalidDynamicStorageBufferOffsetsParameterIndex);
    return mDynamicStorageBufferOffsetsParameterIndex;
}

uint32_t PipelineLayout::GetImmediatesRegisterSpace() const {
    return kImmediatesRegisterSpace;
}

uint32_t PipelineLayout::GetImmediatesShaderRegister() const {
    return kImmediatesBaseRegister;
}

uint32_t PipelineLayout::GetImmediatesParameterIndex() const {
    DAWN_ASSERT(mImmediatesParameterIndex != kInvalidImmediatesParameterIndex);
    return mImmediatesParameterIndex;
}

ID3D12CommandSignature* PipelineLayout::GetDispatchIndirectCommandSignatureWithNumWorkgroups() {
    // mDispatchIndirectCommandSignatureWithNumWorkgroups won't be created until it is needed.
    if (mDispatchIndirectCommandSignatureWithNumWorkgroups.Get() != nullptr) {
        return mDispatchIndirectCommandSignatureWithNumWorkgroups.Get();
    }

    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = GetNumWorkgroupsParameterIndex();
    argumentDescs[0].Constant.Num32BitValuesToSet = 3;
    argumentDescs[0].Constant.DestOffsetIn32BitValues = 0;

    // A command signature must contain exactly 1 Draw / Dispatch / DispatchMesh / DispatchRays
    // command. That command must come last.
    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 6 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 2;
    programDesc.pArgumentDescs = argumentDescs;

    // The root signature must be specified if and only if the command signature changes one of
    // the root arguments.
    ToBackend(GetDevice())
        ->GetD3D12Device()
        ->CreateCommandSignature(&programDesc, GetRootSignature(),
                                 IID_PPV_ARGS(&mDispatchIndirectCommandSignatureWithNumWorkgroups));
    return mDispatchIndirectCommandSignatureWithNumWorkgroups.Get();
}

ID3D12CommandSignature* PipelineLayout::GetDrawIndirectCommandSignatureWithInstanceVertexOffsets() {
    // mDrawIndirectCommandSignatureWithInstanceVertexOffsets won't be created until it is
    // needed.
    if (mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get() != nullptr) {
        return mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get();
    }

    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = GetFirstIndexOffsetParameterIndex();
    argumentDescs[0].Constant.Num32BitValuesToSet = 2;
    argumentDescs[0].Constant.DestOffsetIn32BitValues = 0;

    // A command signature must contain exactly 1 Draw / Dispatch / DispatchMesh / DispatchRays
    // command. That command must come last.
    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 6 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 2;
    programDesc.pArgumentDescs = argumentDescs;

    // The root signature must be specified if and only if the command signature changes one of
    // the root arguments.
    ToBackend(GetDevice())
        ->GetD3D12Device()
        ->CreateCommandSignature(
            &programDesc, GetRootSignature(),
            IID_PPV_ARGS(&mDrawIndirectCommandSignatureWithInstanceVertexOffsets));
    return mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get();
}

ID3D12CommandSignature*
PipelineLayout::GetDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets() {
    // mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets won't be created until it
    // is needed.
    if (mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get() != nullptr) {
        return mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get();
    }

    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = GetFirstIndexOffsetParameterIndex();
    argumentDescs[0].Constant.Num32BitValuesToSet = 2;
    argumentDescs[0].Constant.DestOffsetIn32BitValues = 0;

    // A command signature must contain exactly 1 Draw / Dispatch / DispatchMesh / DispatchRays
    // command. That command must come last.
    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 7 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 2;
    programDesc.pArgumentDescs = argumentDescs;

    // The root signature must be specified if and only if the command signature changes one of
    // the root arguments.
    ToBackend(GetDevice())
        ->GetD3D12Device()
        ->CreateCommandSignature(
            &programDesc, GetRootSignature(),
            IID_PPV_ARGS(&mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets));
    return mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get();
}

}  // namespace dawn::native::d3d12
