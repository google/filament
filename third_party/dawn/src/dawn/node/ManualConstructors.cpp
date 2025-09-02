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

#include "src/dawn/node/ManualConstructors.h"

namespace wgpu::interop {

ManualConstructors::ManualConstructors(Napi::Env env) {
    // Define GPUUncapturedErrorEvent as a JavaScript object because it
    // must inherit from Event otherwise it can not be passed to an EventTarget
    // and this is apparently impossible in Napi if it's a C++ based object.
    const char* gpuUncapturedErrorEventJs = R"(
    (function() {
      return class GPUUncapturedErrorEvent extends Event {
        constructor(type, eventInitDict) {
          super(type, eventInitDict);
          if (!eventInitDict || eventInitDict.error === undefined) {
            throw new TypeError("Failed to construct 'GPUUncapturedErrorEvent': Required member 'error' in 'eventInitDict' is undefined.");
          }
          this.error = eventInitDict.error;
        }
      };
    })())";
    Napi::Value gpuUncapturedErrorEvent_value =
        env.RunScript(Napi::String::New(env, gpuUncapturedErrorEventJs));
    GPUUncapturedErrorEvent_ctor =
        Napi::Persistent(gpuUncapturedErrorEvent_value.As<Napi::Function>());

    // Lookup the DOMException class
    Napi::Object global = env.Global();
    DOMException_ctor = Napi::Persistent(global.Get("DOMException").As<Napi::Function>());

    // Lookup the EventTarget class
    EventTarget_ctor = Napi::Persistent(global.Get("EventTarget").As<Napi::Function>());

    // Define GPUUncapturedErrorEvent as a JavaScript object because it
    // must inherit from Event otherwise it can not be passed to an EventTarget
    // and this is apparently impossible in Napi if it's a C++ based object.
    // We have to pass in the DOMException constructor from above so we first
    // generate a function we can pass it to using RunScript. We then call
    // that function and pass in the DOMException constructor so we can
    // make a class that extends it.
    const char* gpuPipelineErrorJs = R"(
    (function() {
      return class GPUPipelineError extends DOMException {
        constructor(message, gpuPipelineErrorInit) {
          super(message, "GPUPipelineError");
          if (!gpuPipelineErrorInit || gpuPipelineErrorInit.reason === undefined) {
            throw new TypeError("Failed to construct 'GPUPipelineError': Required member 'reason' in 'gpuPipelineErrorInit' is undefined.");
          }
          this.reason = gpuPipelineErrorInit.reason;
        }
      };
    })())";
    Napi::Value gpuPipelineError_value = env.RunScript(Napi::String::New(env, gpuPipelineErrorJs));
    GPUPipelineError_ctor = Napi::Persistent(gpuPipelineError_value.As<Napi::Function>());
}

}  // namespace wgpu::interop
