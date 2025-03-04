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

#ifndef SRC_DAWN_NATIVE_D3D_D3DERROR_H_
#define SRC_DAWN_NATIVE_D3D_D3DERROR_H_

#include <winerror.h>
#include "dawn/native/Error.h"
#include "dawn/native/ErrorInjector.h"

namespace dawn::native::d3d {

const char* HRESULTAsString(HRESULT result);

constexpr HRESULT E_FAKE_ERROR_FOR_TESTING = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF);
constexpr HRESULT E_FAKE_OUTOFMEMORY_ERROR_FOR_TESTING =
    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFE);

// Returns a success only if result of HResult is success
MaybeError CheckHRESULTImpl(HRESULT result, const char* context);

// Uses CheckRESULT but returns OOM specific error when recoverable.
MaybeError CheckOutOfMemoryHRESULTImpl(HRESULT result, const char* context);

#define CheckHRESULT(resultIn, contextIn)  \
    ::dawn::native::d3d::CheckHRESULTImpl( \
        INJECT_ERROR_OR_RUN(resultIn, ::dawn::native::d3d::E_FAKE_ERROR_FOR_TESTING), contextIn)
#define CheckOutOfMemoryHRESULT(resultIn, contextIn)                                             \
    ::dawn::native::d3d::CheckOutOfMemoryHRESULTImpl(                                            \
        INJECT_ERROR_OR_RUN(resultIn, ::dawn::native::d3d::E_FAKE_OUTOFMEMORY_ERROR_FOR_TESTING, \
                            ::dawn::native::d3d::E_FAKE_ERROR_FOR_TESTING),                      \
        contextIn)

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_D3DERROR_H_
