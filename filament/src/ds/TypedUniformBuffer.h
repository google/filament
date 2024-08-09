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

#ifndef TNT_FILAMENT_TYPEDUNIFORMBUFFER_H
#define TNT_FILAMENT_TYPEDUNIFORMBUFFER_H

#include "TypedBuffer.h"

#include <backend/BufferDescriptor.h>
#include <backend/DriverApiForward.h>
#include <backend/Handle.h>

#include <stddef.h>

namespace filament {

template<typename T, size_t N = 1>
class TypedUniformBuffer {
public:

    explicit TypedUniformBuffer(backend::DriverApi& driver) noexcept {
        mUboHandle = driver.createBufferObject(
                mTypedBuffer.getSize(),
                backend::BufferObjectBinding::UNIFORM,
                backend::BufferUsage::DYNAMIC);
    }

    void terminate(backend::DriverApi& driver) noexcept {
        driver.destroyBufferObject(mUboHandle);
    }

    TypedBuffer<T,N>& getTypedBuffer() noexcept {
        return mTypedBuffer;
    }

    backend::BufferObjectHandle getUboHandle() const noexcept {
        return mUboHandle;
    }

    T& itemAt(size_t i) noexcept {
        return mTypedBuffer.itemAt(i);
    }

    T& edit() noexcept {
        return mTypedBuffer.itemAt(0);
    }

    // size of the uniform buffer in bytes
    size_t getSize() const noexcept { return mTypedBuffer.getSize(); }

    // return if any uniform has been changed
    bool isDirty() const noexcept { return mTypedBuffer.isDirty(); }

    // mark the whole buffer as "clean" (no modified uniforms)
    void clean() const noexcept { mTypedBuffer.clean(); }

    // helper functions
    backend::BufferDescriptor toBufferDescriptor(backend::DriverApi& driver) const noexcept {
        return mTypedBuffer.toBufferDescriptor(driver);
    }

    // copy the UBO data and cleans the dirty bits
    backend::BufferDescriptor toBufferDescriptor(
            backend::DriverApi& driver, size_t offset, size_t size) const noexcept {
        return mTypedBuffer.toBufferDescriptor(driver, offset, size);
    }

private:
    TypedBuffer<T,N> mTypedBuffer;
    backend::BufferObjectHandle mUboHandle;
};

} // namespace filament


#endif //TNT_FILAMENT_TYPEDUNIFORMBUFFER_H
