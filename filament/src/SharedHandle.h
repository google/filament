/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_SHARED_HANDLE_H
#define TNT_FILAMENT_SHARED_HANDLE_H

#include <backend/Handle.h>

#include <stdint.h>

namespace filament {

/*
 * SharedHandle is a bit like shared_ptr<> but for Handle<>, the destruction is
 * performed by a Deleter functor that needs to be provided. We only support strong
 * references for now.
 *
 * caveat: The current implementation is not thread-safe.
 */
template<typename T, typename Deleter>
struct SharedHandle {
    SharedHandle() noexcept = default;

    ~SharedHandle() noexcept {
        dec(mControlBlockPtr);
    }

    SharedHandle(SharedHandle const& rhs) noexcept
            : mControlBlockPtr(inc(rhs.mControlBlockPtr)) {
    }

    SharedHandle(SharedHandle&& rhs) noexcept {
        std::swap(mControlBlockPtr, rhs.mControlBlockPtr);
    }

    SharedHandle& operator=(SharedHandle const& rhs) noexcept {
        if (this != &rhs) {
            inc(rhs.mControlBlockPtr);  // add a reference to other control block
            dec(mControlBlockPtr);      // drop a reference from ours (possibly destroying it)
            mControlBlockPtr = rhs.mControlBlockPtr; // adopt the new control block
        }
        return *this;
    }

    SharedHandle& operator=(SharedHandle&& rhs) noexcept {
        if (this != &rhs) {
            std::swap(mControlBlockPtr, rhs.mControlBlockPtr);
        }
        return *this;
    }

    // initialize the SharedHandle and provide a Deleter
    template<typename ... ARGS>
    explicit SharedHandle(backend::Handle<T> const& rhs, ARGS&& ... args) noexcept
            : mControlBlockPtr(new ControlBlock(rhs, std::forward<ARGS>(args)...)) {
    }

    // initialize the SharedHandle and provide a Deleter
    template<typename ... ARGS>
    explicit SharedHandle(backend::Handle<T>&& rhs, ARGS&& ... args) noexcept
            : mControlBlockPtr(new ControlBlock(rhs, std::forward<ARGS>(args)...)) {
    }

    // automatically converts to Handle<T>
    operator backend::Handle<T>() const noexcept { // NOLINT(*-explicit-constructor)
        return mControlBlockPtr ? mControlBlockPtr->handle : backend::Handle<T>{};
    }

    explicit operator bool() const noexcept {
        return mControlBlockPtr ? (bool)mControlBlockPtr->handle : false;
    }

    void clear() noexcept { dec(mControlBlockPtr); }

private:
    struct ControlBlock {
        template<typename ... ARGS>
        explicit ControlBlock(backend::Handle<T> handle, ARGS&& ... args) noexcept
                : deleter(std::forward<ARGS>(args)...), handle(std::move(handle)) {
        }
        void inc() noexcept {
            ++count;
        }
        void dec() noexcept {
            if (--count == 0) {
                deleter(handle);
                delete this;
            }
        }
        Deleter deleter;
        int32_t count = 1;
        backend::Handle<T> handle;
    };

    ControlBlock* inc(ControlBlock* const ctrlBlk) noexcept {
        if (ctrlBlk) {
            ctrlBlk->inc();
        }
        return ctrlBlk;
    }

    void dec(ControlBlock* const ctrlBlk) noexcept {
        if (ctrlBlk) {
            ctrlBlk->dec();
        }
    }

    ControlBlock* mControlBlockPtr = nullptr;
};

} // namespace filament

#endif // TNT_FILAMENT_SHARED_HANDLE_H
