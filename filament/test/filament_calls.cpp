/*
 * Copyright (C) 2017 The Android Open Source Project
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
#include "details/Engine.h"
#include <iostream>
#include <utils/compiler.h>

using namespace filament;
using namespace utils;

UTILS_NOINLINE
void printResults(char const* name, size_t REPEAT, Profiler::Counters const& c) {
    std::cout << name << ":" << std::endl;
    std::cout << "instructions: " << c.getInstructions() / float(REPEAT) << std::endl;
    std::cout << "cycles:       " << c.getCpuCycles() / float(REPEAT) << std::endl;
    std::cout << "bpu misses:   " << c.getBranchMisses() / float(REPEAT) << " (" << c.getBranchMisses() << "/" << REPEAT << ")" << std::endl;
    std::cout << "CPI:          " << c.getCPI() << std::endl;
    std::cout << "" << std::endl;
}

template <typename T>
void benchmark(Profiler& p, const char* const name, T f) {
    size_t REPEAT = 128;
    p.start();

    #pragma nounroll
    for (size_t j=0 ; j<2 ; j++) {
        p.reset();
        #pragma unroll
        for (size_t i = 0; i < REPEAT; i++) {
            f();
        }
    }

    p.stop();
    Profiler::Counters c = p.readCounters();
    printResults(name, REPEAT, c);
}


template <typename T>
void benchmark_nounroll(Profiler& p, const char* const name, T f) {
    size_t REPEAT = 128;
    p.start();

#pragma nounroll
    for (size_t j=0 ; j<2 ; j++) {
        p.reset();
#pragma nounroll
        for (size_t i = 0; i < REPEAT; i++) {
            f();
        }
    }

    p.stop();
    Profiler::Counters c = p.readCounters();
    printResults(name, REPEAT, c);
}


UTILS_NOINLINE
void activateBigBang(filament::details::EnginePerformanceTest* ei) {
    ei->activateBigBang();
}

UTILS_NOINLINE
void foo(void*) noexcept {
    __asm__ __volatile__( "" : : : "memory" );
}

int main() {
    filament::details::EnginePerformanceTest* engine = new filament::details::EnginePerformanceTest();
    filament::details::EnginePerformanceTest* ei = engine;
    filament::details::EnginePerformanceTest::PFN destroyUniverse = engine->getDestroyUniverseApi();

    Profiler p(Profiler::EV_CPU_CYCLES | Profiler::EV_BPU_MISSES);

    benchmark(p, "Local function call", [engine]() {
        foo(engine);
    });

    benchmark(p, "Library API call", [engine]() {
        engine->activateOmegaThirteen();
    });

    benchmark(p, "Virtual interface call", [ei]() {
        ei->activateBigBang();
    });

    benchmark_nounroll(p, "Virtual interface call (no unroll)", [ei]() {
        ei->activateBigBang();
    });

    benchmark(p, "Virtual interface call + trampoline", [ei]() {
        activateBigBang(ei);
    });

    benchmark(p, "Function pointer call", [engine, destroyUniverse]() {
        destroyUniverse(engine);
    });

    delete engine;
    return 0;
}

