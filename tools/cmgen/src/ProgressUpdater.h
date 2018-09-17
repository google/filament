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

#ifndef SRC_PROGRESS_UPDATER_H
#define SRC_PROGRESS_UPDATER_H

#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <vector>
#include <thread>

#include "JobQueue.h"

class ProgressUpdater {
public:
    explicit ProgressUpdater(size_t numProgressBars) : m_numBars(numProgressBars) {
    }

    /**
     * Initializes progress bars and starts the rendering thread.
     * Do not output to stdout before calling stop().
     */
    void start();
    /**
     * Shutdowns the rendering thread.
     */
    void stop();

    void update(size_t progressBarIndex, float value);
    void update(size_t progressBarIndex, size_t value, size_t max);

private:
    void initProgressValues();
    void printProgressBars();

    size_t m_numBars;
    std::vector<float> m_progress;

    std::mutex m_mutex;
    std::condition_variable m_condition;

    JobQueue m_jobQueue;
    std::thread m_thread;
    bool mExitRequested = false;
};

#endif //SRC_PROGRESS_UPDATER_H
