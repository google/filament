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

#ifndef SRC_DAWN_NATIVE_D3D_D3D_PLATFORM_H_
#define SRC_DAWN_NATIVE_D3D_D3D_PLATFORM_H_

// Pre-emptively include windows.h but remove its macros so that they aren't set when declaring the
// COM interfaces. Otherwise ID3D12InfoQueue::GetMessage would be either GetMessageA or GetMessageW
// which causes compilation errors.
// NOLINTNEXTLINE(build/include_order)
#include "dawn/common/windows_with_undefs.h"

#include <d3d11_4.h>  // NOLINT(build/include_order)
#include <dxcapi.h>   // NOLINT(build/include_order)
#include <dxgi1_6.h>  // NOLINT(build/include_order)
#include <wrl.h>      // NOLINT(build/include_order)

// DXProgrammableCapture.h takes a dependency on other platform header
// files, so it must be defined after them.
#include <DXProgrammableCapture.h>  // NOLINT(build/include_order)
#include <dxgidebug.h>              // NOLINT(build/include_order)

#include <functional>  // NOLINT(build/include_order)
#include <utility>     // NOLINT(build/include_order)

using Microsoft::WRL::ComPtr;
template <typename T>
struct std::hash<ComPtr<T>> {
    std::size_t operator()(const ComPtr<T>& v) const noexcept { return std::hash<T*>{}(v.Get()); }
};
template <typename T, typename H>
H AbslHashValue(H state, const ComPtr<T>& v) {
    return H::combine(std::move(state), v.Get());
}

#endif  // SRC_DAWN_NATIVE_D3D_D3D_PLATFORM_H_
