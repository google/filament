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

//! \file

#ifndef TNT_FILAMENT_BACKEND_BUFFERDESCRIPTOR_H
#define TNT_FILAMENT_BACKEND_BUFFERDESCRIPTOR_H

#include <utils/compiler.h>
#include <utils/ostream.h>

#include <stddef.h>

namespace filament::backend {

class CallbackHandler;

/**
 * A CPU memory-buffer descriptor, typically used to transfer data from the CPU to the GPU.
 *
 * A BufferDescriptor owns the memory buffer it references, therefore BufferDescriptor cannot
 * be copied, but can be moved.
 *
 * BufferDescriptor releases ownership of the memory-buffer when it's destroyed.
 */
class UTILS_PUBLIC BufferDescriptor {
public:
    /**
     * Callback used to destroy the buffer data.
     * Guarantees:
     *      Called on the main filament thread.
     *
     * Limitations:
     *      Must be lightweight.
     *      Must not call filament APIs.
     */
    using Callback = void(*)(void* buffer, size_t size, void* user);

    //! creates an empty descriptor
    BufferDescriptor() noexcept = default;

    //! calls the callback to advertise BufferDescriptor no-longer owns the buffer
    ~BufferDescriptor() noexcept {
        if (mCallback) {
            mCallback(buffer, size, mUser);
        }
    }

    BufferDescriptor(const BufferDescriptor& rhs) = delete;
    BufferDescriptor& operator=(const BufferDescriptor& rhs) = delete;

    BufferDescriptor(BufferDescriptor&& rhs) noexcept
        : buffer(rhs.buffer), size(rhs.size),
          mCallback(rhs.mCallback), mUser(rhs.mUser), mHandler(rhs.mHandler) {
            rhs.buffer = nullptr;
            rhs.mCallback = nullptr;
    }

    BufferDescriptor& operator=(BufferDescriptor&& rhs) noexcept {
        if (this != &rhs) {
            buffer = rhs.buffer;
            size = rhs.size;
            mCallback = rhs.mCallback;
            mUser = rhs.mUser;
            mHandler = rhs.mHandler;
            rhs.buffer = nullptr;
            rhs.mCallback = nullptr;
        }
        return *this;
    }

    /**
     * Creates a BufferDescriptor that references a CPU memory-buffer
     * @param buffer    Memory address of the CPU buffer to reference
     * @param size      Size of the CPU buffer in bytes
     * @param callback  A callback used to release the CPU buffer from this BufferDescriptor
     * @param user      An opaque user pointer passed to the callback function when it's called
     */
    BufferDescriptor(void const* buffer, size_t size,
            Callback callback = nullptr, void* user = nullptr) noexcept
                : buffer(const_cast<void*>(buffer)), size(size), mCallback(callback), mUser(user) {
    }

    /**
     * Creates a BufferDescriptor that references a CPU memory-buffer
     * @param buffer    Memory address of the CPU buffer to reference
     * @param size      Size of the CPU buffer in bytes
     * @param callback  A callback used to release the CPU buffer from this BufferDescriptor
     * @param user      An opaque user pointer passed to the callback function when it's called
     */
    BufferDescriptor(void const* buffer, size_t size,
            CallbackHandler* handler, Callback callback, void* user = nullptr) noexcept
                : buffer(const_cast<void*>(buffer)), size(size),
                mCallback(callback), mUser(user), mHandler(handler) {
    }

    // --------------------------------------------------------------------------------------------

    /**
     * Helper to create a BufferDescriptor that uses a KNOWN method pointer w/ object passed
     * by pointer as the callback. e.g.:
     *     auto bd = BufferDescriptor::make<Foo, &Foo::method>(buffer, size, foo);
     *
     * @param buffer    Memory address of the CPU buffer to reference
     * @param size      Size of the CPU buffer in bytes
     * @param handler   Handler to use to dispatch the callback, or nullptr for the default handler
     * @return          a new BufferDescriptor
     */
    template<typename T, void(T::*method)(void const*, size_t)>
    static BufferDescriptor make(void const* buffer, size_t size, T* data,
            CallbackHandler* handler = nullptr) noexcept {
        return {
                buffer, size,
                handler, [](void* b, size_t s, void* u) {
                    (static_cast<T*>(u)->*method)(b, s);
                }, data
        };
    }

    /**
     * Helper to create a BufferDescriptor that uses a functor as the callback.
     *
     * Caveats:
     *      - DO NOT CALL setCallback() when using this helper.
     *      - This make a heap allocation
     *
     * @param buffer    Memory address of the CPU buffer to reference
     * @param size      Size of the CPU buffer in bytes
     * @param functor   functor of type f(void const* buffer, size_t size)
     * @param handler   Handler to use to dispatch the callback, or nullptr for the default handler
     * @return          a new BufferDescriptor
     */
    template<typename T>
    static BufferDescriptor make(void const* buffer, size_t size, T&& functor,
            CallbackHandler* handler = nullptr) noexcept {
        return {
                buffer, size,
                handler, [](void* b, size_t s, void* u) {
                    T* const that = static_cast<T*>(u);
                    that->operator()(b, s);
                    delete that;
                },
                new T(std::forward<T>(functor))
        };
    }

    // --------------------------------------------------------------------------------------------

    /**
     * Set or replace the release callback function
     * @param callback  The new callback function
     * @param user      An opaque user pointer passed to the callbeck function when it's called
     */
    void setCallback(Callback callback, void* user = nullptr) noexcept {
        this->mCallback = callback;
        this->mUser = user;
        this->mHandler = nullptr;
    }

    /**
     * Set or replace the release callback function
     * @param handler   The Handler to use to dispatch the callback
     * @param callback  The new callback function
     * @param user      An opaque user pointer passed to the callbeck function when it's called
     */
    void setCallback(CallbackHandler* handler, Callback callback, void* user = nullptr) noexcept {
        mCallback = callback;
        mUser = user;
        mHandler = handler;
    }

    //! Returns whether a release callback is set
    bool hasCallback() const noexcept { return mCallback != nullptr; }

    //! Returns the currently set release callback function
    Callback getCallback() const noexcept {
        return mCallback;
    }

    //! Returns the handler for this callback or nullptr if the default handler is to be used.
    CallbackHandler* getHandler() const noexcept {
        return mHandler;
    }

    //! Returns the user opaque pointer associated to this BufferDescriptor
    void* getUser() const noexcept {
        return mUser;
    }

    //! CPU memory-buffer virtual address
    void* buffer = nullptr;

    //! CPU memory-buffer size in bytes
    size_t size = 0;

private:
    // callback when the buffer is consumed.
    Callback mCallback = nullptr;
    void* mUser = nullptr;
    CallbackHandler* mHandler = nullptr;
};

} // namespace filament::backend

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::BufferDescriptor& b);
#endif

#endif // TNT_FILAMENT_BACKEND_BUFFERDESCRIPTOR_H
