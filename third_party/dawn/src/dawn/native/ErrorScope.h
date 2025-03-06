// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_ERRORSCOPE_H_
#define SRC_DAWN_NATIVE_ERRORSCOPE_H_

#include <string>
#include <vector>

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class ErrorScope {
  public:
    ErrorScope(wgpu::ErrorType error, std::string_view message);

    wgpu::ErrorType GetErrorType() const;
    WGPUStringView GetErrorMessage() const;

  private:
    friend class ErrorScopeStack;
    explicit ErrorScope(wgpu::ErrorFilter errorFilter);

    wgpu::ErrorType mMatchedErrorType;
    wgpu::ErrorType mCapturedError = wgpu::ErrorType::NoError;
    std::string mErrorMessage = "";
};

class ErrorScopeStack {
  public:
    ErrorScopeStack();
    ~ErrorScopeStack();

    void Push(wgpu::ErrorFilter errorFilter);
    ErrorScope Pop();

    bool Empty() const;

    // Pass an error to the scopes in the stack. Returns true if one of the scopes
    // captured the error. Returns false if the error should be forwarded to the
    // uncaptured error callback.
    bool HandleError(wgpu::ErrorType type, std::string_view message);

  private:
    std::vector<ErrorScope> mScopes;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ERRORSCOPE_H_
