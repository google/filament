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

#include "src/dawn/native/d3d12/PipelineLayoutD3D12.h"

#include <limits>
#include <sstream>
#include <utility>

#include "src/dawn/native/d3d/D3DError.h"
#include "src/dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "src/dawn/native/d3d12/DeviceD3D12.h"
#include "src/dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "src/dawn/native/d3d12/ResourceTableD3D12.h"
#include "src/dawn/native/d3d12/UtilsD3D12.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"

using Microsoft::WRL::ComPtr;

namespace dawn::native::d3d12 {
namespace {

static constexpr uint32_t kImmediatesRegisterSpace = kMaxBindGroups + 4;
static constexpr uint32_t kImmediatesBaseRegister = 0;

// Keep this last as we need a large range of register spaces for resource table descriptors,
// one per resource type. For example, for all texture types, we need 26 spaces.
static constexpr uint32_t kBaseResourceTableRegisterSpace = kMaxBindGroups + 5;

static constexpr uint32_t kInvalidResourceTableRootParameterIndex =
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
    // SAFETY: pParameters + NumParameters must define a valid range of D2D12_ROOT_PARAMETER1s.
    Span<const D3D12_ROOT_PARAMETER1> rootParameters1_1 = DAWN_UNSAFE_BUFFERS(
        {rootSignature1_1.Desc_1_1.pParameters, rootSignature1_1.Desc_1_1.NumParameters});

    std::vector<D3D12_ROOT_PARAMETER> rootParameters1_0(rootParameters1_1.size());
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> allDescriptorRanges1_0;

    for (size_t i = 0; i < rootParameters1_0.size(); ++i) {
        const D3D12_ROOT_PARAMETER1& rootParameter1_1 = rootParameters1_1[i];

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

            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: {
                Span<const D3D12_DESCRIPTOR_RANGE1> descriptorRanges1_1 =
                    // SAFETY: pDescriptorRanges + NumDescriptorRanges must define a valid range of
                    // D3D12_DESCRIPTOR_RANGE1 values.
                    DAWN_UNSAFE_BUFFERS({rootParameter1_1.DescriptorTable.pDescriptorRanges,
                                         rootParameter1_1.DescriptorTable.NumDescriptorRanges});

                rootParameters1_0[i].DescriptorTable.NumDescriptorRanges =
                    static_cast<UINT>(descriptorRanges1_1.size());
                if (rootParameters1_0[i].DescriptorTable.NumDescriptorRanges > 0) {
                    std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges1_0(
                        rootParameters1_0[i].DescriptorTable.NumDescriptorRanges);
                    for (uint32_t index = 0;
                         index < rootParameter1_1.DescriptorTable.NumDescriptorRanges; ++index) {
                        const D3D12_DESCRIPTOR_RANGE1& descriptorRange1_1 =
                            descriptorRanges1_1[index];
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
            }

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
    BindGroupMask bindGroupMask = GetBindGroupLayoutsMask();
    BindGroupIndex highestBindGroupIndex = GetHighestBitIndexPlusOne(bindGroupMask);
    PerBindGroup<const CachedObject*> cachedObjects;
    for (BindGroupIndex i : Range(highestBindGroupIndex)) {
        if (bindGroupMask[i]) {
            cachedObjects[i] = GetBindGroupLayout(i);
        } else {
            cachedObjects[i] = GetDevice()->GetEmptyBindGroupLayout()->GetInternalBindGroupLayout();
        }
    }

    // Record bind group layout objects and user immediate data size into pipeline layout cache key.
    // It represents pipeline layout base attributes and ignored future changes caused by internal
    // immediate data size from pipeline.
    uint32_t numSetLayoutsWithHoles =
        static_cast<uint32_t>(GetHighestBitIndexPlusOne(bindGroupMask));
    StreamIn(&mCacheKey, stream::Iterable(cachedObjects.data(), numSetLayoutsWithHoles),
             GetImmediateDataRangeByteSize());

    DAWN_TRY(BuildBaseRootParameters());

    return {};
}

ResultOrError<Ref<PipelineLayoutHandle>> PipelineLayout::GetOrCreatePipelineLayoutHandle(
    const ImmediateMask& pipelineImmediateMask) {
    // Check cache
    Ref<PipelineLayoutHandle> pipelineLayoutHandle;
    mPipelineLayoutHandles.Use([&](auto pipelineLayoutHandles) {
        auto it = pipelineLayoutHandles->find(pipelineImmediateMask);
        if (it != pipelineLayoutHandles->end()) {
            pipelineLayoutHandle = it->second;
        }
    });

    if (pipelineLayoutHandle != nullptr) {
        return pipelineLayoutHandle;
    }

    DAWN_TRY_ASSIGN(pipelineLayoutHandle, CreatePipelineLayoutHandle(pipelineImmediateMask));

    return mPipelineLayoutHandles.Use([&](auto pipelineLayoutHandles) {
        return pipelineLayoutHandles
            ->insert({pipelineImmediateMask, std::move(pipelineLayoutHandle)})
            .first->second;
    });
}

MaybeError PipelineLayout::BuildBaseRootParameters() {
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

    ResourceTable::DescriptorRanges resourceTableRanges;
    if (UsesResourceTable()) {
        DAWN_TRY_ASSIGN(resourceTableRanges, ResourceTable::GetDescriptorRanges(*this));
        rangesCount += resourceTableRanges.cbvUavSrvs.size() + resourceTableRanges.samplers.size();
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

    mResourceTableCbvUavSrvRootParameterIndex = kInvalidResourceTableRootParameterIndex;
    mResourceTableSamplerRootParameterIndex = kInvalidResourceTableRootParameterIndex;
    if (UsesResourceTable()) {
        if (auto paramIndex = SetRootDescriptorTable(resourceTableRanges.cbvUavSrvs)) {
            mResourceTableCbvUavSrvRootParameterIndex = *paramIndex;
        }
        if (auto paramIndex = SetRootDescriptorTable(resourceTableRanges.samplers)) {
            mResourceTableSamplerRootParameterIndex = *paramIndex;
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

            mDynamicUniformRootParameterIndices[group][dynamicBindingIndex] =
                static_cast<uint32_t>(rootParameters.size());
            rootParameters.emplace_back(rootParameter);
        }
    }

    // Make sure that we added exactly the number of elements we expected. If we added more,
    // |ranges| will have resized and the pointers in the |rootParameter|s will be invalid.
    DAWN_ASSERT(rangeIndex == rangesCount);

    // For dynamic storage buffers, we store the length and offset of each binding in the immediate
    // block. Here we populate mDynamicStorageBufferInfo with the mapping of dynamic storage buffer
    // bind group to its index into that immediate data, which is used both to update the immediate
    // values and to tell Tint to emit loads from them for lengths and offsets. Each bind group's
    // length/offset data is stored contiguously, so we also compute and store the first index for
    // each group where the data should start.
    uint32_t dynamicStorageBufferCount = 0;
    for (BindGroupIndex group : GetBindGroupLayoutsMask()) {
        const BindGroupLayoutInternalBase* bgl = GetBindGroupLayout(group);
        const size_t bglDynamicStorageBufferCount =
            static_cast<size_t>(bgl->GetDynamicStorageBufferCount());

        BindGroupDynamicStorageBufferInfo info;
        info.firstImmediateIndex = dynamicStorageBufferCount;
        info.bindingAndImmediateIndices.reserve(bglDynamicStorageBufferCount);

        for (BindingIndex bindingIndex : bgl->GetDynamicBufferIndices()) {
            if (bgl->IsStorageBufferBinding(bindingIndex)) {
                info.bindingAndImmediateIndices.push_back(
                    {bgl->GetBindingInfo(bindingIndex).binding, dynamicStorageBufferCount++});
            }
        }
        DAWN_ASSERT(info.bindingAndImmediateIndices.size() == bglDynamicStorageBufferCount);
        mDynamicStorageBufferInfo[group] = std::move(info);
    }

    mDynamicStorageBufferCount = dynamicStorageBufferCount;

    // Stash the layout-invariant parameters so each PipelineLayoutHandle can be built from them
    // without recomputing or mutating shared state. The descriptor table entries in rootParameters
    // point into ranges; moving the vectors into the const struct preserves those pointers and
    // keeps them immutable thereafter.
    mInvariantParams.emplace(std::move(rootParameters), std::move(ranges),
                             std::move(staticSamplers));

    return {};
}

ResultOrError<Ref<PipelineLayoutHandle>> PipelineLayout::CreatePipelineLayoutHandle(
    const ImmediateMask& pipelineImmediateMask) {
    Device* device = ToBackend(GetDevice());
    DAWN_ASSERT(mInvariantParams.has_value());

    // Start from the layout-invariant base parameters and append the immediates root parameter,
    // whose size depends on the pipeline's internal immediate usage. Everything here operates on
    // locals so that this method has no side effects and can run concurrently on pipeline-creation
    // worker threads. The descriptor table entries copied from rootParameters keep pointing into
    // mInvariantParams->ranges, which is stable for the lifetime of the layout.
    std::vector<D3D12_ROOT_PARAMETER1> rootParameters = mInvariantParams->rootParameters;
    uint32_t immediatesParameterIndex = kInvalidImmediatesParameterIndex;
    if (pipelineImmediateMask.count() > 0) {
        D3D12_ROOT_PARAMETER1 immediates{};
        immediates.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        immediates.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        immediates.Constants.Num32BitValues = static_cast<UINT>(pipelineImmediateMask.count());
        immediates.Constants.RegisterSpace = kImmediatesRegisterSpace;
        immediates.Constants.ShaderRegister = kImmediatesBaseRegister;
        immediatesParameterIndex = static_cast<uint32_t>(rootParameters.size());
        rootParameters.emplace_back(immediates);
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDescriptor = {};
    versionedRootSignatureDescriptor.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    versionedRootSignatureDescriptor.Desc_1_1.NumParameters =
        static_cast<uint32_t>(rootParameters.size());
    versionedRootSignatureDescriptor.Desc_1_1.pParameters = rootParameters.data();
    versionedRootSignatureDescriptor.Desc_1_1.NumStaticSamplers =
        static_cast<uint32_t>(mInvariantParams->staticSamplers.size());
    versionedRootSignatureDescriptor.Desc_1_1.pStaticSamplers =
        mInvariantParams->staticSamplers.data();
    versionedRootSignatureDescriptor.Desc_1_1.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> rootSignatureBlob;
    DAWN_TRY([&]() -> MaybeError {
        ComPtr<ID3DBlob> error;
        if (device->IsToggleEnabled(Toggle::D3D12UseRootSignatureVersion1_1) &&
            SUCCEEDED(device->GetFunctions()->SerializeVersionedRootSignature(
                &versionedRootSignatureDescriptor, &rootSignatureBlob, &error))) [[likely]] {
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
                                               &rootSignatureBlob, &error);
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

    ComPtr<ID3D12RootSignature> rootSignature;
    DAWN_TRY(CheckHRESULT(device->GetD3D12Device()->CreateRootSignature(
                              0, rootSignatureBlob->GetBufferPointer(),
                              rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)),
                          "D3D12 create root signature"));

    return PipelineLayoutHandle::Create(ToBackend(GetDevice()), std::move(rootSignature),
                                        std::move(rootSignatureBlob), immediatesParameterIndex,
                                        pipelineImmediateMask);
}

void PipelineLayout::DestroyImpl(DestroyReason reason) {
    PipelineLayoutBase::DestroyImpl(reason);
    mPipelineLayoutHandles->clear();
}

uint32_t PipelineLayout::GetResourceTableCbvUavSrvRootParameterIndex() const {
    DAWN_ASSERT(mResourceTableCbvUavSrvRootParameterIndex !=
                kInvalidResourceTableRootParameterIndex);
    return mResourceTableCbvUavSrvRootParameterIndex;
}

uint32_t PipelineLayout::GetResourceTableSamplerRootParameterIndex() const {
    DAWN_ASSERT(mResourceTableSamplerRootParameterIndex != kInvalidResourceTableRootParameterIndex);
    return mResourceTableSamplerRootParameterIndex;
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

const PipelineLayout::DynamicStorageBufferInfo& PipelineLayout::GetDynamicStorageBufferInfo()
    const {
    return mDynamicStorageBufferInfo;
}

uint32_t PipelineLayout::GetDynamicStorageBufferCount() const {
    return mDynamicStorageBufferCount;
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

uint32_t PipelineLayout::GetImmediatesRegisterSpace() const {
    return kImmediatesRegisterSpace;
}

uint32_t PipelineLayout::GetImmediatesShaderRegister() const {
    return kImmediatesBaseRegister;
}

}  // namespace dawn::native::d3d12
