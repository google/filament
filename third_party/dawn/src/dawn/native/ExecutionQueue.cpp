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

#include "dawn/native/ExecutionQueue.h"

#include <algorithm>
#include <atomic>
#include <utility>
#include <vector>

#include "dawn/common/Atomic.h"
#include "dawn/native/Device.h"
#include "dawn/native/Error.h"

namespace dawn::native {
namespace {
void PopWaitingTasksInto(ExecutionSerial serial,
                         SerialMap<ExecutionSerial, ExecutionQueueBase::Task>& waitingTasks,
                         std::vector<ExecutionQueueBase::Task>& tasks) {
    for (auto task : waitingTasks.IterateUpTo(serial)) {
        tasks.push_back(std::move(task));
    }
    waitingTasks.ClearUpTo(serial);
}
}  // namespace

inline QueuePriority& operator-=(QueuePriority& priority, size_t i) {
    DAWN_ASSERT(static_cast<size_t>(priority) >= i);
    priority = static_cast<QueuePriority>(static_cast<size_t>(priority) - i);
    return priority;
}

ExecutionQueueBase::~ExecutionQueueBase() {
    for (QueuePriority p = QueuePriority::Highest; p >= QueuePriority::Lowest; p -= 1) {
        DAWN_ASSERT(mState->mWaitingTasks[p].Empty());
    }
}

ExecutionSerial ExecutionQueueBase::GetPendingCommandSerial() const {
    return ExecutionSerial(mLastSubmittedSerial.load(std::memory_order_acquire) + 1);
}

ExecutionSerial ExecutionQueueBase::GetLastSubmittedCommandSerial() const {
    return ExecutionSerial(mLastSubmittedSerial.load(std::memory_order_acquire));
}

ExecutionSerial ExecutionQueueBase::GetCompletedCommandSerial() const {
    return ExecutionSerial(
        mCompletedSerial.Use([](const auto completedSerial) { return *completedSerial; }));
}

ResultOrError<ExecutionSerial> ExecutionQueueBase::WaitForQueueSerialImpl(
    ExecutionSerial waitSerial,
    Nanoseconds timeout) {
    return mCompletedSerial.Use<NotifyType::None>([&](auto completed) {
        if (completed.WaitFor(timeout,
                              [&](auto& x) { return x >= static_cast<uint64_t>(waitSerial); })) {
            return ExecutionSerial(*completed);
        }
        return kWaitSerialTimeout;
    });
}

MaybeError ExecutionQueueBase::WaitForQueueSerial(ExecutionSerial waitSerial, Nanoseconds timeout) {
    // Serial is already complete.
    if (waitSerial <= GetCompletedCommandSerial()) {
        // Ensure that all tasks related to the serial have been triggered.
        UpdateCompletedSerialTo(QueuePriority::UserVisible, waitSerial);
        return {};
    }

    // We currently have two differing implementations for this function depending on whether the
    // backend supports thread safe waits. Note that while currently only the Metal backend
    // explicitly enables thread safe wait, the main blocking backend is D3D11 which is using the
    // value of |mCompletedSerial| within it's implementation of |CheckAndUpdateCompletedSerials|.
    if (GetDevice()->IsToggleEnabled(Toggle::WaitIsThreadSafe)) {
        if (waitSerial > GetLastSubmittedCommandSerial()) {
            auto deviceGuard = GetDevice()->GetGuard();
            // Check submitted command serial again since it could have been incremented already.
            if (waitSerial > GetLastSubmittedCommandSerial()) {
                // Serial has not been submitted yet. Submit it now.
                DAWN_TRY(EnsureCommandsFlushed(waitSerial));
            }
        }

        if (timeout > Nanoseconds(0)) {
            // Wait on the serial if it hasn't passed yet.
            ExecutionSerial completedSerial = kWaitSerialTimeout;
            DAWN_TRY_ASSIGN(completedSerial, WaitForQueueSerialImpl(waitSerial, timeout));
            UpdateCompletedSerialTo(QueuePriority::UserVisible, completedSerial);
            return {};
        }
        return UpdateCompletedSerial(QueuePriority::UserVisible);
    } else {
        // Otherwise, we need to acquire the device lock first.
        auto deviceGuard = GetDevice()->GetGuard();
        if (waitSerial > GetLastSubmittedCommandSerial()) {
            // Serial has not been submitted yet. Submit it now.
            DAWN_TRY(EnsureCommandsFlushed(waitSerial));
        }

        if (timeout > Nanoseconds(0)) {
            // Wait on the serial if it hasn't passed yet.
            ExecutionSerial completedSerial = kWaitSerialTimeout;
            DAWN_TRY_ASSIGN(completedSerial, WaitForQueueSerialImpl(waitSerial, timeout));

            // It's critical to update the completed serial right away. If fences are processed
            // by another thread before CheckAndUpdateCompletedSerials() runs on the current
            // thread, the fence list will be empty, preventing the current thread from
            // determining the true latest serial. Preemptively updating mCompletedSerial
            // ensures CheckAndUpdateCompletedSerials() returns an accurate value, preventing
            // stale data.
            mCompletedSerial.Use(
                [&](auto old) { *old = std::max(*old, static_cast<uint64_t>(completedSerial)); });
        }
        return UpdateCompletedSerial(QueuePriority::UserVisible);
    }
}

MaybeError ExecutionQueueBase::WaitForIdleForDestruction() {
    // Currently waiting for idle for destruction requires the device lock to be held.
    DAWN_ASSERT(GetDevice()->IsLockedByCurrentThreadIfNeeded());
    mState.Use<NotifyType::None>([](auto state) {
        DAWN_ASSERT(!state->mWaitingForIdle);
        state->mWaitingForIdle = true;
    });

    IgnoreErrors(WaitForIdleForDestructionImpl());

    // Prepare to call any remaining outstanding callbacks now.
    QueuePriorityArray<std::vector<Ref<SerialProcessor>>>* processors = nullptr;
    std::vector<Task> tasks;
    ExecutionSerial serial;

    mState.Use<NotifyType::None>([&](auto state) {
        // Wait until we can exclusively call callbacks.
        state.Wait([](auto& x) { return !x.mCallingCallbacks; });

        // We finish tasks all the way up to the pending command serial because otherwise, pending
        // tasks that may be for cleanup won't every be completed. Also, for |buffer.MapAsync|, a
        // lot of backends queue up a clear to initialize the data on those buffers and that clear
        // is pushed into the front of the next pending command, and the buffer's last usage serial
        // is set to the pending command serial to reflect that. If the device is lost before that
        // pending command is ever submitted, the map async task will be left dangling if we only
        // clear up to the completed serial.
        serial = GetPendingCommandSerial();

        // Call all callbacks that for all priorities.
        processors = &state->mWaitingProcessors;
        for (QueuePriority p = QueuePriority::Highest; p >= QueuePriority::Lowest; p -= 1) {
            PopWaitingTasksInto(serial, state->mWaitingTasks[p], tasks);
        }
        state->mCallingCallbacks = true;
    });

    // Always call the processors before processing individual tasks.
    DAWN_ASSERT(processors);
    for (QueuePriority p = QueuePriority::Highest; p >= QueuePriority::Lowest; p -= 1) {
        for (auto& processor : (*processors)[p]) {
            processor->UpdateCompletedSerialTo(serial);
        }
    }

    // Call the callbacks without holding the lock on the ExecutionQueue to avoid lock-inversion
    // issues when dealing with potential re-entrant callbacks.
    for (auto task : tasks) {
        task();
    }

    mState->mCallingCallbacks = false;
    return {};
}

MaybeError ExecutionQueueBase::CheckPassedSerials() {
    ExecutionSerial completedSerial;
    DAWN_TRY_ASSIGN(completedSerial, CheckAndUpdateCompletedSerials());

    DAWN_ASSERT(completedSerial <=
                ExecutionSerial(mLastSubmittedSerial.load(std::memory_order_acquire)));

    // Atomically set mCompletedSerial to completedSerial if completedSerial is larger.
    mCompletedSerial.Use(
        [&](auto old) { *old = std::max(*old, static_cast<uint64_t>(completedSerial)); });
    return {};
}

MaybeError ExecutionQueueBase::UpdateCompletedSerial(QueuePriority priority) {
    ExecutionSerial completedSerial;
    DAWN_TRY_ASSIGN(completedSerial, CheckAndUpdateCompletedSerials());

    DAWN_ASSERT(completedSerial <=
                ExecutionSerial(mLastSubmittedSerial.load(std::memory_order_acquire)));
    UpdateCompletedSerialTo(priority, completedSerial);
    return {};
}

void ExecutionQueueBase::RegisterSerialProcessor(QueuePriority priority,
                                                 Ref<SerialProcessor>&& serialProcessor) {
    // Serial processor registration should always happen at queue initialization.
    DAWN_ASSERT(mCompletedSerial.Use<NotifyType::None>([](auto completedSerial) {
        return *completedSerial;
    }) == static_cast<uint64_t>(kBeginningOfGPUTime));
    mState.Use<NotifyType::None>([&](auto state) {
        state->mWaitingProcessors[priority].push_back(std::move(serialProcessor));
    });
}

// Tasks may execute synchronously if the given serial has already passed or during device
// destruction. As a result, callers should ensure that the calling thread releases any locks that
// will be taken by the task prior to calling TrackSerialTask.
void ExecutionQueueBase::TrackSerialTask(QueuePriority priority,
                                         ExecutionSerial serial,
                                         Task&& task) {
    bool tracked = mState.Use<NotifyType::None>([&](auto state) {
        if (!state->mAssumeCompleted && serial > GetCompletedCommandSerial()) {
            state->mWaitingTasks[priority].Enqueue(std::move(task), serial);
            return true;
        }
        return false;
    });
    if (!tracked) {
        task();
    }
}

void ExecutionQueueBase::UpdateCompletedSerialTo(QueuePriority priority,
                                                 ExecutionSerial completedSerial) {
    UpdateCompletedSerialToInternal(priority, completedSerial);
}

void ExecutionQueueBase::UpdateCompletedSerialToInternal(QueuePriority priority,
                                                         ExecutionSerial newCompletedSerial,
                                                         bool forceTasksForDestroy) {
    QueuePriorityArray<std::vector<Ref<SerialProcessor>>>* processors = nullptr;
    std::vector<Task> tasks;

    // Note that we need to determine whether we are waiting for idle before updating the completed
    // serial because some backends WaitForIdleForDestructionImpl may be implemented via a call to
    // WaitForQueueSerial which (by default without overrides), waits on the completed serial value.
    // If we updated the serial value before checking the other pieces of state, a thread destroying
    // the Queue calling WaitForIdleForDestruction, could end up being woken up and destroying the
    // Queue device before the rest of this function completes. By checking the state first before
    // updating the serial, however, we avoid waking up the thread that's waiting for idle until we
    // have completed using the queue.
    bool waitingForIdle = mState.Use<NotifyType::None>([&](auto state) {
        if (state->mWaitingForIdle && !forceTasksForDestroy) {
            // If we are waiting for idle, then the callbacks will be fired there. It is currently
            // necessary to avoid calling the callbacks in this function and doing it in the
            // |WaitForIdleForDestruction| call because |WaitForIdleForDestruction| is called while
            // holding the device lock and any re-entrant callbacks may also try to acquire the
            // device lock. As a result, if the main thread is waiting for idle, and another thread
            // is trying to update the completed serial and call callbacks, it could deadlock. Once
            // we update |WaitForIdleForDestruction| to release the device lock on the wait, we may
            // be able to simplify the code here. Note that skipping this when
            // |forceTasksForDestroy| is currently ok because that branch is only called when we are
            // also holding the device lock, either via a Destroy or via an error that is being
            // handled.
            return true;
        }

        // Wait until we can exclusively call callbacks.
        state.Wait([](auto& x) { return !x.mCallingCallbacks; });

        // Call all callbacks that for the given priority and anything of higher priority as well.
        processors = &state->mWaitingProcessors;
        for (QueuePriority p = QueuePriority::Highest; p >= priority; p -= 1) {
            PopWaitingTasksInto(newCompletedSerial, state->mWaitingTasks[p], tasks);
        }
        state->mCallingCallbacks = true;
        return false;
    });

    // Update the serial now that we know whether we are waiting for idle.
    mCompletedSerial.Use([&](auto completedSerial) {
        *completedSerial = std::max(*completedSerial, static_cast<uint64_t>(newCompletedSerial));
    });

    // Always call the processors before processing individual tasks.
    if (processors) {
        for (QueuePriority p = QueuePriority::Highest; p >= priority; p -= 1) {
            for (auto& processor : (*processors)[p]) {
                processor->UpdateCompletedSerialTo(newCompletedSerial);
            }
        }
    }

    // Call the callbacks without holding the lock on the ExecutionQueue to avoid lock-inversion
    // issues when dealing with potential re-entrant callbacks.
    for (auto task : tasks) {
        task();
    }

    if (!waitingForIdle) {
        mState->mCallingCallbacks = false;
    }
}

MaybeError ExecutionQueueBase::EnsureCommandsFlushed(ExecutionSerial serial) {
    DAWN_ASSERT(serial <= GetPendingCommandSerial());
    if (serial > GetLastSubmittedCommandSerial()) {
        ForceEventualFlushOfCommands();
        DAWN_TRY(SubmitPendingCommands());
        DAWN_ASSERT(serial <= GetLastSubmittedCommandSerial());
    }
    return {};
}

MaybeError ExecutionQueueBase::SubmitPendingCommands() {
    return SubmitPendingCommandsImpl();
}

void ExecutionQueueBase::AssumeCommandsComplete() {
    mState.Use<NotifyType::None>([](auto state) { state->mAssumeCompleted = true; });

    // Bump serials so any pending callbacks can be fired.
    // TODO(crbug.com/dawn/831): This is called during device destroy, which is not
    // thread-safe yet. Two threads calling destroy would race setting these serials.
    ExecutionSerial completed =
        ExecutionSerial(mLastSubmittedSerial.fetch_add(1u, std::memory_order_release) + 1);
    // Force any waiting tasks to execute. This will ensure that any tasks that were scheduled
    // after WaitForIdleForDestruction being called are completed.
    UpdateCompletedSerialToInternal(QueuePriority::Lowest, completed, true);

    // Update all the processors to let them know that they should assume commands are complete,
    // then release our reference to them.
    std::vector<Ref<SerialProcessor>> processors;
    mState.Use<NotifyType::None>([&](auto state) {
        for (QueuePriority p = QueuePriority::Highest; p >= QueuePriority::Lowest; p -= 1) {
            processors.insert(processors.end(), state->mWaitingProcessors[p].begin(),
                              state->mWaitingProcessors[p].end());
            state->mWaitingProcessors[p].clear();
        }
    });
    for (auto& processor : processors) {
        processor->AssumeCommandsComplete();
    }
}

void ExecutionQueueBase::IncrementLastSubmittedCommandSerial() {
    mLastSubmittedSerial.fetch_add(1u, std::memory_order_release);
}

bool ExecutionQueueBase::HasScheduledCommands() const {
    return mLastSubmittedSerial.load(std::memory_order_acquire) >
               mCompletedSerial.Use([](auto completedSerial) { return *completedSerial; }) ||
           HasPendingCommands();
}

// All prevously submitted works at the moment will supposedly complete at this serial.
// Internally the serial is computed according to whether frontend and backend have pending
// commands. There are 4 cases of combination:
//   1) Frontend(No), Backend(No)
//   2) Frontend(No), Backend(Yes)
//   3) Frontend(Yes), Backend(No)
//   4) Frontend(Yes), Backend(Yes)
// For case 1, we don't need the serial to track the task as we can ack it right now.
// For case 2 and 4, there will be at least an eventual submission, so we can use
// 'GetPendingCommandSerial' as the serial.
// For case 3, we can't use 'GetPendingCommandSerial' as it won't be submitted surely. Instead we
// use 'GetLastSubmittedCommandSerial', which must be fired eventually.
ExecutionSerial ExecutionQueueBase::GetScheduledWorkDoneSerial() const {
    return HasPendingCommands() ? GetPendingCommandSerial() : GetLastSubmittedCommandSerial();
}

}  // namespace dawn::native
