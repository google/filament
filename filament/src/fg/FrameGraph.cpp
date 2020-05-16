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

#include "FrameGraph.h"

#include "FrameGraphPassResources.h"
#include "FrameGraphHandle.h"

#include "fg/ResourceNode.h"
#include "fg/PassNode.h"
#include "fg/VirtualResource.h"

#include "details/Engine.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/Panic.h>
#include <utils/Log.h>

using namespace utils;

namespace filament {

using namespace backend;
using namespace fg;

// ------------------------------------------------------------------------------------------------

struct fg::Alias { //4
    FrameGraphHandle from, to;
};

FrameGraph::Builder::Builder(FrameGraph& fg, PassNode& pass) noexcept
    : mFrameGraph(fg), mPass(pass) {
}

FrameGraph::Builder::~Builder() noexcept = default;

const char* FrameGraph::Builder::getPassName() const noexcept {
    return mPass.name;
}

const char* FrameGraph::Builder::getName(FrameGraphHandle const& r) const noexcept {
    ResourceNode& resourceNode = mFrameGraph.getResourceNodeUnchecked(r);
    fg::ResourceEntryBase* pResource = resourceNode.resource;
    assert(pResource);
    return pResource ? pResource->name : "(invalid)";
}

FrameGraphId<FrameGraphRenderTarget> FrameGraph::Builder::createRenderTargetImpl(
        const char* name, typename FrameGraphRenderTarget::Descriptor const& desc) noexcept {
    auto handle = mFrameGraph.create<FrameGraphRenderTarget>(name, desc);
    return mPass.use(mFrameGraph, handle);
}

FrameGraphHandle FrameGraph::Builder::readImpl(FrameGraphHandle input) {
    return mPass.read(mFrameGraph, input);
}

FrameGraphHandle FrameGraph::Builder::writeImpl(FrameGraphHandle output) {
    return mPass.write(mFrameGraph, output);
}

FrameGraphId<FrameGraphTexture> FrameGraph::Builder::sample(FrameGraphId<FrameGraphTexture> input) {
    return mPass.sample(mFrameGraph, input);
}

FrameGraph::Builder& FrameGraph::Builder::sideEffect() noexcept {
    mPass.hasSideEffect = true;
    return *this;
}

// ------------------------------------------------------------------------------------------------

FrameGraph::FrameGraph(fg::ResourceAllocatorInterface& resourceAllocator)
        : mResourceAllocator(resourceAllocator),
          mArena("FrameGraph Arena", 65536), // TODO: the Area will eventually come from outside
          mPassNodes(mArena),
          mResourceNodes(mArena),
          mResourceNodeEntries(mArena),
          mResourceEntries(mArena) {
    mPassNodes.reserve(32);
    mResourceNodes.reserve(64);
    mResourceNodeEntries.reserve(64);
    mResourceEntries.reserve(48);
//    slog.d << "PassNode: " << sizeof(PassNode) << io::endl;
//    slog.d << "ResourceNode: " << sizeof(ResourceNode) << io::endl;
//    slog.d << "Resource: " << sizeof(Resource) << io::endl;
//    slog.d << "RenderTargetResourceEntry: " << sizeof(RenderTargetResourceEntry) << io::endl;
//    slog.d << "Alias: " << sizeof(Alias) << io::endl;
//    slog.d << "Vector: " << sizeof(Vector<fg::PassNode>) << io::endl;
}

FrameGraph::~FrameGraph() = default;

bool FrameGraph::isValid(FrameGraphHandle handle) const noexcept {
    if (!handle.isValid()) return false;
    auto const& registry = mResourceNodes;
    assert(handle.index < registry.size());
    ResourceNode const& node = *registry[handle.index];
    return node.version == node.resource->version;
}

bool FrameGraph::equal(FrameGraphHandle lhs, FrameGraphHandle rhs) const noexcept {
    if (lhs == rhs) {
        return true;
    }
    if (lhs.isValid() != rhs.isValid()) {
        return false;
    }
    auto const& registry = mResourceNodes;
    assert(lhs.index < registry.size());
    assert(rhs.index < registry.size());
    assert(registry[lhs.index]->resource);
    assert(registry[rhs.index]->resource);
    return registry[lhs.index]->resource == registry[rhs.index]->resource;
}

FrameGraphHandle FrameGraph::createResourceNode(fg::ResourceEntryBase* resource) noexcept {
    auto& resourceNodes = mResourceNodes;
    auto& resourceNodeEntries = mResourceNodeEntries;
    size_t index = resourceNodes.size();
    ResourceNode* pBase = mArena.make<ResourceNode>(resource, resource->version);
    resourceNodeEntries.emplace_back(pBase, *this);
    resourceNodes.emplace_back(pBase);
    return FrameGraphHandle{ (uint16_t)index };
}

void FrameGraph::moveResourceBase(FrameGraphHandle fromHandle, FrameGraphHandle toHandle) {
    // 'to' becomes 'from'
    ResourceNode& from = getResourceNode(fromHandle);
    ResourceNode& to   = getResourceNode(toHandle);
    Vector<ResourceNode*>& resourceNodes = mResourceNodes;

    // The pass that is writing to "fromHandle" no longer does (and might be culled)
    // (note: there can only be a single pass that can be a writer)
    PassNode* const pass = from.writer;
    if (pass) {
        auto pos = std::find_if(pass->writes.begin(), pass->writes.end(),
                [fromHandle](auto handle) { return handle == fromHandle; });
        assert(pos != pass->writes.end());
        pass->writes.erase(pos);
        from.writer = to.writer;
    }

    // The 'to' node becomes the 'from' node and therefore inherits passes that are reading
    // from it (in fact they'll share the same node).
    resourceNodes[toHandle.index] = resourceNodes[fromHandle.index];
    // Then all the ResourceNodes that match this resource, are modified to point to the
    // 'from' resource -- but the nodes themselves are not changed. i.e. they keep their
    // own reader/writers.
    for (auto& cur : resourceNodes) {
        if (cur->resource == to.resource) {
            cur->resource = from.resource;
        }
    }
}

void FrameGraph::moveResource(
        FrameGraphId<FrameGraphRenderTarget> fromHandle,
        FrameGraphId<FrameGraphTexture> toHandle) {
    // 'to' becomes 'from'
    // all rendertargets that have toHandle as attachment become fromHandle RTs

    // getResourceNode() validates the handles
    ResourceNode& from = getResourceNode(fromHandle);
    ResourceNode& to   = getResourceNode(toHandle);
    Vector<ResourceNode*>& resourceNodes = mResourceNodes;

    // NOTE: We're guaranteed that fromHandle's resource is unique (i.e. it's the only hande that
    //       references this resource -- this is because FrameGraphRenderTarget can't be
    //       written to).

    // TODO: 'toHandle' cannot be sampleable if the rendertarget is imported. If it's not imported,
    //       the corresponding attachment must be sampleable.

    // TODO: 'toHandle' must be replaced by the corresponding resource (or dummy if imported).
    //       so it's not devirtualized

    // TODO: check that 'from.resource' at least has the same attachments than 'cur.resource' needs

    auto hasAttachment = [&](RenderTargetResourceEntry const* rt, ResourceEntryBase const* r) {
        for (auto h : rt->descriptor.attachments.textures) {
            if (h.isValid() && resourceNodes[h.getHandle().index]->resource == r) {
                return true;
            }
        }
        return false;
    };

    // find all ResourceNode of type RenderTargetResource that have 'to' as attachment and replace
    // them with the 'from' ResourceNode
    for (auto& cur : resourceNodes) {
        auto p = cur->resource->asRenderTargetResourceEntry();
        if (p) {
            if (hasAttachment(p, to.resource)) {
                cur = &from;
            }
        }
    }
}

void FrameGraph::present(FrameGraphHandle input) {
    addPass<Empty>("Present",
            [&](Builder& builder, auto& data) {
                builder.readImpl(input);
                builder.sideEffect();
            }, [](FrameGraphPassResources const& resources, auto const& data, DriverApi&) {});
}

PassNode& FrameGraph::createPass(const char* name, FrameGraphPassExecutor* base) noexcept {
    auto& frameGraphPasses = mPassNodes;
    const uint32_t id = (uint32_t)frameGraphPasses.size();
    frameGraphPasses.emplace_back(*this, name, id, base);
    return frameGraphPasses.back();
}

FrameGraphHandle FrameGraph::create(fg::ResourceEntryBase* pResourceEntry) noexcept {
    mResourceEntries.emplace_back(pResourceEntry, *this);
    return createResourceNode(pResourceEntry);
}

ResourceNode& FrameGraph::getResourceNodeUnchecked(FrameGraphHandle r) {
    auto& resourceNodes = mResourceNodes;
    assert(r.index < resourceNodes.size());
    ResourceNode& node = *resourceNodes[r.index];
    assert(node.resource);
    return node;
}

ResourceNode& FrameGraph::getResourceNode(FrameGraphHandle r) {
    ASSERT_POSTCONDITION(r.isValid(), "using an uninitialized resource handle");
    ResourceNode& node = getResourceNodeUnchecked(r);
    ASSERT_POSTCONDITION(node.resource->version == node.version,
            "using an invalid resource handle (version=%u) for resource=\"%s\" (id=%u, version=%u)",
            node.resource->version, node.resource->name, node.resource->id, node.version);

    return node;
}

fg::ResourceEntryBase& FrameGraph::getResourceEntryBase(FrameGraphHandle r) noexcept {
    ResourceNode& node = getResourceNode(r);
    assert(node.resource);
    return *node.resource;
}

fg::ResourceEntryBase& FrameGraph::getResourceEntryBaseUnchecked(FrameGraphHandle r) noexcept {
    ResourceNode& node = getResourceNodeUnchecked(r);
    assert(node.resource);
    return *node.resource;
}

FrameGraph& FrameGraph::compile() noexcept {
    Vector<fg::PassNode>& passNodes = mPassNodes;
    Vector<ResourceNode*>& resourceNodes = mResourceNodes;
    Vector<UniquePtr<fg::ResourceEntryBase>>& resourceRegistry = mResourceEntries;

    /*
     * compute passes and resource reference counts
     */

    for (PassNode& pass : passNodes) {
        // compute passes reference counts (i.e. resources we're writing to)
        pass.refCount = (uint32_t)pass.writes.size() + (uint32_t)pass.hasSideEffect;

        // compute resources reference counts (i.e. resources we're reading from)
        for (FrameGraphHandle resource : pass.reads) {
            assert(resource.isValid());
            // add a reference for each pass that reads from this resource
            resourceNodes[resource.index]->readerCount++;
        }

#ifndef NDEBUG
        for (FrameGraphHandle resource : pass.writes) {
            assert(resource.isValid());
            assert(resourceNodes[resource.index]->writer == &pass);
        }
#endif
    }

    /*
     * cull passes and resources...
     */

    Vector<ResourceNode*> stack(mArena);
    stack.reserve(resourceNodes.size());
    for (ResourceNode* node : resourceNodes) {
        if (node->readerCount == 0) {
            stack.push_back(node);
        }
    }
    while (!stack.empty()) {
        ResourceNode const* const pNode = stack.back();
        stack.pop_back();
        PassNode* const writer = pNode->writer;
        if (writer) {
            assert(writer->refCount >= 1);
            if (--writer->refCount == 0) {
                // this pass is culled
                assert(!writer->hasSideEffect);
                auto const& reads = writer->reads;
                for (FrameGraphHandle resource : reads) {
                    ResourceNode& r = *resourceNodes[resource.index];
                    if (--r.readerCount == 0) {
                        stack.push_back(&r);
                    }
                }
            }
        }
    }
    // update the final reference counts
    for (ResourceNode const* node : resourceNodes) {
        node->resource->refs += node->readerCount;
    }

    /*
     * compute first/last users for active passes
     */

    for (PassNode& pass : passNodes) {
        if (!pass.refCount) {
            continue;
        }
        for (FrameGraphHandle resource : pass.reads) {
            VirtualResource* const pResource = resourceNodes[resource.index]->resource;
            // figure out which is the first pass to need this resource
            pResource->first = pResource->first ? pResource->first : &pass;
            // figure out which is the last pass to need this resource
            pResource->last = &pass;
        }
        for (FrameGraphHandle resource : pass.writes) {
            VirtualResource* const pResource = resourceNodes[resource.index]->resource;
            // figure out which is the first pass to need this resource
            pResource->first = pResource->first ? pResource->first : &pass;
            // figure out which is the last pass to need this resource
            pResource->last = &pass;
        }
    }

    // update the SAMPLEABLE bit, now that we culled unneeded passes
    for (PassNode& pass : passNodes) {
        if (pass.refCount) {
            for (auto handle : pass.samples) {
                auto& texture = getResourceEntryUnchecked(handle);
                texture.descriptor.usage |= backend::TextureUsage::SAMPLEABLE;
            }
        }
    }

    for (UniquePtr<fg::ResourceEntryBase> const& resource : resourceRegistry) {
        if (resource->refs) {
            resource->resolve(*this);
        }
    }

    // add resource to de-virtualize or destroy to the corresponding list for each active pass
    // but add them in priority order (this is so that rendertargets are added after textures)
    for (size_t priority = 0; priority < 2; priority++) {
        for (UniquePtr<fg::ResourceEntryBase> const& resource : resourceRegistry) {
            if (resource->priority == priority && resource->refs) {
                auto pFirst = resource->first;
                auto pLast = resource->last;
                assert(!pFirst == !pLast);
                if (pFirst && pLast) {
                    pFirst->devirtualize.push_back(resource.get());
                    pLast->destroy.push_back(resource.get());
                }
            }
        }
    }

    return *this;
}

void FrameGraph::executeInternal(PassNode const& node, DriverApi& driver) noexcept {
    assert(node.base);
    // create concrete resources and rendertargets
    for (VirtualResource* resource : node.devirtualize) {
        resource->preExecuteDevirtualize(*this);
    }
    // the destroy list is ran backward, so that objects are destroyed in reverse order
    std::for_each(node.destroy.rbegin(), node.destroy.rend(), [this](auto* resource){
        resource->preExecuteDestroy(*this);
    });

    // update the RenderTarget discard flags
    for (auto handle : node.renderTargets) {
        // here we're guaranteed we have a RenderTargetResourceEntry&
        auto& entry = getResourceEntryUnchecked<FrameGraphRenderTarget>(handle);
        static_cast<RenderTargetResourceEntry&>(entry).update(*this, node);
    }

    // execute the pass
    FrameGraphPassResources resources(*this, node);
    node.base->execute(resources, driver);

    for (VirtualResource* resource : node.devirtualize) {
        resource->postExecuteDevirtualize(*this);
    };

    // destroy concrete resources
    // the destroy list is ran backward, so that objects are destroyed in reverse order
    std::for_each(node.destroy.rbegin(), node.destroy.rend(), [this](auto* resource){
        resource->postExecuteDestroy(*this);
    });
}

void FrameGraph::reset() noexcept {
    // reset the frame graph state
    mPassNodes.clear();
    mResourceNodes.clear();
    mResourceNodeEntries.clear();
    mResourceEntries.clear();
    mId = 0;
}

void FrameGraph::execute(FEngine& engine, DriverApi& driver) noexcept {
    auto const& passNodes = mPassNodes;
    driver.pushGroupMarker("FrameGraph");
    for (PassNode const& node : passNodes) {
        if (node.refCount) {
            driver.pushGroupMarker(node.name);
            executeInternal(node, driver);
            driver.popGroupMarker();
        }
    }
    // this is a good place to kick the GPU, since we've just done a bunch of work
    driver.flush();
    driver.popGroupMarker();
    reset();
}

void FrameGraph::execute(DriverApi& driver) noexcept {
    for (PassNode const& node : mPassNodes) {
        if (node.refCount) {
            executeInternal(node, driver);
        }
    }
    // this is a good place to kick the GPU, since we've just done a bunch of work
    driver.flush();
    reset();
}

void FrameGraph::export_graphviz(utils::io::ostream& out) {
#ifndef NDEBUG
    out << "digraph framegraph {\n";
    out << "rankdir = LR\n";
    out << "bgcolor = black\n";
    out << "node [shape=rectangle, fontname=\"helvetica\", fontsize=10]\n\n";

    auto const& registry = mResourceNodes;
    auto const& frameGraphPasses = mPassNodes;

    // declare passes
    for (PassNode const& node : frameGraphPasses) {
        out << "\"P" << node.id << "\" [label=\"" << node.name
               << "\\nrefs: " << node.refCount
               << "\\nseq: " << node.id
               << "\", style=filled, fillcolor="
               << (node.refCount ? "darkorange" : "darkorange4") << "]\n";
    }

    // declare resources nodes
    out << "\n";
    for (ResourceNode const* node : registry) {
        ResourceEntryBase* subresource = node->resource;

        out << "\"R" << node->resource->id << "_" << +node->version << "\""
            "[label=\"" << node->resource->name << "\\n(version: " << +node->version << ")"
            "\\nid:" << node->resource->id <<
            "\\nrefs:" << node->resource->refs;

#if UTILS_HAS_RTTI
        auto textureResource = dynamic_cast<ResourceEntry<FrameGraphTexture> const*>(subresource);
        if (textureResource) {
            out << ", " << (bool(textureResource->descriptor.usage & TextureUsage::SAMPLEABLE) ? "texture" : "renderbuffer");
        }
#endif
        auto rendertarget = subresource->asRenderTargetResourceEntry();
        if (rendertarget) {
            out << ", " << "RenderTarget";
        }
        out << "\", style=filled, fillcolor="
            << ((subresource->imported) ?
                (node->resource->refs ? "palegreen" : "palegreen4") :
                (node->resource->refs ? "skyblue" : "skyblue4"))
            << "]\n";
    }

    // connect passes to resources
    out << "\n";
    for (auto const& node : frameGraphPasses) {
        out << "P" << node.id << " -> { ";
        for (auto const& writer : node.writes) {
            out << "R" << registry[writer.index]->resource->id << "_" << +registry[writer.index]->version << " ";
            out << "R" << registry[writer.index]->resource->id << "_" << +registry[writer.index]->version << " ";
        }
        out << "} [color=red2]\n";
    }

    // connect resources to passes
    out << "\n";
    for (ResourceNode const* node : registry) {
        out << "R" << node->resource->id << "_" << +node->version << " -> { ";

        // who reads us...
        for (PassNode const& pass : frameGraphPasses) {
            for (FrameGraphHandle const& read : pass.reads) {
                if (registry[read.index]->resource->id == node->resource->id &&
                    registry[read.index]->version == node->version) {
                    out << "P" << pass.id << " ";
                }
            }
        }
        out << "} [color=lightgreen]\n";
    }

    out << "}" << utils::io::endl;
#endif
}

// avoid creating a .o just for these
FrameGraphPassExecutor::FrameGraphPassExecutor() = default;
FrameGraphPassExecutor::~FrameGraphPassExecutor() = default;

} // namespace filament
