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

#include "fg/details/ResourceNode.h"

#include "FrameGraphId.h"

#include "details/DependencyGraph.h"

#include "fg/details/PassNode.h"
#include "fg/FrameGraph.h"

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/debug.h>

#include <cstdint>
#include <new>

namespace filament {

ResourceNode::ResourceNode(FrameGraph& fg, FrameGraphHandle const h, FrameGraphHandle const parent) noexcept
        : Node(fg.getGraph()),
          resourceHandle(h), mFrameGraph(fg), mParentHandle(parent) {
}

ResourceNode::~ResourceNode() noexcept {
    VirtualResource* resource = mFrameGraph.getResource(resourceHandle);
    assert_invariant(resource);
    resource->destroyEdge(mWriterPass);
    for (ResourceEdgeBase* pEdge = mReaderPassesHead; pEdge; ) {
        ResourceEdgeBase* const next = pEdge->next;
        resource->destroyEdge(pEdge);
        pEdge = next;
    }
    delete mParentReadEdge;
    delete mParentWriteEdge;
    delete mForwardedEdge;
}

ResourceNode* ResourceNode::getParentNode() noexcept {
    ResourceNode* const parentNode = mParentHandle ?
            mFrameGraph.getActiveResourceNode(mParentHandle) : nullptr;
    assert_invariant(mParentHandle == ResourceNode::getHandle(parentNode));
    return parentNode;
}

ResourceNode* ResourceNode::getAncestorNode(ResourceNode* node) noexcept {
    ResourceNode* ancestor = node;
    do {
        node = node->getParentNode();
        ancestor = node ? node : ancestor;
    } while (node);
    return ancestor;
}

char const* ResourceNode::getName() const noexcept {
    return mFrameGraph.getResource(resourceHandle)->name.c_str();
}

void ResourceNode::addOutgoingEdge(ResourceEdgeBase* const edge) noexcept {
    edge->next = mReaderPassesHead;
    mReaderPassesHead = edge;
}

void ResourceNode::setIncomingEdge(ResourceEdgeBase* edge) noexcept {
    assert_invariant(mWriterPass == nullptr);
    mWriterPass = edge;
}

bool ResourceNode::hasActiveReaders() const noexcept {
    // here we don't use mReaderPasses because this wouldn't account for subresources
    DependencyGraph const& dependencyGraph = mFrameGraph.getGraph();
    return dependencyGraph.findOutgoingEdge(this, [&dependencyGraph](DependencyGraph::Edge const* reader) {
        return !dependencyGraph.getNode(reader->to)->isCulled();
    }) != nullptr;
}

bool ResourceNode::hasActiveWriters() const noexcept {
    // here we don't use mReaderPasses because this wouldn't account for subresources
    DependencyGraph const& dependencyGraph = mFrameGraph.getGraph();
    // writers are not culled by definition if we're not culled ourselves
    return dependencyGraph.findIncomingEdge(this, [](DependencyGraph::Edge const*) {
        return true;
    }) != nullptr;
}

ResourceEdgeBase* ResourceNode::getReaderEdgeForPass(PassNode const* node) const noexcept {
    DependencyGraph::NodeID const targetId = node->getId();
    for (ResourceEdgeBase* edge = mReaderPassesHead; edge; edge = edge->next) {
        if (edge->to == targetId) {
            return edge;
        }
    }
    return nullptr;
}

ResourceEdgeBase* ResourceNode::getWriterEdgeForPass(PassNode const* node) const noexcept {
    return mWriterPass && mWriterPass->from == node->getId() ? mWriterPass : nullptr;
}

bool ResourceNode::hasWriteFrom(PassNode const* node) const noexcept {
    return bool(getWriterEdgeForPass(node));
}


void ResourceNode::setParentReadDependency(ResourceNode* parent) noexcept {
    if (!mParentReadEdge) {
        mParentReadEdge = new(std::nothrow) DependencyGraph::Edge(mFrameGraph.getGraph(), parent, this);
    }
}


void ResourceNode::setParentWriteDependency(ResourceNode* parent) noexcept {
    if (!mParentWriteEdge) {
        mParentWriteEdge = new(std::nothrow) DependencyGraph::Edge(mFrameGraph.getGraph(), this, parent);
    }
}

void ResourceNode::setForwardResourceDependency(ResourceNode* source) noexcept {
    assert_invariant(!mForwardedEdge);
    mForwardedEdge = new(std::nothrow) DependencyGraph::Edge(mFrameGraph.getGraph(), this, source);
}


void ResourceNode::resolveResourceUsage(DependencyGraph& graph) noexcept {
    VirtualResource* pResource = mFrameGraph.getResource(resourceHandle);
    assert_invariant(pResource);
    if (pResource->refcount) {
        pResource->resolveUsage(graph, mReaderPassesHead, mWriterPass);
    }
}

utils::CString ResourceNode::graphvizify() const noexcept {
#ifndef NDEBUG
    utils::CString s;

    uint32_t const id = getId();
    const char* const nodeName = getName();
    VirtualResource const* const pResource = mFrameGraph.getResource(resourceHandle);
    FrameGraph::ResourceSlot const& slot = mFrameGraph.getResourceSlot(resourceHandle);

    s.append("[label=\"");
    s.append(nodeName);
    s.append("\\nrefs: ");
    s.append(utils::to_string(pResource->refcount));
    s.append(", id: ");
    s.append(utils::to_string(id));
    s.append("\\nversion: ");
    s.append(utils::to_string(resourceHandle.version));
    s.append("/");
    s.append(utils::to_string(slot.version));
    if (pResource->isImported()) {
        s.append(", imported");
    }
    s.append("\\nusage: ");
    s.append(pResource->usageString().c_str());
    s.append("\", ");

    s.append("style=filled, fillcolor=");
    s.append(pResource->refcount ? "skyblue" : "skyblue4");
    s.append("]");

    return s;
#else
    return {};
#endif
}

utils::CString ResourceNode::graphvizifyEdgeColor() const noexcept {
    return "darkolivegreen";
}

} // namespace filament
