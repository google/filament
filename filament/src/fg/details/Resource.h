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

#ifndef TNT_FILAMENT_FG_DETAILS_RESOURCE_H
#define TNT_FILAMENT_FG_DETAILS_RESOURCE_H

#include "fg/FrameGraphId.h"
#include "fg/FrameGraphTexture.h"
#include "fg/FrameGraphRenderPass.h"
#include "fg/details/DependencyGraph.h"

#include <utils/Panic.h>

namespace filament {
class ResourceAllocatorInterface;
} // namespace::filament

namespace filament {

class PassNode;
class ResourceNode;
class ImportedRenderTarget;

/*
 * ResourceEdgeBase only exists to enforce type safety
 */
class ResourceEdgeBase : public DependencyGraph::Edge {
public:
    using DependencyGraph::Edge::Edge;
};

/*
 * The generic parts of virtual resources.
 */
class VirtualResource {
public:
    // constants
    VirtualResource* parent;
    const char* const name;

    // computed during compile()
    uint32_t refcount = 0;
    PassNode* first = nullptr;  // pass that needs to instantiate the resource
    PassNode* last = nullptr;   // pass that can destroy the resource

    explicit VirtualResource(const char* name) noexcept : parent(this), name(name) { }
    VirtualResource(VirtualResource* parent, const char* name) noexcept : parent(parent), name(name) { }
    VirtualResource(VirtualResource const& rhs) noexcept = delete;
    VirtualResource& operator=(VirtualResource const&) = delete;
    virtual ~VirtualResource() noexcept;

    // updates first/last/refcount
    void neededByPass(PassNode* pNode) noexcept;

    bool isSubResource() const noexcept { return parent != this; }

    VirtualResource* getResource() noexcept {
        VirtualResource* p = this;
        while (p->parent != p) {
            p = p->parent;
        }
        return p;
    }

    /*
     * Called during FrameGraph::compile(), this gives an opportunity for this resource to
     * calculate its effective usage flags.
     */
    virtual void resolveUsage(DependencyGraph& graph,
            ResourceEdgeBase const* const* edges, size_t count,
            ResourceEdgeBase const* writer) noexcept = 0;

    /* Instantiate the concrete resource */
    virtual void devirtualize(ResourceAllocatorInterface& resourceAllocator,
            bool useProtectedMemory) noexcept = 0;

    /* Destroy the concrete resource */
    virtual void destroy(ResourceAllocatorInterface& resourceAllocator) noexcept = 0;

    /* Destroy an Edge instantiated by this resource */
    virtual void destroyEdge(DependencyGraph::Edge* edge) noexcept = 0;

    virtual utils::CString usageString() const noexcept = 0;

    virtual bool isImported() const noexcept { return false; }

    // this is to workaround our lack of RTTI -- otherwise we could use dynamic_cast
    virtual ImportedRenderTarget* asImportedRenderTarget() noexcept { return nullptr; }

protected:
    void addOutgoingEdge(ResourceNode* node, ResourceEdgeBase* edge) noexcept;
    void setIncomingEdge(ResourceNode* node, ResourceEdgeBase* edge) noexcept;
    // these exist only so we don't have to include PassNode.h or ResourceNode.h
    static DependencyGraph::Node* toDependencyGraphNode(ResourceNode* node) noexcept;
    static DependencyGraph::Node* toDependencyGraphNode(PassNode* node) noexcept;
    static ResourceEdgeBase* getReaderEdgeForPass(ResourceNode* resourceNode, PassNode* passNode) noexcept;
    static ResourceEdgeBase* getWriterEdgeForPass(ResourceNode* resourceNode, PassNode* passNode) noexcept;
};

// ------------------------------------------------------------------------------------------------

/*
 * Resource specific parts of a VirtualResource
 */
template<typename RESOURCE>
class Resource : public VirtualResource {
    using Usage = typename RESOURCE::Usage;

public:
    using Descriptor = typename RESOURCE::Descriptor;
    using SubResourceDescriptor = typename RESOURCE::SubResourceDescriptor;

    // valid only after devirtualize() has been called
    RESOURCE resource{};

    // valid only after resolveUsage() has been called
    Usage usage{};

    // our concrete (sub)resource descriptors -- used to create it.
    Descriptor descriptor;
    SubResourceDescriptor subResourceDescriptor;

    // whether the resource was detached
    bool detached = false;

    // An Edge with added data from this resource
    class UTILS_PUBLIC ResourceEdge : public ResourceEdgeBase {
    public:
        Usage usage;
        ResourceEdge(DependencyGraph& graph,
                DependencyGraph::Node* from, DependencyGraph::Node* to, Usage usage) noexcept
                : ResourceEdgeBase(graph, from, to), usage(usage) {
        }
    };

    UTILS_NOINLINE
    Resource(const char* name, Descriptor const& desc) noexcept
        : VirtualResource(name), descriptor(desc) {
    }

    UTILS_NOINLINE
    Resource(Resource* parent, const char* name, SubResourceDescriptor const& desc) noexcept
            : VirtualResource(parent, name),
              descriptor(RESOURCE::generateSubResourceDescriptor(parent->descriptor, desc)),
              subResourceDescriptor(desc) {
    }

    ~Resource() noexcept = default;

    // pass Node to resource Node edge (a write to)
    UTILS_NOINLINE
    virtual bool connect(DependencyGraph& graph,
            PassNode* passNode, ResourceNode* resourceNode, Usage u) {
        // TODO: we should check that usage flags are correct (e.g. a write flag is not used for reading)
        ResourceEdge* edge = static_cast<ResourceEdge*>(getWriterEdgeForPass(resourceNode, passNode));
        if (edge) {
            edge->usage |= u;
        } else {
            edge = new ResourceEdge(graph,
                    toDependencyGraphNode(passNode), toDependencyGraphNode(resourceNode), u);
            setIncomingEdge(resourceNode, edge);
        }
        return true;
    }

    // resource Node to pass Node edge (a read from)
    UTILS_NOINLINE
    virtual bool connect(DependencyGraph& graph,
            ResourceNode* resourceNode, PassNode* passNode, Usage u) {
        // TODO: we should check that usage flags are correct (e.g. a write flag is not used for reading)
        // if passNode is already a reader of resourceNode, then just update the usage flags
        ResourceEdge* edge = static_cast<ResourceEdge*>(getReaderEdgeForPass(resourceNode, passNode));
        if (edge) {
            edge->usage |= u;
        } else {
            edge = new ResourceEdge(graph,
                    toDependencyGraphNode(resourceNode), toDependencyGraphNode(passNode), u);
            addOutgoingEdge(resourceNode, edge);
        }
        return true;
    }

protected:
    /*
     * The virtual below must be in a header file as RESOURCE is only known at compile time
     */

    void resolveUsage(DependencyGraph& graph,
            ResourceEdgeBase const* const* edges, size_t count,
            ResourceEdgeBase const* writer) noexcept override {
        for (size_t i = 0; i < count; i++) {
            if (graph.isEdgeValid(edges[i])) {
                // this Edge is guaranteed to be a ResourceEdge<RESOURCE> by construction
                ResourceEdge const* const edge = static_cast<ResourceEdge const*>(edges[i]);
                usage |= edge->usage;
            }
        }

        // here don't check for the validity of Edge because even if the edge is invalid
        // the fact that we're called (not culled) means we need to take it into account
        // e.g. because the resource could be needed in a render target
        if (writer) {
            ResourceEdge const* const edge = static_cast<ResourceEdge const*>(writer);
            usage |= edge->usage;
        }

        // propagate usage bits to the parents
        Resource* p = this;
        while (p != p->parent) {
            p = static_cast<Resource*>(p->parent);
            p->usage |= usage;
        }
    }

    void destroyEdge(DependencyGraph::Edge* edge) noexcept override {
        // this Edge is guaranteed to be a ResourceEdge<RESOURCE> by construction
        delete static_cast<ResourceEdge *>(edge);
    }

    void devirtualize(ResourceAllocatorInterface& resourceAllocator,
            bool useProtectedMemory) noexcept override {
        if (!isSubResource()) {
            resource.create(resourceAllocator, name, descriptor, usage, useProtectedMemory);
        } else {
            // resource is guaranteed to be initialized before we are by construction
            resource = static_cast<Resource const*>(parent)->resource;
        }
    }

    void destroy(ResourceAllocatorInterface& resourceAllocator) noexcept override {
        if (detached || isSubResource()) {
            return;
        }
        resource.destroy(resourceAllocator);
    }

    utils::CString usageString() const noexcept override {
        return utils::to_string(usage);
    }
};

/*
 * An imported resource is just like a regular one, except that it's constructed directly from
 * the concrete resource and it, evidently, doesn't create/destroy the concrete resource.
 */
template<typename RESOURCE>
class ImportedResource : public Resource<RESOURCE> {
public:
    using Descriptor = typename RESOURCE::Descriptor;
    using Usage = typename RESOURCE::Usage;

    UTILS_NOINLINE
    ImportedResource(const char* name, Descriptor const& desc, Usage usage, RESOURCE const& rsrc) noexcept
            : Resource<RESOURCE>(name, desc) {
        this->resource = rsrc;
        this->usage = usage;
    }

protected:
    void devirtualize(ResourceAllocatorInterface&, bool) noexcept override {
        // imported resources don't need to devirtualize
    }
    void destroy(ResourceAllocatorInterface&) noexcept override {
        // imported resources never destroy the concrete resource
    }

    bool isImported() const noexcept override { return true; }

    UTILS_NOINLINE
    bool connect(DependencyGraph& graph,
            PassNode* passNode, ResourceNode* resourceNode, FrameGraphTexture::Usage u) override {
        assertConnect(u);
        return Resource<RESOURCE>::connect(graph, passNode, resourceNode, u);
    }

    UTILS_NOINLINE
    bool connect(DependencyGraph& graph,
            ResourceNode* resourceNode, PassNode* passNode, FrameGraphTexture::Usage u) override {
        assertConnect(u);
        return Resource<RESOURCE>::connect(graph, resourceNode, passNode, u);
    }

private:
    UTILS_NOINLINE
    void assertConnect(FrameGraphTexture::Usage u) {
        FILAMENT_CHECK_PRECONDITION((u & this->usage) == u)
                << "Requested usage " << utils::to_string(u).c_str()
                << " not available on imported resource \"" << this->name << "\" with usage "
                << utils::to_string(this->usage).c_str();
    }
};


class ImportedRenderTarget : public ImportedResource<FrameGraphTexture> {
public:
    backend::Handle<backend::HwRenderTarget> target;
    FrameGraphRenderPass::ImportDescriptor importedDesc;

    UTILS_NOINLINE
    ImportedRenderTarget(const char* name,
            FrameGraphTexture::Descriptor const& mainAttachmentDesc,
            FrameGraphRenderPass::ImportDescriptor const& importedDesc,
            backend::Handle<backend::HwRenderTarget> target);

    ~ImportedRenderTarget() noexcept override;

protected:
    UTILS_NOINLINE
    bool connect(DependencyGraph& graph,
            PassNode* passNode, ResourceNode* resourceNode, FrameGraphTexture::Usage u) override;

    UTILS_NOINLINE
    bool connect(DependencyGraph& graph,
            ResourceNode* resourceNode, PassNode* passNode, FrameGraphTexture::Usage u) override;

    ImportedRenderTarget* asImportedRenderTarget() noexcept override { return this; }

private:
    void assertConnect(FrameGraphTexture::Usage u);

    static FrameGraphTexture::Usage usageFromAttachmentsFlags(
            backend::TargetBufferFlags attachments) noexcept;
};

// ------------------------------------------------------------------------------------------------

// prevent implicit instantiation of Resource<FrameGraphTexture> which is a known type
extern template class Resource<FrameGraphTexture>;
extern template class ImportedResource<FrameGraphTexture>;

} // namespace filament

#endif // TNT_FILAMENT_FG_DETAILS_RESOURCE_H
