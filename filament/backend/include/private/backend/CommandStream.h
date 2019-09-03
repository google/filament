#include <utility>

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

#ifndef TNT_FILAMENT_DRIVER_COMMANDSTREAM_H
#define TNT_FILAMENT_DRIVER_COMMANDSTREAM_H

#include "private/backend/CircularBuffer.h"

#include <backend/BufferDescriptor.h>
#include <backend/Handle.h>
#include <backend/PipelineState.h>
#include <backend/PixelBufferDescriptor.h>
#include <backend/TargetBufferInfo.h>

#include "private/backend/DriverApi.h"
#include "private/backend/Program.h"
#include "private/backend/SamplerGroup.h"

// FIXME: we'd like to not need this header here.
#include "private/backend/Driver.h"

#include <backend/DriverEnums.h>


#include <utils/compiler.h>

#include <functional>
#include <tuple>
#include <thread>
#include <utility>

#include <cassert>
#include <cstddef>
#include <cstdint>

// Set to true to print every commands out on log.d. This requires RTTI and DEBUG
#define DEBUG_COMMAND_STREAM false

namespace filament {
namespace backend {

class Driver;
class CommandBase;

/*
 * Dispatcher is a data structure containing only function pointers.
 * Each function pointer targets code that unpacks the arguments to the driver's method and
 * calls it.
 *
 * Dispatcher's function pointers are populated during initialization and no CommandStream calls
 * can be made before that.
 *
 * When a command is inserted into the stream, the corresponding function pointer is copied
 * directly into CommandBase from Dispatcher.
 */
class Dispatcher {
public:
    using Execute = void (*)(Driver& driver, CommandBase* self, intptr_t* next);
#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params)
#define DECL_DRIVER_API(methodName, paramsDecl, params)                     Execute methodName##_;
#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params)     Execute methodName##_;
#include "DriverAPI.inc"
};

// ------------------------------------------------------------------------------------------------

class CommandBase {
    static constexpr size_t FILAMENT_OBJECT_ALIGNMENT = alignof(std::max_align_t);

protected:
    using Execute = Dispatcher::Execute;

    constexpr explicit CommandBase(Execute execute) noexcept : mExecute(execute) {}

public:
    // alignment of all Commands in the CommandStream
    static constexpr size_t align(size_t v) {
        return (v + (FILAMENT_OBJECT_ALIGNMENT - 1)) & -FILAMENT_OBJECT_ALIGNMENT;
    }

    // executes this command and returns the next one
    inline CommandBase* execute(Driver& driver) {
        // it is important to return the next command offset by output parameter instead
        // of return value -- it allows the compiler to perform the tail call optimization.
        intptr_t next;
        mExecute(driver, this, &next);
        return reinterpret_cast<CommandBase*>(reinterpret_cast<intptr_t>(this) + next);
    }

    inline ~CommandBase() noexcept = default;

private:
    Execute mExecute;
};

// ------------------------------------------------------------------------------------------------

template<typename M, typename D, typename T, std::size_t... I>
constexpr void trampoline(M&& m, D&& d, T&& t, std::index_sequence<I...>) {
    (d.*m)(std::move(std::get<I>(std::forward<T>(t)))...);
}

template<typename M, typename D, typename T>
constexpr void apply(M&& m, D&& d, T&& t) {
    trampoline(std::forward<M>(m), std::forward<D>(d), std::forward<T>(t),
            std::make_index_sequence< std::tuple_size<std::remove_reference_t<T>>::value >{});
}

/*
 * CommandType<> is just a wrapper class to specialize on a pointer-to-member of Driver
 * (i.e. a method pointer to a method of Driver of a particular type -- but not the
 * method itself). We only do that so we can identify the parameters of that method.
 * We won't call through that pointer though.
 */
template<typename... ARGS>
struct CommandType;

template<typename... ARGS>
struct CommandType<void (Driver::*)(ARGS...)> {

    /*
     * Command is templated on a specific method of Driver, using CommandType's template
     * parameter.
     * Note that we're never calling this method (which is why it doesn't appear in the
     * template parameter below). The actual call is made through Command::execute().
     */
    template<void(Driver::*)(ARGS...)>
    class Command : public CommandBase {
        // We use a std::tuple<> to record the arguments passed to the constructor
        using SavedParameters = std::tuple<typename std::decay<ARGS>::type...>;
        SavedParameters mArgs;

        void log() noexcept;
        template<std::size_t... I> void log(std::index_sequence<I...>) noexcept;

    public:
        template<typename M, typename D>
        static inline void execute(M&& method, D&& driver, CommandBase* base, intptr_t* next) noexcept {
            Command* self = static_cast<Command*>(base);
            *next = align(sizeof(Command));
            if (DEBUG_COMMAND_STREAM) {
                // must call this before invoking the method
                self->log();
            }
            apply(std::forward<M>(method), std::forward<D>(driver), self->mArgs);
            self->~Command();
        }

        // A command can be moved
        inline Command(Command&& rhs) noexcept = default;

        template<typename... A>
        inline explicit constexpr Command(Execute execute, A&& ... args)
                : CommandBase(execute), mArgs(std::move(args)...) {
        }

        // placement new declared as "throw" to avoid the compiler's null-check
        inline void* operator new(std::size_t size, void* ptr) {
            assert(ptr);
            return ptr;
        }
    };
};

// convert an method of "class Driver" into a Command<> type
#define COMMAND_TYPE(method) CommandType<decltype(&Driver::method)>::Command<&Driver::method>

// ------------------------------------------------------------------------------------------------

class CustomCommand : public CommandBase {
    std::function<void()> mCommand;
    static void execute(Driver&, CommandBase* base, intptr_t* next) noexcept;
public:
    inline CustomCommand(CustomCommand&& rhs) = default;
    inline explicit CustomCommand(std::function<void()> cmd)
            : CommandBase(execute), mCommand(std::move(cmd)) { }
};

// ------------------------------------------------------------------------------------------------

class NoopCommand : public CommandBase {
    intptr_t mNext;
    static void execute(Driver&, CommandBase* self, intptr_t* next) noexcept {
        *next = static_cast<NoopCommand*>(self)->mNext;
    }
public:
    inline constexpr explicit NoopCommand(void* next) noexcept
            : CommandBase(execute), mNext(size_t((char *)next - (char *)this)) { }
};

// ------------------------------------------------------------------------------------------------

#if defined(NDEBUG)
    #define DEBUG_COMMAND(methodName, ...)
#else
    // For now, simply pass the method name down as a string and throw away the parameters.
    // This is good enough for certain debugging needs and we can improve this later.
    #define DEBUG_COMMAND(methodName, ...) mDriver->debugCommand(#methodName)
#endif

class CommandStream {
public:
#define DECL_DRIVER_API(methodName, paramsDecl, params)                                         \
    inline void methodName(paramsDecl) {                                                        \
        DEBUG_COMMAND(methodName, params);                                                      \
        using Cmd = COMMAND_TYPE(methodName);                                                   \
        void* const p = allocateCommand(CommandBase::align(sizeof(Cmd)));                       \
        new(p) Cmd(mDispatcher->methodName##_, params);                                         \
    }

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params)                    \
    inline RetType methodName(paramsDecl) {                                                     \
        DEBUG_COMMAND(methodName, params);                                                      \
        return mDriver->methodName(params);                                                     \
    }

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params)                         \
    inline RetType methodName(paramsDecl) {                                                     \
        DEBUG_COMMAND(methodName, params);                                                      \
        RetType result = mDriver->methodName##S();                                              \
        using Cmd = COMMAND_TYPE(methodName##R);                                                \
        void* const p = allocateCommand(CommandBase::align(sizeof(Cmd)));                       \
        new(p) Cmd(mDispatcher->methodName##_, RetType(result), params);                        \
        return result;                                                                          \
    }

#include "DriverAPI.inc"

public:
    CommandStream() noexcept = default;
    CommandStream(Driver& driver, CircularBuffer& buffer) noexcept;

    // This is for debugging only. Currently CircularBuffer can only be written from a
    // single thread. In debug builds we assert this condition.
    // Call this first in the render loop.
    inline void debugThreading() noexcept {
#ifndef NDEBUG
        mThreadId = std::this_thread::get_id();
#endif
    }

    void execute(void* buffer);

    /*
     * queueCommand() allows to queue a lambda function as a command.
     * This is much less efficient than using the Driver* API.
     */
    void queueCommand(std::function<void()> command);

    /*
     * Allocates memory associated to the current CommandStreamBuffer.
     * This memory will be automatically freed after this command buffer is processed.
     * IMPORTANT: Destructors ARE NOT called
     */
    inline void* allocate(size_t size, size_t alignment = 8) noexcept;

    /*
     * Helper to allocate an array of trivially destructible objects
     */
    template<typename PodType,
            typename = typename std::enable_if<std::is_trivially_destructible<PodType>::value>::type>
    inline PodType* allocatePod(
            size_t count = 1, size_t alignment = alignof(PodType)) noexcept;

private:
    // Dispatcher could be a value (instead of pointer), which saves a load when writing commands
    // at the expense of a larger CommandStream object (about ~400 bytes)
    Dispatcher* mDispatcher = nullptr;
    Driver* mDriver = nullptr;
    CircularBuffer* UTILS_RESTRICT mCurrentBuffer = nullptr;

#ifndef NDEBUG
    // just for debugging...
    std::thread::id mThreadId;
#endif

    inline void* allocateCommand(size_t size) {
        assert(mThreadId == std::this_thread::get_id());
        return mCurrentBuffer->allocate(size);
    }
};

void* CommandStream::allocate(size_t size, size_t alignment) noexcept {
    // make sure alignment is a power of two
    assert(alignment && !(alignment & alignment-1));

    // pad the requested size to accommodate NoopCommand and alignment
    const size_t s = CustomCommand::align(sizeof(NoopCommand) + size + alignment - 1);

    // allocate space in the command stream and insert a NoopCommand
    char* const p = (char *)allocateCommand(s);
    new(p) NoopCommand(p + s);

    // calculate the "user" data pointer
    void* data = (void *)((uintptr_t(p) + sizeof(NoopCommand) + alignment - 1) & ~(alignment - 1));
    assert(data >= p + sizeof(NoopCommand));
    return data;
}

template<typename PodType, typename>
PodType* CommandStream::allocatePod(size_t count, size_t alignment) noexcept {
    return static_cast<PodType*>(allocate(count * sizeof(PodType), alignment));
}

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_COMMANDSTREAM_H
