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

#include "src/dawn/node/binding/AsyncRunner.h"

#include <cassert>
#include <limits>

namespace wgpu::binding {

// static
std::shared_ptr<AsyncRunner> AsyncRunner::Create(dawn::native::Instance* instance) {
    auto runner = std::make_shared<AsyncRunner>(instance);
    runner->weak_this_ = runner;
    return runner;
}

AsyncRunner::AsyncRunner(dawn::native::Instance* instance) : instance_(instance) {}

void AsyncRunner::Begin(Napi::Env env) {
    assert(tasks_waiting_ != std::numeric_limits<decltype(tasks_waiting_)>::max());
    if (tasks_waiting_++ == 0) {
        ScheduleProcessEvents(env);
    }
}

void AsyncRunner::End() {
    assert(tasks_waiting_ > 0);
    tasks_waiting_--;
}

void AsyncRunner::ScheduleProcessEvents(Napi::Env env) {
    // TODO(crbug.com/dawn/1127): We probably want to reduce the frequency at which this gets
    // called.
    if (process_events_queued_) {
        return;
    }
    if (tasks_waiting_ == 0) {
        return;
    }
    process_events_queued_ = true;

    auto weak_self = weak_this_;
    env.Global()
        .Get("setImmediate")
        .As<Napi::Function>()
        .Call({
            // TODO(crbug.com/dawn/1127): Create once, reuse.
            Napi::Function::New(env,
                                [weak_self, env](const Napi::CallbackInfo&) {
                                    auto self = weak_self.lock();
                                    if (self == nullptr) {
                                        return;
                                    }

                                    self->process_events_queued_ = false;
                                    wgpuInstanceProcessEvents(self->instance_->Get());
                                    self->ScheduleProcessEvents(env);
                                }),
        });
}

void AsyncRunner::Reject(Napi::Env env, interop::Promise<void> promise, Napi::Error error) {
    env.Global()
        .Get("setImmediate")
        .As<Napi::Function>()
        .Call({Napi::Function::New(
            env, [promise, error](const Napi::CallbackInfo&) { promise.Reject(error); })});
}

}  // namespace wgpu::binding
