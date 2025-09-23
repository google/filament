// Copyright 2025 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
