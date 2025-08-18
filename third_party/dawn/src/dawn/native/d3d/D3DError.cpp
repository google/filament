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

#include "dawn/native/d3d/D3DError.h"

#include <iomanip>
#include <sstream>
#include <string>

#include "dawn/common/windows_with_undefs.h"

namespace dawn::native::d3d {
const char* HRESULTAsString(HRESULT result) {
    // There's a lot of possible HRESULTS, but these ones are the ones specifically listed as
    // being returned from D3D11 and D3D12, in addition to fake codes used internally for testing.
    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/d3d12-graphics-reference-returnvalues
    switch (result) {
        case S_OK:
            return "S_OK";
        case S_FALSE:
            return "S_FALSE";

        // Wait results that are not errors:
        case WAIT_ABANDONED:
            return "WAIT_ABAONDONED";
        case WAIT_TIMEOUT:
            return "WAIT_TIMEOUT";

        // Generic errors:
        case E_FAIL:
            return "E_FAIL";
        case E_INVALIDARG:
            return "E_INVALIDARG";
        case E_OUTOFMEMORY:
            return "E_OUTOFMEMORY";
        case E_NOTIMPL:
            return "E_NOTIMPL";

        // DXGI errors:
        case DXGI_ERROR_INVALID_CALL:
            return "DXGI_ERROR_INVALID_CALL";
        case DXGI_ERROR_UNSUPPORTED:
            return "DXGI_ERROR_UNSUPPORTED";
        case DXGI_ERROR_DEVICE_REMOVED:
            return "DXGI_ERROR_DEVICE_REMOVED";
        case DXGI_ERROR_DEVICE_HUNG:
            return "DXGI_ERROR_DEVICE_HUNG";
        case DXGI_ERROR_DEVICE_RESET:
            return "DXGI_ERROR_DEVICE_RESET";
        case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
            return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
        case DXGI_ERROR_WAS_STILL_DRAWING:
            return "DXGI_ERROR_WAS_STILL_DRAWING";
        case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
            return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";

        // D3D11 errors:
        case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
            return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
        case D3D11_ERROR_FILE_NOT_FOUND:
            return "D3D11_ERROR_FILE_NOT_FOUND";
        case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
            return "D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS";
        case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
            return "D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD";

        // D3D12 errors:
        case D3D12_ERROR_ADAPTER_NOT_FOUND:
            return "D3D12_ERROR_ADAPTER_NOT_FOUND";
        case D3D12_ERROR_DRIVER_VERSION_MISMATCH:
            return "D3D12_ERROR_DRIVER_VERSION_MISMATCH";

        // Fake errors used for testing:
        case E_FAKE_ERROR_FOR_TESTING:
            return "E_FAKE_ERROR_FOR_TESTING";
        case E_FAKE_OUTOFMEMORY_ERROR_FOR_TESTING:
            return "E_FAKE_OUTOFMEMORY_ERROR_FOR_TESTING";

        default:
            return "<Unknown HRESULT>";
    }
}

MaybeError CheckHRESULTImpl(HRESULT result, const char* context) {
    if (SUCCEEDED(result)) [[likely]] {
        return {};
    }

    std::ostringstream messageStream;
    messageStream << context << " failed with " << HRESULTAsString(result) << " (0x"
                  << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << result
                  << ")";

    if (result == DXGI_ERROR_DEVICE_REMOVED) {
        return DAWN_DEVICE_LOST_ERROR(messageStream.str());
    } else {
        return DAWN_INTERNAL_ERROR(messageStream.str());
    }
}

MaybeError CheckOutOfMemoryHRESULTImpl(HRESULT result, const char* context) {
    if (result == E_OUTOFMEMORY || result == E_FAKE_OUTOFMEMORY_ERROR_FOR_TESTING) {
        return DAWN_OUT_OF_MEMORY_ERROR(context);
    }

    return CheckHRESULTImpl(result, context);
}

}  // namespace dawn::native::d3d
