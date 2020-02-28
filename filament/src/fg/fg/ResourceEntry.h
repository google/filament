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

#ifndef TNT_FILAMENT_FG_RESOURCEENTRY_H
#define TNT_FILAMENT_FG_RESOURCEENTRY_H

#include "VirtualResource.h"

#include <stdint.h>

namespace filament {

class FrameGraph;

namespace fg {

struct PassNode;
class RenderTargetResourceEntry;

class ResourceEntryBase : public VirtualResource {
public:
    explicit ResourceEntryBase(const char* name, uint16_t id, bool imported, uint8_t priority) noexcept;
    ResourceEntryBase(ResourceEntryBase const&) = default;
    ~ResourceEntryBase() override;

    virtual RenderTargetResourceEntry* asRenderTargetResourceEntry() noexcept {
        return nullptr;
    }

    void preExecuteDestroy(FrameGraph& fg) noexcept override {
        discardEnd = true;
    }

    void postExecuteDevirtualize(FrameGraph& fg) noexcept override {
        discardStart = false;
    }

    // constants
    const char* const name;
    const uint16_t id;                      // for debugging and graphing
    const bool imported;
    const uint8_t priority;

    // updated by builder
    uint8_t version = 0;

    // computed during compile()
    uint32_t refs = 0;                      // final reference count

    // updated during execute()
    bool discardStart = true;
    bool discardEnd = false;
};


template<typename T>
class ResourceEntry : public ResourceEntryBase {
    T resource{};

public:
    using Descriptor = typename T::Descriptor;
    Descriptor descriptor;

    ResourceEntry(const char* name, Descriptor const& desc, uint16_t id, uint8_t priority) noexcept
            : ResourceEntryBase(name, id, false, priority), descriptor(desc) {
    }

    ResourceEntry(const char* name, Descriptor const& desc, const T& r, uint16_t id,
            uint8_t priority) noexcept
            : ResourceEntryBase(name, id, true, priority), resource(r), descriptor(desc) {
    }

    T const& getResource() const noexcept { return resource; }

    T& getResource() noexcept { return resource; }

    void resolve(FrameGraph& fg) noexcept override { }

    void preExecuteDevirtualize(FrameGraph& fg) noexcept override {
        if (!imported) {
            resource.create(fg, name, descriptor);
        }
    }

    void postExecuteDestroy(FrameGraph& fg) noexcept override {
        if (!imported) {
            resource.destroy(fg);
        }
    }
};

} // namespace fg
} // namespace filament

#endif //TNT_FILAMENT_FG_RESOURCEENTRY_H
