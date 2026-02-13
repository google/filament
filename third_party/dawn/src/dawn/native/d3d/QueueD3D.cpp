// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/d3d/QueueD3D.h"

#include <algorithm>
#include <array>
#include <utility>

#include "dawn/native/WaitAnySystemEvent.h"

namespace dawn::native::d3d {

Queue::~Queue() = default;

ResultOrError<SystemEventReceiver> Queue::GetSystemEventReceiver() {
    SystemEventReceiver receiver;
    bool result = mAvailableEventReceivers.Use([&](auto receivers) -> auto {
        if (receivers->empty()) {
            return false;
        }
        receiver = std::move(receivers->back());
        receivers->pop_back();
        return true;
    });
    if (!result) {
        HANDLE fenceEvent =
            ::CreateEvent(nullptr, /*bManualReset=*/true, /*bInitialState=*/false, nullptr);
        if (fenceEvent == nullptr) {
            return DAWN_INTERNAL_ERROR("CreateEvent failed");
        }
        receiver = SystemEventReceiver(SystemHandle::Acquire(fenceEvent));
    }

    return receiver;
}

MaybeError Queue::ReturnSystemEventReceivers(std::span<SystemEventReceiver> receivers) {
    for (const auto& receiver : receivers) {
        if (!ResetEvent(receiver.GetPrimitive().Get())) {
            return DAWN_INTERNAL_ERROR("ResetEvent failed");
        }
    }
    mAvailableEventReceivers.Use([&](auto availableEventReceivers) {
        size_t count =
            std::min(receivers.size(), kMaxEventReceivers - availableEventReceivers->size());
        availableEventReceivers->insert(availableEventReceivers->end(),
                                        std::make_move_iterator(receivers.begin()),
                                        std::make_move_iterator(receivers.begin() + count));
    });
    return {};
}

ResultOrError<ExecutionSerial> Queue::WaitForQueueSerialImpl(ExecutionSerial waitSerial,
                                                             Nanoseconds timeout) {
    ExecutionSerial completedSerial = GetCompletedCommandSerial();
    if (waitSerial <= completedSerial) {
        return waitSerial;
    }

    auto receiver = mSystemEventReceivers->TakeOne(waitSerial);
    if (!receiver) {
        DAWN_TRY_ASSIGN(receiver, GetSystemEventReceiver());
        SetEventOnCompletion(waitSerial, receiver->GetPrimitive().Get());
    }

    bool ready = false;
    std::array<std::pair<const dawn::native::SystemEventReceiver&, bool*>, 1> events{
        {{*receiver, &ready}}};
    DAWN_ASSERT(waitSerial <= GetLastSubmittedCommandSerial());
    bool didComplete = WaitAnySystemEvent(events.begin(), events.end(), timeout);
    // Return the SystemEventReceiver to the pool of receivers so it can be re-waited in the
    // future.
    // The caller should call UpdateCompletedSerial() which will clear passed system events.
    mSystemEventReceivers->Enqueue(std::move(*receiver), waitSerial);

    return didComplete ? waitSerial : kWaitSerialTimeout;
}

MaybeError Queue::RecycleSystemEventReceivers(ExecutionSerial completedSerial) {
    std::vector<SystemEventReceiver> receivers;
    mSystemEventReceivers.Use([&](auto systemEventReceivers) {
        for (auto& receiver : systemEventReceivers->IterateUpTo(completedSerial)) {
            receivers.emplace_back(std::move(receiver));
        }
        systemEventReceivers->ClearUpTo(completedSerial);
    });

    DAWN_TRY(ReturnSystemEventReceivers(std::span(receivers.data(), receivers.size())));

    return {};
}

}  // namespace dawn::native::d3d
