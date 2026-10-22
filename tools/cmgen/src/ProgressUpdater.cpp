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
#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
    #include <signal.h>
    #define STDOUT_FILENO _fileno(stdout)
    #define write _write

    static void setSigintHandler(void (*handler)(int)) {
        signal(SIGINT, handler);
    }
#else
    #include <unistd.h>
    #include <signal.h>

    static void setSigintHandler(void (*handler)(int)) {
        struct sigaction sa;
        sa.sa_handler = handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, nullptr);
    }
#endif

static void moveCursorUp(size_t n) {
    std::cout << "\033[" << n << "F";
}

static void write_safe(const char* seq, size_t len) {
    write(STDOUT_FILENO, seq, len);
}

static void showCursor() {
    setSigintHandler(SIG_DFL);
    const char* show_cursor_seq = "\033[?25h";
    write_safe(show_cursor_seq, 6);
}

static void showCursorFromSignal(int) {
    showCursor();
    raise(SIGINT);
}

static void hideCursor() {
    setSigintHandler(showCursorFromSignal);
    const char* hide_cursor_seq = "\033[?25l";
    write_safe(hide_cursor_seq, 6);
}

static inline void printProgress(float v, size_t width) {
    size_t c = (size_t) (v * width);

    std::cout << std::setw(3) << (int) (v * 100) << "% ";
    for (size_t i = 0; i < c; i++) std::cout << "\u2588";
    std::cout << std::string(width - c, ' ');
    std::cout << "\u25C0" << std::flush;
}

void ProgressUpdater::printUpdates() {
    std::vector<Update> updates;

    std::unique_lock<std::mutex> lock(mMutex);
    std::swap(updates, mUpdates);
    lock.unlock();

    printUpdates(updates);
}

void ProgressUpdater::printUpdates(const std::vector<Update>& updates) {
    for (const auto update : updates) {
        mProgress[update.index] = std::max(mProgress[update.index], update.value);
    }

    moveCursorUp(mNumBars);
    printProgressBars();
}

void ProgressUpdater::start() {
    auto threadMain = [this]() -> int {
        bool exitRequested = false;
        while (!exitRequested) {
            std::unique_lock<std::mutex> lock(mMutex);
            while (!mExitRequested && mUpdates.empty()) {
                mCondition.wait(lock);
            }
            exitRequested = mExitRequested;
            lock.unlock();
            if (!exitRequested) {
                printUpdates();
            }
        };
        printUpdates();
        return 0;
    };

    mThread = std::thread(threadMain);

    initProgressValues();
    printProgressBars();

    hideCursor();
}

void ProgressUpdater::stop() {
    std::unique_lock<std::mutex> lock(mMutex);
    mExitRequested = true;
    lock.unlock();
    mCondition.notify_all();
    mThread.join();
    showCursor();
}

void ProgressUpdater::update(size_t progressBarIndex, float value) {
    if (progressBarIndex >= mNumBars) {
        return;
    }

    value = std::max(0.0f, std::min(value, 1.0f));
    std::unique_lock<std::mutex> lock(mMutex);
    mUpdates.push_back({ progressBarIndex, value });
    lock.unlock();
    mCondition.notify_one();
}

void ProgressUpdater::update(size_t progressBarIndex, size_t value, size_t max) {
    if ((value != max) && (value % (max / 100 + 1) != 0)) return;
    update(progressBarIndex, value / (float) max);
}

void ProgressUpdater::initProgressValues() {
    mProgress.resize(mNumBars);
    std::fill(mProgress.begin(), mProgress.end(), 0.0f);
}

void ProgressUpdater::printProgressBars() {
    for (size_t i = 0; i < mNumBars; i++) {
        printProgress(mProgress[i], 32);
        std::cout << std::endl;
    }
}
