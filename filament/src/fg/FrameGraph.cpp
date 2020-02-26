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

#include "fg/RenderTargetResource.h"
#include "fg/ResourceNode.h"
#include "fg/RenderTarget.h"
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
using namespace details;

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

bool FrameGraph::Builder::isAttachment(FrameGraphId<FrameGraphTexture> r) const noexcept {
    fg::ResourceEntry<FrameGraphTexture>& entry = mFrameGraph.getResourceEntryUnchecked(r);
    return any(entry.descriptor.usage & (
            TextureUsage::COLOR_ATTACHMENT |
            TextureUsage::DEPTH_ATTACHMENT |
            TextureUsage::STENCIL_ATTACHMENT));
}

FrameGraphRenderTarget::Descriptor&
FrameGraph::Builder::getRenderTargetDescriptor(FrameGraphRenderTargetHandle handle) {
    FrameGraph& fg = mFrameGraph;
    assert(handle < fg.mRenderTargets.size());
    return fg.mRenderTargets[handle].desc;
}

FrameGraphRenderTargetHandle FrameGraph::Builder::createRenderTarget(const char* name,
        FrameGraphRenderTarget::Descriptor const& desc, TargetBufferFlags clearFlags) noexcept {
    // TODO: add support for cubemaps and arrays
    // TODO: enforce that we can't have a resource used in 2 rendertarget in the same pass
    FrameGraph& fg = mFrameGraph;
    fg::RenderTarget& renderTarget = fg.createRenderTarget(name, desc);
    renderTarget.userClearFlags = clearFlags;
    mPass.declareRenderTarget(renderTarget);
    return FrameGraphRenderTargetHandle(renderTarget.index);
}

FrameGraphRenderTargetHandle FrameGraph::Builder::createRenderTarget(FrameGraphId<FrameGraphTexture>& texture,
        TargetBufferFlags clearFlags) noexcept {
    texture = this->write(texture);
    FrameGraphRenderTarget::Descriptor desc;
    desc.attachments.color = texture;
    return createRenderTarget(getName(texture), desc, clearFlags);
}

FrameGraphHandle FrameGraph::Builder::read(FrameGraphHandle input) {
    return mPass.read(mFrameGraph, input);
}

FrameGraphId<FrameGraphTexture> FrameGraph::Builder::sample(FrameGraphId<FrameGraphTexture> input) {
    return mPass.sample(mFrameGraph, input);
}

FrameGraphHandle FrameGraph::Builder::write(FrameGraphHandle output) {
    return mPass.write(mFrameGraph, output);
}

FrameGraph::Builder& FrameGraph::Builder::sideEffect() noexcept {
    mPass.hasSideEffect = true;
    return *this;
}

// ------------------------------------------------------------------------------------------------

FrameGraphPassResources::FrameGraphPassResources(FrameGraph& fg, fg::PassNode const& pass) noexcept
        : mFrameGraph(fg), mPass(pass) {
}

const char* FrameGraphPassResources::getPassName() const noexcept {
    return mPass.name;
}

fg::ResourceEntryBase const& FrameGraphPassResources::getResourceEntryBase(FrameGraphHandle r) const noexcept {
    ResourceNode& node = mFrameGraph.getResourceNodeUnchecked(r);

    fg::ResourceEntryBase const* const pResource = node.resource;
    assert(pResource);

// TODO: we should check for write to
//    // check that this FrameGraphHandle is indeed used by this pass
//    ASSERT_POSTCONDITION_NON_FATAL(mPass.isReadingFrom(r),
//            "Pass \"%s\" doesn't declare reads to resource \"%s\" -- expect graphic corruptions",
//            mPass.name, pResource->name);

    return *pResource;
}

FrameGraphPassResources::RenderTargetInfo
FrameGraphPassResources::getRenderTarget(FrameGraphRenderTargetHandle handle, uint8_t level) const noexcept {
    FrameGraphPassResources::RenderTargetInfo info{};
    FrameGraph& fg = mFrameGraph;

    fg::RenderTarget& renderTarget = fg.mRenderTargets[handle];
    assert(renderTarget.cache);

    info = renderTarget.cache->targetInfo;
    assert(info.target);

    // overwrite discard flags with the per-rendertarget (per-pass) computed value
    info.params.flags.discardStart = TargetBufferFlags::NONE;
    info.params.flags.discardEnd   = TargetBufferFlags::NONE;
    info.params.flags.clear        = renderTarget.userClearFlags;

    static constexpr TargetBufferFlags flags[] = {
            TargetBufferFlags::COLOR,
            TargetBufferFlags::DEPTH,
            TargetBufferFlags::STENCIL };

    auto& resourceNodes = fg.mResourceNodes;
    for (size_t i = 0; i <renderTarget.desc.attachments.textures.size(); i++) {
        FrameGraphHandle attachment = renderTarget.desc.attachments.textures[i];
        if (attachment.isValid()) {
            if (resourceNodes[attachment.index].resource->discardStart) {
                info.params.flags.discardStart |= flags[i];
            }
            if (resourceNodes[attachment.index].resource->discardEnd) {
                info.params.flags.discardEnd |= flags[i];
            }
        }
    }

    // clear implies discarding the content of the buffer
    info.params.flags.discardStart |= (renderTarget.userClearFlags & TargetBufferFlags::ALL);
    if (renderTarget.cache->imported) {
        // we never discard more than the user flags
        info.params.flags.discardStart &= renderTarget.cache->discardStart;
        info.params.flags.discardEnd   &= renderTarget.cache->discardEnd;
    }

    // check that this FrameGraphRenderTarget is indeed declared by this pass
    ASSERT_POSTCONDITION_NON_FATAL(info.target,
            "Pass \"%s\" doesn't declare rendertarget \"%s\" -- expect graphic corruptions",
            mPass.name, renderTarget.name);
    
//    slog.d << mPass.name << ": resource = \"" << renderTarget.name << "\", flags = "
//        << io::hex
//        << renderTarget.targetInfo.params.discardStart << ", "
//        << renderTarget.targetInfo.params.discardEnd << io::endl;

    return info;
}

// ------------------------------------------------------------------------------------------------

FrameGraph::FrameGraph(fg::ResourceAllocatorInterface& resourceAllocator)
        : mResourceAllocator(resourceAllocator),
          mArena("FrameGraph Arena", 65536), // TODO: the Area will eventually come from outside
          mPassNodes(mArena),
          mResourceNodes(mArena),
          mRenderTargets(mArena),
          mAliases(mArena),
          mResourceEntries(mArena),
          mRenderTargetCache(mArena) {
//    slog.d << "PassNode: " << sizeof(PassNode) << io::endl;
//    slog.d << "ResourceNode: " << sizeof(ResourceNode) << io::endl;
//    slog.d << "Resource: " << sizeof(Resource) << io::endl;
//    slog.d << "RenderTarget: " << sizeof(fg::RenderTarget) << io::endl;
//    slog.d << "RenderTargetResource: " << sizeof(RenderTargetResource) << io::endl;
//    slog.d << "Alias: " << sizeof(Alias) << io::endl;
//    slog.d << "Vector: " << sizeof(Vector<fg::PassNode>) << io::endl;
}

FrameGraph::~FrameGraph() = default;

bool FrameGraph::isValid(FrameGraphHandle handle) const noexcept {
    if (!handle.isValid()) return false;
    auto const& registry = mResourceNodes;
    assert(handle.index < registry.size());
    ResourceNode const& node = registry[handle.index];
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
    assert(registry[lhs.index].resource);
    assert(registry[rhs.index].resource);
    return registry[lhs.index].resource == registry[rhs.index].resource;
}

FrameGraphHandle FrameGraph::createResourceNode(fg::ResourceEntryBase* resource) noexcept {
    auto& resourceNodes = mResourceNodes;
    size_t index = resourceNodes.size();
    resourceNodes.emplace_back(resource, resource->version);
    return FrameGraphHandle{ (uint16_t)index };
}

FrameGraphHandle FrameGraph::moveResource(FrameGraphHandle from, FrameGraphHandle to) {
    // this is just used to validate the 'to' handle
    getResourceNode(to);
    // validate and rename the 'from' handle
    ResourceNode const& node = getResourceNode(from);
    ++node.resource->version;
    mAliases.push_back({from, to});
    return createResourceNode(node.resource);
}

void FrameGraph::present(FrameGraphHandle input) {
    addPass<Empty>("Present",
            [&](Builder& builder, auto& data) {
                builder.read(input);
                builder.sideEffect();
            }, [](FrameGraphPassResources const& resources, auto const& data, DriverApi&) {});
}

PassNode& FrameGraph::createPass(const char* name, FrameGraphPassExecutor* base) noexcept {
    auto& frameGraphPasses = mPassNodes;
    const uint32_t id = (uint32_t)frameGraphPasses.size();
    frameGraphPasses.emplace_back(*this, name, id, base);
    return frameGraphPasses.back();
}

fg::RenderTarget& FrameGraph::createRenderTarget(const char* name,
        FrameGraphRenderTarget::Descriptor const& desc) noexcept {
    auto& renderTargets = mRenderTargets;
    const uint16_t id = (uint16_t)renderTargets.size();
    renderTargets.emplace_back(name, desc, id);
    return renderTargets.back();
}

FrameGraphHandle FrameGraph::create(fg::ResourceEntryBase* pResourceEntry) noexcept {
    mResourceEntries.emplace_back(pResourceEntry, *this);
    return createResourceNode(pResourceEntry);
}

ResourceNode& FrameGraph::getResourceNodeUnchecked(FrameGraphHandle r) {
    ASSERT_POSTCONDITION(r.isValid(), "using an uninitialized resource handle");

    auto& resourceNodes = mResourceNodes;
    assert(r.index < resourceNodes.size());

    ResourceNode& node = resourceNodes[r.index];
    assert(node.resource);

    return node;
}

ResourceNode& FrameGraph::getResourceNode(FrameGraphHandle r) {
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

FrameGraphRenderTarget::Descriptor const& FrameGraph::getDescriptor(
        FrameGraphRenderTargetHandle handle) const noexcept {
    assert(handle < mRenderTargets.size());
    return mRenderTargets[handle].desc;
}

bool FrameGraph::equals(FrameGraphRenderTarget::Descriptor const& cacheEntry,
        FrameGraphRenderTarget::Descriptor const& rt) const noexcept {
    const Vector<ResourceNode>& resourceNodes = mResourceNodes;

    // if the rendertarget we're looking up doesn't have the sample field set to 0, it means the
    // user didn't specify it, and it's okay to match it to any sample count.
    // Otherwise, sample count must match, with the caveat that 0 or 1 are treated the same.
    const bool samplesMatch = (!rt.samples) ||
            (std::max<uint8_t>(1u, cacheEntry.samples) == std::max<uint8_t>(1u, rt.samples));

    return std::equal(
            cacheEntry.attachments.textures.begin(), cacheEntry.attachments.textures.end(),
            rt.attachments.textures.begin(), rt.attachments.textures.end(),
            [&resourceNodes](
                    FrameGraphRenderTarget::Attachments::AttachmentInfo lhs,
                    FrameGraphRenderTarget::Attachments::AttachmentInfo rhs) {
                // both resource must be the same level to match
                if (lhs.getLevel() != rhs.getLevel()) {
                    return false;
                }
                const FrameGraphHandle lHandle = lhs.getHandle();
                const FrameGraphHandle rHandle = rhs.getHandle();
                if (lHandle == rHandle) {
                    // obviously resources match if they're the same
                    return true;
                }
                if (lHandle.isValid() && rHandle.isValid()) {
                    if (resourceNodes[lHandle.index].resource == resourceNodes[rHandle.index].resource) {
                        // they also match if they're the same concrete resource
                        return true;
                    }
                }
                if (!rHandle.isValid()) {
                    // it's okay if the cached RT has more attachments than we require
                    return true;
                }
                return false;
            }) && samplesMatch;
}

FrameGraphRenderTargetHandle FrameGraph::importRenderTarget(const char* name,
        FrameGraphRenderTarget::Descriptor descriptor,
        backend::Handle<backend::HwRenderTarget> target, uint32_t width, uint32_t height,
        TargetBufferFlags discardStart, TargetBufferFlags discardEnd) {

    // Imported render target can't specify textures -- it's meaningless
    assert(std::all_of(
            descriptor.attachments.textures.begin(), descriptor.attachments.textures.end(),
            [](FrameGraphHandle t) { return !t.isValid(); }));

    // create a fake imported attachment for this render target,
    // so we can do a moveResource() for instance.
    FrameGraphTexture::Descriptor desc{
            .width = width, .height = height, .usage = TextureUsage::COLOR_ATTACHMENT };
    descriptor.attachments.color = import<FrameGraphTexture>(name, desc, {});

    // create a fg::RenderTarget, so we can get an handle
    fg::RenderTarget& renderTarget = createRenderTarget(name, descriptor);

    // And pre-populate the cache with the concrete render target
    RenderTargetResource* pRenderTargetResource = mArena.make<RenderTargetResource>(name,
            descriptor, true,TargetBufferFlags::COLOR, width, height, TextureFormat{});
    pRenderTargetResource->targetInfo.target = target;
    pRenderTargetResource->discardStart = discardStart;
    pRenderTargetResource->discardEnd = discardEnd;
    mRenderTargetCache.emplace_back(pRenderTargetResource, *this);

    return FrameGraphRenderTargetHandle(renderTarget.index);
}

bool FrameGraph::computeDiscard(const Vector<FrameGraphHandle> fg::PassNode::* list,
        PassNode const* curr, PassNode const* first, FrameGraphHandle attachment) {
    auto& resourceNodes = mResourceNodes;
    while (curr != first) {
        PassNode const& pass = *curr++;
        // TODO: maybe find a more efficient way of figuring this out
        // for each resource written or read...
        for (FrameGraphHandle cur : pass.*list) {
            // for all possible attachments of our renderTarget...
            ResourceEntryBase const* const pResource = resourceNodes[cur.index].resource;
            if (attachment.isValid() && resourceNodes[attachment.index].resource == pResource) {
                return false;
            }
        }
    }
    return true;
}


FrameGraph& FrameGraph::compile() noexcept {
    Vector<fg::PassNode>& passNodes = mPassNodes;
    Vector<fg::RenderTarget>& renderTargets = mRenderTargets;
    Vector<ResourceNode>& resourceNodes = mResourceNodes;
    Vector<UniquePtr<fg::ResourceEntryBase>>& resourceRegistry = mResourceEntries;
    Vector<UniquePtr<RenderTargetResource>>& renderTargetCache = mRenderTargetCache;

    /*
     * remap aliased resources
     */

    if (!mAliases.empty()) {
        for (fg::Alias const& alias : mAliases) {
            // disconnect all writes to "from"
            ResourceNode& from = resourceNodes[alias.from.index];
            ResourceNode& to   = resourceNodes[alias.to.index];

            // remap "to" resources to "from" resources
            for (ResourceNode& cur : resourceNodes) {
                if (cur.resource == to.resource) {
                    cur.resource = from.resource;
                }
            }

            // TODO: make this better
            //  When replacing a resource from an imported render-target, we find all existing
            //  render-target that used that resource as the color attachment, and we clear
            //  all other attachments -- we have to do this because the resource aliasing
            //  must also alias the render targets.
            //  For instance, if a render-target was declared with a color+depth attachment,
            //  and the color attachment was aliased with an imported render-target, the
            //  render-target cache (see: equals()) wouldn't match it (because the depth
            //  attachment would be missing).
            for (fg::RenderTarget& rt : renderTargets) {
                auto& textures = rt.desc.attachments.textures;
                FrameGraphHandle handle = textures[0];
                if (handle.isValid()) {
                    ResourceNode const& node = resourceNodes[handle.index];
                    if (node.resource->imported && node.resource == from.resource) {
                        for (size_t i = 1; i < textures.size(); ++i) {
                            // FIXME: here we're likely leaking the resources we're clearing,
                            //  however, we can't know for sure because currently RenderTargets
                            //  don't hold references to their attachment (Passes do). We can't
                            //  go back to the passes to clear the reference because we can't know
                            //  for sure that the pass doesn't need the resource (i.e. we don't
                            //  know if it did a read() or write() for the purpose of the
                            //  RT attachment or something else).
                            //  To fix this, we need the RT to hold references to attachments,
                            //  and Passes to RTs. This way passes wouldn't have to add references
                            //  to attachments, and clearing attachments would release their
                            //  resources.
                            textures[i] = {};
                        }
                    }
                }
            }

            for (PassNode& pass : passNodes) {
                // passes that were reading from "from node", now read from "to node" as well
                for (FrameGraphHandle handle : pass.reads) {
                    if (handle == alias.from) {
                        if (!pass.isReadingFrom(alias.to)) {
                            pass.reads.push_back(alias.to);
                        }
                        break;
                    }
                }

                for (FrameGraphHandle handle : pass.samples) {
                    if (handle == alias.from) {
                        if (!pass.isSamplingFrom(alias.to)) {
                            pass.samples.push_back(
                                    static_cast<FrameGraphId<FrameGraphTexture>>(alias.to));
                        }
                        break;
                    }
                }

                // Passes that were writing to "from node", no longer do
                pass.writes.erase(
                        std::remove_if(pass.writes.begin(), pass.writes.end(),
                                [&alias](auto handle) { return handle == alias.from; }),
                        pass.writes.end());
            }
        }
    }

    /*
     * compute passes and resource reference counts
     */

    for (PassNode& pass : passNodes) {
        // compute passes reference counts (i.e. resources we're writing to)
        pass.refCount = (uint32_t)pass.writes.size() + (uint32_t)pass.hasSideEffect;

        // compute resources reference counts (i.e. resources we're reading from)
        for (FrameGraphHandle resource : pass.reads) {
            // add a reference for each pass that reads from this resource
            ResourceNode& node = resourceNodes[resource.index];
            node.readerCount++;
        }

        // set the writers
        for (FrameGraphHandle resource : pass.writes) {
            ResourceNode& node = resourceNodes[resource.index];
            node.writer = &pass;
        }
    }

    /*
     * cull passes and resources...
     */

    Vector<ResourceNode*> stack(mArena);
    stack.reserve(resourceNodes.size());
    for (ResourceNode& node : resourceNodes) {
        if (node.readerCount == 0) {
            stack.push_back(&node);
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
                auto const& reads = writer->reads;
                for (FrameGraphHandle resource : reads) {
                    ResourceNode& r = resourceNodes[resource.index];
                    if (--r.readerCount == 0) {
                        stack.push_back(&r);
                    }
                }
            }
        }
    }
    // update the final reference counts
    for (ResourceNode const& node : resourceNodes) {
        node.resource->refs += node.readerCount;
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

    // resolve render targets
    for (PassNode& pass : passNodes) {
        if (pass.refCount) {
            for (auto i : pass.renderTargets) {
                renderTargets[i].resolve(*this);
            }
        }
    }

    /*
     * compute first/last users for active passes
     */

    for (PassNode& pass : passNodes) {
        if (!pass.refCount) {
            assert(!pass.hasSideEffect);
            continue;
        }
        for (FrameGraphHandle resource : pass.reads) {
            VirtualResource* const pResource = resourceNodes[resource.index].resource;
            // figure out which is the first pass to need this resource
            pResource->first = pResource->first ? pResource->first : &pass;
            // figure out which is the last pass to need this resource
            pResource->last = &pass;
        }
        for (FrameGraphHandle resource : pass.writes) {
            VirtualResource* const pResource = resourceNodes[resource.index].resource;
            // figure out which is the first pass to need this resource
            pResource->first = pResource->first ? pResource->first : &pass;
            // figure out which is the last pass to need this resource
            pResource->last = &pass;
        }
        for (auto i : pass.renderTargets) {
            VirtualResource* const pResource = renderTargets[i].cache;
            // figure out which is the first pass to need this resource
            pResource->first = pResource->first ? pResource->first : &pass;
            // figure out which is the last pass to need this resource
            pResource->last = &pass;
        }
    }

    // add resource to de-virtualize or destroy to the corresponding list for each active pass
    for (UniquePtr<fg::ResourceEntryBase> const& resource : resourceRegistry) {
        if (resource->refs) {
            assert(!resource->first == !resource->last);
            if (resource->first && resource->last) {
                resource->first->devirtualize.push_back(resource.get());
                resource->last->destroy.push_back(resource.get());
            }
        }
    }

    // *THEN* add the virtual rendertargets
    for (UniquePtr<RenderTargetResource> const& entry : renderTargetCache) {
        assert(!entry->first == !entry->last);
        if (entry->first && entry->last) {
            entry->first->devirtualize.push_back(entry.get());
            entry->last->destroy.push_back(entry.get());
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
    for (VirtualResource* resource : node.destroy) {
        resource->preExecuteDestroy(*this);
    }

    // execute the pass
    FrameGraphPassResources resources(*this, node);
    node.base->execute(resources, driver);

    for (VirtualResource* resource : node.devirtualize) {
        resource->postExecuteDevirtualize(*this);
    }
    // destroy concrete resources
    for (VirtualResource* resource : node.destroy) {
        resource->postExecuteDestroy(*this);
    }
}

void FrameGraph::reset() noexcept {
    // reset the frame graph state
    mPassNodes.clear();
    mResourceNodes.clear();
    mResourceEntries.clear();
    mAliases.clear();
    mRenderTargetCache.clear();
    mId = 0;
}

void FrameGraph::execute(FEngine& engine, DriverApi& driver) noexcept {
    auto const& passNodes = mPassNodes;
    for (PassNode const& node : passNodes) {
        if (node.refCount) {
            executeInternal(node, driver);
            if (&node != &passNodes.back()) {
                // wake-up the driver thread and consume data in the command queue, this helps with
                // latency, parallelism and memory pressure in the command queue.
                // As an optimization, we don't do this on the last execute() because
                // 1) we're adding a driver flush command (below) and
                // 2) an engine.flush() is always performed by Renderer at the end of a renderJob.
                engine.flush();
            }
        }
    }
    // this is a good place to kick the GPU, since we've just done a bunch of work
    driver.flush();
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
    for (ResourceNode const& node : registry) {
        ResourceEntryBase const* subresource = node.resource;

        out << "\"R" << node.resource->id << "_" << +node.version << "\""
            "[label=\"" << node.resource->name << "\\n(version: " << +node.version << ")"
            "\\nid:" << node.resource->id <<
            "\\nrefs:" << node.resource->refs;

#if UTILS_HAS_RTTI
        auto textureResource = dynamic_cast<ResourceEntry<FrameGraphTexture> const*>(subresource);
        if (textureResource) {
            out << ", " << (bool(textureResource->descriptor.usage & TextureUsage::SAMPLEABLE) ? "texture" : "renderbuffer");
        }
#endif
        out << "\", style=filled, fillcolor="
            << ((subresource->imported) ?
                (node.resource->refs ? "palegreen" : "palegreen4") :
                (node.resource->refs ? "skyblue" : "skyblue4"))
            << "]\n";
    }

    // connect passes to resources
    out << "\n";
    for (auto const& node : frameGraphPasses) {
        out << "P" << node.id << " -> { ";
        for (auto const& writer : node.writes) {
            out << "R" << registry[writer.index].resource->id << "_" << +registry[writer.index].version << " ";
            out << "R" << registry[writer.index].resource->id << "_" << +registry[writer.index].version << " ";
        }
        out << "} [color=red2]\n";
    }

    // connect resources to passes
    out << "\n";
    for (ResourceNode const& node : registry) {
        out << "R" << node.resource->id << "_" << +node.version << " -> { ";

        // who reads us...
        for (PassNode const& pass : frameGraphPasses) {
            for (FrameGraphHandle const& read : pass.reads) {
                if (registry[read.index].resource->id == node.resource->id &&
                    registry[read.index].version == node.version) {
                    out << "P" << pass.id << " ";
                }
            }
        }
        out << "} [color=lightgreen]\n";
    }

    // aliases...
    if (!mAliases.empty()) {
        out << "\n";
        for (fg::Alias const& alias : mAliases) {
            out << "R" << registry[alias.from.index].resource->id << "_" << +registry[alias.from.index].version << " -> ";
            out << "R" << registry[alias.to.index].resource->id << "_" << +registry[alias.to.index].version;
            out << " [color=yellow, style=dashed]\n";
        }
    }

    out << "}" << utils::io::endl;
#endif
}

// avoid creating a .o just for these
fg::RenderTargetResource::~RenderTargetResource() = default;
FrameGraphPassExecutor::FrameGraphPassExecutor() = default;
FrameGraphPassExecutor::~FrameGraphPassExecutor() = default;

} // namespace filament
