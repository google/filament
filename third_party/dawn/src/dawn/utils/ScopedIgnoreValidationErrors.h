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

#ifndef SRC_DAWN_UTILS_SCOPEDIGNOREVALIDATIONERRORS_H_
#define SRC_DAWN_UTILS_SCOPEDIGNOREVALIDATIONERRORS_H_

#include <webgpu/webgpu_cpp.h>

#include "dawn/common/NonCopyable.h"

namespace dawn::utils {

// Sometimes it is useful to ignore validation errors, for example when testing the behavior of
// client-side state tracking on error objects. (the state tracking is what's tested, not the
// potential validation errors that are generated). This scoper class wraps a validation error
// scope.
class ScopedIgnoreValidationErrors : NonCopyable {
  public:
    // Noop if `device` is nullptr.
    explicit ScopedIgnoreValidationErrors(wgpu::Device device);
    ~ScopedIgnoreValidationErrors();

    ScopedIgnoreValidationErrors(ScopedIgnoreValidationErrors&& other);
    ScopedIgnoreValidationErrors& operator=(ScopedIgnoreValidationErrors&& other);

  private:
    wgpu::Device mDevice = nullptr;
};

}  // namespace dawn::utils

#endif  // SRC_DAWN_UTILS_SCOPEDIGNOREVALIDATIONERRORS_H_
