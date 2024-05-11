/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "fg/FrameGraph.h"
#include "fg/FrameGraphResources.h"
#include "fg/details/PassNode.h"
#include "fg/details/ResourceNode.h"

namespace filament {

FrameGraphResources::FrameGraphResources(FrameGraph& fg, PassNode& passNode) noexcept
    : mFrameGraph(fg), mPassNode(passNode) {
}

const char* FrameGraphResources::getPassName() const noexcept {
    return mPassNode.getName();
}

// this perhaps weirdly returns a reference, this is to express the fact that if this method
// fails, it has to assert (or throw), it can't return for e.g. a nullptr, because the public
// API doesn't return pointers.
// We still use ASSERT_PRECONDITION() because these failures are due to post conditions not met.
VirtualResource& FrameGraphResources::getResource(FrameGraphHandle handle) const {
    ASSERT_PRECONDITION(handle, "Uninitialized handle when using FrameGraphResources.");

    VirtualResource* const resource = mFrameGraph.getResource(handle);

    auto& declaredHandles = mPassNode.mDeclaredHandles;
    const bool hasReadOrWrite = declaredHandles.find(handle.index) != declaredHandles.cend();

    ASSERT_PRECONDITION(hasReadOrWrite,
            "Pass \"%s\" didn't declare any access to resource \"%s\"",
            mPassNode.getName(), resource->name);

    assert_invariant(resource->refcount);

    return *resource;
}

FrameGraphResources::RenderPassInfo FrameGraphResources::getRenderPassInfo(uint32_t id) const {
    // this cast is safe because this can only be called from a RenderPassNode
    RenderPassNode const& renderPassNode = static_cast<RenderPassNode const&>(mPassNode);
    RenderPassNode::RenderPassData const* pRenderPassData = renderPassNode.getRenderPassData(id);

    ASSERT_PRECONDITION(pRenderPassData,
            "using invalid RenderPass index %u in Pass \"%s\"",
            id, mPassNode.getName());

    return { pRenderPassData->backend.target, pRenderPassData->backend.params };
}

} // namespace filament
