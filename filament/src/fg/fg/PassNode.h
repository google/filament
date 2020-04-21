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

struct PassNode {
    template <typename T>
    using Vector = FrameGraph::Vector<T>;

    PassNode(FrameGraph& fg, const char* name, uint32_t id, FrameGraphPassExecutor* base) noexcept;
    PassNode(PassNode&& rhs) noexcept;
    ~PassNode();

    PassNode(PassNode const&) = delete;
    PassNode& operator=(PassNode const&) = delete;
    PassNode& operator=(PassNode&&) = delete;

    // for Builder
    FrameGraphHandle read(FrameGraph& fg, FrameGraphHandle handle);
    FrameGraphId<FrameGraphTexture> sample(FrameGraph& fg, FrameGraphId<FrameGraphTexture> handle);
    FrameGraphId<FrameGraphRenderTarget> use(FrameGraph& fg, FrameGraphId<FrameGraphRenderTarget> handle);
    FrameGraphHandle write(FrameGraph& fg, const FrameGraphHandle& handle);

    // constants
    const char* const name = nullptr;                       // our name
    const uint32_t id = 0;                                  // a unique id (only for debugging)
    FrameGraph::UniquePtr<FrameGraphPassExecutor> base;     // type eraser for calling execute()

    // set by the builder
    Vector<FrameGraphHandle> reads;                     // resources we're reading from
    Vector<FrameGraphHandle> writes;                    // resources we're writing to
    Vector<FrameGraphId<FrameGraphTexture>> samples;    // resources we're sampling from
    Vector<FrameGraphId<FrameGraphRenderTarget>> renderTargets;

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
