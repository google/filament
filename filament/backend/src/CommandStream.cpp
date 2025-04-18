/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "private/backend/CommandStream.h"

#if DEBUG_COMMAND_STREAM
#include <utils/CallStack.h>
#endif

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/ostream.h>
#include <utils/Profiler.h>
#include <utils/Systrace.h>

#include <cstddef>
#include <functional>
#include <string>
#include <utility>

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif

using namespace utils;

namespace filament::backend {

// ------------------------------------------------------------------------------------------------
// A few utility functions for debugging...

inline void printParameterPack(io::ostream&) { }

template<typename LAST>
static void printParameterPack(io::ostream& out, const LAST& t) { out << t; }

template<typename FIRST, typename... REMAINING>
static void printParameterPack(io::ostream& out, const FIRST& first, const REMAINING& ... rest) {
    out << first << ", ";
    printParameterPack(out, rest...);
}

static UTILS_NOINLINE UTILS_UNUSED std::string extractMethodName(std::string& command) noexcept {
    constexpr const char startPattern[] = "::Command<&filament::backend::Driver::";
    auto pos = command.rfind(startPattern);
    auto end = command.rfind('(');
    pos += sizeof(startPattern) - 1;
    return command.substr(pos, end-pos);
}

// ------------------------------------------------------------------------------------------------

CommandStream::CommandStream(Driver& driver, CircularBuffer& buffer) noexcept
        : mDriver(driver),
          mCurrentBuffer(buffer),
          mDispatcher(driver.getDispatcher())
#ifndef NDEBUG
          , mThreadId(ThreadUtils::getThreadId())
#endif
{
#ifdef __ANDROID__
    char property[PROP_VALUE_MAX];
    __system_property_get("debug.filament.perfcounters", property);
    mUsePerformanceCounter = bool(atoi(property));
#endif
}

void CommandStream::execute(void* buffer) {
    // NOTE: we can't use SYSTRACE_CALL() or similar here because, execute() below, also
    // uses systrace BEGIN/END and the END is not guaranteed to be happening in this scope.

    Profiler profiler;

    if constexpr (SYSTRACE_TAG) {
        if (UTILS_UNLIKELY(mUsePerformanceCounter)) {
            // we want to remove all this when tracing is completely disabled
            profiler.resetEvents(Profiler::EV_CPU_CYCLES  | Profiler::EV_BPU_MISSES);
            profiler.start();
        }
    }

    mDriver.execute([this, buffer]() {
        Driver& UTILS_RESTRICT driver = mDriver;
        CommandBase* UTILS_RESTRICT base = static_cast<CommandBase*>(buffer);
        while (UTILS_LIKELY(base)) {
            base = base->execute(driver);
        }
    });

    if constexpr (SYSTRACE_TAG) {
        if (UTILS_UNLIKELY(mUsePerformanceCounter)) {
            // we want to remove all this when tracing is completely disabled
            profiler.stop();
            UTILS_UNUSED Profiler::Counters const counters = profiler.readCounters();
            SYSTRACE_CONTEXT();
            SYSTRACE_VALUE32("GLThread (I)", counters.getInstructions());
            SYSTRACE_VALUE32("GLThread (C)", counters.getCpuCycles());
            SYSTRACE_VALUE32("GLThread (CPI x10)", counters.getCPI() * 10);
            SYSTRACE_VALUE32("GLThread (BPU miss)", counters.getBranchMisses());
            SYSTRACE_VALUE32("GLThread (I / BPU miss)",
                    counters.getInstructions() / counters.getBranchMisses());
        }
    }
}

void CommandStream::queueCommand(std::function<void()> command) {
    new(allocateCommand(CustomCommand::align(sizeof(CustomCommand)))) CustomCommand(std::move(command));
}

template<typename... ARGS>
template<void (Driver::*METHOD)(ARGS...)>
template<std::size_t... I>
void CommandType<void (Driver::*)(ARGS...)>::Command<METHOD>::log(std::index_sequence<I...>) noexcept  {
#if DEBUG_COMMAND_STREAM
    static_assert(UTILS_HAS_RTTI, "DEBUG_COMMAND_STREAM can only be used with RTTI");
    std::string command = utils::CallStack::demangleTypeName(typeid(Command).name()).c_str();
    slog.d << extractMethodName(command) << " : size=" << sizeof(Command) << "\n\t";
    printParameterPack(slog.d, std::get<I>(mArgs)...);
    slog.d << io::endl;
#endif
}

template<typename... ARGS>
template<void (Driver::*METHOD)(ARGS...)>
void CommandType<void (Driver::*)(ARGS...)>::Command<METHOD>::log() noexcept  {
    log(std::make_index_sequence<std::tuple_size<SavedParameters>::value>{});
}

/*
 * When DEBUG_COMMAND_STREAM is activated, we need to explicitly instantiate the log() method
 * (this is because we don't want it in the header file)
 */

#if DEBUG_COMMAND_STREAM
#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params)
#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    template void CommandType<decltype(&Driver::methodName)>::Command<&Driver::methodName>::log() noexcept;
#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    template void CommandType<decltype(&Driver::methodName##R)>::Command<&Driver::methodName##R>::log() noexcept;
#include "private/backend/DriverAPI.inc"
#endif

// ------------------------------------------------------------------------------------------------

void CustomCommand::execute(Driver&, CommandBase* base, intptr_t* next) {
    *next = CustomCommand::align(sizeof(CustomCommand));
    static_cast<CustomCommand*>(base)->mCommand();
    static_cast<CustomCommand*>(base)->~CustomCommand();
}

} // namespace filament::backend
