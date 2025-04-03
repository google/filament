// Copyright 2025 The Dawn & Tint Authors
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

#include <limits>

#include "dawn/common/Assert.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

WireResult Server::DoRenderPassEncoderSetImmediateData(
    Known<WGPURenderPassEncoder> renderPassEncoder,
    uint32_t immediateDataRangeOffsetBytes,
    const uint8_t* data,
    size_t size) {
    mProcs.renderPassEncoderSetImmediateData(renderPassEncoder->handle,
                                             immediateDataRangeOffsetBytes, data, size);
    return WireResult::Success;
}

WireResult Server::DoRenderBundleEncoderSetImmediateData(
    Known<WGPURenderBundleEncoder> renderBundleEncoder,
    uint32_t immediateDataRangeOffsetBytes,
    const uint8_t* data,
    size_t size) {
    mProcs.renderBundleEncoderSetImmediateData(renderBundleEncoder->handle,
                                               immediateDataRangeOffsetBytes, data, size);
    return WireResult::Success;
}

WireResult Server::DoComputePassEncoderSetImmediateData(
    Known<WGPUComputePassEncoder> computePassEncoder,
    uint32_t immediateDataRangeOffsetBytes,
    const uint8_t* data,
    size_t size) {
    mProcs.computePassEncoderSetImmediateData(computePassEncoder->handle,
                                              immediateDataRangeOffsetBytes, data, size);
    return WireResult::Success;
}

}  // namespace dawn::wire::server
