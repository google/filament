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

#ifndef SRC_DAWN_NATIVE_EXECUTIONQUEUE_H_
#define SRC_DAWN_NATIVE_EXECUTIONQUEUE_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/SerialMap.h"
#include "dawn/common/Time.h"
#include "dawn/common/ityp_array.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

// Queue task priority can be specified such that when user facing code is called, i.e. |WaitAny|,
// we can do the minimum amount of work to remain responsive. For other classes of priority, we may
// defer and execute them either on another thread if full spontaneous is enabled, or when
// |Tick| is called in the meantime. Higher numeric values correspond to higher priority. Note that
// this is loosely based off of TaskPriority in Chromium:
//   https://source.chromium.org/chromium/chromium/src/+/main:base/task/task_traits.h
// Note that when the queue is asked to process tasks of a given priorty, it will process any tasks
// of that priority or higher.
enum class QueuePriority : size_t {
    // This will always be the lowest priority available. We use 1 instead of 0 because we implement
    // a decrement operator to iterate through the priorities and by reserving 0, we can assert that
    // we don't underflow.
    Lowest = 1,

    BestEffort = Lowest,
    UserVisible,

    Highest = UserVisible,
};

// Represents an engine which processes a stream of GPU work. It handles the tracking and
// update of the various ExecutionSerials related to that work.
// TODO(dawn:831, dawn:1413): Make usage of the ExecutionQueue thread-safe. Right now it is
// only partially safe - where observation of the last-submitted and pending serials is atomic.
class ExecutionQueueBase : public ApiObjectBase {
  public:
    using Task = std::function<void()>;

    // Implementations of this class are registered to the queue such that when the serial tracked
    // by the queue increases, the processor is notified and updated as well. Prefer registering a
    // SerialProcessor over tracking a SerialTask when the task would need to keep track of extended
    // state, happen frequently, or acquire the same lock multiple times if split into separate
    // tasks. Implementations must also be thread-safe.
    class SerialProcessor : public RefCounted {
      public:
        // This is the entry point that's fired whenever the queue's internal completed serial is
        // updated.
        virtual void UpdateCompletedSerialTo(ExecutionSerial completedSerial) = 0;

        // Implementations need to implement this shut down function to set some internal state such
        // that any further tracking on the processor is processed immediately since the
        // |UpdateCompletedSerialTo| entry point will no longer be called after this point.
        virtual void AssumeCommandsComplete() = 0;
    };

    // The latest serial known to have completed execution on the queue.
    ExecutionSerial GetCompletedCommandSerial() const;
    // The serial of the latest batch of work sent for execution.
    ExecutionSerial GetLastSubmittedCommandSerial() const;
    // The serial of the batch that is currently pending submission.
    ExecutionSerial GetPendingCommandSerial() const;
    // The serial by which time all currently submitted or pending operations will be completed.
    ExecutionSerial GetScheduledWorkDoneSerial() const;
    // Whether the execution queue has scheduled commands to be submitted or executing.
    bool HasScheduledCommands() const;

    // Check for passed fences and set the new completed serial. Note that the two functions below
    // effectively do the same thing initially, however, |UpdateCompletedSerials| may additionally
    // trigger user callbacks. Note that for the purpose of going forwards, |CheckPassedSerials|
    // should not be used anymore.
    // TODO(crbug.com/42240396): Remove |CheckPassedSerials| in favor of |UpdateCompletedSerial|.
    MaybeError CheckPassedSerials();
    MaybeError UpdateCompletedSerial(QueuePriority priority);

    // For the commands being internally recorded in backend, that were not urgent to submit, this
    // method makes them to be submitted as soon as possible in next ticks.
    virtual void ForceEventualFlushOfCommands() = 0;

    // Ensures that all commands which were recorded are flushed upto the given serial.
    MaybeError EnsureCommandsFlushed(ExecutionSerial serial);

    // Submit any pending commands that are enqueued.
    MaybeError SubmitPendingCommands();

    // During shut down of device, some operations might have been started since the last submit
    // and waiting on a serial that doesn't have a corresponding fence enqueued. Fake serials to
    // make all commands look completed.
    void AssumeCommandsComplete();

    // Increment mLastSubmittedSerial when we submit the next serial
    void IncrementLastSubmittedCommandSerial();

    // Waits for GPU to finish, checks errors and gets ready for destruction. This is only used when
    // properly destructing the device. For a real device loss, this function doesn't need to be
    // called since the driver already closed all resources.
    MaybeError WaitForIdleForDestruction();

    // Wait at most `timeout` synchronously for the ExecutionSerial to pass.
    MaybeError WaitForQueueSerial(ExecutionSerial serial, Nanoseconds timeout);

    // Registers a SerialProcessor that will be notified of serial updates. Note that serial
    // processors are always notified before handling serial tasks.
    void RegisterSerialProcessor(QueuePriority priority, Ref<SerialProcessor>&& serialProcessor);

    // Tracks new tasks to complete when |serial| is reached.
    void TrackSerialTask(QueuePriority priority, ExecutionSerial serial, Task&& task);

    // In the 'Normal' mode, currently recorded commands in the backend submitted in the next Tick.
    // However in the 'Passive' mode, the submission will be postponed as late as possible, for
    // example, until the client has explictly issued a submission.
    enum class SubmitMode { Normal, Passive };

  protected:
    using ApiObjectBase::ApiObjectBase;

    ~ExecutionQueueBase() override;

    static constexpr ExecutionSerial kWaitSerialTimeout = kBeginningOfGPUTime;

    // Currently, the queue has two paths for serial updating, one is via DeviceBase::Tick which
    // calls into the backend specific polling mechanisms implemented in
    // CheckAndUpdateCompletedSerials. Alternatively, the backend can actively call
    // UpdateCompletedSerialTo when a new serial is complete to make forward progress proactively.
    void UpdateCompletedSerialTo(QueuePriority priority, ExecutionSerial completedSerial);

  private:
    // Each backend should implement to check their passed fences if there are any and return a
    // completed serial. Return 0 should indicate no fences to check.
    virtual ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() = 0;

    // Backend specific wait function, returns kWaitSerialTimeout if we did not successfully wait
    // for |waitSerial|. The default implementations just waits on the internal condition variable
    // protected |mCompletedSerial|.
    virtual ResultOrError<ExecutionSerial> WaitForQueueSerialImpl(ExecutionSerial waitSerial,
                                                                  Nanoseconds timeout);

    // Backend specific wait for idle function.
    virtual MaybeError WaitForIdleForDestructionImpl() = 0;

    // Indicates whether the backend has pending commands to be submitted as soon as possible.
    virtual bool HasPendingCommands() const = 0;

    // Submit any pending commands that are enqueued.
    virtual MaybeError SubmitPendingCommandsImpl() = 0;

    void UpdateCompletedSerialToInternal(QueuePriority priority,
                                         ExecutionSerial completedSerial,
                                         bool forceTasksForDestroy = false);

    // |mCompletedSerial| tracks the last completed command serial that the fence has returned.
    // |mLastSubmittedSerial| tracks the last submitted command serial.
    // During device removal, the serials could be artificially incremented to make it appear as if
    // commands have been completed.
    MutexCondVarProtected<uint64_t> mCompletedSerial = static_cast<uint64_t>(kBeginningOfGPUTime);
    std::atomic<uint64_t> mLastSubmittedSerial = static_cast<uint64_t>(kBeginningOfGPUTime);

    // Mutex, condition variable, and boolean statuses are used by the class to synchronize task
    // completion to ensure that:
    //   1) Callback ordering is guaranteed.
    //   2) Re-entrant callbacks do not cause lock-inversion issues w.r.t this lock and the
    //      device lock.
    template <typename T>
    using QueuePriorityArray =
        ityp::array<QueuePriority, T, static_cast<size_t>(QueuePriority::Highest) + 1>;

    struct State {
        bool mCallingCallbacks = false;
        bool mWaitingForIdle = false;
        bool mAssumeCompleted = false;
        QueuePriorityArray<std::vector<Ref<SerialProcessor>>> mWaitingProcessors;
        QueuePriorityArray<SerialMap<ExecutionSerial, Task>> mWaitingTasks;
    };
    MutexCondVarProtected<State> mState;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_EXECUTIONQUEUE_H_
