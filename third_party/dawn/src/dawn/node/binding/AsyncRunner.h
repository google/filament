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

#ifndef SRC_DAWN_NODE_BINDING_ASYNCRUNNER_H_
#define SRC_DAWN_NODE_BINDING_ASYNCRUNNER_H_

#include <stdint.h>

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <utility>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/interop/Core.h"
#include "src/dawn/node/interop/NodeAPI.h"

namespace wgpu::binding {

// AsyncRunner is used to poll a wgpu::Device with calls to Tick() while there are asynchronous
// tasks in flight.
class AsyncRunner {
  public:
    // Creates an AsyncRunner to use to process events on the instance.
    static std::shared_ptr<AsyncRunner> Create(dawn::native::Instance* instance);

    // Begin() should be called when a new asynchronous task is started.
    // If the number of executing asynchronous tasks transitions from 0 to 1, then a function
    // will be scheduled on the main JavaScript thread to call wgpu::Device::Tick() whenever the
    // thread is idle. This will be repeatedly called until the number of executing asynchronous
    // tasks reaches 0 again.
    void Begin(Napi::Env env);

    // End() should be called once the asynchronous task has finished.
    // Every call to Begin() should eventually result in a call to End().
    void End();

    // Rejects the promise after the current task in the event loop. This is useful to preserve
    // some of the semantics of WebGPU w.r.t. the JavaScript event loop. Reject() can be called
    // any time, but callers need to make sure that the Promise is (rejected or resolved) only
    // once.
    void Reject(Napi::Env env, interop::Promise<void> promise, Napi::Error error);

    // Use AsyncRunner::Create instead of this constructor.
    explicit AsyncRunner(dawn::native::Instance* instance);

  private:
    void ScheduleProcessEvents(Napi::Env env);

    std::weak_ptr<AsyncRunner> weak_this_;
    const dawn::native::Instance* const instance_;
    uint64_t tasks_waiting_ = 0;
    bool process_events_queued_ = false;
};

// AsyncTask is a RAII helper for calling AsyncRunner::Begin() on construction, and
// AsyncRunner::End() on destruction, that also encapsulates the promise generally
// associated with any async task.
template <typename T>
class AsyncContext {
  public:
    // Constructor.
    // Calls AsyncRunner::Begin()
    inline AsyncContext(Napi::Env env,
                        const interop::PromiseInfo& info,
                        std::shared_ptr<AsyncRunner> runner)
        : env(env), promise(env, info), runner_(runner) {
        runner_->Begin(env);
    }

    // Destructor.
    // Calls AsyncRunner::End()
    inline ~AsyncContext() { runner_->End(); }

    // Note these are public to allow for access for the callbacks that take ownership of this
    // context.
    Napi::Env env;
    interop::Promise<T> promise;

  private:
    AsyncContext(const AsyncContext&) = delete;
    AsyncContext& operator=(const AsyncContext&) = delete;

    std::shared_ptr<AsyncRunner> runner_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_ASYNCRUNNER_H_
