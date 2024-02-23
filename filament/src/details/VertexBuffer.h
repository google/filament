/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_VERTEXBUFFER_H
#define TNT_FILAMENT_DETAILS_VERTEXBUFFER_H

#include "downcast.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <filament/VertexBuffer.h>

#include <utils/bitset.h>
#include <utils/compiler.h>

#include <math/vec2.h>

#include <array>
#include <memory>
#include <type_traits>

namespace filament {

class FBufferObject;
class FEngine;

class FVertexBuffer : public VertexBuffer {
public:
    using VertexBufferInfoHandle = backend::VertexBufferInfoHandle;
    using VertexBufferHandle = backend::VertexBufferHandle;
    using BufferObjectHandle = backend::BufferObjectHandle;

    FVertexBuffer(FEngine& engine, const Builder& builder);
    FVertexBuffer(FEngine& engine, FVertexBuffer* buffer);

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    VertexBufferHandle getHwHandle() const noexcept { return mHandle; }

    size_t getVertexCount() const noexcept;

    AttributeBitset getDeclaredAttributes() const noexcept {
        return mDeclaredAttributes;
    }

    // no-op if bufferIndex out of range
    void setBufferAt(FEngine& engine, uint8_t bufferIndex,
            backend::BufferDescriptor&& buffer, uint32_t byteOffset = 0);

    void setBufferObjectAt(FEngine& engine, uint8_t bufferIndex,
            FBufferObject const * bufferObject);

    void updateBoneIndicesAndWeights(FEngine& engine, std::unique_ptr<uint16_t[]> skinJoints,
                                        std::unique_ptr<float[]> skinWeights);

private:
    friend class VertexBuffer;
    VertexBufferInfoHandle mVertexBufferInfoHandle;
    VertexBufferHandle mHandle;
    backend::AttributeArray mAttributes;
    std::array<BufferObjectHandle, backend::MAX_VERTEX_BUFFER_COUNT> mBufferObjects;
    AttributeBitset mDeclaredAttributes;
    uint32_t mVertexCount = 0;
    uint8_t mBufferCount = 0;
    bool mBufferObjectsEnabled = false;
    bool mAdvancedSkinningEnabled = false;
};

FILAMENT_DOWNCAST(VertexBuffer)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_VERTEXBUFFER_H
