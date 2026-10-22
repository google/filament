/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_UTILS_STOPWATCH_H
#define TNT_UTILS_STOPWATCH_H

#include <utils/Log.h>
#include <utils/ostream.h>

#include <chrono>
#include <limits>
#include <ratio>

#include <stddef.h>

namespace utils {

/*
 * A very basic Stopwatch class
 */

template<typename Clock = std::chrono::steady_clock>
class Stopwatch {
public:
    using duration = typename Clock::duration;
    using time_point = typename Clock::time_point;

private:
    time_point mStart;
    duration mMinLap = std::numeric_limits<duration>::max();
    duration mMaxLap{};
    std::chrono::duration<double, std::nano> mAvgLap{};
    size_t mCount = 0;
    const char* mName = nullptr;

public:
    // Create a Stopwatch with a name and clock
    explicit Stopwatch(const char* name) noexcept: mName(name) {}

    // Logs min/avg/max lap time
    ~Stopwatch() noexcept;

    // start the stopwatch
    inline void start() noexcept {
        mStart = Clock::now();
    }

    // stop the stopwatch
    inline void stop() noexcept {
        auto d = Clock::now() - mStart;
        mMinLap = std::min(mMinLap, d);
        mMaxLap = std::max(mMaxLap, d);
        mAvgLap = (mAvgLap * mCount + d) / (mCount + 1);
        mCount++;
    }

    // get the minimum lap time recorded
    duration getMinLapTime() const noexcept { return mMinLap; }

    // get the maximum lap time recorded
    duration getMaxLapTime() const noexcept { return mMaxLap; }

    // get the average lap time
    duration getAverageLapTime() const noexcept { return mAvgLap; }
};

template<typename Clock>
Stopwatch<Clock>::~Stopwatch() noexcept {
    slog.d << "Stopwatch \"" << mName << "\" : ["
           << mMinLap.count() << ", "
           << std::chrono::duration_cast<duration>(mAvgLap).count() << ", "
           << mMaxLap.count() << "] ns" << io::endl;
}

/*
 * AutoStopwatch can be used to start and stop a Stopwatch automatically
 * when entering and exiting a scope.
 */
template<typename Stopwatch>
class AutoStopwatch {
    Stopwatch& stopwatch;
public:
    inline explicit AutoStopwatch(Stopwatch& stopwatch) noexcept: stopwatch(stopwatch) {
        stopwatch.start();
    }

    inline ~AutoStopwatch() noexcept { stopwatch.stop(); }
};

} // namespace utils

#endif // TNT_UTILS_STOPWATCH_H
