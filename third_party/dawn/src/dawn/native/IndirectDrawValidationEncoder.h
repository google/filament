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

#ifndef SRC_DAWN_NATIVE_INDIRECTDRAWVALIDATIONENCODER_H_
#define SRC_DAWN_NATIVE_INDIRECTDRAWVALIDATIONENCODER_H_

#include "dawn/native/Error.h"
#include "dawn/native/IndirectDrawMetadata.h"

namespace dawn::native {

class CommandEncoder;
struct CombinedLimits;
class DeviceBase;
class RenderPassResourceUsageTracker;

// The maximum number of draws call we can fit into a single validation batch. This is
// essentially limited by the number of indirect parameter blocks that can fit into the maximum
// allowed storage binding size (with the base limits, it is about 6.7M).
uint32_t ComputeMaxDrawCallsPerIndirectValidationBatch(const CombinedLimits& limits);

MaybeError EncodeIndirectDrawValidationCommands(DeviceBase* device,
                                                CommandEncoder* commandEncoder,
                                                RenderPassResourceUsageTracker* usageTracker,
                                                IndirectDrawMetadata* indirectDrawMetadata);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_INDIRECTDRAWVALIDATIONENCODER_H_
