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

#ifndef TNT_FILAMENT_DETAILS_INDEXBUFFER_H
#define TNT_FILAMENT_DETAILS_INDEXBUFFER_H

#include "downcast.h"

#include "details/CreationStatus.h"

#include <filament/IndexBuffer.h>

#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/debug.h>

#include <atomic>

namespace filament {

class FEngine;

class FIndexBuffer : public IndexBuffer {
public:
    FIndexBuffer(FEngine& engine, const Builder& builder);

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    // Only meaningful once the creation succeeded. A canceled asynchronous creation leaves
    // mHandle referring to backend resources that were never generated.
    backend::Handle<backend::HwIndexBuffer> getHwHandle() const noexcept {
        assert_invariant(isCreationSuccessful());
        return mHandle;
    }

    size_t getIndexCount() const noexcept { return mIndexCount; }

    void setBuffer(FEngine& engine, BufferDescriptor&& buffer, uint32_t byteOffset = 0);

    AsyncCallId setBufferAsync(FEngine& engine, BufferDescriptor&& buffer, uint32_t byteOffset,
            backend::CallbackHandler* handler, AsyncCompletionCallback callback, void* user);

    // Whether the asynchronous pipeline is done with this object, whether or not it succeeded.
    // This is a *lifetime* gate: FEngine::destroy detects this method by name and waits on it
    // before freeing the object, so it must become true even when creation is canceled.
    // Use isCreationSuccessful() to know whether the resource can be used.
    bool isCreationSettled() const noexcept {
        return mCreationStatus.load(std::memory_order_relaxed) != CreationStatus::CREATING;
    }

    // Whether creation finished *and* actually populated the resource. A canceled creation
    // finishes without ever running, so the resource is not usable. This is what the public
    // IndexBuffer::isCreationComplete() reports.
    bool isCreationSuccessful() const noexcept {
        return mCreationStatus.load(std::memory_order_relaxed) == CreationStatus::CREATED;
    }

private:
    friend class IndexBuffer;
    backend::Handle<backend::HwIndexBuffer> mHandle;
    uint32_t mIndexCount;

    // Where the creation process is. This is especially useful for asynchronous creation; it only
    // ever moves out of CREATING once, to one of the two terminal states.
    std::atomic<CreationStatus> mCreationStatus{ CreationStatus::CREATING };
};

FILAMENT_DOWNCAST(IndexBuffer)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_INDEXBUFFER_H
