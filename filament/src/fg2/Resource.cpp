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

#include "fg2/details/Resource.h"

#include "fg2/details/PassNode.h"
#include "fg2/details/ResourceNode.h"

#include <utils/Panic.h>

namespace filament::fg2 {

VirtualResource::~VirtualResource() noexcept = default;

void VirtualResource::addOutgoingEdge(ResourceNode* node, ResourceEdgeBase* edge) noexcept {
    node->addOutgoingEdge(edge);
}

void VirtualResource::setIncomingEdge(ResourceNode* node, ResourceEdgeBase* edge) noexcept {
    node->setIncomingEdge(edge);
}

DependencyGraph::Node* VirtualResource::toDependencyGraphNode(ResourceNode* node) noexcept {
    // this can't go to the header file, because it would add a dependency on ResourceNode.h,
    // which we prefer to avoid
    return node;
}

DependencyGraph::Node* VirtualResource::toDependencyGraphNode(PassNode* node) noexcept {
    // this can't go to the header file, because it would add a dependency on PassNode.h
    // which we prefer to avoid
    return node;
}

ResourceEdgeBase* VirtualResource::getReaderEdgeForPass(
        ResourceNode* resourceNode, PassNode* passNode) noexcept {
    // this can't go to the header file, because it would add a dependency on PassNode.h
    // which we prefer to avoid
    return resourceNode->getReaderEdgeForPass(passNode);
}

ResourceEdgeBase* VirtualResource::getWriterEdgeForPass(
        ResourceNode* resourceNode, PassNode* passNode) noexcept {
    // this can't go to the header file, because it would add a dependency on PassNode.h
    // which we prefer to avoid
    return resourceNode->getWriterEdgeForPass(passNode);
}

void VirtualResource::neededByPass(PassNode* pNode) noexcept {
    refcount++;
    // figure out which is the first pass to need this resource
    first = first ? first : pNode;
    // figure out which is the last pass to need this resource
    last = pNode;

    // also extend the lifetime of our parent resource if any
    if (parent != this) {
        parent->neededByPass(pNode);
    }
}

// ------------------------------------------------------------------------------------------------

ImportedRenderTarget::~ImportedRenderTarget() noexcept = default;

ImportedRenderTarget::ImportedRenderTarget(char const* name,
        ImportedRenderTarget::Descriptor const& tdesc, FrameGraphRenderPass::Descriptor const& desc,
        backend::Handle<backend::HwRenderTarget> target)
        : ImportedResource<FrameGraphTexture>(name, tdesc, FrameGraphTexture::Usage::COLOR_ATTACHMENT, {}),
          target(target), rtdesc(desc) {
}

bool ImportedRenderTarget::connect(DependencyGraph& graph, PassNode* passNode,
        ResourceNode* resourceNode, backend::TextureUsage u) {
    // pass Node to resource Node edge (a write to)
    if (!ASSERT_PRECONDITION_NON_FATAL(!u || u == FrameGraphTexture::Usage::COLOR_ATTACHMENT,
            "Imported render target resource \"%s\" can only be used as a COLOR_ATTACHMENT", name)) {
        return false;
    }
    return Resource::connect(graph, passNode, resourceNode, u);
}

bool ImportedRenderTarget::connect(DependencyGraph& graph, ResourceNode* resourceNode,
        PassNode* passNode, backend::TextureUsage u) {
    // resource Node to pass Node edge (a read from)
    if (!ASSERT_PRECONDITION_NON_FATAL(!u || u == FrameGraphTexture::Usage::COLOR_ATTACHMENT,
            "Imported render target resource \"%s\" can only be used as a COLOR_ATTACHMENT", name)) {
        return false;
    }
    return Resource::connect(graph, resourceNode, passNode, u);
}

// ------------------------------------------------------------------------------------------------

template class Resource<FrameGraphTexture>;
template class ImportedResource<FrameGraphTexture>;

} // namespace filament::fg2
