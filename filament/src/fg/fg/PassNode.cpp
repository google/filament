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

#include "fg/fg/PassNode.h"
#include "fg/fg/ResourceNode.h"

namespace filament {
namespace fg {


PassNode::PassNode(FrameGraph& fg, const char* name, uint32_t id,
        FrameGraphPassExecutor* base) noexcept
        : name(name), id(id), base(base, fg),
          reads(fg.getArena()),
          writes(fg.getArena()),
          samples(fg.getArena()),
          renderTargets(fg.getArena()),
          devirtualize(fg.getArena()),
          destroy(fg.getArena()) {
}

PassNode::PassNode(PassNode&& rhs) noexcept = default;

PassNode::~PassNode() = default;

// for Builder
FrameGraphHandle PassNode::read(FrameGraph& fg, FrameGraphHandle handle) {
    // don't allow multiple reads of the same resource -- it's just redundant.
    auto pos = std::find_if(reads.begin(), reads.end(),
            [&handle](FrameGraphHandle cur) { return handle.index == cur.index; });
    if (pos == reads.end()) {
        // just record that we're reading from this resource (at the given version)
        reads.push_back(handle);
    }
    return handle;
}

FrameGraphId<FrameGraphTexture> PassNode::sample(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> handle) {
    // sample() implies a read
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

FrameGraphId<FrameGraphRenderTarget> PassNode::use(FrameGraph& fg,
        FrameGraphId<FrameGraphRenderTarget> handle) {
    // use() implies a read
    read(fg, handle);

    // don't allow multiple use() of the same FrameGraphRenderTarget -- it's just redundant.
    auto pos = std::find_if(renderTargets.begin(), renderTargets.end(),
            [&handle](FrameGraphHandle cur) { return handle.index == cur.index; });
    if (pos == renderTargets.end()) {
        // just record that we're reading from this resource (at the given version)
        renderTargets.push_back(handle);
    }
    return handle;
}

FrameGraphHandle PassNode::write(FrameGraph& fg, const FrameGraphHandle& handle) {
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
    auto& newNode = fg.getResourceNodeUnchecked(r);
    assert(!newNode.writer);
    newNode.writer = this;     // needed by move resources

    writes.push_back(r);
    return r;
}


} // namespace fg
} // namespace filament
