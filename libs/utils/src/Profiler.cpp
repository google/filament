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

#include <utils/Profiler.h>

#include <stdlib.h>
#include <string.h>

#if !defined(WIN32)
#   include <unistd.h>
#else
#   include <io.h>
#   define close _close
#endif

#include <algorithm>
#include <iterator>
#include <memory>

#if defined(__linux__)

#include <sched.h>
#include <sys/syscall.h>

#ifdef __ARM_ARCH
    enum ARMv8PmuPerfTypes {
        // Common micro-architecture events (ARMv8/ARMv9 PMUv3)
        ARMV8_PMUV3_PERFCTR_L1_ICACHE_REFILL    = 0x01,
        ARMV8_PMUV3_PERFCTR_L1_DCACHE_REFILL    = 0x03,
        ARMV8_PMUV3_PERFCTR_L1_DCACHE_ACCESS    = 0x04,
        ARMV8_PMUV3_PERFCTR_INST_RETIRED        = 0x08,
        ARMV8_PMUV3_PERFCTR_BR_MIS_PRED         = 0x10,
        ARMV8_PMUV3_PERFCTR_CPU_CYCLES          = 0x11,
        ARMV8_PMUV3_PERFCTR_L1_ICACHE_ACCESS    = 0x14,
        ARMV8_PMUV3_PERFCTR_L2_CACHE_ACCESS     = 0x16,
        ARMV8_PMUV3_PERFCTR_L2_CACHE_REFILL     = 0x17,
        ARMV8_PMUV3_PERFCTR_L2_CACHE_WB         = 0x18,
        ARMV8_PMUV3_PERFCTR_BR_RETIRED          = 0x21,
    };
#endif

static int perf_event_open(perf_event_attr* hw_event, pid_t pid,
        int cpu, int group_fd, unsigned long flags) {
    return (int)syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

#endif // __linux__

namespace utils {

Profiler::Profiler() noexcept {
    for (size_t i = 0; i < size_t(EVENT_COUNT); ++i) {
        for (size_t c = 0; c < MAX_CPUS; ++c) {
            mCountersFd[i][c] = -1;
        }
    }
}

Profiler::Profiler(uint32_t eventMask) noexcept : Profiler() {
    resetEvents(eventMask);
}

Profiler::~Profiler() noexcept {
    for (size_t i = 0; i < size_t(EVENT_COUNT); ++i) {
        for (size_t c = 0; c < MAX_CPUS; ++c) {
            if (mCountersFd[i][c] >= 0) {
                close(mCountersFd[i][c]);
            }
        }
    }
}

uint32_t Profiler::resetEvents(uint32_t eventMask) noexcept {
    // close all counters
    for (size_t i = 0; i < size_t(EVENT_COUNT); ++i) {
        for (size_t c = 0; c < MAX_CPUS; ++c) {
            if (mCountersFd[i][c] >= 0) {
                close(mCountersFd[i][c]);
                mCountersFd[i][c] = -1;
            }
        }
    }
    mEnabledEvents = 0;

#if defined(__linux__)
    long num_cpus = sysconf(_SC_NPROCESSORS_CONF);
    if (num_cpus <= 0) {
        num_cpus = 1;
    }
    mCpuCount = std::min(size_t(num_cpus), MAX_CPUS);

    perf_event_attr pe{};
    pe.size = sizeof(perf_event_attr);
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED |
                     PERF_FORMAT_TOTAL_TIME_RUNNING |
                     PERF_FORMAT_ID;

    auto openCounter = [this, &pe](size_t eventIndex) {
        bool anySuccess = false;
        for (size_t c = 0; c < mCpuCount; ++c) {
            mCountersFd[eventIndex][c] = perf_event_open(&pe, 0, (int)c, -1, 0);
            if (mCountersFd[eventIndex][c] >= 0) {
                anySuccess = true;
            }
        }
        return anySuccess;
    };

    // INSTRUCTIONS is always enabled
#ifdef __ARM_ARCH
    pe.type = PERF_TYPE_RAW;
    pe.config = ARMV8_PMUV3_PERFCTR_INST_RETIRED;
#else
    pe.type = PERF_TYPE_HARDWARE;
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
#endif
    if (openCounter(INSTRUCTIONS)) {
        if (eventMask & EV_CPU_CYCLES) {
#ifdef __ARM_ARCH
            pe.type = PERF_TYPE_RAW;
            pe.config = ARMV8_PMUV3_PERFCTR_CPU_CYCLES;
#else
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_CPU_CYCLES;
#endif
            if (openCounter(CPU_CYCLES)) {
                mEnabledEvents |= EV_CPU_CYCLES;
            }
        }

        if (eventMask & EV_L1D_REFS) {
#ifdef __ARM_ARCH
            pe.type = PERF_TYPE_RAW;
            pe.config = ARMV8_PMUV3_PERFCTR_L1_DCACHE_ACCESS;
#else
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_CACHE_REFERENCES;
#endif
            if (openCounter(DCACHE_REFS)) {
                mEnabledEvents |= EV_L1D_REFS;
            }
        }

        if (eventMask & EV_L1D_MISSES) {
#ifdef __ARM_ARCH
            pe.type = PERF_TYPE_RAW;
            pe.config = ARMV8_PMUV3_PERFCTR_L1_DCACHE_REFILL;
#else
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_CACHE_MISSES;
#endif
            if (openCounter(DCACHE_MISSES)) {
                mEnabledEvents |= EV_L1D_MISSES;
            }
        }

        if (eventMask & EV_BPU_REFS) {
#ifdef __ARM_ARCH
            pe.type = PERF_TYPE_RAW;
            pe.config = ARMV8_PMUV3_PERFCTR_BR_RETIRED;
#else
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
#endif
            if (openCounter(BRANCHES)) {
                mEnabledEvents |= EV_BPU_REFS;
            }
        }

        if (eventMask & EV_BPU_MISSES) {
#ifdef __ARM_ARCH
            pe.type = PERF_TYPE_RAW;
            pe.config = ARMV8_PMUV3_PERFCTR_BR_MIS_PRED;
#else
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_BRANCH_MISSES;
#endif
            if (openCounter(BRANCH_MISSES)) {
                mEnabledEvents |= EV_BPU_MISSES;
            }
        }

#ifdef __ARM_ARCH
        if (eventMask & EV_L1I_REFS) {
            pe.type = PERF_TYPE_RAW;
            pe.config = ARMV8_PMUV3_PERFCTR_L1_ICACHE_ACCESS;
            if (openCounter(ICACHE_REFS)) {
                mEnabledEvents |= EV_L1I_REFS;
            }
        }

        if (eventMask & EV_L1I_MISSES) {
            pe.type = PERF_TYPE_RAW;
            pe.config = ARMV8_PMUV3_PERFCTR_L1_ICACHE_REFILL;
            if (openCounter(ICACHE_MISSES)) {
                mEnabledEvents |= EV_L1I_MISSES;
            }
        }
#else
        if (eventMask & EV_L1I_REFS) {
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = PERF_COUNT_HW_CACHE_L1I | 
                (PERF_COUNT_HW_CACHE_OP_READ<<8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS<<16);
            if (openCounter(ICACHE_REFS)) {
                mEnabledEvents |= EV_L1I_REFS;
            }
        }

        if (eventMask & EV_L1I_MISSES) {
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = PERF_COUNT_HW_CACHE_L1I | 
                (PERF_COUNT_HW_CACHE_OP_READ<<8) | (PERF_COUNT_HW_CACHE_RESULT_MISS<<16);
            if (openCounter(ICACHE_MISSES)) {
                mEnabledEvents |= EV_L1I_MISSES;
            }
        }
#endif
    }
#endif // __linux__
    return mEnabledEvents;
}

#if defined(__linux__)

Profiler::Counters Profiler::readCounters() noexcept {
    Counters outCounters{};
    struct ReadFormat {
        uint64_t value;
        uint64_t time_enabled;
        uint64_t time_running;
        uint64_t id;
    };

    ReadFormat rf[EVENT_COUNT][MAX_CPUS]{};
    uint64_t core_active_time[MAX_CPUS]{};
    uint64_t max_time_enabled = 0;

    for (size_t i = 0; i < size_t(EVENT_COUNT); i++) {
        for (size_t c = 0; c < mCpuCount; ++c) {
            int fd = mCountersFd[i][c];
            if (fd >= 0) {
                ssize_t n = read(fd, &rf[i][c], sizeof(ReadFormat));
                if (n >= (ssize_t)sizeof(ReadFormat)) {
                    core_active_time[c] = std::max(core_active_time[c], rf[i][c].time_running);
                    max_time_enabled = std::max(max_time_enabled, rf[i][c].time_enabled);
                }
            }
        }
    }

    for (size_t i = 0; i < size_t(EVENT_COUNT); i++) {
        uint64_t total_val = 0;
        uint64_t total_time_running = 0;
        uint64_t last_id = 0;
        size_t valid_count = 0;

        for (size_t c = 0; c < mCpuCount; ++c) {
            if (mCountersFd[i][c] >= 0 && rf[i][c].time_running > 0) {
                uint64_t val = rf[i][c].value;
                if (rf[i][c].time_running < core_active_time[c]) {
                    // Physical counter multiplexing occurred on core c: scale by the core's active time
                    val = (uint64_t)((double)val * ((double)core_active_time[c] / (double)rf[i][c].time_running));
                }
                total_val += val;
                total_time_running += rf[i][c].time_running;
                last_id = rf[i][c].id;
                valid_count++;
            }
        }

        if (valid_count > 0) {
            outCounters.counters[i].value = total_val;
            outCounters.counters[i].id = last_id;
            outCounters.time_enabled = std::max(outCounters.time_enabled, max_time_enabled);
            outCounters.time_running = std::max(outCounters.time_running, total_time_running);
            outCounters.nr++;
        }
    }
    return outCounters;
}

#endif // __linux__

} // namespace utils
