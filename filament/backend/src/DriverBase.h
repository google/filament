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

#ifndef TNT_FILAMENT_DRIVER_DRIVERBASE_H
#define TNT_FILAMENT_DRIVER_DRIVERBASE_H

#include <utils/compiler.h>
#include <utils/CString.h>

#include <backend/Platform.h>

#include <backend/DriverEnums.h>

#include "private/backend/AcquiredImage.h"
#include "private/backend/Driver.h"
#include "private/backend/SamplerGroup.h"

#include <array>
#include <mutex>
#include <utility>

#include <assert.h>
#include <stdint.h>

namespace filament {
namespace backend {

class Dispatcher;

/*
 * Hardware handles
 */

struct HwBase {
#if !defined(NDEBUG) && UTILS_HAS_RTTI
    const char* typeId = nullptr;
#endif
};

struct HwVertexBuffer : public HwBase {
    AttributeArray attributes{};          // 8 * MAX_VERTEX_ATTRIBUTE_COUNT
    uint32_t vertexCount{};               //   4
    uint8_t bufferCount{};                //   1
    uint8_t attributeCount{};             //   1
    uint8_t padding[2]{};                 //   2 -> total struct is 136 bytes

    HwVertexBuffer() noexcept = default;
    HwVertexBuffer(uint8_t bufferCount, uint8_t attributeCount, uint32_t elementCount,
            AttributeArray const& attributes) noexcept
            : attributes(attributes),
              vertexCount(elementCount),
              bufferCount(bufferCount),
              attributeCount(attributeCount) {
    }
};

struct HwIndexBuffer : public HwBase {
    uint32_t count{};
    uint8_t elementSize{};

    HwIndexBuffer() noexcept = default;
    HwIndexBuffer(uint8_t elementSize, uint32_t indexCount) noexcept :
            count(indexCount), elementSize(elementSize) {
    }
};

struct HwRenderPrimitive : public HwBase {
    uint32_t offset{};
    uint32_t minIndex{};
    uint32_t maxIndex{};
    uint32_t count{};
    uint32_t maxVertexCount{};
    PrimitiveType type = PrimitiveType::TRIANGLES;
};

struct HwProgram : public HwBase {
#ifndef NDEBUG
    utils::CString name;
    explicit HwProgram(utils::CString name) noexcept : name(std::move(name)) { }
#else
    explicit HwProgram(const utils::CString&) noexcept { }
#endif
    HwProgram() noexcept = default;
};

struct HwSamplerGroup : public HwBase {
    // NOTE: we have to use out-of-line allocation here because the size of a Handle<> is limited
    std::unique_ptr<SamplerGroup> sb; // FIXME: this shouldn't depend on filament::SamplerGroup
    HwSamplerGroup() noexcept = default;
    explicit HwSamplerGroup(size_t size) noexcept : sb(new SamplerGroup(size)) { }
};

struct HwUniformBuffer : public HwBase {
};

struct HwTexture : public HwBase {
    uint32_t width{};
    uint32_t height{};
    uint32_t depth{};
    SamplerType target{};
    uint8_t levels : 4;  // This allows up to 15 levels (max texture size of 32768 x 32768)
    uint8_t samples : 4; // In practice this is always 1.
    TextureFormat format{};
    TextureUsage usage{};
    HwStream* hwStream = nullptr;

    HwTexture() noexcept : levels{}, samples{} {}
    HwTexture(backend::SamplerType target, uint8_t levels, uint8_t samples,
              uint32_t width, uint32_t height, uint32_t depth, TextureFormat fmt, TextureUsage usage) noexcept
            : width(width), height(height), depth(depth),
              target(target), levels(levels), samples(samples), format(fmt), usage(usage) { }
};

struct HwRenderTarget : public HwBase {
    uint32_t width{};
    uint32_t height{};
    HwRenderTarget() noexcept = default;
    HwRenderTarget(uint32_t w, uint32_t h) : width(w), height(h) { }
};

struct HwFence : public HwBase {
    Platform::Fence* fence = nullptr;
};

struct HwSync : public HwBase {
};

struct HwSwapChain : public HwBase {
    Platform::SwapChain* swapChain = nullptr;
};

struct HwStream : public HwBase {
    Platform::Stream* stream = nullptr;
    StreamType streamType = StreamType::ACQUIRED;
    uint32_t width{};
    uint32_t height{};

    HwStream() noexcept = default;
    explicit HwStream(Platform::Stream* stream) noexcept
            : stream(stream), streamType(StreamType::NATIVE) {
    }
};

struct HwTimerQuery : public HwBase {
};

/*
 * Base class of all Driver implementations
 */

class DriverBase : public Driver {
public:
    DriverBase() = delete;
    explicit DriverBase(Dispatcher* dispatcher) noexcept;
    ~DriverBase() noexcept override;

    void purge() noexcept final;

    Dispatcher& getDispatcher() noexcept final { return *mDispatcher; }

    // --------------------------------------------------------------------------------------------
    // Privates
    // --------------------------------------------------------------------------------------------

protected:
    Dispatcher* mDispatcher;

    inline void scheduleDestroy(BufferDescriptor&& buffer) noexcept {
        if (buffer.hasCallback()) {
            scheduleDestroySlow(std::move(buffer));
        }
    }

    void scheduleDestroySlow(BufferDescriptor&& buffer) noexcept;

    void scheduleRelease(AcquiredImage&& image) noexcept;

private:
    std::mutex mPurgeLock;
    std::vector<BufferDescriptor> mBufferToPurge;
    std::vector<AcquiredImage> mImagesToPurge;
};


} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_DRIVERBASE_H
