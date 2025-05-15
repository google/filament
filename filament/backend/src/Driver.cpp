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

#include "DriverBase.h"

#include "private/backend/Driver.h"
#include "private/backend/CommandStream.h"

#include <backend/AcquiredImage.h>
#include <backend/BufferDescriptor.h>
#include <backend/DriverEnums.h>

#include <private/utils/Tracing.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Log.h>
#include <utils/ostream.h>

#include <math/half.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <functional>
#include <mutex>
#include <utility>

#include <stddef.h>
#include <stdint.h>

using namespace utils;
using namespace filament::math;

namespace filament::backend {

DriverBase::DriverBase() noexcept {
    if constexpr (UTILS_HAS_THREADING) {
        // This thread services user callbacks
        mServiceThread = std::thread([this]() {
            do {
                auto& serviceThreadCondition = mServiceThreadCondition;
                auto& serviceThreadCallbackQueue = mServiceThreadCallbackQueue;

                // wait for some callbacks to dispatch
                std::unique_lock<std::mutex> lock(mServiceThreadLock);
                while (serviceThreadCallbackQueue.empty() && !mExitRequested) {
                    serviceThreadCondition.wait(lock);
                }
                if (mExitRequested) {
                    break;
                }
                // move the callbacks to a temporary vector
                auto callbacks(std::move(serviceThreadCallbackQueue));
                lock.unlock();
                // and make sure to call them without our lock held
                for (auto[handler, callback, user]: callbacks) {
                    handler->post(user, callback);
                }
            } while (true);
        });
    }
}

DriverBase::~DriverBase() noexcept {
    assert_invariant(mCallbacks.empty());
    assert_invariant(mServiceThreadCallbackQueue.empty());
    if constexpr (UTILS_HAS_THREADING) {
        // quit our service thread
        std::unique_lock<std::mutex> lock(mServiceThreadLock);
        mExitRequested = true;
        mServiceThreadCondition.notify_one();
        lock.unlock();
        mServiceThread.join();
    }
}

// ------------------------------------------------------------------------------------------------


class DriverBase::CallbackDataDetails : public DriverBase::CallbackData {
    UTILS_UNUSED DriverBase* mAllocator;
public:
    explicit CallbackDataDetails(DriverBase* allocator) : mAllocator(allocator) {}
};

DriverBase::CallbackData* DriverBase::CallbackData::obtain(DriverBase* allocator) {
    // todo: use a pool
    return new CallbackDataDetails(allocator);
}

void DriverBase::CallbackData::release(CallbackData* data) {
    // todo: use a pool
    delete static_cast<CallbackDataDetails*>(data);
}


void DriverBase::scheduleCallback(CallbackHandler* handler, void* user, CallbackHandler::Callback callback) {
    if (handler && UTILS_HAS_THREADING) {
        std::lock_guard<std::mutex> const lock(mServiceThreadLock);
        mServiceThreadCallbackQueue.emplace_back(handler, callback, user);
        mServiceThreadCondition.notify_one();
    } else {
        std::lock_guard<std::mutex> const lock(mPurgeLock);
        mCallbacks.emplace_back(user, callback);
    }
}

void DriverBase::purge() noexcept {
    decltype(mCallbacks) callbacks;
    std::unique_lock<std::mutex> lock(mPurgeLock);
    std::swap(callbacks, mCallbacks);
    lock.unlock(); // don't remove this, it ensures callbacks are called without lock held
    for (auto& item : callbacks) {
        item.second(item.first);
    }
}

// ------------------------------------------------------------------------------------------------

void DriverBase::scheduleDestroySlow(BufferDescriptor&& buffer) noexcept {
    auto const handler = buffer.getHandler();
    scheduleCallback(handler, [buffer = std::move(buffer)]() {
        // user callback is called when BufferDescriptor gets destroyed
    });
}

// This is called from an async driver method so it's in the GL thread, but purge is called
// on the user thread. This is typically called 0 or 1 times per frame.
void DriverBase::scheduleRelease(AcquiredImage const& image) noexcept {
    scheduleCallback(image.handler, [callback = image.callback, image = image.image, userData = image.userData]() {
        callback(image, userData);
    });
}

void DriverBase::debugCommandBegin(CommandStream* cmds, bool synchronous, const char* methodName) noexcept {
    if constexpr (bool(FILAMENT_DEBUG_COMMANDS > FILAMENT_DEBUG_COMMANDS_NONE)) {
        if constexpr (bool(FILAMENT_DEBUG_COMMANDS & FILAMENT_DEBUG_COMMANDS_LOG)) {
            utils::slog.d << methodName << utils::io::endl;
        }
        if constexpr (bool(FILAMENT_DEBUG_COMMANDS & FILAMENT_DEBUG_COMMANDS_SYSTRACE)) {
            FILAMENT_TRACING_CONTEXT(FILAMENT_TRACING_CATEGORY_FILAMENT);
            FILAMENT_TRACING_NAME_BEGIN(FILAMENT_TRACING_CATEGORY_FILAMENT, methodName);

            if (!synchronous) {
                cmds->queueCommand([=]() {
                    FILAMENT_TRACING_CONTEXT(FILAMENT_TRACING_CATEGORY_FILAMENT);
                    FILAMENT_TRACING_NAME_BEGIN(FILAMENT_TRACING_CATEGORY_FILAMENT, methodName);
                });
            }
        }
    }
}

void DriverBase::debugCommandEnd(CommandStream* cmds, bool synchronous,
        const char* methodName) noexcept {
    if constexpr (bool(FILAMENT_DEBUG_COMMANDS > FILAMENT_DEBUG_COMMANDS_NONE)) {
        if constexpr (bool(FILAMENT_DEBUG_COMMANDS & FILAMENT_DEBUG_COMMANDS_SYSTRACE)) {
            if (!synchronous) {
                cmds->queueCommand([]() {
                    FILAMENT_TRACING_CONTEXT(FILAMENT_TRACING_CATEGORY_FILAMENT);
                    FILAMENT_TRACING_NAME_END(FILAMENT_TRACING_CATEGORY_FILAMENT);
                });
            }
            FILAMENT_TRACING_CONTEXT(FILAMENT_TRACING_CATEGORY_FILAMENT);
            FILAMENT_TRACING_NAME_END(FILAMENT_TRACING_CATEGORY_FILAMENT);
        }
    }
}

size_t Driver::getElementTypeSize(ElementType type) noexcept {
    switch (type) {
        case ElementType::BYTE:     return sizeof(int8_t);
        case ElementType::BYTE2:    return sizeof(byte2);
        case ElementType::BYTE3:    return sizeof(byte3);
        case ElementType::BYTE4:    return sizeof(byte4);
        case ElementType::UBYTE:    return sizeof(uint8_t);
        case ElementType::UBYTE2:   return sizeof(ubyte2);
        case ElementType::UBYTE3:   return sizeof(ubyte3);
        case ElementType::UBYTE4:   return sizeof(ubyte4);
        case ElementType::SHORT:    return sizeof(int16_t);
        case ElementType::SHORT2:   return sizeof(short2);
        case ElementType::SHORT3:   return sizeof(short3);
        case ElementType::SHORT4:   return sizeof(short4);
        case ElementType::USHORT:   return sizeof(uint16_t);
        case ElementType::USHORT2:  return sizeof(ushort2);
        case ElementType::USHORT3:  return sizeof(ushort3);
        case ElementType::USHORT4:  return sizeof(ushort4);
        case ElementType::INT:      return sizeof(int32_t);
        case ElementType::UINT:     return sizeof(uint32_t);
        case ElementType::FLOAT:    return sizeof(float);
        case ElementType::FLOAT2:   return sizeof(float2);
        case ElementType::FLOAT3:   return sizeof(float3);
        case ElementType::FLOAT4:   return sizeof(float4);
        case ElementType::HALF:     return sizeof(half);
        case ElementType::HALF2:    return sizeof(half2);
        case ElementType::HALF3:    return sizeof(half3);
        case ElementType::HALF4:    return sizeof(half4);
    }
}

// ------------------------------------------------------------------------------------------------

Driver::~Driver() noexcept = default;

void Driver::execute(std::function<void(void)> const& fn) {
    fn();
}

} // namespace filament::backend
