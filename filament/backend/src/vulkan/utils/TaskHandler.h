/*
* Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef TNT_TASKHANDLER_H
#define TNT_TASKHANDLER_H

#include <utils/compiler.h>
#include <utils/Condition.h>
#include <utils/Invocable.h>
#include <utils/Mutex.h>

#include <queue>
#include <thread>


namespace filament::backend::fvkutils {

// A helper class that runs tasks on a separate thread.
class TaskHandler {
public:
    // A task is invoked with `executed = true` from the handler thread. If the handler is shut
    // down before the task is picked up, the task is instead invoked with `executed = false` so
    // that clients can still release whatever the task owns (the user's PixelBufferDescriptor
    // and the Vulkan objects of the request).
    using Task = utils::Invocable<void(bool executed)>;

    TaskHandler();

    // Joins the thread if `shutdown()` was not called: destroying a joinable std::thread
    // terminates the process. Unlike `shutdown()`, this cannot panic: throwing out of a
    // destructor terminates the process as well.
    ~TaskHandler();

    void post(Task&& task);

    // This will block until all of the tasks are done.
    void drain();

    // This will quit without running the pending tasks, but they will still be invoked with
    // `executed = false` so that they can clean up after themselves.
    void shutdown();

private:
    void loop();

    // Stops the thread and flushes the queue. Unlike `shutdown()` this makes no assertion, so
    // it is safe to call from the destructor.
    void stop() noexcept;

    utils::Mutex mTaskQueueMutex;
    utils::Condition mHasTaskCondition;
    bool mShouldStop UTILS_GUARDED_BY(mTaskQueueMutex);
    std::queue<Task> mTaskQueue UTILS_GUARDED_BY(mTaskQueueMutex);
    // Must be declared last: the thread runs `loop()`, which uses all of the above.
    std::thread mThread;
};
}// filament::backend::fvkutils
#endif // TNT_TASKHANDLER_H
