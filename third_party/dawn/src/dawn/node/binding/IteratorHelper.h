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

#ifndef SRC_DAWN_NODE_BINDING_ITERATORHELPER_H_
#define SRC_DAWN_NODE_BINDING_ITERATORHELPER_H_

#include <string>

#include "src/dawn/node/interop/NodeAPI.h"

namespace wgpu::binding {

template <typename T>
class IteratorHelper {
  public:
    IteratorHelper(const T& collection, Napi::Object parent)
        : iter(collection.begin()), end(collection.end()), ref(Napi::Persistent(parent)) {}
    ~IteratorHelper() { ref.Unref(); }

    static void Cleanup(Napi::Env env, IteratorHelper* helper) { delete helper; }

    bool HaveMoreValues() { return iter != end; }

    Napi::Value GetNextValue(Napi::Env env) { return interop::ToJS(env, *iter++); }

  private:
    typename T::const_iterator iter;
    typename T::const_iterator end;
    Napi::ObjectReference ref;
};

template <typename T>
Napi::Object CreateIterator(const Napi::CallbackInfo& info, const T& collection) {
    Napi::Env env = info.Env();
    Napi::Object iterObj = Napi::Object::New(env);

    auto* helper = new IteratorHelper{collection, info.This().As<Napi::Object>()};
    iterObj.AddFinalizer(std::remove_reference_t<decltype(*helper)>::Cleanup, helper);

    iterObj.Set("next", Napi::Function::New(
                            env,
                            [helper](const Napi::CallbackInfo& info) {
                                Napi::Env env = info.Env();
                                Napi::Object result = Napi::Object::New(env);

                                if (helper->HaveMoreValues()) {
                                    result.Set("value", helper->GetNextValue(env));
                                    result.Set("done", Napi::Boolean::New(env, false));
                                } else {
                                    result.Set("done", Napi::Boolean::New(env, true));
                                }

                                return result;
                            },
                            "next"));

    // Set the iterator symbol to make it iterable
    iterObj.Set(
        Napi::Symbol::WellKnown(env, "iterator"),
        Napi::Function::New(
            env, [](const Napi::CallbackInfo& info) { return info.This(); }, "[Symbol.iterator]"));

    return iterObj;
}

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_ITERATORHELPER_H_
