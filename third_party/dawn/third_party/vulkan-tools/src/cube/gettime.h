/**************************************************************************
 *
 * Copyright 2014, 2017 The Android Open Source Project
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
 *
 * Ported from drawElements Utility Library (Google, Inc.)
 * Port done by: Ian Elliott <ianelliott@google.com>
 **************************************************************************/

#include <time.h>
#include <assert.h>
#include <vulkan/vk_platform.h>

#if defined(_WIN32)

#include <windows.h>

#elif defined(__unix__) || defined(__linux) || defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__ANDROID__) || defined(__EPOC32__) || defined(__QNX__)

#include <time.h>

#elif defined(__APPLE__)

#include <sys/time.h>

#endif

uint64_t getTimeInNanoseconds(void) {
    const long long ns_in_us = 1000;
    const long long ns_in_ms = 1000 * ns_in_us;
    const long long ns_in_s = 1000 * ns_in_ms;
#if defined(_WIN32)
    LARGE_INTEGER freq;
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    QueryPerformanceFrequency(&freq);
    assert(freq.LowPart != 0 || freq.HighPart != 0);

    if (count.QuadPart < MAXLONGLONG / ns_in_s) {
        assert(freq.QuadPart != 0);
        return count.QuadPart * ns_in_s / freq.QuadPart;
    } else {
        assert(freq.QuadPart >= ns_in_s);
        return count.QuadPart / (freq.QuadPart / ns_in_s);
    }

#elif defined(__unix__) || defined(__linux) || defined(__linux__) || defined(__ANDROID__) || defined(__QNX__)
    struct timespec currTime;
    clock_gettime(CLOCK_MONOTONIC, &currTime);
    return (uint64_t)currTime.tv_sec * ns_in_s + (uint64_t)currTime.tv_nsec;

#elif defined(__EPOC32__)
    struct timespec currTime;
    /* Symbian supports only realtime clock for clock_gettime. */
    clock_gettime(CLOCK_REALTIME, &currTime);
    return (uint64_t)currTime.tv_sec * ns_in_s + (uint64_t)currTime.tv_nsec;

#elif defined(__APPLE__)
    struct timeval currTime;
    gettimeofday(&currTime, NULL);
    return (uint64_t)currTime.tv_sec * ns_in_s + (uint64_t)currTime.tv_usec * ns_in_us;

#else
#error getTimeInNanoseconds Not implemented for target OS
#endif
}
