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

#ifndef TNT_FILAMENT_FG_PASSNODE_H
#define TNT_FILAMENT_FG_PASSNODE_H

#include "fg/FrameGraph.h"

#include <vector>

#include <stdint.h>

namespace filament {
namespace fg {

struct ResourceNode;
struct VirtualResource;

struct PassNode { // 200
    template <typename T>
    using Vector = FrameGraph::Vector<T>;

    PassNode(FrameGraph& fg, const char* name, uint32_t id, FrameGraphPassExecutor* base) noexcept
            : name(name), id(id), base(base, fg),
              reads(fg.getArena()),
              writes(fg.getArena()),
              samples(fg.getArena()),
              renderTargets(fg.getArena()),
              devirtualize(fg.getArena()),
              destroy(fg.getArena()) {
    }
    PassNode(PassNode const&) = delete;
    PassNode(PassNode&& rhs) noexcept = default;
    PassNode& operator=(PassNode const&) = delete;
    PassNode& operator=(PassNode&&) = delete;
    ~PassNode() = default;

    // for Builder
    void declareRenderTarget(fg::RenderTarget& renderTarget) noexcept {
        renderTargets.push_back(renderTarget.index);
    }

    FrameGraphHandle read(FrameGraph& fg, FrameGraphHandle handle) {
        // don't allow multiple reads of the same resource -- it's just redundant.
        auto pos = std::find_if(reads.begin(), reads.end(),
                [&handle](FrameGraphHandle cur) { return handle.index == cur.index; });
        if (pos == reads.end()) {
            // just record that we're reading from this resource (at the given version)
            reads.push_back(handle);
        }
        return handle;
    }

    FrameGraphId<FrameGraphTexture> sample(FrameGraph& fg, FrameGraphId<FrameGraphTexture> handle) {
        // sample implies a read
        read(fg, handle);

        // don't allow multiple reads of the same resource -- it's just redundant.
        auto pos = std::find_if(samples.begin(), samples.end(),
                [&handle](FrameGraphHandle cur) { return handle.index == cur.index; });
        if (pos == samples.end()) {
            // just record that we're reading from this resource (at the given version)
            samples.push_back(handle);
        }
        return handle;
    }

    bool isReadingFrom(FrameGraphHandle resource) const noexcept {
        auto pos = std::find_if(reads.begin(), reads.end(),
                [resource](FrameGraphHandle cur) { return resource.index == cur.index; });
        return (pos != reads.end());
    }

    bool isSamplingFrom(FrameGraphHandle resource) const noexcept {
        auto pos = std::find_if(samples.begin(), samples.end(),
                [resource](FrameGraphHandle cur) { return resource.index == cur.index; });
        return (pos != samples.end());
    }

    FrameGraphHandle write(FrameGraph& fg, const FrameGraphHandle& handle) {
        ResourceNode const& node = fg.getResourceNode(handle);

        // don't allow multiple writes of the same resource -- it's just redundant.
        auto pos = std::find_if(writes.begin(), writes.end(),
                [&handle](FrameGraphHandle cur) { return handle.index == cur.index; });
        if (pos != writes.end()) {
            return *pos;
        }

        /*
         * We invalidate and rename handles that are written into, to avoid undefined order
         * access to the resources.
         *
         * e.g. forbidden graphs
         *
         *         +-> [R1] -+
         *        /           \
         *  (A) -+             +-> (A)
         *        \           /
         *         +-> [R2] -+        // failure when setting R2 from (A)
         *
         */

        ++node.resource->version;

        // writing to an imported resource should count as a side-effect
        if (node.resource->imported) {
            hasSideEffect = true;
        }

        FrameGraphHandle r = fg.createResourceNode(node.resource);

        // record the write
        //FrameGraphHandle r{ resource->index, resource->version };
        writes.push_back(r);
        return r;
    }

    // constants
    const char* const name;                             // our name
    const uint32_t id;                                  // a unique id (only for debugging)
    FrameGraph::UniquePtr<FrameGraphPassExecutor> base; // type eraser for calling execute()

    // set by the builder
    Vector<FrameGraphHandle> reads;                     // resources we're reading from
    Vector<FrameGraphHandle> writes;                    // resources we're writing to
    Vector<FrameGraphId<FrameGraphTexture>> samples;    // resources we're sampling from
    Vector<uint16_t> renderTargets;

    // computed during compile()
    Vector<VirtualResource*> devirtualize;         // resources we need to create before executing
    Vector<VirtualResource*> destroy;              // resources we need to destroy after executing
    uint32_t refCount = 0;                  // count resources that have a reference to us

    // set by the builder
    bool hasSideEffect = false;             // whether this pass has side effects
};

} // namespace fg
} // namespace filament

#endif //TNT_FILAMENT_FG_PASSNODE_H
