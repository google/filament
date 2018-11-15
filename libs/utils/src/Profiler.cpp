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
#include <unistd.h>
#else
#include <io.h>
#define close _close
#endif

#include <algorithm>
#include <memory>

#if defined(__linux__)

#include <sys/syscall.h>

#ifdef __ARM_ARCH
    enum ARMv8PmuPerfTypes{
        // Common micro-architecture events
        ARMV8_PMUV3_PERFCTR_L1_ICACHE_REFILL    = 0x01,
        ARMV8_PMUV3_PERFCTR_L1_ICACHE_ACCESS    = 0x14,
        ARMV8_PMUV3_PERFCTR_L2_CACHE_ACCESS     = 0x16,
        ARMV8_PMUV3_PERFCTR_L2_CACHE_REFILL     = 0x17,
        ARMV8_PMUV3_PERFCTR_L2_CACHE_WB         = 0x18,
    };
#endif

static int perf_event_open(struct perf_event_attr* hw_event, pid_t pid,
        int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

#endif // __linux__

namespace utils {

Profiler& Profiler::get() noexcept {
    static Profiler sProfiler;
    return sProfiler;
}

Profiler::Profiler() noexcept {
    std::uninitialized_fill(std::begin(mCountersFd), std::end(mCountersFd), -1);
    Profiler::resetEvents(EV_CPU_CYCLES | EV_L1D_RATES | EV_BPU_RATES);
}

Profiler::~Profiler() noexcept {
    #pragma nounroll
    for (int fd : mCountersFd) {
        if (fd >= 0) {
            close(fd);
        }
    }
}

uint32_t Profiler::resetEvents(uint32_t eventMask) noexcept {
    // close all counters
    #pragma nounroll
    for (int& fd : mCountersFd) {
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
    }
    mEnabledEvents = 0;

#if defined(__linux__)

    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    pe.read_format = PERF_FORMAT_GROUP |
                     PERF_FORMAT_ID |
                     PERF_FORMAT_TOTAL_TIME_ENABLED |
                     PERF_FORMAT_TOTAL_TIME_RUNNING;

    uint8_t count = 0;
    int fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (fd >= 0) {
        const int groupFd = fd;
        mIds[INSTRUCTIONS] = count++;
        mCountersFd[INSTRUCTIONS] = fd;

        pe.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

        if (eventMask & EV_CPU_CYCLES) {
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_CPU_CYCLES;
            mCountersFd[CPU_CYCLES] = perf_event_open(&pe, 0, -1, groupFd, 0);
            if (mCountersFd[CPU_CYCLES] > 0) {
                mIds[CPU_CYCLES] = count++;
                mEnabledEvents |= EV_CPU_CYCLES;
            }
        }

        if (eventMask & EV_L1D_REFS) {
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_CACHE_REFERENCES;
            mCountersFd[DCACHE_REFS] = perf_event_open(&pe, 0, -1, groupFd, 0);
            if (mCountersFd[DCACHE_REFS] > 0) {
                mIds[DCACHE_REFS] = count++;
                mEnabledEvents |= EV_L1D_REFS;
            }
        }

        if (eventMask & EV_L1D_MISSES) {
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_CACHE_MISSES;
            mCountersFd[DCACHE_MISSES] = perf_event_open(&pe, 0, -1, groupFd, 0);
            if (mCountersFd[DCACHE_MISSES] > 0) {
                mIds[DCACHE_MISSES] = count++;
                mEnabledEvents |= EV_L1D_MISSES;
            }
        }

        if (eventMask & EV_BPU_REFS) {
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
            mCountersFd[BRANCHES] = perf_event_open(&pe, 0, -1, groupFd, 0);
            if (mCountersFd[BRANCHES] > 0) {
                mIds[BRANCHES] = count++;
                mEnabledEvents |= EV_BPU_REFS;
            }
        }

        if (eventMask & EV_BPU_MISSES) {
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_BRANCH_MISSES;
            mCountersFd[BRANCH_MISSES] = perf_event_open(&pe, 0, -1, groupFd, 0);
            if (mCountersFd[BRANCH_MISSES] > 0) {
                mIds[BRANCH_MISSES] = count++;
                mEnabledEvents |= EV_BPU_MISSES;
            }
        }

#ifdef __ARM_ARCH
        if (eventMask & EV_L1I_REFS) {
            pe.type = PERF_TYPE_RAW;
            pe.config = ARMV8_PMUV3_PERFCTR_L1_ICACHE_ACCESS;
            mCountersFd[ICACHE_REFS] = perf_event_open(&pe, 0, -1, groupFd, 0);
            if (mCountersFd[ICACHE_REFS] > 0) {
                mIds[ICACHE_REFS] = count++;
                mEnabledEvents |= EV_L1I_REFS;
            }
        }

        if (eventMask & EV_L1I_MISSES) {
            pe.type = PERF_TYPE_RAW;
            pe.config = ARMV8_PMUV3_PERFCTR_L1_ICACHE_REFILL;
            mCountersFd[ICACHE_MISSES] = perf_event_open(&pe, 0, -1, groupFd, 0);
            if (mCountersFd[ICACHE_MISSES] > 0) {
                mIds[ICACHE_MISSES] = count++;
                mEnabledEvents |= EV_L1I_MISSES;
            }
        }
#else
        if (eventMask & EV_L1I_REFS) {
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = PERF_COUNT_HW_CACHE_L1I | 
                (PERF_COUNT_HW_CACHE_OP_READ<<8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS<<16);
            mCountersFd[ICACHE_REFS] = perf_event_open(&pe, 0, -1, groupFd, 0);
            if (mCountersFd[ICACHE_REFS] > 0) {
                mIds[ICACHE_REFS] = count++;
                mEnabledEvents |= EV_L1I_REFS;
            }
        }

        if (eventMask & EV_L1I_MISSES) {
            pe.type = PERF_TYPE_HW_CACHE;
            pe.config = PERF_COUNT_HW_CACHE_L1I | 
                (PERF_COUNT_HW_CACHE_OP_READ<<8) | (PERF_COUNT_HW_CACHE_RESULT_MISS<<16);
            mCountersFd[ICACHE_MISSES] = perf_event_open(&pe, 0, -1, groupFd, 0);
            if (mCountersFd[ICACHE_MISSES] > 0) {
                mIds[ICACHE_MISSES] = count++;
                mEnabledEvents |= EV_L1I_MISSES;
            }
        }
#endif
    }
#endif // __linux__
    return mEnabledEvents;
}

} // namespace utils
