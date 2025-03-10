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

#ifndef SRC_DAWN_NATIVE_X11FUNCTIONS_H_
#define SRC_DAWN_NATIVE_X11FUNCTIONS_H_

#include "dawn/common/DynamicLib.h"
#include "dawn/native/Error.h"

#include "dawn/common/xlib_with_undefs.h"

class DynamicLib;

namespace dawn::native {

// A helper class that dynamically loads the x11 and x11-xcb libraries that might not be present
// on all platforms Dawn is deployed on. Note that x11-xcb might not be present even if x11 is.
class X11Functions {
  public:
    X11Functions();
    ~X11Functions();

    bool IsX11Loaded() const;
    bool IsX11XcbLoaded() const;

    // Functions from x11
    decltype(&::XSetErrorHandler) xSetErrorHandler = nullptr;
    decltype(&::XGetWindowAttributes) xGetWindowAttributes = nullptr;

    // Calling XSynchronize(display, true) can help debug "X Error of failed request" messages.
    decltype(&::XSynchronize) xSynchronize = nullptr;

    // Functions from x11-xcb
    decltype(&::XGetXCBConnection) xGetXCBConnection = nullptr;

  private:
    DynamicLib mX11Lib;
    DynamicLib mX11XcbLib;
};

// Make future X11 calls synchronously trigger a breakpoint if they cause an X11 error.
void SynchronouslyDebugX11(Display* display);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_X11FUNCTIONS_H_
