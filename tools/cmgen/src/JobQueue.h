/*
 * Copyright (C) 2015 The Android Open Source Project
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

#pragma once

#include <deque>
#include <functional>
#include <mutex>

/**
 * A simple thread-safe job queue.
 *
 * JobQueue allows to push "jobs" (i.e.: code and its state) into a queue and later execute these
 * job in FIFO order, possibly (and generally) from another thread.
 *
 * example:
 * @code
 * using utils::JobQueue;
 * static JobQueue sJobs;
 *
 * struct Foo {
 *   void bar();
 *   void baz(int);
 * } foo;
 *
 * sJobs.push([]() {
 *   // one can use a lambda
 * });
 *
 * sJobs.push(&Foo::bar, foo) {
 *   // ...or a method of a class
 * }
 *
 * sJobs.push(&Foo::bar, baz, 42) {
 *   // ... even a method with arguments
 * }
 *
 * // Later, in another thread for instance...
 *
 * // empty the queue and runs all jobs in FIFO order
 * sJobs.runAllJobs();
 *
 * // runs the oldest job if there is one
 * bool got_one = sJobs.runJobIfAny();
 *
 * // dequeue a job, but run it manually.
 * Job job(sJobs.pop());
 * if (job) {
 *   job();
 * }
 *
 * @endcode
 *
 * @warning When both sides of the JobQueue are in different threads (as it is usually the case),
 * make sure to capture all stack parameters of the job by value (i.e.: do not capture by
 * reference, parameters that live on the stack).
 */
class JobQueue {
public:
    using Job = std::function<void()>;

    JobQueue() = default;

    /**
     * Push a job to the back of the queue.
     * @param func anything that can be called
     *        (e.g.: lambda, function or method with optional parameters)
     * @param args optional parameters to method or function.
     */
    template<typename CALLABLE, typename ... ARGS>
    void push(CALLABLE&& func, ARGS&&... args) {
        enqueue(Job(std::bind(std::forward<CALLABLE>(func), std::forward<ARGS>(args)...)));
    }

    /**
     * Checks whether the JobQueue is empty.
     * @return true if there is no jobs in the queue.
     */
    bool isEmpty() const;

    /**
     * Dequeues the oldest job and executes (runs) it.
     * @return true if a job was run.
     */
    bool runJobIfAny();

    /**
     * Empties the queue and runs all jobs atomically w.r.t. the queue. Jobs are run in FIFO order.
     */
    void runAllJobs();

    /**
     * Dequeues the oldest job.
     * @return a handle to the oldest job or null if the queue was empty.
     * @note the job can be manually run by calling job().
     */
    Job pop();

private:
    JobQueue(const JobQueue& queue) = delete;
    JobQueue& operator=(const JobQueue& queue) = delete;

    void enqueue(Job&& job);

    std::deque<Job> m_queue;
    mutable std::mutex m_lock;
};
