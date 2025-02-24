//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MultiThreadSteps.h:
//   Synchronization help for tests that use multiple threads.

#include "gl_raii.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

class EGLWindow;

namespace angle
{
namespace
{
// The following class is used by tests that need multiple threads that coordinate their actions
// via an enum of "steps".  This enum is the template type E.  The enum must have at least the
// following values:
//
// - Finish   This value indicates that one thread has finished its last step and is cleaning up.
//            The other thread waits for this before it does its last step and cleans up.
// - Abort    This value indicates that one thread encountered a GL error and has exited.  This
//            will cause the other thread (that is waiting for a different step) to also abort.
//
// This class is RAII.  It is declared at the top of a thread, and will be deconstructed at the end
// of the thread's outer block.  If the thread encounters a GL error, the deconstructor will abort
// the other thread using the E:Abort step.
template <typename E>
class ThreadSynchronization
{
  public:
    ThreadSynchronization(E *currentStep, std::mutex *mutex, std::condition_variable *condVar)
        : mCurrentStep(currentStep), mMutex(mutex), mCondVar(condVar)
    {}
    ~ThreadSynchronization()
    {
        bool isAborting = false;
        {
            // If the other thread isn't finished, cause it to abort.
            std::unique_lock<std::mutex> lock(*mMutex);
            isAborting = *mCurrentStep != E::Finish;

            if (isAborting)
            {
                *mCurrentStep = E::Abort;
            }
        }
        mCondVar->notify_all();
    }

    // Helper functions to synchronize the threads so that the operations are executed in the
    // specific order the test is written for.
    bool waitForStep(E waitStep)
    {
        std::unique_lock<std::mutex> lock(*mMutex);
        while (*mCurrentStep != waitStep)
        {
            // If necessary, abort execution as the other thread has encountered a GL error.
            if (*mCurrentStep == E::Abort)
            {
                return false;
            }
            // Expect increasing order to reduce risk of race conditions / deadlocks.
            if (*mCurrentStep > waitStep)
            {
                FATAL() << "waitForStep requires increasing order. mCurrentStep="
                        << (int)*mCurrentStep << ", waitStep=" << (int)waitStep;
            }
            mCondVar->wait(lock);
        }

        return true;
    }

    void nextStep(E newStep)
    {
        {
            std::unique_lock<std::mutex> lock(*mMutex);
            *mCurrentStep = newStep;
        }
        mCondVar->notify_all();
    }

  private:
    E *mCurrentStep;
    std::mutex *mMutex;
    std::condition_variable *mCondVar;
};
}  // anonymous namespace

using LockStepThreadFunc = std::function<void(EGLDisplay, EGLSurface, EGLContext)>;
void RunLockStepThreads(EGLWindow *window, size_t threadCount, LockStepThreadFunc threadFuncs[]);
void RunLockStepThreadsWithSize(EGLWindow *window,
                                EGLint width,
                                EGLint height,
                                size_t threadCount,
                                LockStepThreadFunc threadFuncs[]);
}  // namespace angle
