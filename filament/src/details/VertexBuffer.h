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

#include "upcast.h"

#include "driver/Driver.h"
#include "driver/Handle.h"

#include <private/filament/EngineEnums.h>
#include <filament/VertexBuffer.h>

#include <utils/bitset.h>
#include <utils/compiler.h>

#include <array>
#include <type_traits>

namespace filament {
namespace details {

class FEngine;

class FVertexBuffer : public VertexBuffer {
public:
    FVertexBuffer(FEngine& engine, const Builder& builder);

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    Handle<HwVertexBuffer> getHwHandle() const noexcept { return mHandle; }

    size_t getVertexCount() const noexcept;

    AttributeBitset getDeclaredAttributes() const noexcept {
        return mDeclaredAttributes;
    }

    // no-op if bufferIndex out of range
    void setBufferAt(FEngine& engine, uint8_t bufferIndex,
            driver::BufferDescriptor&& buffer, uint32_t byteOffset = 0);

private:
    friend class VertexBuffer;

    struct AttributeData : Driver::Attribute {
        AttributeData() : Driver::Attribute{ .type = Driver::ElementType::FLOAT4 } {}
    };

    Handle<HwVertexBuffer> mHandle;
    std::array<AttributeData, MAX_ATTRIBUTE_BUFFERS_COUNT> mAttributes;
    AttributeBitset mDeclaredAttributes;
    uint32_t mVertexCount = 0;
    uint8_t mBufferCount = 0;
};

FILAMENT_UPCAST(VertexBuffer)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_VERTEXBUFFER_H
