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
#include "fg2/details/DependencyGraph.h"

#include "details/Engine.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/Panic.h>

namespace filament::fg2 {

FrameGraph::Builder::Builder(FrameGraph& fg, PassNode* passNode) noexcept
        : mFrameGraph(fg), mPassNode(passNode) {
}

FrameGraph::Builder::~Builder() noexcept = default;

void FrameGraph::Builder::sideEffect() noexcept {
    mPassNode->makeTarget();
}

const char* FrameGraph::Builder::getName(FrameGraphHandle handle) const noexcept {
    return mFrameGraph.getResource(handle)->name;
}

uint32_t FrameGraph::Builder::declareRenderPass(const char* name,
        FrameGraphRenderPass::Descriptor const& desc) {
    // it's safe here to cast to RenderPassNode because we can't be here for a PresentPassNode
    // also only RenderPassNodes have the concept of render targets.
    return static_cast<RenderPassNode*>(mPassNode)->declareRenderTarget(mFrameGraph, *this, name, desc);
}

FrameGraphId<FrameGraphTexture> FrameGraph::Builder::declareRenderPass(
        FrameGraphId<FrameGraphTexture> color, uint32_t* index) {
    color = write(color);
    uint32_t id = declareRenderPass(getName(color),
            { .attachments = { .color = { color }}});
    if (index) *index = id;
    return color;
}

// ------------------------------------------------------------------------------------------------

FrameGraph::FrameGraph(ResourceAllocatorInterface& resourceAllocator)
        : mResourceAllocator(resourceAllocator),
          mArena("FrameGraph Arena", 131072),
          mResourceSlots(mArena),
          mResources(mArena),
          mResourceNodes(mArena),
          mPassNodes(mArena)
{
    mResourceSlots.reserve(256);
    mResources.reserve(256);
    mResourceNodes.reserve(256);
    mPassNodes.reserve(64);
}

FrameGraph::~FrameGraph() = default;

void FrameGraph::reset() noexcept {
    // the order of destruction is important here
    mPassNodes.clear();
    mResourceNodes.clear();
    mResources.clear();
    mResourceSlots.clear();
}

FrameGraph& FrameGraph::compile() noexcept {
    DependencyGraph& dependencyGraph = mGraph;

    // first we cull unreachable nodes
    dependencyGraph.cull();

    /*
     * update the reference counter of the resource themselves and
     * compute first/last users for active passes
     */

    mActivePassNodesEnd = std::stable_partition(
            mPassNodes.begin(), mPassNodes.end(), [](auto const& pPassNode) {
        return !pPassNode->isCulled();
    });

    auto first = mPassNodes.begin();
    const auto activePassNodesEnd = mActivePassNodesEnd;
    while (first != activePassNodesEnd) {
        PassNode* const passNode = first->get();
        first++;
        assert_invariant(!passNode->isCulled());


        auto const& reads = dependencyGraph.getIncomingEdges(passNode);
        for (auto const& edge : reads) {
            // all incoming edges should be valid by construction
            assert_invariant(dependencyGraph.isEdgeValid(edge));
            auto pNode = static_cast<ResourceNode*>(dependencyGraph.getNode(edge->from));
            passNode->registerResource(pNode->resourceHandle);
        }

        auto const& writes = dependencyGraph.getOutgoingEdges(passNode);
        for (auto const& edge : writes) {
            // an outgoing edge might be invalid if the node it points to has been culled
            // but, because we are not culled and we're a pass, we add a reference to
            // the resource we are writing to.
            auto pNode = static_cast<ResourceNode*>(dependencyGraph.getNode(edge->to));
            passNode->registerResource(pNode->resourceHandle);
        }

        passNode->resolve();
    }

    // add resource to de-virtualize or destroy to the corresponding list for each active pass
    for (auto& pResource : mResources) {
        VirtualResource* resource = pResource.get();
        if (resource->refcount) {
            PassNode* pFirst = resource->first;
            PassNode* pLast = resource->last;
            assert_invariant(!pFirst == !pLast);
            if (pFirst && pLast) {
                assert_invariant(!pFirst->isCulled());
                assert_invariant(!pLast->isCulled());
                pFirst->devirtualize.push_back(resource);
                pLast->destroy.push_back(resource);
            }
        }
    }

    /*
     * Resolve Usage bits
     */
    for (auto& pNode : mResourceNodes) {
        // we can't use isCulled() here because some culled resource are still active
        // we could use "getResource(pNode->resourceHandle)->refcount" but that's expensive.
        // We also can't remove or reorder this array, as handles are indices to it.
        // We might need to build an array of indices to active resources.
        pNode->resolveResourceUsage(dependencyGraph);
    }

    return *this;
}

void FrameGraph::execute(backend::DriverApi& driver) noexcept {
    auto const& passNodes = mPassNodes;
    auto& resourceAllocator = mResourceAllocator;

    driver.pushGroupMarker("FrameGraph");

    auto first = passNodes.begin();
    const auto activePassNodesEnd = mActivePassNodesEnd;
    while (first != activePassNodesEnd) {
        PassNode* const node = first->get();
        first++;
        assert_invariant(!node->isCulled());

        driver.pushGroupMarker(node->getName());

        // devirtualize resourcesList
        for (VirtualResource* resource : node->devirtualize) {
            assert_invariant(resource->first == node);
            resource->devirtualize(resourceAllocator);
        }

        // call execute
        FrameGraphResources resources(*this, *node);
        node->execute(resources, driver);

        // destroy concrete resources
        for (VirtualResource* resource : node->destroy) {
            assert_invariant(resource->last == node);
            resource->destroy(resourceAllocator);
        }

        driver.popGroupMarker();
    }
    // this is a good place to kick the GPU, since we've just done a bunch of work
    driver.flush();
    driver.popGroupMarker();
}

void FrameGraph::addPresentPass(std::function<void(FrameGraph::Builder&)> setup) noexcept {
    PresentPassNode* node = mArena.make<PresentPassNode>(*this);
    mPassNodes.emplace_back(node, mArena);
    Builder builder(*this, node);
    setup(builder);
    builder.sideEffect();
}

FrameGraph::Builder FrameGraph::addPassInternal(char const* name, FrameGraphPassBase* base) noexcept {
    // record in our pass list and create the builder
    PassNode* node = mArena.make<RenderPassNode>(*this, name, base);
    base->setNode(node);
    mPassNodes.emplace_back(node, mArena);
    return Builder(*this, node);
}

FrameGraphHandle FrameGraph::createNewVersion(FrameGraphHandle handle, FrameGraphHandle parent) noexcept {
    ResourceSlot& slot = getResourceSlot(handle);
    slot.version = ++handle.version;   // increase the parent's version
    slot.nid = mResourceNodes.size();   // create the new parent node
    ResourceNode* newNode = mArena.make<ResourceNode>(*this, handle, parent);
    mResourceNodes.emplace_back(newNode, mArena);
    return handle;
}

FrameGraphHandle FrameGraph::createNewVersionForSubresourceIfNeeded(FrameGraphHandle handle) noexcept {
    ResourceSlot& slot = getResourceSlot(handle);
    if (slot.sid < 0) {
        // if we don't already have a new ResourceNode for this resource, create one.
        // we keep the old ResourceNode index so we can direct all the reads to it.
        slot.sid = slot.nid; // record the current ResourceNode of the parent
        slot.nid = mResourceNodes.size();   // create the new parent node
        ResourceNode* newNode = mArena.make<ResourceNode>(*this, handle, FrameGraphHandle{});
        mResourceNodes.emplace_back(newNode, mArena);
    }
    return handle;
}

FrameGraphHandle FrameGraph::addResourceInternal(UniquePtr<VirtualResource> resource) noexcept {
    return addSubResourceInternal(FrameGraphHandle{}, std::move(resource));
}

FrameGraphHandle FrameGraph::addSubResourceInternal(FrameGraphHandle parent,
        UniquePtr<VirtualResource> resource) noexcept {
    FrameGraphHandle handle(mResourceSlots.size());
    ResourceSlot& slot = mResourceSlots.emplace_back();
    slot.rid = mResources.size();
    slot.nid = mResourceNodes.size();
    mResources.push_back(std::move(resource));
    ResourceNode* pNode = mArena.make<ResourceNode>(*this, handle, parent);
    mResourceNodes.emplace_back(pNode, mArena);
    return handle;
}

FrameGraphHandle FrameGraph::readInternal(FrameGraphHandle handle, PassNode* passNode,
        std::function<bool(ResourceNode*, VirtualResource*)> connect) {

    if (!assertValid(handle)) {
        return {};
    }

    VirtualResource* const resource = getResource(handle);
    ResourceNode* const node = getActiveResourceNode(handle);

    // Check preconditions
    bool passAlreadyAWriter = node->hasWriteFrom(passNode);
    if (!ASSERT_PRECONDITION_NON_FATAL(!passAlreadyAWriter,
            "Pass \"%s\" already writes to \"%s\"",
            passNode->getName(), node->getName())) {
        return {};
    }

    if (!node->hasWriterPass() && !resource->isImported()) {
        // TODO: we're attempting to read from a resource that was never written and is not
        //       imported either, so it can't have valid data in it.
        //       Should this be an error?
    }

    // Connect can fail if usage flags are incorrectly used
    if (connect(node, resource)) {
        if (resource->isSubResource()) {
            // this is read() to a subresource, so we need to add a "read" from the the parent's
            // node to the subresource -- but we may have two parent nodes, one for reads and
            // one for writes, so we need to use the one for reads.
            auto* parentNode = node->getParentNode();
            ResourceSlot& slot = getResourceSlot(parentNode->resourceHandle);
            if (slot.sid >= 0) {
                // we have a parent's node for reads, use the one
                parentNode = mResourceNodes[slot.sid].get();
            }
            node->setParentReadDependency(parentNode);
        }

        // if a resource has a subresource, then its handle becomes valid again as soon as it's used.
        ResourceSlot& slot = getResourceSlot(handle);
        if (slot.sid >= 0) {
            // we can now forget the "read" parent node, which becomes the current one again
            // until the next write.
            slot.sid = -1;
        }

        return handle;
    }

    return {};
}

FrameGraphHandle FrameGraph::writeInternal(FrameGraphHandle handle, PassNode* passNode,
        std::function<bool(ResourceNode*, VirtualResource*)> connect) {
    if (!assertValid(handle)) {
        return {};
    }

    VirtualResource* const resource = getResource(handle);
    ResourceNode* node = getActiveResourceNode(handle);
    ResourceNode* parentNode = node->getParentNode();

    // if we're writing into a subresource, we also need to add a "write" from the subresource
    // node to a new version of the parent's node, if we don't already have one.
    if (resource->isSubResource()) {
        assert_invariant(parentNode);
        // FIXME: do we need the equivalent of hasWriterPass() test below
        createNewVersionForSubresourceIfNeeded(parentNode->resourceHandle);
        parentNode = node->getParentNode();
    }

    // if this node already writes to this resource, just update the used bits
    if (!node->hasWriteFrom(passNode)) {
        if (!node->hasWriterPass() && !node->hasReaders()) {
            // FIXME: should this also take subresource writes into account
            // if we don't already have a writer or a reader, it just means the resource was just created
            // and was never written to, so we don't need a new node or increase the version number
        } else {
            handle = createNewVersion(handle,
                    parentNode ? parentNode->resourceHandle : FrameGraphHandle{});
            // refresh the node
            node = getActiveResourceNode(handle);
        }
    }

    if (connect(node, resource)) {
        if (resource->isSubResource()) {
            node->setParentWriteDependency(parentNode);
        }
        if (resource->isImported()) {
            // writing to an imported resource implies a side-effect
            passNode->makeTarget();
        }
        return handle;
    } else {
        // FIXME: we need to undo everything we did to this point
    }

    return {};
}

FrameGraphHandle FrameGraph::forwardResourceInternal(FrameGraphHandle resourceHandle,
        FrameGraphHandle replaceResourceHandle) {

    if (!assertValid(resourceHandle)) {
        return {};
    }

    if (!assertValid(replaceResourceHandle)) {
        return {};
    }

    getActiveResourceNode(replaceResourceHandle)->setForwardResourceDependency(
            getActiveResourceNode(resourceHandle));

    ResourceSlot const& resourceSlot = getResourceSlot(resourceHandle);
    ResourceSlot& replacedResourceSlot = getResourceSlot(replaceResourceHandle);

    replacedResourceSlot.rid = resourceSlot.rid;
    // FIXME: what should happen with .sid and .nid?

    // makes the replaceResourceHandle forever invalid
    replacedResourceSlot.version = -1;

    return resourceHandle;
}

FrameGraphId<FrameGraphTexture> FrameGraph::import(char const* name, FrameGraphRenderPass::Descriptor const& desc,
        backend::Handle<backend::HwRenderTarget> target) {
    UniquePtr<VirtualResource> vresource(
            mArena.make<ImportedRenderTarget>(name,FrameGraphTexture::Descriptor{
                            .width = desc.viewport.width,
                            .height = desc.viewport.height,
                    }, desc, target), mArena);
    return FrameGraphId<FrameGraphTexture>(addResourceInternal(std::move(vresource)));
}

bool FrameGraph::isValid(FrameGraphHandle handle) const {
    // Code below is written this way so we can set breakpoints easily.
    if (!handle.isInitialized()) {
        return false;
    }
    ResourceSlot slot = getResourceSlot(handle);
    if (handle.version != slot.version) {
        return false;
    }
    return true;
}

bool FrameGraph::assertValid(FrameGraphHandle handle) const {
    return ASSERT_PRECONDITION_NON_FATAL(isValid(handle),
            "Resource handle is invalid or uninitialized {id=%u, version=%u}",
            (int)handle.index, (int)handle.version);
}

bool FrameGraph::isCulled(FrameGraphPassBase const& pass) const noexcept {
    return pass.getNode().isCulled();
}

bool FrameGraph::isAcyclic() const noexcept {
    return mGraph.isAcyclic();
}

void FrameGraph::export_graphviz(utils::io::ostream& out, char const* name) {
    mGraph.export_graphviz(out, name);
}

// ------------------------------------------------------------------------------------------------

/*
 * Explicit template instantiation for for FrameGraphTexture which is a known type,
 * to reduce compile time and code size.
 */

template void FrameGraph::present(FrameGraphId<FrameGraphTexture> input);

template FrameGraphId<FrameGraphTexture> FrameGraph::create(char const* name,
        FrameGraphTexture::Descriptor const& desc) noexcept;

template FrameGraphId<FrameGraphTexture> FrameGraph::createSubresource(FrameGraphId<FrameGraphTexture> parent,
        char const* name, FrameGraphTexture::SubResourceDescriptor const& desc) noexcept;

template FrameGraphId<FrameGraphTexture> FrameGraph::import(char const* name,
        FrameGraphTexture::Descriptor const& desc, FrameGraphTexture::Usage usage, FrameGraphTexture const& resource) noexcept;

template FrameGraphId<FrameGraphTexture> FrameGraph::read(PassNode* passNode,
        FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Usage usage);

template FrameGraphId<FrameGraphTexture> FrameGraph::write(PassNode* passNode,
        FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Usage usage);

template FrameGraphId<FrameGraphTexture> FrameGraph::forwardResource(
        FrameGraphId<FrameGraphTexture> resource, FrameGraphId<FrameGraphTexture> replacedResource);

} // namespace filament::fg2
