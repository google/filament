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

#ifndef SRC_DAWN_NODE_BINDING_EVENTTARGET_H_
#define SRC_DAWN_NODE_BINDING_EVENTTARGET_H_

#include <string>
#include <unordered_map>

#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// This is used for attribute based listeners like onuncapturederror.
// The way attribute based listeners work is, setting onuncapturederror
// to a callback creates a listener that calls the callback. That listener
// is then added to the EventTarget via addEventListener.
// If onuncapturederror is set again then the callback on the listener is updated.
// If onuncapturederror is set to null then the listener is removed.
// This design dictates the order that listeners are called.
// This is tested in
// webgpu:api,operation,uncapturederror:onuncapturederror_order_wrt_addEventListener:*
class RegisteredEventListener {
  public:
    RegisteredEventListener(Napi::Env env, interop::EventHandler callback);
    Napi::Function listener() const;
    Napi::Function callback() const;
    void setCallback(interop::EventHandler callback);

  private:
    // Static Napi callback, 'this' (RegisteredEventListener instance) is passed as data.
    static Napi::Value StaticCallCallback(Napi::Env env, const Napi::CallbackInfo& info);

    // TODO(crbug.com/420932896): The Reference here could cause a GC loop and prevent cleanup
    Napi::FunctionReference listener_;  // this is passed to add/removeEventListener
    Napi::FunctionReference callback_;  // this is called by listener_
};

// Holds a reference to a node native EventTarget and forward calls to these
// functions to that native object. We do this because GPUDevice is supposed
// to be an actual EventTarget. By wrapping we can make sure the behavior is
// the same. Ideally GPUDevice would inherit from EventTarget but that appears
// to be impossible using Napi.
class EventTarget : public virtual interop::EventTarget {
  public:
    explicit EventTarget(Napi::Env env);
    ~EventTarget() override;

    void addEventListener(
        Napi::Env,
        std::string type,
        std::optional<interop::EventListener> callback,
        std::optional<std::variant<interop::AddEventListenerOptions, bool>> options) override;
    void removeEventListener(
        Napi::Env,
        std::string type,
        std::optional<interop::EventListener> callback,
        std::optional<std::variant<interop::EventListenerOptions, bool>> options) override;
    bool dispatchEvent(Napi::Env, interop::Event event) override;

    const RegisteredEventListener* getAttributeRegisteredEventListener(
        const std::string& type) const;
    void setAttributeEventListener(Napi::Env env,
                                   const std::string& type,
                                   interop::EventHandler callback);

  private:
    // TODO(crbug.com/420932896): The Reference here is a GC root and would do
    // eventTargetRef->EventTarget->closure->JS
    // GPUDevice->binding::GPUDevice->bindings::EventTargetRef->eventTargetRef. which would prevent
    // cleanup.
    Napi::Reference<Napi::Object> eventTargetRef_;

    // Maps a RegisteredEventListener to an attribute name (eg: 'uncapturederror')
    std::unordered_map<std::string, RegisteredEventListener> attributeListeners_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_EVENTTARGET_H_
