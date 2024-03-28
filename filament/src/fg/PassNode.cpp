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

#include "fg/details/PassNode.h"

#include "fg/FrameGraph.h"
#include "fg/details/ResourceNode.h"

#include "ResourceAllocator.h"

#include <details/Texture.h>

#include <string>

using namespace filament::backend;

namespace filament {

PassNode::PassNode(FrameGraph& fg) noexcept
        : DependencyGraph::Node(fg.getGraph()),
          mFrameGraph(fg),
          devirtualize(fg.getArena()),
          destroy(fg.getArena()) {
}

PassNode::PassNode(PassNode&& rhs) noexcept = default;

PassNode::~PassNode() noexcept = default;

utils::CString PassNode::graphvizifyEdgeColor() const noexcept {
    return utils::CString{"red"};
}

void PassNode::registerResource(FrameGraphHandle resourceHandle) noexcept {
    VirtualResource* resource = mFrameGraph.getResource(resourceHandle);
    resource->neededByPass(this);
    mDeclaredHandles.insert(resourceHandle.index);
}

// ------------------------------------------------------------------------------------------------

RenderPassNode::RenderPassNode(FrameGraph& fg, const char* name, FrameGraphPassBase* base) noexcept
        : PassNode(fg), mName(name), mPassBase(base, fg.getArena()) {
}
RenderPassNode::RenderPassNode(RenderPassNode&& rhs) noexcept = default;
RenderPassNode::~RenderPassNode() noexcept = default;

void RenderPassNode::execute(FrameGraphResources const& resources, DriverApi& driver) noexcept {

    FrameGraph& fg = mFrameGraph;
    ResourceAllocatorInterface& resourceAllocator = fg.getResourceAllocator();

    // create the render targets
    for (auto& rt : mRenderTargetData) {
        rt.devirtualize(fg, resourceAllocator);
    }

    mPassBase->execute(resources, driver);
    
    // destroy the render targets
    for (auto& rt : mRenderTargetData) {
        rt.destroy(resourceAllocator);
    }
}

uint32_t RenderPassNode::declareRenderTarget(FrameGraph& fg, FrameGraph::Builder&,
        const char* name, FrameGraphRenderPass::Descriptor const& descriptor) {

    RenderPassData data;
    data.name = name;
    data.descriptor = descriptor;

    // retrieve the ResourceNode of the attachments coming to us -- this will be used later
    // to compute the discard flags.

    DependencyGraph const& dependencyGraph = fg.getGraph();
    auto incomingEdges = dependencyGraph.getIncomingEdges(this);

    for (size_t i = 0; i < RenderPassData::ATTACHMENT_COUNT; i++) {
        FrameGraphId<FrameGraphTexture> const& handle =
                data.descriptor.attachments.array[i];
        if (handle) {
            data.attachmentInfo[i] = handle;

            // TODO: this is not very efficient
            auto incomingPos = std::find_if(incomingEdges.begin(), incomingEdges.end(),
                    [&dependencyGraph, handle]
                            (DependencyGraph::Edge const* edge) {
                        ResourceNode const* node = static_cast<ResourceNode const*>(
                                dependencyGraph.getNode(edge->from));
                        return node->resourceHandle == handle;
                    });

            if (incomingPos != incomingEdges.end()) {
                data.incoming[i] = const_cast<ResourceNode*>(
                        static_cast<ResourceNode const*>(
                                dependencyGraph.getNode((*incomingPos)->from)));
            }

            // this could be either outgoing or incoming (if there are no outgoing)
            data.outgoing[i] = fg.getActiveResourceNode(handle);
            if (data.outgoing[i] == data.incoming[i]) {
                data.outgoing[i] = nullptr;
            }
        }
    }

    uint32_t const id = mRenderTargetData.size();
    mRenderTargetData.push_back(data);
    return id;
}

void RenderPassNode::resolve() noexcept {
    using namespace backend;

    for (auto& rt : mRenderTargetData) {

        uint32_t minWidth = std::numeric_limits<uint32_t>::max();
        uint32_t minHeight = std::numeric_limits<uint32_t>::max();
        uint32_t maxWidth = 0;
        uint32_t maxHeight = 0;

        /*
         * Compute discard flags
         */

        ImportedRenderTarget* pImportedRenderTarget = nullptr;
        rt.backend.params.flags.discardStart    = TargetBufferFlags::NONE;
        rt.backend.params.flags.discardEnd      = TargetBufferFlags::NONE;
        rt.backend.params.readOnlyDepthStencil  = 0;

        constexpr size_t DEPTH_INDEX = MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 0;
        constexpr size_t STENCIL_INDEX = MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 1;

        for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 2; i++) {
            if (rt.descriptor.attachments.array[i]) {
                const TargetBufferFlags target = getTargetBufferFlagsAt(i);

                rt.targetBufferFlags |= target;

                // Discard at the end only if we are writing to this attachment AND later reading
                // from it. (in particular, don't discard if we're not writing at all, because this
                // attachment might have other readers after us).
                // TODO: we could set the discard flag if we are the last reader, i.e.
                //       if rt->incoming[i] last reader is us.
                if (rt.outgoing[i] && !rt.outgoing[i]->hasActiveReaders()) {
                    rt.backend.params.flags.discardEnd |= target;
                }
                if (!rt.outgoing[i] || !rt.outgoing[i]->hasWriterPass()) {
                    if (i == DEPTH_INDEX) {
                        rt.backend.params.readOnlyDepthStencil |= RenderPassParams::READONLY_DEPTH;
                    } else if (i == STENCIL_INDEX) {
                        rt.backend.params.readOnlyDepthStencil |= RenderPassParams::READONLY_STENCIL;
                    }
                }
                // Discard at the start if this attachment has no prior writer
                if (!rt.incoming[i] || !rt.incoming[i]->hasActiveWriters()) {
                    rt.backend.params.flags.discardStart |= target;
                }
                VirtualResource* pResource = mFrameGraph.getResource(rt.descriptor.attachments.array[i]);
                Resource<FrameGraphTexture>* pTextureResource = static_cast<Resource<FrameGraphTexture>*>(pResource);

                pImportedRenderTarget = pImportedRenderTarget ?
                        pImportedRenderTarget : pResource->asImportedRenderTarget();

                // update attachment sample count if not specified and usage permits it
                if (!rt.descriptor.samples &&
                    none(pTextureResource->usage & TextureUsage::SAMPLEABLE)) {
                    pTextureResource->descriptor.samples = rt.descriptor.samples;
                }

                // figure out the min/max dimensions across all attachments
                const uint32_t w = pTextureResource->descriptor.width;
                const uint32_t h = pTextureResource->descriptor.height;
                minWidth = std::min(minWidth, w);
                maxWidth = std::max(maxWidth, w);
                minHeight = std::min(minHeight, h);
                maxHeight = std::max(maxHeight, h);
            }
            // additionally, clear implies discardStart
            rt.backend.params.flags.discardStart |= (
                    rt.descriptor.clearFlags & rt.targetBufferFlags);
        }

        assert_invariant(minWidth == maxWidth);
        assert_invariant(minHeight == maxHeight);
        assert_invariant(any(rt.targetBufferFlags));

        // of all attachments size matches there are no ambiguity about the RT size.
        // if they don't match however, we select a size that will accommodate all attachments.
        uint32_t const width = maxWidth;
        uint32_t const height = maxHeight;

        // Update the descriptor if no size was specified (auto mode)
        if (!rt.descriptor.viewport.width) {
            rt.descriptor.viewport.width = width;
        }
        if (!rt.descriptor.viewport.height) {
            rt.descriptor.viewport.height = height;
        }

        /*
         * Handle the special imported render target
         * To do this we check the first color attachment for an ImportedRenderTarget
         * and we override the parameters we just calculated
         */

        if (pImportedRenderTarget) {
            rt.imported = true;

            // override the values we just calculated with the actual values from the imported target
            rt.targetBufferFlags     = pImportedRenderTarget->importedDesc.attachments;
            rt.descriptor.viewport   = pImportedRenderTarget->importedDesc.viewport;
            rt.descriptor.clearColor = pImportedRenderTarget->importedDesc.clearColor;
            rt.descriptor.clearFlags = pImportedRenderTarget->importedDesc.clearFlags;
            rt.descriptor.samples    = pImportedRenderTarget->importedDesc.samples;
            rt.backend.target        = pImportedRenderTarget->target;

            // We could end-up here more than once, for instance if the rendertarget is used
            // by multiple passes (this would imply a read-back, btw). In this case, we don't want
            // to clear it the 2nd time, so we clear the imported pass's clear flags.
            pImportedRenderTarget->importedDesc.clearFlags = TargetBufferFlags::NONE;

            // but don't discard attachments the imported target tells us to keep
            rt.backend.params.flags.discardStart &= ~pImportedRenderTarget->importedDesc.keepOverrideStart;
            rt.backend.params.flags.discardEnd   &= ~pImportedRenderTarget->importedDesc.keepOverrideEnd;
        }

        rt.backend.params.viewport = rt.descriptor.viewport;
        rt.backend.params.clearColor = rt.descriptor.clearColor;
        rt.backend.params.flags.clear = rt.descriptor.clearFlags & rt.targetBufferFlags;
    }
}

void RenderPassNode::RenderPassData::devirtualize(FrameGraph& fg,
        ResourceAllocatorInterface& resourceAllocator) noexcept {
    assert_invariant(any(targetBufferFlags));
    if (UTILS_LIKELY(!imported)) {

        MRT colorInfo{};
        for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
            if (attachmentInfo[i]) {
                auto const* pResource = static_cast<Resource<FrameGraphTexture> const*>(
                        fg.getResource(attachmentInfo[i]));
                colorInfo[i].handle = pResource->resource.handle;
                colorInfo[i].level = pResource->subResourceDescriptor.level;
                colorInfo[i].layer = pResource->subResourceDescriptor.layer;
            }
        }

        TargetBufferInfo info[2] = {};
        for (size_t i = 0; i < 2; i++) {
            if (attachmentInfo[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + i]) {
                auto const* pResource = static_cast<Resource<FrameGraphTexture> const*>(
                        fg.getResource(attachmentInfo[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + i]));
                info[i].handle = pResource->resource.handle;
                info[i].level = pResource->subResourceDescriptor.level;
                info[i].layer = pResource->subResourceDescriptor.layer;
            }
        }

        backend.target = resourceAllocator.createRenderTarget(
                name, targetBufferFlags,
                backend.params.viewport.width,
                backend.params.viewport.height,
                descriptor.samples, descriptor.layerCount,
                colorInfo, info[0], info[1]);
    }
}

void RenderPassNode::RenderPassData::destroy(
        ResourceAllocatorInterface& resourceAllocator) const noexcept {
    if (UTILS_LIKELY(!imported)) {
        resourceAllocator.destroyRenderTarget(backend.target);
    }
}

RenderPassNode::RenderPassData const* RenderPassNode::getRenderPassData(uint32_t id) const noexcept {
    return id < mRenderTargetData.size() ? &mRenderTargetData[id] : nullptr;
}

utils::CString RenderPassNode::graphvizify() const noexcept {
#ifndef NDEBUG
    std::string s;

    uint32_t const id = getId();
    const char* const nodeName = getName();
    uint32_t const refCount = getRefCount();

    s.append("[label=\"");
    s.append(nodeName);
    s.append("\\nrefs: ");
    s.append(std::to_string(refCount));
    s.append(", id: ");
    s.append(std::to_string(id));

    for (auto const& rt :mRenderTargetData) {
        s.append("\\nS:");
        s.append(utils::to_string(rt.backend.params.flags.discardStart).c_str());
        s.append(", E:");
        s.append(utils::to_string(rt.backend.params.flags.discardEnd).c_str());
        s.append(", C:");
        s.append(utils::to_string(rt.backend.params.flags.clear).c_str());
    }

    s.append("\", ");

    s.append("style=filled, fillcolor=");
    s.append(refCount ? "darkorange" : "darkorange4");
    s.append("]");

    return utils::CString{ s.c_str() };
#else
    return {};
#endif
}

// ------------------------------------------------------------------------------------------------

PresentPassNode::PresentPassNode(FrameGraph& fg) noexcept
        : PassNode(fg) {
}
PresentPassNode::PresentPassNode(PresentPassNode&& rhs) noexcept = default;
PresentPassNode::~PresentPassNode() noexcept = default;

char const* PresentPassNode::getName() const noexcept {
    return "Present";
}

utils::CString PresentPassNode::graphvizify() const noexcept {
#ifndef NDEBUG
    std::string s;
    s.reserve(128);
    uint32_t const id = getId();
    s.append("[label=\"Present , id: ");
    s.append(std::to_string(id));
    s.append("\", style=filled, fillcolor=red3]");
    s.shrink_to_fit();
    return utils::CString{ s.c_str() };
#else
    return {};
#endif
}

void PresentPassNode::execute(FrameGraphResources const&, DriverApi&) noexcept {
}

void PresentPassNode::resolve() noexcept {
}

} // namespace filament
