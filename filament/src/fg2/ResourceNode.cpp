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

#include "fg2/FrameGraph.h"
#include "fg2/details/PassNode.h"
#include "fg2/details/ResourceNode.h"

namespace filament {

ResourceNode::ResourceNode(FrameGraph& fg, FrameGraphHandle h, FrameGraphHandle parent) noexcept
        : DependencyGraph::Node(fg.getGraph()),
          resourceHandle(h), mFrameGraph(fg), mReaderPasses(fg.getArena()), mParentHandle(parent) {
}

ResourceNode::~ResourceNode() noexcept {
    VirtualResource* resource = mFrameGraph.getResource(resourceHandle);
    assert_invariant(resource);
    resource->destroyEdge(mWriterPass);
    for (auto* pEdge : mReaderPasses) {
        resource->destroyEdge(pEdge);
    }
    delete mParentReadEdge;
    delete mParentWriteEdge;
    delete mForwardedEdge;
}

ResourceNode* ResourceNode::getParentNode() noexcept {
    return mParentHandle.isInitialized() ?
           mFrameGraph.getActiveResourceNode(mParentHandle) : nullptr;
}

char const* ResourceNode::getName() const noexcept {
    return mFrameGraph.getResource(resourceHandle)->name;
}

void ResourceNode::addOutgoingEdge(ResourceEdgeBase* edge) noexcept {
    mReaderPasses.push_back(edge);
}

void ResourceNode::setIncomingEdge(ResourceEdgeBase* edge) noexcept {
    assert_invariant(mWriterPass == nullptr);
    mWriterPass = edge;
}

bool ResourceNode::hasActiveReaders() const noexcept {
    // here we don't use mReaderPasses because this wouldn't account for subresources
    DependencyGraph& dependencyGraph = mFrameGraph.getGraph();
    auto const& readers = dependencyGraph.getOutgoingEdges(this);
    for (auto const& reader : readers) {
        if (!dependencyGraph.getNode(reader->to)->isCulled()) {
            return true;
        }
    }
    return false;
}

bool ResourceNode::hasActiveWriters() const noexcept {
    // here we don't use mReaderPasses because this wouldn't account for subresources
    DependencyGraph& dependencyGraph = mFrameGraph.getGraph();
    auto const& writers = dependencyGraph.getIncomingEdges(this);
    // writers are not culled by definition if we're not culled ourselves
    return !writers.empty();
}

ResourceEdgeBase* ResourceNode::getReaderEdgeForPass(PassNode const* node) const noexcept {
    auto pos = std::find_if(mReaderPasses.begin(), mReaderPasses.end(),
            [node](ResourceEdgeBase const* edge) {
                return edge->to == node->getId();
            });
    return pos != mReaderPasses.end() ? *pos : nullptr;
}

ResourceEdgeBase* ResourceNode::getWriterEdgeForPass(PassNode const* node) const noexcept {
    return mWriterPass && mWriterPass->from == node->getId() ? mWriterPass : nullptr;
}

bool ResourceNode::hasWriteFrom(PassNode const* node) const noexcept {
    return bool(getWriterEdgeForPass(node));
}


void ResourceNode::setParentReadDependency(ResourceNode* parent) noexcept {
    if (!mParentReadEdge) {
        mParentReadEdge = new DependencyGraph::Edge(mFrameGraph.getGraph(), parent, this);
    }
}


void ResourceNode::setParentWriteDependency(ResourceNode* parent) noexcept {
    if (!mParentWriteEdge) {
        mParentWriteEdge = new DependencyGraph::Edge(mFrameGraph.getGraph(), this, parent);
    }
}

void ResourceNode::setForwardResourceDependency(ResourceNode* source) noexcept {
    assert_invariant(!mForwardedEdge);
    mForwardedEdge = new DependencyGraph::Edge(mFrameGraph.getGraph(), this, source);
}


void ResourceNode::resolveResourceUsage(DependencyGraph& graph) noexcept {
    VirtualResource* pResource = mFrameGraph.getResource(resourceHandle);
    assert_invariant(pResource);
    if (pResource->refcount) {
        pResource->resolveUsage(graph, mReaderPasses.data(), mReaderPasses.size(), mWriterPass);
    }
}

utils::CString ResourceNode::graphvizify() const noexcept {
#ifndef NDEBUG
    std::string s;
    s.reserve(128);

    uint32_t id = getId();
    const char* const nodeName = getName();
    VirtualResource* const pResource = mFrameGraph.getResource(resourceHandle);
    FrameGraph::ResourceSlot const& slot = mFrameGraph.getResourceSlot(resourceHandle);

    s.append("[label=\"");
    s.append(nodeName);
    s.append("\\nrefs: ");
    s.append(std::to_string(pResource->refcount));
    s.append(", id: ");
    s.append(std::to_string(id));
    s.append("\\nversion: ");
    s.append(std::to_string(resourceHandle.version));
    s.append("/");
    s.append(std::to_string(slot.version));
    if (pResource->isImported()) {
        s.append(", imported");
    }
    s.append("\\nusage: ");
    s.append(pResource->usageString().c_str());
    s.append("\", ");

    s.append("style=filled, fillcolor=");
    s.append(pResource->refcount ? "skyblue" : "skyblue4");
    s.append("]");
    s.shrink_to_fit();

    return utils::CString{ s.c_str() };
#else
    return {};
#endif
}

utils::CString ResourceNode::graphvizifyEdgeColor() const noexcept {
    return utils::StaticString{ "darkolivegreen" };
}

} // namespace filament
