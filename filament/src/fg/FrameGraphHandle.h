/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FRAMEGRAPHRESOURCE_H
#define TNT_FILAMENT_FRAMEGRAPHRESOURCE_H

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <filament/Viewport.h>

#include <stdint.h>

#include <array>
#include <limits>

namespace filament {

namespace fg {
struct PassNode;
struct RenderTarget;
struct RenderTargetResource;
} // namespace fg

class FrameGraph;
class FrameGraphPassResources;

// ------------------------------------------------------------------------------------------------

struct FrameGraphTexture {
    struct Descriptor {
        uint32_t width = 1;     // width of resource in pixel
        uint32_t height = 1;    // height of resource in pixel
        uint32_t depth = 1;     // # of images for 3D textures
        uint8_t levels = 1;     // # of levels for textures
        uint8_t samples = 0;    // 0=auto, 1=request not multisample, >1 only for NOT SAMPLEABLE
        backend::SamplerType type = backend::SamplerType::SAMPLER_2D;     // texture target type
        backend::TextureFormat format = backend::TextureFormat::RGBA8;    // resource internal format
        backend::TextureUsage usage = (backend::TextureUsage)0; // don't need to set this one
    };

    void create(FrameGraph& fg, const char* name, Descriptor const& desc) noexcept;
    void destroy(FrameGraph& fg) noexcept;

    backend::Handle<backend::HwTexture> texture;
};

// ------------------------------------------------------------------------------------------------
template<typename T>
class FrameGraphId;

class FrameGraphHandle {
    template<typename T>
    friend class FrameGraphId;
    friend class FrameGraph;
    friend class FrameGraphPassResources;
    friend struct fg::PassNode;
    friend struct fg::RenderTarget;
    friend struct fg::RenderTargetResource;

    // private ctor -- this cannot be constructed by users
    FrameGraphHandle() noexcept = default;
    explicit FrameGraphHandle(uint16_t index) noexcept : index(index) {}

    static constexpr uint16_t UNINITIALIZED = std::numeric_limits<uint16_t>::max();
    // index to the resource handle
    uint16_t index = UNINITIALIZED;

public:
    bool isValid() const noexcept { return index != UNINITIALIZED; }

    bool operator < (const FrameGraphHandle& rhs) const noexcept {
        return index < rhs.index;
    }

    bool operator == (const FrameGraphHandle& rhs) const noexcept {
        return (index == rhs.index);
    }

    bool operator != (const FrameGraphHandle& rhs) const noexcept {
        return !operator==(rhs);
    }
};

/*
 * A FrameGraph resource ID.
 *
 * This is used to represent a virtual resource.
 */

template<typename T>
class FrameGraphId : public FrameGraphHandle {
public:
    using FrameGraphHandle::FrameGraphHandle;
    FrameGraphId() noexcept = default;
    explicit FrameGraphId(FrameGraphHandle r) : FrameGraphHandle(r) { }
};

using FrameGraphRenderTargetHandle = uint16_t;

namespace FrameGraphRenderTarget {

struct Attachments {
    struct AttachmentInfo {
        // auto convert to FrameGraphHandle (allows: handle = desc.attachments.color;)
        operator FrameGraphId<FrameGraphTexture>() const noexcept { return mHandle; } // NOLINT

        AttachmentInfo() noexcept = default;

        // auto convert from FrameGraphHandle (allows: desc.attachments.color = handle;)
        AttachmentInfo(FrameGraphId<FrameGraphTexture> handle) noexcept : mHandle(handle) {} // NOLINT

        // allows: desc.attachments.color = { handle, level };
        AttachmentInfo(FrameGraphId<FrameGraphTexture> handle, uint8_t level) noexcept
                : mHandle(handle), mLevel(level) {}

        bool isValid() const noexcept { return mHandle.isValid(); }

        FrameGraphId<FrameGraphTexture> getHandle() const noexcept { return mHandle; }
        uint8_t getLevel() const noexcept { return mLevel; }

    private:
        FrameGraphId<FrameGraphTexture> mHandle{};
        uint8_t mLevel = 0;
    };

    constexpr Attachments() noexcept : textures{} {}
    Attachments(AttachmentInfo c, AttachmentInfo d) noexcept : color(c), depth(d) {}

    enum { COLOR = 0, DEPTH = 1 };
    static constexpr size_t COUNT = 2;
    union {
        std::array<AttachmentInfo, COUNT> textures = {};
        struct {
            AttachmentInfo color;
            AttachmentInfo depth;
        };
    };
};

struct Descriptor {
    Attachments attachments;
    Viewport viewport;
    uint8_t samples = 0; // # of samples (0 = unset, default)
};

} // namespace FrameGraphRenderTarget


} // namespace filament

#endif //TNT_FILAMENT_FRAMEGRAPHRESOURCE_H
