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

#include "driver/DriverBase.h"
#include "driver/Driver.h"
#include "driver/CommandStream.h"

#include <math/half.h>
#include <math/quat.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <filament/driver/BufferDescriptor.h>
#include <filament/driver/PixelBufferDescriptor.h>

using namespace utils;

namespace filament {

using namespace driver;

DriverBase::DriverBase(Dispatcher* dispatcher) noexcept
        : mDispatcher(dispatcher) {
    // make sure mTextureInfo entries are sorted
    assert(checkTextureInfo());
}

DriverBase::~DriverBase() noexcept {
    delete mDispatcher;
}

void DriverBase::purge() noexcept {
    std::vector<BufferDescriptor> buffersToPurge;
    std::unique_lock<std::mutex> lock(mPurgeLock);
    std::swap(buffersToPurge, mBufferToPurge);
    lock.unlock(); // don't remove this, it ensures mBufferToPurge is destroyed without lock held
}

void DriverBase::scheduleDestroySlow(BufferDescriptor&& buffer) noexcept {
    std::lock_guard<std::mutex> lock(mPurgeLock);
    mBufferToPurge.push_back(std::move(buffer));
}

// ------------------------------------------------------------------------------------------------
// Texture format data...
// ------------------------------------------------------------------------------------------------

/*
 * This array contains information we might need about each texture internal formats.
 *
 * All entries MUST be sorted by Driver::TextureFormat.
 */
const DriverBase::Entry DriverBase::mTextureInfo[] = {
        // 8-bits per element
        { TF::R8,                SF::FLOAT,  SP::LOW    },
        { TF::R8_SNORM,          SF::FLOAT,  SP::LOW    },
        { TF::R8UI,              SF::UINT,   SP::LOW    },
        { TF::R8I,               SF::INT,    SP::LOW    },

        // 16-bits per element
        { TF::R16F,              SF::FLOAT,  SP::MEDIUM },
        { TF::R16UI,             SF::UINT,   SP::MEDIUM },
        { TF::R16I,              SF::INT,    SP::MEDIUM },
        { TF::RG8,               SF::FLOAT,  SP::LOW    },
        { TF::RG8_SNORM,         SF::FLOAT,  SP::LOW    },
        { TF::RG8UI,             SF::UINT,   SP::LOW    },
        { TF::RG8I,              SF::INT,    SP::LOW    },
        { TF::RGB565,            SF::FLOAT,  SP::LOW    },
        { TF::RGB9_E5,           SF::FLOAT,  SP::MEDIUM }, // This one is actually 32 bpp.
        { TF::RGB5_A1,           SF::FLOAT,  SP::LOW    },
        { TF::RGBA4,             SF::FLOAT,  SP::LOW    },
        { TF::DEPTH16,           SF::SHADOW, SP::MEDIUM },

        // 24-bits per element
        { TF::RGB8,              SF::FLOAT,  SP::LOW    },
        { TF::SRGB8,             SF::FLOAT,  SP::LOW    },
        { TF::RGB8_SNORM,        SF::FLOAT,  SP::LOW    },
        { TF::RGB8UI,            SF::UINT,   SP::LOW    },
        { TF::RGB8I,             SF::INT,    SP::LOW    },
        { TF::DEPTH24,           SF::SHADOW, SP::HIGH   },

        // 32-bits per element
        { TF::R32F,              SF::FLOAT,  SP::HIGH   },
        { TF::R32UI,             SF::UINT,   SP::HIGH   },
        { TF::R32I,              SF::INT,    SP::HIGH   },
        { TF::RG16F,             SF::FLOAT,  SP::MEDIUM },
        { TF::RG16UI,            SF::UINT,   SP::MEDIUM },
        { TF::RG16I,             SF::INT,    SP::MEDIUM },
        { TF::R11F_G11F_B10F,    SF::FLOAT,  SP::MEDIUM },
        { TF::RGBA8,             SF::FLOAT,  SP::LOW    },
        { TF::SRGB8_A8,          SF::FLOAT,  SP::LOW    },
        { TF::RGBA8_SNORM,       SF::FLOAT,  SP::LOW    },
        { TF::RGB10_A2,          SF::FLOAT,  SP::MEDIUM },
        { TF::RGBA8UI,           SF::UINT,   SP::LOW    },
        { TF::RGBA8I,            SF::INT,    SP::LOW    },
        { TF::DEPTH32F,          SF::SHADOW, SP::HIGH   },
        { TF::DEPTH24_STENCIL8,  SF::SHADOW, SP::HIGH   },
        { TF::DEPTH32F_STENCIL8, SF::SHADOW, SP::HIGH   },

        // 48-bits per element
        { TF::RGB16F,            SF::FLOAT,  SP::MEDIUM },
        { TF::RGB16UI,           SF::UINT,   SP::MEDIUM },
        { TF::RGB16I,            SF::INT,    SP::MEDIUM },

        // 64-bits per element
        { TF::RG32F,             SF::FLOAT,  SP::HIGH   },
        { TF::RG32UI,            SF::UINT,   SP::HIGH   },
        { TF::RG32I,             SF::INT,    SP::HIGH   },
        { TF::RGBA16F,           SF::FLOAT,  SP::MEDIUM },
        { TF::RGBA16UI,          SF::UINT,   SP::MEDIUM },
        { TF::RGBA16I,           SF::INT,    SP::MEDIUM },

        // 96-bits per element
        { TF::RGB32F,            SF::FLOAT,  SP::HIGH   },
        { TF::RGB32UI,           SF::UINT,   SP::HIGH   },
        { TF::RGB32I,            SF::INT,    SP::HIGH   },

        // 128-bits per element
        { TF::RGBA32F,           SF::FLOAT,  SP::HIGH   },
        { TF::RGBA32UI,          SF::UINT,   SP::HIGH   },
        { TF::RGBA32I,           SF::INT,    SP::HIGH   },
};

bool DriverBase::checkTextureInfo() noexcept {
    DriverBase::Entry const* first = mTextureInfo;
    DriverBase::Entry const* const last = first + sizeof(mTextureInfo) / sizeof(*mTextureInfo);
    DriverBase::Entry const* i = first;
    while (++i != last) {
        if (first->textureFormat >= i->textureFormat) {
            return false;
        }
        first = i;
    }
    return true;
}

const DriverBase::Entry* DriverBase::findTextureInfo(TextureFormat format) noexcept {
    static constexpr const size_t size = sizeof(mTextureInfo) / sizeof(mTextureInfo[0]);
    Entry e = { format, SF::INT, SP::DEFAULT };
    auto begin = &mTextureInfo[0];
    auto end = &mTextureInfo[size];
    auto pos = std::lower_bound(begin, end, e);
    assert(pos < end && pos->textureFormat == format);
    return pos;
}

Driver::SamplerPrecision DriverBase::getSamplerPrecision(TextureFormat format) noexcept {
    return findTextureInfo(format)->samplerPrecision;
}

Driver::SamplerFormat DriverBase::getSamplerFormat(TextureFormat format) noexcept {
    return findTextureInfo(format)->samplerFormat;
}

// ------------------------------------------------------------------------------------------------

Driver::~Driver() noexcept = default;

// forward to DriverBase, where the implementation really is
Driver::SamplerPrecision Driver::getSamplerPrecision(TextureFormat format) noexcept {
    return DriverBase::getSamplerPrecision(format);
}

// forward to DriverBase, where the implementation really is
Driver::SamplerFormat Driver::getSamplerFormat(TextureFormat format) noexcept {
    return DriverBase::getSamplerFormat(format);
}

size_t Driver::getElementTypeSize(ElementType type) noexcept {
    switch (type) {
        case ElementType::BYTE:     return sizeof(int8_t);
        case ElementType::BYTE2:    return sizeof(math::byte2);
        case ElementType::BYTE3:    return sizeof(math::byte3);
        case ElementType::BYTE4:    return sizeof(math::byte4);
        case ElementType::UBYTE:    return sizeof(uint8_t);
        case ElementType::UBYTE2:   return sizeof(math::ubyte2);
        case ElementType::UBYTE3:   return sizeof(math::ubyte3);
        case ElementType::UBYTE4:   return sizeof(math::ubyte4);
        case ElementType::SHORT:    return sizeof(int16_t);
        case ElementType::SHORT2:   return sizeof(math::short2);
        case ElementType::SHORT3:   return sizeof(math::short3);
        case ElementType::SHORT4:   return sizeof(math::short4);
        case ElementType::USHORT:   return sizeof(uint16_t);
        case ElementType::USHORT2:  return sizeof(math::ushort2);
        case ElementType::USHORT3:  return sizeof(math::ushort3);
        case ElementType::USHORT4:  return sizeof(math::ushort4);
        case ElementType::INT:      return sizeof(int32_t);
        case ElementType::UINT:     return sizeof(uint32_t);
        case ElementType::FLOAT:    return sizeof(float);
        case ElementType::FLOAT2:   return sizeof(math::float2);
        case ElementType::FLOAT3:   return sizeof(math::float3);
        case ElementType::FLOAT4:   return sizeof(math::float4);
        case ElementType::HALF:     return sizeof(math::half);
        case ElementType::HALF2:    return sizeof(math::half2);
        case ElementType::HALF3:    return sizeof(math::half3);
        case ElementType::HALF4:    return sizeof(math::half4);
    }
}

} // namespace filament
