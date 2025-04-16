// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_PROGRAMMABLEENCODER_H_
#define SRC_DAWN_NATIVE_PROGRAMMABLEENCODER_H_

#include <string>

#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "partition_alloc/pointers/raw_ptr.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;

// Base class for shared functionality between programmable encoders.
class ProgrammableEncoder : public ApiObjectBase {
  public:
    ProgrammableEncoder(DeviceBase* device, StringView label, EncodingContext* encodingContext);

    void APIInsertDebugMarker(StringView groupLabel);
    void APIPopDebugGroup();
    void APIPushDebugGroup(StringView groupLabel);
    void APISetImmediateData(uint32_t offset, const void* data, size_t size);

  protected:
    bool IsValidationEnabled() const;
    MaybeError ValidateProgrammableEncoderEnd() const;

    // Compute and render passes do different things on SetBindGroup. These are helper functions
    // for the logic they have in common.
    MaybeError ValidateSetBindGroup(BindGroupIndex index,
                                    BindGroupBase* group,
                                    uint32_t dynamicOffsetCountIn,
                                    const uint32_t* dynamicOffsetsIn) const;
    void RecordSetBindGroup(CommandAllocator* allocator,
                            BindGroupIndex index,
                            BindGroupBase* group,
                            uint32_t dynamicOffsetCount,
                            const uint32_t* dynamicOffsets) const;

    // Construct an "error" programmable pass encoder.
    ProgrammableEncoder(DeviceBase* device,
                        EncodingContext* encodingContext,
                        ErrorTag errorTag,
                        StringView label);

    raw_ptr<EncodingContext> mEncodingContext = nullptr;

    uint64_t mDebugGroupStackSize = 0;

    bool mEnded = false;

  private:
    const bool mValidationEnabled;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PROGRAMMABLEENCODER_H_
