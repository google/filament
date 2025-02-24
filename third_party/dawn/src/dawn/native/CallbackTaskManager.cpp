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

#include "dawn/native/CallbackTaskManager.h"

#include <utility>

#include "dawn/common/Assert.h"

namespace dawn::native {

namespace {
struct GenericFunctionTask : CallbackTask {
  public:
    explicit GenericFunctionTask(std::function<void()> func) : mFunction(std::move(func)) {}

  private:
    void FinishImpl() override { mFunction(); }
    void HandleShutDownImpl() override { mFunction(); }
    void HandleDeviceLossImpl() override { mFunction(); }

    std::function<void()> mFunction;
};
}  // namespace

void CallbackTask::Execute() {
    switch (mState) {
        case CallbackState::DeviceLoss:
            HandleDeviceLossImpl();
            break;
        case CallbackState::ShutDown:
            HandleShutDownImpl();
            break;
        default:
            FinishImpl();
    }
}

void CallbackTask::OnShutDown() {
    // Only first state change will have effects in final Execute().
    if (mState != CallbackState::Normal) {
        return;
    }
    mState = CallbackState::ShutDown;
}

void CallbackTask::OnDeviceLoss() {
    if (mState != CallbackState::Normal) {
        return;
    }
    mState = CallbackState::DeviceLoss;
}

CallbackTaskManager::CallbackTaskManager() = default;

CallbackTaskManager::~CallbackTaskManager() = default;

bool CallbackTaskManager::IsEmpty() {
    return mStateAndQueue.Use([](auto stateAndQueue) { return stateAndQueue->mTaskQueue.empty(); });
}

void CallbackTaskManager::AddCallbackTask(std::unique_ptr<CallbackTask> callbackTask) {
    mStateAndQueue.Use([&](auto stateAndQueue) {
        switch (stateAndQueue->mState) {
            case CallbackState::ShutDown:
                callbackTask->OnShutDown();
                break;
            case CallbackState::DeviceLoss:
                callbackTask->OnDeviceLoss();
                break;
            default:
                break;
        }
        stateAndQueue->mTaskQueue.push_back(std::move(callbackTask));
    });
}

void CallbackTaskManager::AddCallbackTask(std::function<void()> callback) {
    AddCallbackTask(std::make_unique<GenericFunctionTask>(std::move(callback)));
}

void CallbackTaskManager::HandleDeviceLoss() {
    mStateAndQueue.Use([&](auto stateAndQueue) {
        if (stateAndQueue->mState != CallbackState::Normal) {
            return;
        }
        stateAndQueue->mState = CallbackState::DeviceLoss;
        for (auto& task : stateAndQueue->mTaskQueue) {
            task->OnDeviceLoss();
        }
    });
}

void CallbackTaskManager::HandleShutDown() {
    mStateAndQueue.Use([&](auto stateAndQueue) {
        if (stateAndQueue->mState != CallbackState::Normal) {
            return;
        }
        stateAndQueue->mState = CallbackState::ShutDown;
        for (auto& task : stateAndQueue->mTaskQueue) {
            task->OnShutDown();
        }
    });
}

void CallbackTaskManager::Flush() {
    if (IsEmpty()) {
        return;
    }

    // If a user calls Queue::Submit inside the callback, then the device will be ticked,
    // which in turns ticks the tracker, causing reentrance and dead lock here. To prevent
    // such reentrant call, we remove all the callback tasks from mCallbackTaskManager,
    // update mCallbackTaskManager, then call all the callbacks.
    std::vector<std::unique_ptr<CallbackTask>> allTasks;
    mStateAndQueue.Use([&](auto stateAndQueue) { allTasks.swap(stateAndQueue->mTaskQueue); });

    for (auto& callbackTask : allTasks) {
        callbackTask->Execute();
    }
}

}  // namespace dawn::native
