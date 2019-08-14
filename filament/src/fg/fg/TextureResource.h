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

#ifndef TNT_FILAMENT_FG_RESOURCE_H
#define TNT_FILAMENT_FG_RESOURCE_H

#include "fg/FrameGraphPassResources.h"

#include "VirtualResource.h"

#include <backend/DriverEnums.h>

#include <stdint.h>

namespace filament {
namespace fg {

struct TextureResource final : public VirtualResource { // 72
    enum Type {
        TEXTURE
    };

    TextureResource(const char* name, uint16_t id,
            Type type, FrameGraphResource::Descriptor desc, bool imported) noexcept;
    TextureResource(TextureResource const&) = delete;
    TextureResource& operator=(TextureResource const&) = delete;
    ~TextureResource() noexcept override;

    // concrete resource -- set when the resource is created
    void create(FrameGraph& fg) noexcept override;
    void destroy(FrameGraph& fg) noexcept override;

    // constants
    const char* const name;
    const uint16_t id;            // for debugging and graphing
    const Type type;
    bool imported;

    // updated by builder
    uint8_t version = 0;
    backend::TextureUsage usage = (backend::TextureUsage)0;
    FrameGraphResource::Descriptor desc;

    // computed during compile()
    uint32_t refs = 0;                      // final reference count

    // set during execute(), as needed
    backend::Handle<backend::HwTexture> texture;
};

} // namespace fg
} // namespace filament

#endif //TNT_FILAMENT_FG_RESOURCE_H
