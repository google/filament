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

#ifndef TNT_FILAMENT_DRIVER_BUFFERDESCRIPTOR_H
#define TNT_FILAMENT_DRIVER_BUFFERDESCRIPTOR_H

#include <stddef.h>
#include <stdint.h>

#include <utils/compiler.h>

namespace filament {
namespace driver {

class UTILS_PUBLIC BufferDescriptor {
public:
    /**
     * Callback used to destroy the buffer data.
     * It is guaranteed to be called on the main filament thread.
     */
    using Callback = void(*)(void* buffer, size_t size, void* user);

    BufferDescriptor() noexcept = default;

    ~BufferDescriptor() noexcept {
        if (callback) {
            callback(buffer, size, user);
        }
    }

    BufferDescriptor(const BufferDescriptor& rhs) = delete;
    BufferDescriptor& operator=(const BufferDescriptor& rhs) = delete;

    BufferDescriptor(BufferDescriptor&& rhs) noexcept
        : buffer(rhs.buffer), size(rhs.size), callback(rhs.callback), user(rhs.user) {
            rhs.buffer = nullptr;
            rhs.callback = nullptr;
    }

    BufferDescriptor& operator=(BufferDescriptor&& rhs) noexcept {
        if (this != &rhs) {
            buffer = rhs.buffer;
            size = rhs.size;
            callback = rhs.callback;
            user = rhs.user;
            rhs.buffer = nullptr;
            rhs.callback = nullptr;
        }
        return *this;
    }

    BufferDescriptor(void const* buffer, size_t size,
            Callback callback = nullptr, void* user = nullptr) noexcept
                : buffer(const_cast<void*>(buffer)), size(size), callback(callback), user(user) {
    }

    void setCallback(Callback callback, void* user = nullptr) noexcept {
        this->callback = callback;
        this->user = user;
    }

    bool hasCallback() const noexcept { return callback != nullptr; }


    Callback getCallback() const noexcept {
        return callback;
    }

    void* getUser() const noexcept {
        return user;
    }

    // buffer virtual address
    void* buffer = nullptr;

    // buffer size in bytes
    size_t size = 0;

    // -------------------------------------------------------------------------------------------
    // no user serviceable parts below ...

private:
    // callback when the buffer is consumed.
    Callback callback = nullptr;
    void* user = nullptr;
};

} // namespace driver
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_BUFFERDESCRIPTOR_H
