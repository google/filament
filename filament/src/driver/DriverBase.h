#include <utility>

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

#include <array>
#include <mutex>
#include <assert.h>
#include <stdint.h>

#include <utils/compiler.h>
#include <utils/CString.h>

#include <filament/driver/DriverEnums.h>

#include "driver/Driver.h"
#include "driver/SamplerBuffer.h"

namespace filament {

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
    static constexpr size_t MAX_ATTRIBUTE_BUFFER_COUNT = Driver::MAX_ATTRIBUTE_BUFFER_COUNT;

    Driver::AttributeArray attributes;    // 8*8
    uint32_t vertexCount;                 //   4
    uint8_t bufferCount;                  //   1
    uint8_t attributeCount;               //   1
    uint8_t padding[2]{};                 //   2 -> 56 bytes

    HwVertexBuffer(uint8_t bufferCount, uint8_t attributeCount, uint32_t elementCount,
            Driver::AttributeArray const& attributes) noexcept
            : attributes(attributes),
              vertexCount(elementCount),
              bufferCount(bufferCount),
              attributeCount(attributeCount) {
    }
};

struct HwIndexBuffer : public HwBase {
    HwIndexBuffer(uint8_t elementSize, uint32_t indexCount) noexcept :
            count(indexCount), elementSize(elementSize) {
    }
    uint32_t count;
    uint8_t elementSize;
};

struct HwRenderPrimitive : public HwBase {
    HwRenderPrimitive() noexcept = default;
    uint32_t offset = 0;
    uint32_t minIndex = 0;
    uint32_t maxIndex = 0;
    uint32_t count = 0;
    uint32_t maxVertexCount = 0;
    Driver::PrimitiveType type = Driver::PrimitiveType::TRIANGLES;
};

struct HwProgram : public HwBase {
#ifndef NDEBUG
    explicit HwProgram(utils::CString name) noexcept : name(std::move(name)) { }
    utils::CString name;
#else
    explicit HwProgram(const utils::CString&) noexcept { }
#endif
};

struct HwSamplerBuffer : public HwBase {
    explicit HwSamplerBuffer(size_t size) noexcept : sb(new SamplerBuffer(size)) { }
    // NOTE: we have to use out-of-line allocation here because the size of a Handle<> is limited
    std::unique_ptr<SamplerBuffer> sb;
};

struct HwUniformBuffer : public HwBase {
};

struct HwTexture : public HwBase {
    HwTexture(driver::SamplerType target, uint8_t levels, uint8_t samples,
              uint32_t width, uint32_t height, uint32_t depth) noexcept
            : width(width), height(height), depth(depth),
              target(target), levels(levels), samples(samples) { }
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    driver::SamplerType target;
    uint8_t levels;
    uint8_t samples;
    HwStream* hwStream = nullptr;
};

struct HwRenderTarget : public HwBase {
    HwRenderTarget(uint32_t w, uint32_t h) : width(w), height(h) { }
    uint32_t width;
    uint32_t height;
};

struct HwFence : public HwBase {
    driver::Platform::Fence* fence = nullptr;
};

struct HwSwapChain : public HwBase {
    driver::Platform::SwapChain* swapChain = nullptr;
};

struct HwStream : public HwBase {
    HwStream() = default;
    explicit HwStream(driver::Platform::Stream* stream) : stream(stream) { }
    driver::Platform::Stream* stream = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
};

/*
 * Base class of all Driver implementations
 */

class DriverBase : public Driver {
public:
    DriverBase() = delete;
    explicit DriverBase(Dispatcher* dispatcher) noexcept;
    ~DriverBase() noexcept override;

    static SamplerFormat getSamplerFormat(TextureFormat format) noexcept;
    static SamplerPrecision getSamplerPrecision(TextureFormat format) noexcept;

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

private:
    using TF = Driver::TextureFormat;
    using SF = Driver::SamplerFormat;
    using SP = Driver::SamplerPrecision;

    struct Entry {
        TF textureFormat;
        SF samplerFormat;
        SP samplerPrecision;

        bool operator<(const Entry& rhs) const noexcept {
            return textureFormat < rhs.textureFormat;
        }
    };

    static const Entry mTextureInfo[];
    static const Entry* findTextureInfo(TextureFormat format) noexcept;
    static bool checkTextureInfo() noexcept;

    std::mutex mPurgeLock;
    std::vector<BufferDescriptor> mBufferToPurge;
};


} // namespace filament

#endif // TNT_FILAMENT_DRIVER_DRIVERBASE_H
