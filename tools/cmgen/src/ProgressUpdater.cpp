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

#include "ProgressUpdater.h"

#include <algorithm>
#include <string>

#include <signal.h>

static void moveCursorUp(size_t n) {
    std::cout << "\033[" << n << "F";
}

static void showCursor() {
    signal(SIGINT, SIG_DFL);
    std::cout << "\033[?25h" << std::flush;
}

static void showCursorFromSignal(int signal) {
    showCursor();
    raise(SIGINT);
}

static void hideCursor() {
    signal(SIGINT, showCursorFromSignal);
    std::cout << "\033[?25l" << std::flush;
}

static inline void printProgress(float v, size_t width) {
    size_t c = (size_t) (v * width);

    std::cout << std::setw(3) << (int) (v * 100) << "% ";
    for (size_t i = 0; i < c; i++) std::cout << "\u2588";
    std::cout << std::string(width - c, ' ');
    std::cout << "\u25C0" << std::flush;
}

void ProgressUpdater::start() {
    auto thread_main = [this]() -> int {
        bool exitRequested = false;
        while (!exitRequested) {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (!mExitRequested && m_jobQueue.isEmpty()) {
                m_condition.wait(lock);
            }
            exitRequested = mExitRequested;
            lock.unlock();
            if (!exitRequested) {
                m_jobQueue.runAllJobs();
            }
        };
        m_jobQueue.runAllJobs();
        return 0;
    };

    m_thread = std::thread(thread_main);

    initProgressValues();
    printProgressBars();

    hideCursor();
}

void ProgressUpdater::stop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    mExitRequested = true;
    lock.unlock();
    m_condition.notify_all();
    m_thread.join();
    showCursor();
}

void ProgressUpdater::update(size_t progressBarIndex, float value) {
    if (progressBarIndex >= m_numBars)
        return;

    value = std::max(0.0f, std::min(value, 1.0f));
    std::unique_lock<std::mutex> lock(m_mutex);
    m_jobQueue.push([value, progressBarIndex, this]() {
        m_progress[progressBarIndex] = value;
        moveCursorUp(m_numBars);
        printProgressBars();
    });
    lock.unlock();
    m_condition.notify_one();
}

void ProgressUpdater::update(size_t progressBarIndex, size_t value, size_t max) {
    if ((value != max) && (value % (max / 100 + 1) != 0)) return;
    update(progressBarIndex, value / (float) max);
}

void ProgressUpdater::initProgressValues() {
    m_progress.resize(m_numBars);
    std::fill(m_progress.begin(), m_progress.end(), 0.0f);
}

void ProgressUpdater::printProgressBars() {
    for (size_t i = 0; i < m_numBars; i++) {
        printProgress(m_progress[i], 32);
        std::cout << std::endl;
    }
}
