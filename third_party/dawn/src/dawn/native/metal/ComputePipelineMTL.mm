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

#include "dawn/native/metal/ComputePipelineMTL.h"

#include "dawn/common/Math.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/CreatePipelineAsyncEvent.h"
#include "dawn/native/Instance.h"
#include "dawn/native/metal/BackendMTL.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/ShaderModuleMTL.h"
#include "dawn/native/metal/UtilsMetal.h"
#include "dawn/platform/metrics/HistogramMacros.h"

namespace dawn::native::metal {

// static
Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return AcquireRef(new ComputePipeline(device, descriptor));
}

ComputePipeline::ComputePipeline(DeviceBase* dev,
                                 const UnpackedPtr<ComputePipelineDescriptor>& desc)
    : ComputePipelineBase(dev, desc) {}

ComputePipeline::~ComputePipeline() = default;

MaybeError ComputePipeline::InitializeImpl() {
    auto mtlDevice = ToBackend(GetDevice())->GetMTLDevice();

    const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);
    ShaderModule::MetalFunctionData computeData;

    DAWN_TRY(ToBackend(computeStage.module.Get())
                 ->CreateFunction(SingleShaderStage::Compute, computeStage, ToBackend(GetLayout()),
                                  &computeData,
                                  /* sampleMask */ 0xFFFFFFFF,
                                  /* renderPipeline */ nullptr));

    NSError* error = nullptr;
    NSRef<NSString> label = MakeDebugName(GetDevice(), "Dawn_ComputePipeline", GetLabel());

    NSRef<MTLComputePipelineDescriptor> descriptorRef =
        AcquireNSRef([MTLComputePipelineDescriptor new]);
    MTLComputePipelineDescriptor* descriptor = descriptorRef.Get();
    descriptor.computeFunction = computeData.function.Get();
    descriptor.label = label.Get();

    platform::metrics::DawnHistogramTimer timer(GetDevice()->GetPlatform());
    mMtlComputePipelineState.Acquire([mtlDevice
        newComputePipelineStateWithDescriptor:descriptor
                                      options:MTLPipelineOptionNone
                                   reflection:nil
                                        error:&error]);
    if (error != nullptr) {
        return DAWN_INTERNAL_ERROR("Error creating pipeline state " +
                                   std::string([error.localizedDescription UTF8String]));
    }
    DAWN_ASSERT(mMtlComputePipelineState != nil);
    timer.RecordMicroseconds("Metal.newComputePipelineStateWithDescriptor.CacheMiss");

    // Copy over the local workgroup size as it is passed to dispatch explicitly in Metal
    mLocalWorkgroupSize = computeData.localWorkgroupSize;

    mRequiresStorageBufferLength = computeData.needsStorageBufferLength;
    mWorkgroupAllocations = std::move(computeData.workgroupAllocations);
    return {};
}

void ComputePipeline::Encode(id<MTLComputeCommandEncoder> encoder) {
    [encoder setComputePipelineState:mMtlComputePipelineState.Get()];
    for (size_t i = 0; i < mWorkgroupAllocations.size(); ++i) {
        if (mWorkgroupAllocations[i] == 0) {
            continue;
        }
        // Size must be a multiple of 16 bytes.
        uint32_t rounded = Align<uint32_t>(mWorkgroupAllocations[i], 16);
        [encoder setThreadgroupMemoryLength:rounded atIndex:i];
    }
}

MTLSize ComputePipeline::GetLocalWorkGroupSize() const {
    return mLocalWorkgroupSize;
}

bool ComputePipeline::RequiresStorageBufferLength() const {
    return mRequiresStorageBufferLength;
}

}  // namespace dawn::native::metal
