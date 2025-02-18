// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/native/d3d/KeyedMutex.h"

#include <utility>

#include "dawn/native/D3DBackend.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/DeviceD3D.h"

namespace dawn::native::d3d {

KeyedMutex::KeyedMutex(Device* device, ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex)
    : mDevice(device), mDXGIKeyedMutex(std::move(dxgiKeyedMutex)) {
    DAWN_ASSERT(mDevice != nullptr);
    DAWN_ASSERT(mDXGIKeyedMutex);
}

KeyedMutex::~KeyedMutex() {
    mDevice->DisposeKeyedMutex(std::move(mDXGIKeyedMutex));
}

MaybeError KeyedMutex::AcquireKeyedMutex() {
    // Explicitly check against S_OK since AcquireSync can return non-error status codes like
    // WAIT_ABANDONED or WAIT_TIMEOUT.
    const HRESULT hr = mDXGIKeyedMutex->AcquireSync(d3d::kDXGIKeyedMutexAcquireKey, INFINITE);
    if (hr == S_OK) {
        return {};
    }

    std::ostringstream msg;
    msg << "Failed to acquire keyed mutex for external image with " << HRESULTAsString(hr) << " (0x"
        << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << hr << ")";
    if (hr == DXGI_ERROR_DEVICE_REMOVED) {
        return DAWN_DEVICE_LOST_ERROR(msg.str());
    } else {
        return DAWN_INTERNAL_ERROR(msg.str());
    }
}

void KeyedMutex::ReleaseKeyedMutex() {
    // ReleaseSync only returns an error if the keyed mutex is invalid or if it wasn't acquired by
    // the device - both should be considered programming errors so an assert is appropriate.
    const HRESULT hr = mDXGIKeyedMutex->ReleaseSync(d3d::kDXGIKeyedMutexAcquireKey);
    DAWN_ASSERT(hr == S_OK);
}

}  // namespace dawn::native::d3d
