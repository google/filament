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

#include "driver/Driver.h"
#include "driver/Handle.h"
#include "driver/CommandStream.h"

#include <filament/driver/DriverEnums.h>

#include <utils/Panic.h>
#include <utils/Log.h>

using namespace utils;

namespace filament {

using namespace driver;
using namespace fg;

// ------------------------------------------------------------------------------------------------

namespace fg {

struct Resource {
    explicit Resource(const char* name, bool imported) noexcept;
    Resource(Resource const&) = delete;
    Resource(Resource&&) = default;
    Resource& operator=(Resource const&) = delete;

    ~Resource() noexcept;

    // constants
    const char* const name;         // for debugging
    bool imported;

    // computed during compile()
    PassNode* writer = nullptr;     // last writer to this resource
    PassNode* first = nullptr;      // pass that needs to instantiate the resource
    PassNode* last = nullptr;       // pass that can destroy the resource
    uint32_t writerCount = 0;       // # of passes writing to this resource
    uint32_t readerCount = 0;       // # of passes reading from this resource
    FrameGraphResource::Descriptor desc;
    FrameGraph::Builder::RWFlags readFlags = 0;
    FrameGraph::Builder::RWFlags writeFlags = 0;

    // concrete resource -- set when the resource is created
    void create(DriverApi& driver) noexcept;
    void destroy(DriverApi& driver) noexcept;
    Handle<HwTexture> textures[2] = {};  // color, depth
    FrameGraphPassResources::RenderTarget target;
};

struct ResourceNode {
    ResourceNode(const char* name, FrameGraphResource::Descriptor const& desc, bool imported,
                 uint16_t index) noexcept
            : name(name), imported(imported), index(index), desc(desc) {
    }
    ResourceNode(ResourceNode const&) = delete;
    ResourceNode(ResourceNode&&) noexcept = default;

    // constants
    const char* const name;
    const bool imported;
    const uint16_t index;

    // updated by the builder
    uint8_t version = 0;
    FrameGraphResource::Descriptor desc;
    FrameGraph::Builder::RWFlags readFlags  = 0;
    FrameGraph::Builder::RWFlags writeFlags = 0;

    // set during compile()
    union {
        Resource* resource = nullptr;
        size_t offset;
    };
};

struct PassNode {
    template <typename T>
    using Vector = FrameGraph::Vector<T>;

    PassNode(FrameGraph& fg, const char* name, uint32_t id, FrameGraphPassExecutor* base) noexcept
            : name(name), id(id), base(base),
              reads(fg.getArena()),
              writes(fg.getArena()),
              targetFlags(fg.getArena()),
              devirtualize(fg.getArena()),
              destroy(fg.getArena()) {
    }
    PassNode(PassNode const&) = delete;
    PassNode(PassNode&& rhs) noexcept
            : name(rhs.name), id(rhs.id), base(rhs.base),
              reads(std::move(rhs.reads)),
              writes(std::move(rhs.writes)),
              targetFlags(std::move(rhs.targetFlags)),
              devirtualize(std::move(rhs.devirtualize)),
              destroy(std::move(rhs.destroy)),
              refCount(rhs.refCount) {
        rhs.base = nullptr;
    }

    PassNode& operator=(PassNode const&) = delete;
    PassNode& operator=(PassNode&&) = delete;

    ~PassNode() { delete base; }

    // for Builder
    FrameGraphResource read(ResourceNode const& resource, FrameGraph::Builder::RWFlags flags) {
        // don't allow multiple reads of the same resource -- it's just redundant.
        auto pos = std::find_if(reads.begin(), reads.end(),
                [&resource](FrameGraphResource cur) { return resource.index == cur.index; });
        if (pos != reads.end()) {
            return *pos;
        }

        FrameGraphResource r{ resource.index, resource.version, flags };

        // now figure out if we already recorded a write to this resource, and if so, use the
        // previous version number to record the read. i.e. pretend the read() was recorded first.
        pos = std::find_if(writes.begin(), writes.end(),
                [&resource](FrameGraphResource cur) { return resource.index == cur.index; });
        if (pos != writes.end()) {
            r.version--;
        }

        // just record that we're reading from this resource (at the given version)
        reads.push_back(r);
        return r;
    }

    bool isReadingFrom(FrameGraphResource resource) const noexcept {
        auto pos = std::find_if(reads.begin(), reads.end(),
                [resource](FrameGraphResource cur) { return resource.index == cur.index; });
        return (pos != reads.end());
    }

    FrameGraphResource write(ResourceNode& resource, FrameGraph::Builder::RWFlags flags) {
        // don't allow multiple writes of the same resource -- it's just redundant.
        auto pos = std::find_if(writes.begin(), writes.end(),
                [&resource](FrameGraphResource cur) { return resource.index == cur.index; });
        if (pos != writes.end()) {
            return *pos;
        }

        /*
         * We invalidate and rename handles that are written into, to avoid undefined order
         * access to the resources.
         *
         * e.g. forbidden graphs
         *
         *         +-> [R1] -+
         *        /           \
         *  (A) -+             +-> (A)
         *        \           /
         *         +-> [R2] -+        // failure when setting R2 from (A)
         *
         */
        ++resource.version;
        // writing to an imported resource should count as a side-effect
        if (resource.imported) {
            hasSideEffect = true;
        }
        // record the write
        FrameGraphResource r{ resource.index, resource.version, flags };
        writes.push_back(r);
        return r;
    }

    bool isWritingTo(FrameGraphResource resource) const noexcept {
        auto pos = std::find_if(writes.begin(), writes.end(),
                [resource](FrameGraphResource cur) { return resource.index == cur.index; });
        return (pos != writes.end());
    }

    // constants
    const char* const name;                     // our name
    const uint32_t id;                          // a unique id (only for debugging)
    FrameGraphPassExecutor* base = nullptr;     // type eraser for calling execute()

    // set by the builder
    Vector<FrameGraphResource> reads;           // resources we're reading from
    Vector<FrameGraphResource> writes;          // resources we're writing to

    struct TargetFlags {
        uint8_t clear = 0;
        uint8_t discardStart = 0;
        uint8_t discardEnd = 0;
        uint8_t dependencies = 0;
    };
    Vector<TargetFlags> targetFlags;

    bool hasSideEffect = false;             // whether this pass has side effects

    // computed during compile()
    Vector<uint16_t> devirtualize;          // resources we need to create before executing
    Vector<uint16_t> destroy;               // resources we need to destroy after executing
    uint32_t refCount = 0;                  // count resources that have a reference to us
};

struct Alias {
    FrameGraphResource from, to;
};


Resource::~Resource() noexcept {
    if (!imported) {
        assert(!textures[0]);
        assert(!textures[1]);
        assert(!target.target);
    }
}

// ------------------------------------------------------------------------------------------------
// out-of-line definitions
// ------------------------------------------------------------------------------------------------

Resource::Resource(const char* name, bool imported) noexcept
        : name(name), imported(imported) {
}

void Resource::create(DriverApi& driver) noexcept {
    // some sanity check
    if (readerCount)    assert(readFlags);
    if (writerCount)    assert(writeFlags);

    // technically this doesn't need to be initialized if we're not a rendertarget.
    target.params = {};
    target.params.left = 0;
    target.params.bottom = 0;
    target.params.width = desc.width;
    target.params.height = desc.height;

    if (!imported) {
        if (readFlags & FrameGraph::Builder::COLOR) {
            textures[0] = driver.createTexture(desc.type, desc.levels,
                    desc.format, 1,
                    desc.width, desc.height, desc.depth,
                    TextureUsage::COLOR_ATTACHMENT);
        }
        if (readFlags & FrameGraph::Builder::DEPTH) {
            textures[1] = driver.createTexture(desc.type, desc.levels,
                    TextureFormat::DEPTH24, 1,
                    desc.width, desc.height, desc.depth,
                    TextureUsage::DEPTH_ATTACHMENT);
        }

        // Note: if the resource is a source of a blit, it needs a rendertarget (because that's how
        // blits work) -- in that case, the resource would have been declared with Builder::blit()
        // access, and its write flags here would be set.

        uint32_t attachments = 0;
        if (writeFlags & FrameGraph::Builder::COLOR) {
            attachments |= uint32_t(TargetBufferFlags::COLOR);
        }
        if (writeFlags & FrameGraph::Builder::DEPTH) {
            attachments |= TargetBufferFlags::DEPTH;
        }
        if (attachments) {
            target.target = driver.createRenderTarget(TargetBufferFlags(attachments),
                    desc.width, desc.height, desc.samples, desc.format,
                    { textures[0] }, { textures[1] }, {});
        }
    }
}

void Resource::destroy(DriverApi& driver) noexcept {
    // we don't own the handles of imported resources
    if (imported) return;

    for (auto& texture : textures) {
        if (texture) {
            driver.destroyTexture(texture);
            texture.clear(); // needed because of noop driver
        }
    }
    if (target.target) {
        driver.destroyRenderTarget(target.target);
        target.target.clear(); // needed because of noop driver
    }
}


} // namespace fg

// ------------------------------------------------------------------------------------------------

FrameGraphPassExecutor::FrameGraphPassExecutor() = default;
FrameGraphPassExecutor::~FrameGraphPassExecutor() = default;

// ------------------------------------------------------------------------------------------------

FrameGraph::Builder::Builder(FrameGraph& fg, PassNode& pass) noexcept
    : mFrameGraph(fg), mPass(pass) {
}

FrameGraph::Builder::~Builder() noexcept = default;

FrameGraphResource FrameGraph::Builder::createResource(
        const char* name, FrameGraphResource::Descriptor const& desc) noexcept {
    FrameGraph& frameGraph = mFrameGraph;
    ResourceNode& resource = frameGraph.createResource(name, desc, false);
    return { resource.index, resource.version };
}

FrameGraphResource FrameGraph::Builder::read(FrameGraphResource const& input, RWFlags readFlags) {
    ResourceNode* resource = mFrameGraph.getResource(input);
    if (!resource) {
        return {};
    }
    resource->readFlags |= readFlags;
    return mPass.read(*resource, readFlags);
}

FrameGraphResource
FrameGraph::Builder::blit(FrameGraphResource const& input, FrameGraph::Builder::RWFlags readFlags) {
    ResourceNode* resource = mFrameGraph.getResource(input);
    if (!resource) {
        return {};
    }
    resource->readFlags |= readFlags;
    // resource used in a blit, it needs a rendertarget, so we set its write flags.
    resource->writeFlags |= readFlags;
    return mPass.read(*resource, readFlags);
}

FrameGraphResource FrameGraph::Builder::write(FrameGraphResource const& output, RWFlags writeFlags) {
    ResourceNode* resource = mFrameGraph.getResource(output);
    if (!resource) {
        return {};
    }
    resource->writeFlags |= writeFlags;
    return mPass.write(*resource, writeFlags);
}

FrameGraph::Builder& FrameGraph::Builder::sideEffect() noexcept {
    mPass.hasSideEffect = true;
    return *this;
}

// ------------------------------------------------------------------------------------------------

FrameGraphPassResources::FrameGraphPassResources(FrameGraph& fg, fg::PassNode const& pass) noexcept
    : mFrameGraph(fg), mPass(pass) {

    // copy the target flags for each render target of this pass
    auto const& resourceNodes = fg.mResourceNodes;
    for (size_t i = 0, c = pass.writes.size(); i < c; i++) {
        auto const& r = pass.writes[i];
        Resource* const pResource = resourceNodes[r.index].resource;
        assert(pResource);
        RenderPassParams& targetFlags = pResource->target.params;
        const PassNode::TargetFlags& passTargetFlags = pass.targetFlags[i];
        targetFlags.clear          = passTargetFlags.clear;
        targetFlags.discardStart   = passTargetFlags.discardStart;
        targetFlags.discardEnd     = passTargetFlags.discardEnd;
        targetFlags.dependencies   = passTargetFlags.dependencies;
    }
}

Handle<HwTexture> FrameGraphPassResources::getTexture(
        FrameGraphResource r, TextureUsage attachment) const noexcept {

    Resource const* const pResource = mFrameGraph.mResourceNodes[r.index].resource;
    assert(pResource);

    // check that this FrameGraphResource is indeed used by this pass
    ASSERT_POSTCONDITION_NON_FATAL(mPass.isReadingFrom(r),
            "Pass \"%s\" doesn't declare reads to resource \"%s\" -- expect graphic corruptions",
            mPass.name, pResource->name);


    const char* requested = "unknown";
    Handle<HwTexture> h;
    switch (attachment) {
        case TextureUsage::DEFAULT:
            if (pResource->readFlags == FrameGraph::Builder::DEPTH) {
                h = pResource->textures[1];
                requested = "depth";
                break;
            }
            [[clang::fallthrough]];
        case TextureUsage::COLOR_ATTACHMENT:
            h = pResource->textures[0];
            requested = "color";
            break;

        case TextureUsage::DEPTH_ATTACHMENT:
            h = pResource->textures[1];
            requested = "depth";
            break;
    }

    if (pResource->imported) {
        ASSERT_POSTCONDITION_NON_FATAL(h,
                "Imported resource \"%s\" (id=%u) doesn't have a %s texture",
                pResource->name, r.index, requested);
    } else {
        // for non imported resources that shouldn't happen.
        // FIXME: is it true though? couldn't it happen if user didn't declare dependencies right?
        assert(h);
    }
    return h;
}

FrameGraphPassResources::RenderTarget const&
FrameGraphPassResources::getRenderTarget(FrameGraphResource r) const noexcept {
    Resource const* const pResource = mFrameGraph.mResourceNodes[r.index].resource;
    assert(pResource);

    // We need to check the resource in read or written by this pass (as opposed to written only),
    // because a render target is needed for blit operations, which might be what the caller
    // is doing.
    ASSERT_POSTCONDITION_NON_FATAL(mPass.isWritingTo(r) || mPass.isReadingFrom(r),
            "Pass \"%s\" doesn't declare writes to resource \"%s\" -- expect graphic corruptions",
            mPass.name, pResource->name);

    assert(pResource->target.target);

    if (pResource->imported) {
        ASSERT_POSTCONDITION_NON_FATAL(pResource->target.target,
                "Imported resource \"%s\" (id=%u) doesn't have a render target",
                pResource->name, r.index);
    }

//    slog.d << mPass.name << ": resource = \"" << pResource->name << "\", flags = "
//        << io::hex
//        << pResource->target.params.discardStart << ", "
//        << pResource->target.params.discardEnd << io::endl;

    return pResource->target;
}

FrameGraphResource::Descriptor const& FrameGraphPassResources::getDescriptor(
        FrameGraphResource r) const noexcept {
    // TODO: we should check that this FrameGraphResource is indeed used by this pass
    (void)mPass; // suppress unused warning
    Resource const* const pResource = mFrameGraph.mResourceNodes[r.index].resource;
    assert(pResource);
    return pResource->desc;
}

// ------------------------------------------------------------------------------------------------

FrameGraph::FrameGraph()
        : mArena("FrameGraph Arena", 16384), // TODO: the Area will eventually come from outside
          mPassNodes(mArena),
          mResourceNodes(mArena),
          mResourceRegistry(mArena),
          mAliases(mArena) {
    // some default size to avoid wasting space with the std::vector<>
    mPassNodes.reserve(8);
    mResourceNodes.reserve(8);
    mAliases.reserve(4);
}

FrameGraph::~FrameGraph() = default;

bool FrameGraph::isValid(FrameGraphResource handle) const noexcept {
    if (!handle.isValid()) return false;
    auto const& registry = mResourceNodes;
    assert(handle.index < registry.size());
    auto& resource = registry[handle.index];
    return handle.version == resource.version;
}

bool FrameGraph::moveResource(FrameGraphResource from, FrameGraphResource to) {
    if (!isValid(from) || !isValid(to)) {
        return false;
    }
    mAliases.push_back({from, to});
    return true;
}

void FrameGraph::present(FrameGraphResource input, Builder::RWFlags sideEffects) {
    struct Dummy {
    };
    addPass<Dummy>("Present",
            [&](Builder& builder, Dummy& data) {
                builder.read(input, sideEffects);
                builder.sideEffect();
            },
            [](FrameGraphPassResources const& resources, Dummy const& data, DriverApi&) {
            });
}

PassNode& FrameGraph::createPass(const char* name, FrameGraphPassExecutor* base) noexcept {
    auto& frameGraphPasses = mPassNodes;
    const uint32_t id = (uint32_t)frameGraphPasses.size();
    frameGraphPasses.emplace_back(*this, name, id, base);
    return frameGraphPasses.back();
}

ResourceNode& FrameGraph::createResource(
        const char* name, FrameGraphResource::Descriptor const& desc, bool imported) noexcept {
    auto& registry = mResourceNodes;
    uint16_t id = (uint16_t)registry.size();
    registry.emplace_back(name, desc, imported, id);
    return registry.back();
}

ResourceNode* FrameGraph::getResource(FrameGraphResource r) {
    auto& registry = mResourceNodes;

    if (!ASSERT_POSTCONDITION_NON_FATAL(r.isValid(),
            "using an uninitialized resource handle")) {
        return nullptr;
    }

    assert(r.index < registry.size());
    auto& resource = registry[r.index];

    if (!ASSERT_POSTCONDITION_NON_FATAL(r.version == resource.version,
            "using an invalid resource handle (version=%u) for resource=\"%s\" (id=%u, version=%u)",
            r.version, resource.name, resource.index, resource.version)) {
        return nullptr;
    }

    return &resource;
}
FrameGraphResource::Descriptor* FrameGraph::getDescriptor(FrameGraphResource r) {
    ResourceNode* node = getResource(r);
    return node ? &node->desc : nullptr;
}

FrameGraphResource FrameGraph::importResource(
        const char* name, FrameGraphResource::Descriptor const& descriptor,
        Handle <HwRenderTarget> target) {
    return importResource(name, descriptor, target, {}, {});
}

FrameGraphResource FrameGraph::importResource(
        const char* name, FrameGraphResource::Descriptor const& descriptor,
        Handle<HwTexture> color, Handle<HwTexture> depth) {
    return importResource(name, descriptor, {}, color, depth);
}

FrameGraphResource FrameGraph::importResource(
        const char* name, FrameGraphResource::Descriptor const& descriptor,
        Handle <HwRenderTarget> target,
        Handle <HwTexture> color, Handle <HwTexture> depth) {
    ResourceNode& node = createResource(name, descriptor, true);

    // imported resources are created immediately
    auto& resourceRegistry = mResourceRegistry;
    resourceRegistry.emplace_back(name, true);
    Resource& resource = resourceRegistry.back();
    resource.textures[0] = color;
    resource.textures[1] = depth;
    resource.target.target = target;

    // we store the offset into the array (instead of the pointer) because the storage might
    // move between now and compile().
    node.offset = &resource - resourceRegistry.data();

    return { node.index, node.version };
}

FrameGraph& FrameGraph::compile() noexcept {
    auto& passNodes = mPassNodes;
    auto& resourceNodes = mResourceNodes;
    auto& resourceRegistry = mResourceRegistry;
    resourceRegistry.reserve(resourceNodes.size());

    // create the sub-resources
    for (ResourceNode& node : resourceNodes) {
        if (node.imported) {
            node.resource = &resourceRegistry[node.offset];
        } else {
            resourceRegistry.emplace_back(node.name, false);
            node.resource = &resourceRegistry.back();
        }
        node.resource->desc = node.desc;
        node.resource->readFlags = node.readFlags;
        node.resource->writeFlags = node.writeFlags;
    }

    // remap them
    for (auto const& alias : mAliases) {
        // disconnect all writes to "from"
        auto& from = resourceNodes[alias.from.index];
        auto& to = resourceNodes[alias.to.index];
        for (PassNode& pass : passNodes) {
            auto pos = std::find_if(pass.writes.begin(), pass.writes.end(),
                    [&from](FrameGraphResource const& r) { return r.index == from.index; });
            if (pos != pass.writes.end()) {
                pass.writes.erase(pos);
            }
        }

        // read flags are combined
        from.resource->readFlags |= to.resource->readFlags;

        // write flags are set to the "to" resource, since writers are disconnected from the "from"
        from.resource->writeFlags = to.resource->writeFlags;

        // alias "to" to "from"
        to.resource = from.resource;
    }

    // compute passes and resource reference counts
    for (PassNode& pass : passNodes) {
        // compute passes reference counts (i.e. resources we're writing to)
        pass.refCount = (uint32_t)pass.writes.size() + (uint32_t)pass.hasSideEffect;

        // compute resources reference counts (i.e. resources we're reading from)
        for (FrameGraphResource resource : pass.reads) {
            Resource* subResource = resourceNodes[resource.index].resource;

            // add a reference for each pass that reads from this resource
            subResource->readerCount++;
        }

        // set the writers
        for (FrameGraphResource resource : pass.writes) {
            Resource* subResource = resourceNodes[resource.index].resource;
            subResource->writer = &pass;

            // add a reference for each pass that writes to this resource
            subResource->writerCount++;
        }
    }

    // cull passes and resources...
    Vector<Resource*> stack(mArena);
    stack.reserve(resourceNodes.size());
    for (Resource& resource : resourceRegistry) {
        if (resource.readerCount == 0) {
            stack.push_back(&resource);
        }
    }
    while (!stack.empty()) {
        Resource const* const pSubResource = stack.back();
        stack.pop_back();

        // by construction, this resource cannot have more than one producer because
        // - two unrelated passes can't write in the same resource
        // - passes that read + write into the resource imply that the refcount is not null

        assert(pSubResource->writerCount <= 1);

        PassNode* const writer = pSubResource->writer;
        if (writer) {
            assert(writer->refCount >= 1);
            if (--writer->refCount == 0) {
                // this pass is culled
                auto const& reads = writer->reads;
                for (FrameGraphResource resource : reads) {
                    Resource* r = resourceNodes[resource.index].resource;
                    if (--r->readerCount == 0) {
                        stack.push_back(r);
                    }
                }
            }
        }
    }

    // compute first/last users for active passes
    auto first = passNodes.begin();
    auto last = passNodes.end();
    while (first != last) {
        PassNode& pass = *first;
        if (!pass.refCount) {
            assert(!pass.hasSideEffect);
            ++first;
            continue;
        }
        for (FrameGraphResource resource : pass.reads) {
            Resource* subResource = resourceNodes[resource.index].resource;
            // figure out which is the first pass to need this resource
            subResource->first = subResource->first ? subResource->first : &pass;
            // figure out which is the last pass to need this resource
            subResource->last = &pass;
        }

        pass.targetFlags.resize(pass.writes.size());
        for (size_t i = 0, c = pass.writes.size(); i < c; i++) {
            FrameGraphResource resource = pass.writes[i];
            Resource* subResource = resourceNodes[resource.index].resource;
            // figure out which is the first pass to need this resource
            subResource->first = subResource->first ? subResource->first : &pass;
            // figure out which is the last pass to need this resource
            subResource->last = &pass;
            
            // compute this resource discard flag for this pass
            PassNode::TargetFlags& targetFlags = pass.targetFlags[i];

            // FIXME: the discard flags must be updated for each attachement
            uint8_t discardStart = TargetBufferFlags::ALL;
            uint8_t discardEnd = TargetBufferFlags::ALL;
            // does anyone reads this resource after us...
            auto curr = first;
            while (++curr != last) {
                PassNode& futurePass = *curr;

                // TODO: maybe find a more efficient way of figuring this out

                auto pos = std::find_if(
                        futurePass.reads.begin(), futurePass.reads.end(),
                        [subResource, &resourceNodes](FrameGraphResource cur) {
                            return subResource == resourceNodes[cur.index].resource;
                        });

                const bool isFuturePassReadingFromUs = pos != futurePass.reads.end();

                if (isFuturePassReadingFromUs) {
                    // if a future pass is reading from us, we can't discard when we're done
                    if (pos->flags & Builder::COLOR) {
                        discardEnd &= ~TargetBufferFlags::COLOR;
                    }
                    if (pos->flags & Builder::DEPTH) {
                        discardEnd &= ~TargetBufferFlags::DEPTH;
                    }
                    break;
                }
            }

            // does anyone write this resource before us
            curr = passNodes.begin();
            while (curr != first) {
                PassNode& pastPass = *curr++;

                // TODO: maybe find a more efficient way of figuring this out
                auto pos = std::find_if(
                        pastPass.writes.begin(), pastPass.writes.end(),
                        [subResource, &resourceNodes](FrameGraphResource cur) {
                            return subResource == resourceNodes[cur.index].resource;
                        });

                const bool isPastPassWritingToUs = pos != pastPass.writes.end();

                if (isPastPassWritingToUs) {
                    // if a past pass is writing to us, we can't discard
                    if (pos->flags & Builder::COLOR) {
                        discardStart &= ~TargetBufferFlags::COLOR;
                    }
                    if (pos->flags & Builder::DEPTH) {
                        discardStart &= ~TargetBufferFlags::DEPTH;
                    }
                    break;
                }
            }
            targetFlags.clear = 0;
            targetFlags.discardStart = discardStart;
            targetFlags.discardEnd = discardEnd;
            targetFlags.dependencies = 0;
        }
        ++first;
    }

    // add resource to devirtualize or destroy to the corresponding list for each active pass
    for (size_t index = 0, c = resourceRegistry.size() ; index < c ; index++) {
        auto& resource = resourceRegistry[index];
        assert(!resource.first == !resource.last);
        if (resource.readerCount && resource.first && resource.last) {
            resource.first->devirtualize.push_back((uint16_t)index);
            resource.last->destroy.push_back((uint16_t)index);
        }
    }

    return *this;
}

void FrameGraph::execute(DriverApi& driver) noexcept {
    auto& resourceRegistry = mResourceRegistry;
    for (PassNode const& node : mPassNodes) {
        if (!node.refCount) continue;
        assert(node.base);

        // create concrete resources
        for (size_t id : node.devirtualize) {
            resourceRegistry[id].create(driver);
        }

        // execute the pass
        FrameGraphPassResources resources(*this, node);
        node.base->execute(resources, driver);

        // destroy concrete resources
        for (uint32_t id : node.destroy) {
            resourceRegistry[id].destroy(driver);
        }
    }

    // reset the frame graph state
    mPassNodes.clear();
    mResourceNodes.clear();
    mResourceRegistry.clear();
    mAliases.clear();
}

void FrameGraph::export_graphviz(utils::io::ostream& out) {
    bool removeCulled = false;

    out << "digraph framegraph {\n";
    out << "rankdir = LR\n";
    out << "bgcolor = black\n";
    out << "node [shape=rectangle, fontname=\"helvetica\", fontsize=10]\n\n";

    auto const& registry = mResourceNodes;
    auto const& frameGraphPasses = mPassNodes;

    // declare passes
    for (auto const& node : frameGraphPasses) {
        if (removeCulled && !node.refCount) continue;
        out << "\"P" << node.id << "\" [label=\"" << node.name
               << "\\nrefs: " << node.refCount
               << "\\nseq: " << node.id
               << "\", style=filled, fillcolor="
               << (node.refCount ? "darkorange" : "darkorange4") << "]\n";
    }

    // declare resources nodes
    out << "\n";
    for (auto const& node : registry) {
        auto subresource = registry[node.index].resource;
        if (removeCulled && !subresource->readerCount) continue;
        for (size_t version = 0; version <= node.version; version++) {
            out << "\"R" << node.index << "_" << version << "\""
                   "[label=\"" << node.name << "\\n(version: " << version << ")"
                   "\\nid:" << node.index <<
                   "\\nrefs:" << subresource->readerCount
                   <<"\""
                   ", style=filled, fillcolor="
                   << ((subresource->imported) ?
                        (subresource->readerCount ? "palegreen" : "palegreen4") :
                        (subresource->readerCount ? "skyblue" : "skyblue4"))
                   << "]\n";
        }
    }

    // connect passes to resources
    out << "\n";
    for (auto const& node : frameGraphPasses) {
        if (removeCulled && !node.refCount) continue;
        out << "P" << node.id << " -> { ";
        for (auto const& writer : node.writes) {
            auto resource = registry[writer.index].resource;
            if (removeCulled && !resource->readerCount) continue;
            out << "R" << writer.index << "_" << writer.version << " ";
        }
        out << "} [color=red2]\n";
    }

    // connect resources to passes
    out << "\n";
    for (auto const& node : registry) {
        auto subresource = registry[node.index].resource;
        if (removeCulled && !subresource->readerCount) continue;
        for (size_t version = 0; version <= node.version; version++) {
            out << "R" << node.index << "_" << version << " -> { ";

            // who reads us...
            for (auto const& pass : frameGraphPasses) {
                if (removeCulled && !pass.refCount) continue;
                for (auto const& read : pass.reads) {
                    if (read.index == node.index && read.version == version ) {
                        out << "P" << pass.id << " ";
                    }
                }
            }
            out << "} [color=lightgreen]\n";
        }
    }

    // aliases...
    if (!mAliases.empty()) {
        out << "\n";
        for (auto const& alias : mAliases) {
            out << "R" << alias.from.index << "_" << alias.from.version << " -> ";
            out << "R" << alias.to.index << "_" << alias.to.version;
            out << " [color=yellow, style=dashed]\n";
        }
    }

    out << "}" << utils::io::endl;
}

} // namespace filament
