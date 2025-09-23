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

#include "src/dawn/node/binding/EventTarget.h"

#include <vector>

namespace wgpu::binding {

Napi::Function RegisteredEventListener::listener() const {
    return listener_.Value();
}
Napi::Function RegisteredEventListener::callback() const {
    return callback_.Value();
}
void RegisteredEventListener::setCallback(interop::EventHandler callback) {
    callback_ = Napi::Persistent(callback.value());
}

RegisteredEventListener::RegisteredEventListener(Napi::Env env, interop::EventHandler callback)
    : callback_(Napi::Persistent(callback.value())) {
    listener_ = Napi::Persistent(Napi::Function::New(
        env,
        [](const Napi::CallbackInfo& info) -> Napi::Value {
            return RegisteredEventListener::StaticCallCallback(info.Env(), info);
        },
        "eventListener", this));
}

Napi::Value RegisteredEventListener::StaticCallCallback(Napi::Env env,
                                                        const Napi::CallbackInfo& info) {
    RegisteredEventListener* self = static_cast<RegisteredEventListener*>(info.Data());
    if (self) {
        std::vector<napi_value> args(info.Length());
        for (size_t i = 0; i < info.Length(); ++i) {
            args[i] = info[i];
        }
        return self->callback_.Call(info.This(), args);
    }
    return env.Undefined();
}

// Note: We don't cache many of these lookups since the developer
// is free to patch methods between calls.

// Creates a native EventTarget
EventTarget::EventTarget(Napi::Env env) {
    Napi::Object global = env.Global();
    Napi::Value eventTargetValue = global.Get("EventTarget");
    Napi::Function eventTargetConstructor = eventTargetValue.As<Napi::Function>();
    Napi::Object jsEventTarget = eventTargetConstructor.New({});
    eventTargetRef_ = Napi::Persistent(jsEventTarget);
}
EventTarget::~EventTarget() = default;

// Forward to our internal EventTarget.
void EventTarget::addEventListener(
    Napi::Env env,
    std::string type,
    std::optional<interop::EventListener> callback,
    std::optional<std::variant<interop::AddEventListenerOptions, bool>> options) {
    Napi::Object jsEventTarget = eventTargetRef_.Value();
    Napi::Value addListenerValue = jsEventTarget.Get("addEventListener");
    Napi::Function addListenerFunc = addListenerValue.As<Napi::Function>();

    std::vector<Napi::Value> args({
        interop::ToJS(env, type),
        interop::ToJS(env, callback),
        interop::ToJS(env, options),
    });
    addListenerFunc.Call(jsEventTarget, args);
}

// Forward to our internal EventTarget.
void EventTarget::removeEventListener(
    Napi::Env env,
    std::string type,
    std::optional<interop::EventListener> callback,
    std::optional<std::variant<interop::EventListenerOptions, bool>> options) {
    Napi::Object jsEventTarget = eventTargetRef_.Value();
    Napi::Value removeListenerValue = jsEventTarget.Get("removeEventListener");
    Napi::Function removeListenerFunc = removeListenerValue.As<Napi::Function>();

    std::vector<Napi::Value> args{
        interop::ToJS(env, type),
        interop::ToJS(env, callback),
        interop::ToJS(env, options),
    };
    removeListenerFunc.Call(jsEventTarget, args);
}

// Forward to our internal EventTarget.
bool EventTarget::dispatchEvent(Napi::Env env, interop::Event event) {
    Napi::Object jsEventTarget = eventTargetRef_.Value();
    Napi::Value dispatchEventValue = jsEventTarget.Get("dispatchEvent");
    Napi::Function dispatchEventFunc = dispatchEventValue.As<Napi::Function>();

    std::vector<Napi::Value> args{interop::ToJS(env, event)};
    Napi::Value result = dispatchEventFunc.Call(jsEventTarget, args);
    return result.As<Napi::Boolean>().Value();
}

const RegisteredEventListener* EventTarget::getAttributeRegisteredEventListener(
    const std::string& type) const {
    auto it = attributeListeners_.find(type);
    return it == attributeListeners_.end() ? nullptr : &it->second;
}

void EventTarget::setAttributeEventListener(Napi::Env env,
                                            const std::string& type,
                                            interop::EventHandler callback) {
    auto it = attributeListeners_.find(type);
    bool listener_exists = (it != attributeListeners_.end());

    if (callback.has_value()) {
        if (listener_exists) {
            it->second.setCallback(callback);
        } else {
            auto emplace_result =
                attributeListeners_.emplace(std::piecewise_construct, std::forward_as_tuple(type),
                                            std::forward_as_tuple(env, callback));
            addEventListener(env, type, emplace_result.first->second.listener(), {});
        }
    } else {
        if (listener_exists) {
            removeEventListener(env, type, it->second.listener(), {});
            attributeListeners_.erase(it);
        }
    }
}

}  // namespace wgpu::binding
