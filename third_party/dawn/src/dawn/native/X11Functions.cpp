// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/native/X11Functions.h"

#include <memory>

#include "dawn/common/Log.h"

namespace dawn::native {

X11Functions::X11Functions() {
    if (!mX11Lib.Open("libX11.so.6") || !mX11Lib.GetProc(&xSetErrorHandler, "XSetErrorHandler") ||
        !mX11Lib.GetProc(&xGetWindowAttributes, "XGetWindowAttributes") ||
        !mX11Lib.GetProc(&xSynchronize, "XSynchronize")) {
        mX11Lib.Close();
    }

    if (!mX11XcbLib.Open("libX11-xcb.so.1") ||
        !mX11XcbLib.GetProc(&xGetXCBConnection, "XGetXCBConnection")) {
        mX11XcbLib.Close();
    }
}
X11Functions::~X11Functions() = default;

bool X11Functions::IsX11Loaded() const {
    return mX11Lib.Valid();
}

bool X11Functions::IsX11XcbLoaded() const {
    return mX11XcbLib.Valid();
}

struct DebugX11 {
    static DebugX11* sDebug;
    static void Init(Display* display) {
        if (sDebug == nullptr) {
            auto debug = std::make_unique<DebugX11>();
            if (!debug->x.IsX11Loaded()) {
                return;
            }

            debug->previousHandler = debug->x.xSetErrorHandler(&HandleError);
            sDebug = debug.release();
        }

        // Make all X11 calls synchronous so that the error handler is called immediately.
        sDebug->x.xSynchronize(display, 1);
    }

    static int HandleError(Display* d, XErrorEvent* e) {
        dawn::ErrorLog()
            << "An X11 error happened, triggering a breakpoint, the culprit will be in the stack.";
        dawn::BreakPoint();

        int result = sDebug->previousHandler(d, e);
        return result;
    }

    X11Functions x;
    XErrorHandler previousHandler = nullptr;
};
DebugX11* DebugX11::sDebug = nullptr;

void SynchronouslyDebugX11(Display* display) {
    DebugX11::Init(display);
}

}  // namespace dawn::native
