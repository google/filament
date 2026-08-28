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

#include "TextureCache.h"

#include "details/Texture.h"

#include "fg/details/ResourceNode.h"
#include "fg/FrameGraph.h"

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/debug.h>

using namespace filament::backend;

namespace filament {

PassNode::PassNode(FrameGraph& fg) noexcept
        : Node(fg.getGraph()),
          mFrameGraph(fg),
          devirtualize(fg.getArena()),
          destroy(fg.getArena()) {
    // We don't reserve devirtualize/destroy here because we could have a lot of PassNode (allocations are cheap)
}

PassNode::PassNode(PassNode&& rhs) noexcept = default;

PassNode::~PassNode() noexcept = default;

utils::CString PassNode::graphvizifyEdgeColor() const noexcept {
    return utils::CString{"red"};
}

void PassNode::registerResource(FrameGraphHandle const resourceHandle) noexcept {
    VirtualResource* resource = mFrameGraph.getResource(resourceHandle);
    resource->neededByPass(this);
    mDeclaredHandles.insert(resourceHandle.index);
}

// ------------------------------------------------------------------------------------------------

RenderPassNode::RenderPassNode(FrameGraph& fg, const char* name, FrameGraphPassBase* base) noexcept
    : PassNode(fg),
        mName(name),
        mPassBase(base, fg.getArena()),
        mRenderTargetData(fg.getArena()) {
    // We don't reserve mRenderTargetData here because we could have a lot of RenderPassNode (allocations are cheap)
    // and RenderPassData is 160 bytes.
}

RenderPassNode::RenderPassNode(RenderPassNode&& rhs) noexcept = default;

RenderPassNode::~RenderPassNode() noexcept = default;

void RenderPassNode::execute(FrameGraphResources const& resources, DriverApi& driver) noexcept {

    FrameGraph& fg = mFrameGraph;
    TextureCacheInterface& textureCache = fg.getTextureCache();

    // create the render targets
    for (auto& rt : mRenderTargetData) {
        rt.devirtualize(fg, textureCache);
    }

    mPassBase->execute(resources, driver);

    // destroy the render targets
    for (auto& rt : mRenderTargetData) {
        rt.destroy(textureCache);
    }
}

uint32_t RenderPassNode::declareRenderTarget(FrameGraph&, FrameGraph::Builder&,
        utils::StaticString name, FrameGraphRenderPass::Descriptor const& descriptor) {
    RenderPassData data;
    data.name = name;
    data.attachments = descriptor.attachments;
    data.samples = descriptor.samples;
    data.layerCount = descriptor.layerCount;
    data.clearFlags = descriptor.clearFlags;
    data.backend.params.viewport = descriptor.viewport;
    data.backend.params.clearColor = descriptor.clearColor;
    uint32_t const id = mRenderTargetData.size();
    mRenderTargetData.push_back(data);
    return id;
}

void RenderPassNode::resolve() noexcept {
    using namespace backend;

    DependencyGraph const& dependencyGraph = mFrameGraph.getGraph();

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
            if (rt.attachments[i]) {
                FrameGraphId<FrameGraphTexture> const handle = rt.attachments[i];

                DependencyGraph::Edge const* const incomingEdge = dependencyGraph.findIncomingEdge(this,
                        [&dependencyGraph, handle](DependencyGraph::Edge const* edge) {
                            auto const* node = static_cast<ResourceNode const*>(
                                    dependencyGraph.getNode(edge->from));
                            return node->resourceHandle == handle;
                        });
                ResourceNode const* const incomingNode = incomingEdge ?
                        static_cast<ResourceNode const*>(dependencyGraph.getNode(incomingEdge->from)) : nullptr;

                DependencyGraph::Edge const* const outgoingEdge = dependencyGraph.findOutgoingEdge(this,
                        [&dependencyGraph, handle](DependencyGraph::Edge const* edge) {
                            auto const* node = static_cast<ResourceNode const*>(
                                    dependencyGraph.getNode(edge->to));
                            return node->resourceHandle == handle;
                        });
                ResourceNode const* const outgoingNode = outgoingEdge ?
                        static_cast<ResourceNode const*>(dependencyGraph.getNode(outgoingEdge->to)) : nullptr;

                const TargetBufferFlags target = getTargetBufferFlagsAt(i);

                rt.targetBufferFlags |= target;

                // Discard at the end only if we are writing to this attachment AND later reading
                // from it. (in particular, don't discard if we're not writing at all, because this
                // attachment might have other readers after us).
                // TODO: we could set the discard flag if we are the last reader, i.e.
                //       if rt->incoming[i] last reader is us.
                if (outgoingNode && !outgoingNode->hasActiveReaders()) {
                    rt.backend.params.flags.discardEnd |= target;
                }
                if (!outgoingNode || !outgoingNode->hasWriterPass()) {
                    if (i == DEPTH_INDEX) {
                        rt.backend.params.readOnlyDepthStencil |= RenderPassParams::READONLY_DEPTH;
                    } else if (i == STENCIL_INDEX) {
                        rt.backend.params.readOnlyDepthStencil |= RenderPassParams::READONLY_STENCIL;
                    }
                }
                // Discard at the start if this attachment has no prior writer
                if (!incomingNode || !incomingNode->hasActiveWriters()) {
                    rt.backend.params.flags.discardStart |= target;
                }
                VirtualResource* pResource = mFrameGraph.getResource(rt.attachments[i]);
                Resource<FrameGraphTexture>* pTextureResource = static_cast<Resource<FrameGraphTexture>*>(pResource);

                pImportedRenderTarget = pImportedRenderTarget ?
                        pImportedRenderTarget : pResource->asImportedRenderTarget();

                // FIXME: the code below appears to either a NO-OP, or forcing the sample count to zero.
                //        Originally the code was
                //              if (!rt.descriptor.samples && none(...)) {
                //                  pTextureResource->descriptor.samples = rt.descriptor.samples;
                //              }
                //        which has a similar behavior. I don't recall what was the original intent of:
                //              "update attachment sample count if not specified and usage permits it"
                // update attachment sample count if not specified and usage permits it
                if (!rt.samples && none(pTextureResource->usage & TextureUsage::SAMPLEABLE)) {
                    pTextureResource->descriptor.samples = rt.samples;
                }

                // figure out the min/max dimensions across all attachments
                const uint32_t w = pTextureResource->descriptor.width;
                const uint32_t h = pTextureResource->descriptor.height;
                minWidth = std::min(minWidth, w);
                maxWidth = std::max(maxWidth, w);
                minHeight = std::min(minHeight, h);
                maxHeight = std::max(maxHeight, h);
            }
        }
        // additionally, clear implies discardStart
        rt.backend.params.flags.discardStart |= (
                rt.clearFlags & rt.targetBufferFlags);

        assert_invariant(minWidth == maxWidth);
        assert_invariant(minHeight == maxHeight);
        assert_invariant(any(rt.targetBufferFlags));

        // of all attachments size matches there are no ambiguity about the RT size.
        // if they don't match however, we select a size that will accommodate all attachments.
        uint32_t const width = maxWidth;
        uint32_t const height = maxHeight;

        // Update the viewport if no size was specified (auto mode)
        if (!rt.backend.params.viewport.width) {
            rt.backend.params.viewport.width = width;
        }
        if (!rt.backend.params.viewport.height) {
            rt.backend.params.viewport.height = height;
        }

        /*
         * Handle the special imported render target
         * To do this we check the first color attachment for an ImportedRenderTarget
         * and we override the parameters we just calculated
         */

        if (pImportedRenderTarget) {
            rt.imported = true;

            // override the values we just calculated with the actual values from the imported target
            rt.targetBufferFlags         = pImportedRenderTarget->importedDesc.attachments;
            rt.backend.params.viewport   = pImportedRenderTarget->importedDesc.viewport;
            rt.backend.params.clearColor = pImportedRenderTarget->importedDesc.clearColor;
            rt.clearFlags                = pImportedRenderTarget->importedDesc.clearFlags;
            rt.samples                   = pImportedRenderTarget->importedDesc.samples;
            rt.backend.target            = pImportedRenderTarget->target;

            // We could end-up here more than once, for instance if the rendertarget is used
            // by multiple passes (this would imply a read-back, btw). In this case, we don't want
            // to clear it the 2nd time, so we clear the imported pass's clear flags.
            pImportedRenderTarget->importedDesc.clearFlags = TargetBufferFlags::NONE;

            // but don't discard attachments the imported target tells us to keep
            rt.backend.params.flags.discardStart &= ~pImportedRenderTarget->importedDesc.keepOverrideStart;
            rt.backend.params.flags.discardEnd   &= ~pImportedRenderTarget->importedDesc.keepOverrideEnd;
        }

        rt.backend.params.flags.clear = rt.clearFlags & rt.targetBufferFlags;
    }
}

void RenderPassNode::RenderPassData::devirtualize(FrameGraph& fg,
        TextureCacheInterface& textureCache) noexcept {
    assert_invariant(any(targetBufferFlags));
    if (UTILS_LIKELY(!imported)) {

        MRT colorInfo{};
        for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
            if (attachments[i]) {
                auto const* pResource = static_cast<Resource<FrameGraphTexture> const*>(
                        fg.getResource(attachments[i]));
                colorInfo[i].handle = pResource->resource.handle;
                colorInfo[i].level = pResource->subResourceDescriptor.level;
                colorInfo[i].layer = pResource->subResourceDescriptor.layer;
            }
        }

        TargetBufferInfo info[2] = {};
        for (size_t i = 0; i < 2; i++) {
            size_t const index = MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + i;
            if (attachments[index]) {
                auto const* pResource = static_cast<Resource<FrameGraphTexture> const*>(
                        fg.getResource(attachments[index]));
                info[i].handle = pResource->resource.handle;
                info[i].level = pResource->subResourceDescriptor.level;
                info[i].layer = pResource->subResourceDescriptor.layer;
            }
        }

        backend.target = textureCache.createRenderTarget(
                name, targetBufferFlags,
                backend.params.viewport.width,
                backend.params.viewport.height,
                samples, layerCount,
                colorInfo, info[0], info[1]);
    }
}

void RenderPassNode::RenderPassData::destroy(
        TextureCacheInterface& textureCache) const noexcept {
    if (UTILS_LIKELY(!imported)) {
        textureCache.destroyRenderTarget(backend.target);
    }
}

RenderPassNode::RenderPassData const* RenderPassNode::getRenderPassData(uint32_t const id) const noexcept {
    return id < mRenderTargetData.size() ? &mRenderTargetData[id] : nullptr;
}

utils::CString RenderPassNode::graphvizify() const noexcept {
#ifndef NDEBUG
    utils::CString s;

    uint32_t const id = getId();
    const char* const nodeName = getName();
    uint32_t const refCount = getRefCount();

    s.append("[label=\"");
    s.append(nodeName);
    s.append("\\nrefs: ");
    s.append(utils::to_string(refCount));
    s.append(", id: ");
    s.append(utils::to_string(id));

    for (auto const& rt :mRenderTargetData) {
        s.append("\\nS:");
        s.append(utils::to_string(rt.backend.params.flags.discardStart));
        s.append(", E:");
        s.append(utils::to_string(rt.backend.params.flags.discardEnd));
        s.append(", C:");
        s.append(utils::to_string(rt.backend.params.flags.clear));
    }

    s.append("\", ");

    s.append("style=filled, fillcolor=");
    s.append(refCount ? "darkorange" : "darkorange4");
    s.append("]");

    return s;
#else
    return {};
#endif
}

#if FILAMENT_ENABLE_FGVIEWER
using RenderTargetInfo = fgviewer::FrameGraphInfo::Pass::RenderTargetInfo;
using AttachmentInfo = fgviewer::FrameGraphInfo::Pass::AttachmentInfo;
std::vector<RenderTargetInfo> RenderPassNode::getRenderTargetInfo() const noexcept {
    using namespace backend;
    std::vector<RenderTargetInfo> info;
    info.reserve(mRenderTargetData.size());

    for (auto const& rt: mRenderTargetData) {
        RenderTargetInfo rtInfo;

        auto extractAttachmentInfo = [&](TargetBufferFlags flags,
                                             std::vector<AttachmentInfo>& list) {
            for (size_t i = 0; i < RenderPassData::ATTACHMENT_COUNT; ++i) {
                TargetBufferFlags mask = getTargetBufferFlagsAt(i);
                if (any(flags & mask)) {
                    FrameGraphHandle handle = rt.attachments[i];
                    if (handle) {
                        const char* name = nullptr;
                        if (i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT) name = "color";
                        else if (i == MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT)
                            name = "depth";
                        else if (i == MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 1)
                            name = "stencil";

                        utils::CString slotName(name);
                        if (i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT) {
                            slotName += utils::to_string(i);
                        }

                        list.push_back({ slotName, handle.getIndex() });
                    }
                }
            }
        };

        extractAttachmentInfo(rt.backend.params.flags.discardStart, rtInfo.discardStart);
        extractAttachmentInfo(rt.backend.params.flags.discardEnd, rtInfo.discardEnd);
        extractAttachmentInfo(rt.backend.params.flags.clear, rtInfo.clear);

        info.push_back(std::move(rtInfo));
    }
    return info;
}
#endif

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
    utils::CString s;
    uint32_t const id = getId();
    s.append("[label=\"Present , id: ");
    s.append(utils::to_string(id));
    s.append("\", style=filled, fillcolor=red3]");
    return s;
#else
    return {};
#endif
}

void PresentPassNode::execute(FrameGraphResources const&, DriverApi&) noexcept {
}

void PresentPassNode::resolve() noexcept {
}

} // namespace filament
