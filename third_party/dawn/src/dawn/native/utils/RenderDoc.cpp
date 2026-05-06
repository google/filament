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

#include "dawn/native/utils/RenderDoc.h"

#if defined(DAWN_ENABLE_RENDERDOC)

#include <string>

#include "dawn/common/DynamicLib.h"
#include "dawn/common/Log.h"

namespace dawn::native::utils {

#if DAWN_PLATFORM_IS(WINDOWS)
constexpr const char kRenderDocLibFilename[] = "renderdoc.dll";
#elif DAWN_PLATFORM_IS(ANDROID)
constexpr const char kRenderDocLibFilename[] = "libVkLayer_GLES_RenderDoc.so";
#else
constexpr const char kRenderDocLibFilename[] = "librenderdoc.so";
#endif

RenderDocApiType* GetRenderDocApi(DeviceBase* device) {
    // Use an immediately invoked lambda assigned to a static to ensure function is called only once
    static RenderDocApiType* renderDocApi = [&]() -> RenderDocApiType* {
        if (!device->IsToggleEnabled(Toggle::EnableRenderDocProcessInjection)) {
            return nullptr;
        }

        DynamicLib renderDocLib;
        pRENDERDOC_GetAPI GetApiFunc = nullptr;
        void* api = nullptr;

        // See if RenderDoc has injected its shared library into the current process
        std::string error;
        if (!renderDocLib.OpenLoaded(kRenderDocLibFilename, &error) ||
            !renderDocLib.GetProc(&GetApiFunc, "RENDERDOC_GetAPI", &error)) {
            dawn::ErrorLog() << "Dawn using EnableRenderDocProcessInjection toggle, but was unable "
                                "to load RenderDoc shared library: "
                             << error;
            return nullptr;
        }

        GetApiFunc(kRenderDocApiVersion, &api);
        DAWN_ASSERT(api);
        return reinterpret_cast<RenderDocApiType*>(api);
    }();

    return renderDocApi;
}

}  // namespace dawn::native::utils

#endif  // defined(DAWN_ENABLE_RENDERDOC)
