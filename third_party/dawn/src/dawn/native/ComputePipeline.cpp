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

#include "dawn/native/ComputePipeline.h"

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"

namespace dawn::native {

MaybeError ValidateComputePipelineDescriptor(DeviceBase* device,
                                             const ComputePipelineDescriptor* descriptor) {
    if (descriptor->layout != nullptr) {
        DAWN_TRY(device->ValidateObject(descriptor->layout));
    }

    ShaderModuleEntryPoint entryPoint;
    DAWN_TRY_ASSIGN_CONTEXT(entryPoint,
                            ValidateProgrammableStage(
                                device, descriptor->compute.module, descriptor->compute.entryPoint,
                                descriptor->compute.constantCount, descriptor->compute.constants,
                                descriptor->layout, SingleShaderStage::Compute),
                            "validating compute stage (%s, entryPoint: %s).",
                            descriptor->compute.module, descriptor->compute.entryPoint);
    return {};
}

// ComputePipelineBase

ComputePipelineBase::ComputePipelineBase(DeviceBase* device,
                                         const UnpackedPtr<ComputePipelineDescriptor>& descriptor)
    : PipelineBase(
          device,
          descriptor->layout,
          descriptor->label,
          {{SingleShaderStage::Compute, descriptor->compute.module, descriptor->compute.entryPoint,
            descriptor->compute.constantCount, descriptor->compute.constants}}) {
    SetContentHash(ComputeContentHash());
    GetObjectTrackingList()->Track(this);

    // Initialize the cache key to include the cache type and device information.
    StreamIn(&mCacheKey, CacheKey::Type::ComputePipeline, device->GetCacheKey());
}

ComputePipelineBase::ComputePipelineBase(DeviceBase* device,
                                         ObjectBase::ErrorTag tag,
                                         StringView label)
    : PipelineBase(device, tag, label) {}

ComputePipelineBase::~ComputePipelineBase() = default;

void ComputePipelineBase::DestroyImpl() {
    Uncache();
}

// static
Ref<ComputePipelineBase> ComputePipelineBase::MakeError(DeviceBase* device, StringView label) {
    class ErrorComputePipeline final : public ComputePipelineBase {
      public:
        explicit ErrorComputePipeline(DeviceBase* device, StringView label)
            : ComputePipelineBase(device, ObjectBase::kError, label) {}

        MaybeError InitializeImpl() override {
            DAWN_UNREACHABLE();
            return {};
        }
    };

    return AcquireRef(new ErrorComputePipeline(device, label));
}

ObjectType ComputePipelineBase::GetType() const {
    return ObjectType::ComputePipeline;
}

bool ComputePipelineBase::EqualityFunc::operator()(const ComputePipelineBase* a,
                                                   const ComputePipelineBase* b) const {
    return PipelineBase::EqualForCache(a, b);
}

}  // namespace dawn::native
