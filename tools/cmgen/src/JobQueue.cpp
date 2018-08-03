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

#include "JobQueue.h"

JobQueue::Job JobQueue::pop() {
    Job job;
    std::lock_guard<std::mutex> lock(m_lock);
    if (!m_queue.empty()) {
        std::swap(job, m_queue.front());
        m_queue.pop_front();
    }
    return job;
}

void JobQueue::enqueue(JobQueue::Job&& job) {
    std::lock_guard<std::mutex> lock(m_lock);
    m_queue.push_back(std::forward<JobQueue::Job>(job));
}

bool JobQueue::runJobIfAny() {
    Job job(pop());
    // call the job without holding our lock
    if (job) {
        job();
        return true;
    }
    return false;
}

void JobQueue::runAllJobs() {
    std::deque<Job> q;
    std::unique_lock<std::mutex> lock(m_lock);
    std::swap(q, m_queue);
    lock.unlock();
    for (auto& job : q) {
        job();
    }
}

bool JobQueue::isEmpty() const {
    std::lock_guard<std::mutex> lock(m_lock);
    return m_queue.empty();
}
