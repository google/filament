// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_PLATFORMFUNCTIONSD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_PLATFORMFUNCTIONSD3D11_H_

#include "dawn/native/d3d/PlatformFunctions.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class PlatformFunctions final : public d3d::PlatformFunctions {
  public:
    PlatformFunctions();
    ~PlatformFunctions() override;

    MaybeError LoadFunctions();

    // Functions from D3D11.dll
    PFN_D3D11_CREATE_DEVICE d3d11CreateDevice = nullptr;

  private:
    using Base = d3d::PlatformFunctions;

    MaybeError LoadD3D11();

    DynamicLib mD3D11Lib;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_PLATFORMFUNCTIONSD3D11_H_
