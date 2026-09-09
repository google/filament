// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_DRMUTILS_H_
#define SRC_DAWN_COMMON_DRMUTILS_H_

// Include this before gbm.h, which can transitively include X11 headers with conflicting macros.
#include "src/dawn/common/xlib_with_undefs.h"
// Comment to prevent reordering.

#include <gbm.h>

#include <cstdint>
#include <memory>
#include <string_view>

#include "src/dawn/common/SystemHandle.h"

namespace dawn {

struct GbmBoDeleter {
    void operator()(gbm_bo* bo) const { gbm_bo_destroy(bo); }
};
using OwnedGbmBo = std::unique_ptr<gbm_bo, GbmBoDeleter>;

struct GbmDeviceDeleter {
    void operator()(gbm_device* device) const { gbm_device_destroy(device); }
};
using OwnedGbmDevice = std::unique_ptr<gbm_device, GbmDeviceDeleter>;

bool IsDRMRenderNodeName(std::string_view nodeName);

bool IsMatchingDRMRenderNode(std::string_view nodeName,
                             bool isCharacterDevice,
                             uint64_t nodeMajor,
                             uint64_t nodeMinor,
                             uint64_t renderMajor,
                             uint64_t renderMinor);

SystemHandle OpenDRMRenderNode(uint64_t renderMajor, uint64_t renderMinor);

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_DRMUTILS_H_
