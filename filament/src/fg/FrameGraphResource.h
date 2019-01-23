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

#include <filament/driver/DriverEnums.h>

#include <limits>

#include <stdint.h>

namespace filament {

namespace fg {
struct PassNode;
struct RenderTarget;
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

    FrameGraphResource(uint16_t index, uint8_t version) noexcept
            : index(index), version(version) {}

    static constexpr uint16_t UNINITIALIZED = std::numeric_limits<uint16_t>::max();
    // index to the resource handle
    uint16_t index = UNINITIALIZED;
    // version of the resource when it was created. When this version doesn't match the
    // resource handle's version, this resource has become invalid.
    uint8_t version = 0;

public:
    FrameGraphResource() noexcept = default;

    struct Descriptor {
        uint32_t width = 1;     // width of resource in pixel
        uint32_t height = 1;    // height of resource in pixel
        uint32_t depth = 1;     // # of images for 3D textures
        uint8_t levels = 1;     // # of levels for textures
        driver::SamplerType type = driver::SamplerType::SAMPLER_2D;     // texture target type
        driver::TextureFormat format = driver::TextureFormat::RGBA8;    // resource internal format
    };

    bool isValid() const noexcept { return index != UNINITIALIZED; }

    bool operator < (const FrameGraphResource& rhs) const noexcept {
        return (index == rhs.index) ? (version < rhs.version) : (index < rhs.index);
    }

    bool operator == (const FrameGraphResource& rhs) const noexcept {
        return (index == rhs.index) && (version == rhs.version);
    }

    bool operator != (const FrameGraphResource& rhs) const noexcept {
        return !operator==(rhs);
    }
};


class FrameGraphRenderTarget {
    friend class FrameGraph;
    friend class FrameGraphPassResources;
    friend struct fg::PassNode;

    explicit FrameGraphRenderTarget(uint16_t index) noexcept : index(index) {}

    static constexpr uint16_t UNINITIALIZED = std::numeric_limits<uint16_t>::max();
    // index to the resource handle
    uint16_t index = UNINITIALIZED;

public:
    FrameGraphRenderTarget() noexcept = default;

    struct Descriptor {
        uint32_t width = 1;             // width of resource in pixel
        uint32_t height = 1;            // height of resource in pixel
        uint8_t samples = 1;            // # of samples
        struct {
            FrameGraphResource color;   // color attachment
            FrameGraphResource depth;   // depth attachment
        } attachments;
    };

    bool isValid() const noexcept { return index != UNINITIALIZED; }

    bool operator < (const FrameGraphRenderTarget& rhs) const noexcept {
        return (index < rhs.index);
    }

    bool operator == (const FrameGraphRenderTarget& rhs) const noexcept {
        return (index == rhs.index);
    }

    bool operator != (const FrameGraphRenderTarget& rhs) const noexcept {
        return !operator==(rhs);
    }
};


} // namespace filament

#endif //TNT_FILAMENT_FRAMEGRAPHRESOURCE_H
