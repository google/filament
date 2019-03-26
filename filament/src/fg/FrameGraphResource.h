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
#include <filament/Viewport.h>

#include <array>
#include <limits>

#include <stdint.h>

namespace filament {

namespace fg {
struct PassNode;
struct RenderTarget;
struct RenderTargetResource;
} // namespace fg

class FrameGraph;
class FrameGraphPassResources;

/*
 * A FrameGraph resource.
 *
 * This is used to represent a virtual resource.
 */

class FrameGraphResource {
    friend class FrameGraph;
    friend class FrameGraphPassResources;
    friend struct fg::PassNode;
    friend struct fg::RenderTarget;
    friend struct fg::RenderTargetResource;

    FrameGraphResource(uint16_t index) noexcept : index(index) {}

    static constexpr uint16_t UNINITIALIZED = std::numeric_limits<uint16_t>::max();
    // index to the resource handle
    uint16_t index = UNINITIALIZED;

public:
    FrameGraphResource() noexcept = default;

    struct Descriptor {
        uint32_t width = 1;     // width of resource in pixel
        uint32_t height = 1;    // height of resource in pixel
        uint32_t depth = 1;     // # of images for 3D textures
        uint8_t levels = 1;     // # of levels for textures
        backend::SamplerType type = backend::SamplerType::SAMPLER_2D;     // texture target type
        backend::TextureFormat format = backend::TextureFormat::RGBA8;    // resource internal format
        bool relaxed = false; // dimensions can be slightly adjusted
    };

    bool isValid() const noexcept { return index != UNINITIALIZED; }

    bool operator < (const FrameGraphResource& rhs) const noexcept {
        return index < rhs.index;
    }

    bool operator == (const FrameGraphResource& rhs) const noexcept {
        return (index == rhs.index);
    }

    bool operator != (const FrameGraphResource& rhs) const noexcept {
        return !operator==(rhs);
    }
};


namespace FrameGraphRenderTarget {

struct Attachments {
    enum { COLOR, DEPTH };
    static constexpr size_t COUNT = 2;
    union {
        std::array<FrameGraphResource, COUNT> textures = {};
        struct {
            FrameGraphResource color;
            FrameGraphResource depth;
        };
    };
};

struct Descriptor {
    Attachments attachments;
    Viewport viewport;
    uint8_t samples = 1;            // # of samples
};

} // namespace FrameGraphRenderTarget


} // namespace filament

#endif //TNT_FILAMENT_FRAMEGRAPHRESOURCE_H
