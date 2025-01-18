/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_UTILS_PROFILER_H
#define TNT_UTILS_PROFILER_H

#include <ratio>
#include <chrono>   // note: This is safe (only used inline)

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(__linux__)
#   include <unistd.h>
#   include <sys/ioctl.h>
#   include <linux/perf_event.h>
#endif

#include <utils/compiler.h>

namespace utils {

class Profiler {
public:
    enum {
        INSTRUCTIONS    = 0,   // must be zero
        CPU_CYCLES      = 1,
        DCACHE_REFS     = 2,
        DCACHE_MISSES   = 3,
        BRANCHES        = 4,
        BRANCH_MISSES   = 5,
        ICACHE_REFS     = 6,
        ICACHE_MISSES   = 7,

        // Must be last one
        EVENT_COUNT
    };
	
    enum {
        EV_CPU_CYCLES = 1u << CPU_CYCLES,
        EV_L1D_REFS   = 1u << DCACHE_REFS,
        EV_L1D_MISSES = 1u << DCACHE_MISSES,
        EV_BPU_REFS   = 1u << BRANCHES,
        EV_BPU_MISSES = 1u << BRANCH_MISSES,
        EV_L1I_REFS   = 1u << ICACHE_REFS,
        EV_L1I_MISSES = 1u << ICACHE_MISSES,
        // helpers
        EV_L1D_RATES = EV_L1D_REFS | EV_L1D_MISSES,
        EV_L1I_RATES = EV_L1I_REFS | EV_L1I_MISSES,
        EV_BPU_RATES = EV_BPU_REFS | EV_BPU_MISSES,
    };

    Profiler() noexcept; // must call resetEvents()
    explicit Profiler(uint32_t eventMask) noexcept;
    ~Profiler() noexcept;

    Profiler(const Profiler& rhs) = delete;
    Profiler(Profiler&& rhs) = delete;
    Profiler& operator=(const Profiler& rhs) = delete;
    Profiler& operator=(Profiler&& rhs) = delete;

    // selects which events are enabled. 
    uint32_t resetEvents(uint32_t eventMask) noexcept;

    uint32_t getEnabledEvents() const noexcept { return mEnabledEvents; }

    // could return false if performance counters are not supported/enabled
    bool isValid() const { return mCountersFd[0] >= 0; }

    class Counters {
        friend class Profiler;

        uint64_t nr;
        uint64_t time_enabled;
        uint64_t time_running;
        struct {
            uint64_t value;
            uint64_t id;
        } counters[EVENT_COUNT];

        friend Counters operator-(Counters lhs, const Counters& rhs) noexcept {
            lhs.nr -= rhs.nr;
            lhs.time_enabled -= rhs.time_enabled;
            lhs.time_running -= rhs.time_running;
            for (size_t i = 0; i < EVENT_COUNT; ++i) {
                lhs.counters[i].value -= rhs.counters[i].value;
            }
            return lhs;
        }

    public:
        uint64_t getInstructions() const        { return counters[INSTRUCTIONS].value; }
        uint64_t getCpuCycles() const           { return counters[CPU_CYCLES].value; }
        uint64_t getL1DReferences() const       { return counters[DCACHE_REFS].value; }
        uint64_t getL1DMisses() const           { return counters[DCACHE_MISSES].value; }
        uint64_t getL1IReferences() const       { return counters[ICACHE_REFS].value; }
        uint64_t getL1IMisses() const           { return counters[ICACHE_MISSES].value; }
        uint64_t getBranchInstructions() const  { return counters[BRANCHES].value; }
        uint64_t getBranchMisses() const        { return counters[BRANCH_MISSES].value; }

        std::chrono::duration<uint64_t, std::nano> getWallTime() const {
            return std::chrono::duration<uint64_t, std::nano>(time_enabled);
        }

        std::chrono::duration<uint64_t, std::nano> getRunningTime() const {
            return std::chrono::duration<uint64_t, std::nano>(time_running);
        }

        double getIPC() const noexcept {
            uint64_t cpuCycles = getCpuCycles();
            uint64_t instructions = getInstructions();
            return double(instructions) / double(cpuCycles);
        }

        double getCPI() const noexcept {
            uint64_t cpuCycles = getCpuCycles();
            uint64_t instructions = getInstructions();
            return double(cpuCycles) / double(instructions);
        }

        double getL1DMissRate() const noexcept {
            uint64_t cacheReferences = getL1DReferences();
            uint64_t cacheMisses = getL1DMisses();
            return double(cacheMisses) / double(cacheReferences);
        }

        double getL1DHitRate() const noexcept {
            return 1.0 - getL1DMissRate();
        }

        double getL1IMissRate() const noexcept {
            uint64_t cacheReferences = getL1IReferences();
            uint64_t cacheMisses = getL1IMisses();
            return double(cacheMisses) / double(cacheReferences);
        }

        double getL1IHitRate() const noexcept {
            return 1.0 - getL1IMissRate();
        }

        double getBranchMissRate() const noexcept {
            uint64_t branchReferences = getBranchInstructions();
            uint64_t branchMisses = getBranchMisses();
            return double(branchMisses) / double(branchReferences);
        }

        double getBranchHitRate() const noexcept {
            return 1.0 - getBranchMissRate();
        }

        double getMPKI(uint64_t misses) const noexcept {
            return (misses * 1000.0) / getInstructions();
        }
    };

#if defined(__linux__)

    void reset() noexcept {
        int fd = mCountersFd[0];
        ioctl(fd, PERF_EVENT_IOC_RESET,  PERF_IOC_FLAG_GROUP);
    }

    void start() noexcept {
        int fd = mCountersFd[0];
        ioctl(fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    }

    void stop() noexcept {
        int fd = mCountersFd[0];
        ioctl(fd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
    }

    Counters readCounters() noexcept;

#else // !__linux__

    void reset() noexcept { }
    void start() noexcept { }
    void stop() noexcept { }
    Counters readCounters() noexcept { return {}; }

#endif // __linux__

    bool hasBranchRates() const noexcept {
        return (mCountersFd[BRANCHES] >= 0) && (mCountersFd[BRANCH_MISSES] >= 0);
    }

    bool hasICacheRates() const noexcept {
        return (mCountersFd[ICACHE_REFS] >= 0) && (mCountersFd[ICACHE_MISSES] >= 0);
    }

private:
    UTILS_UNUSED uint8_t mIds[EVENT_COUNT] = {};
    int mCountersFd[EVENT_COUNT];
    uint32_t mEnabledEvents = 0;
};

} // namespace utils

#endif // TNT_UTILS_PROFILER_H
