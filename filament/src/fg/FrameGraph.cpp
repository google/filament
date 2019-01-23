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

struct Alias {
    FrameGraphResource from, to;
};

struct TargetFlags {
    uint8_t clear = 0;
    uint8_t discardStart = 0;
    uint8_t discardEnd = 0;
    uint8_t dependencies = 0;
};

struct Resource {
    Resource(const char* name, bool imported) noexcept;
    Resource(Resource const&) = delete;
    Resource(Resource&&) = default;
    Resource& operator=(Resource const&) = delete;
    ~Resource() noexcept;

    // constants
    const char* const name;         // for debugging
    bool imported;

    // computed during compile()
    PassNode* writer = nullptr;             // last writer to this resource
    PassNode* first = nullptr;              // pass that needs to instantiate the resource
    PassNode* last = nullptr;               // pass that can destroy the resource
    uint32_t writerCount = 0;               // # of passes writing to this resource
    uint32_t readerCount = 0;               // # of passes reading from this resource

    enum Type {
        TEXTURE,
        IMPORTED_RENDER_TARGET
    };
    Type type = TEXTURE;

    FrameGraphResource::Descriptor desc;

    // concrete resource -- set when the resource is created
    void create(DriverApi& driver) noexcept;
    void destroy(DriverApi& driver) noexcept;

    // can't use union without implementing the move ctor manually. not a problem right now.
    //union {
        Handle<HwTexture> texture;
        uint16_t renderTargetIndex;
    //};
};

struct ResourceNode {
    ResourceNode(const char* name,
            FrameGraphResource::Descriptor const& desc, bool imported, uint16_t index) noexcept
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
    FrameGraphResource::Descriptor desc;    // FIXME: it's a shame we store this twice (in Resource too)

    // set during compile()
    union {
        Resource* resource = nullptr;
        size_t offset;
    };
};

struct RenderTarget {
    RenderTarget(const char* name,
            FrameGraphRenderTarget::Descriptor const& desc, bool imported, uint16_t index) noexcept
            : name(name), imported(imported), index(index), desc(desc) {
    }

    // constants
    const char* const name;         // for debugging
    bool imported;
    uint16_t index;

    FrameGraphRenderTarget::Descriptor desc;

    void create(FrameGraph& fg, DriverApi& driver, TargetFlags const& targetFlags) noexcept {
        const auto& resourceNodes = fg.mResourceNodes;
        auto& renderTargets = fg.mRenderTargets;
        uint32_t width = desc.width;
        uint32_t height = desc.height;
        TextureFormat colorFormat = TextureFormat::RGBA8; // only used for the color attachment when not specified

        if (!imported) {
            uint32_t attachments = 0;
            Handle<HwTexture> color;
            Handle<HwTexture> depth;
            fg::RenderTarget* importedRenderTarget = nullptr;

            if (desc.attachments.depth.isValid()) {
                ResourceNode const& depthNode = resourceNodes[desc.attachments.depth.index];
                Resource const* pDepthResource = depthNode.resource;
                assert(pDepthResource);
                if (pDepthResource->type == Resource::IMPORTED_RENDER_TARGET) {
                    importedRenderTarget = &renderTargets[pDepthResource->renderTargetIndex];
                } else {
                    width = pDepthResource->desc.width;
                    height = pDepthResource->desc.height;
                    depth = pDepthResource->texture;
                    attachments |= uint32_t(TargetBufferFlags::DEPTH);
                }
            }

            if (desc.attachments.color.isValid()) {
                ResourceNode const& colorNode = resourceNodes[desc.attachments.color.index];
                Resource const* pColorResource = colorNode.resource;
                assert(pColorResource);

                if (pColorResource->type == Resource::IMPORTED_RENDER_TARGET) {
                    importedRenderTarget = &renderTargets[pColorResource->renderTargetIndex];
                } else {
                    width = pColorResource->desc.width;
                    height = pColorResource->desc.height;
                    colorFormat = pColorResource->desc.format;
                    color = pColorResource->texture;
                    attachments |= uint32_t(TargetBufferFlags::COLOR);
                }
            }

            if (importedRenderTarget) {
                // this rendertarget is created from an imported resource
                assert(!attachments);
                assert(importedRenderTarget->imported);
                imported = true;
                desc = importedRenderTarget->desc;
                targetInfo.target = importedRenderTarget->targetInfo.target;
                width = desc.width;
                height = desc.height;
            } else if (attachments) {
                targetInfo.target = driver.createRenderTarget(TargetBufferFlags(attachments),
                        width, height, desc.samples, colorFormat,
                        { color }, { depth }, {});
            }
        }

        targetInfo.params = {};
        targetInfo.params.clear        = targetFlags.clear;
        targetInfo.params.discardStart = targetFlags.discardStart;
        targetInfo.params.discardEnd   = targetFlags.discardEnd;
        targetInfo.params.dependencies = targetFlags.dependencies;
        targetInfo.params.left = 0;
        targetInfo.params.bottom = 0;
        targetInfo.params.width = width;
        targetInfo.params.height = height;
    }

    void destroy(DriverApi& driver) noexcept {
        if (!imported) {
            if (targetInfo.target) {
                driver.destroyRenderTarget(targetInfo.target);
            }
        }
    }

    FrameGraphPassResources::RenderTargetInfo targetInfo;
};

struct PassNode {
    template <typename T>
    using Vector = FrameGraph::Vector<T>;

    PassNode(FrameGraph& fg, const char* name, uint32_t id, FrameGraphPassExecutor* base) noexcept
            : name(name), id(id), base(base),
              reads(fg.getArena()),
              writes(fg.getArena()),
              renderTargets(fg.getArena()),
              targetFlags(fg.getArena()),
              devirtualize(fg.getArena()),
              destroy(fg.getArena()) {
    }
    PassNode(PassNode const&) = delete;
    PassNode(PassNode&& rhs) noexcept = default;

    PassNode& operator=(PassNode const&) = delete;
    PassNode& operator=(PassNode&&) = delete;

    ~PassNode() = default;

    // for Builder
    void declareRenderTarget(RenderTarget& renderTarget) noexcept {
        renderTargets.push_back(renderTarget.index);
    }

    FrameGraphResource read(ResourceNode const& resource) {
        // don't allow multiple reads of the same resource -- it's just redundant.
        auto pos = std::find_if(reads.begin(), reads.end(),
                [&resource](FrameGraphResource cur) { return resource.index == cur.index; });
        if (pos != reads.end()) {
            return *pos;
        }

        FrameGraphResource r{ resource.index, resource.version };

        // FIXME: it shouldn't be allowed to READ + WRITE from the same pass (but: what about atomic buffers?)

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

    FrameGraphResource write(ResourceNode& resource) {
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
        FrameGraphResource r{ resource.index, resource.version };
        writes.push_back(r);
        return r;
    }

    bool isWritingTo(FrameGraphRenderTarget const& renderTarget) const noexcept {
        auto pos = std::find_if(renderTargets.begin(), renderTargets.end(),
                [index = renderTarget.index](uint16_t cur) { return index == cur; });
        return (pos != renderTargets.end());
    }

    // constants
    const char* const name;                         // our name
    const uint32_t id;                              // a unique id (only for debugging)
    std::unique_ptr<FrameGraphPassExecutor> base;   // type eraser for calling execute()

    // set by the builder
    Vector<FrameGraphResource> reads;           // resources we're reading from
    Vector<FrameGraphResource> writes;          // resources we're writing to
    Vector<uint16_t> renderTargets;             // declared renderTargets

    Vector<TargetFlags> targetFlags;

    bool hasSideEffect = false;             // whether this pass has side effects

    // computed during compile()
    Vector<uint16_t> devirtualize;          // resources we need to create before executing
    Vector<uint16_t> destroy;               // resources we need to destroy after executing
    uint32_t refCount = 0;                  // count resources that have a reference to us
};

// ------------------------------------------------------------------------------------------------
// out-of-line definitions
// ------------------------------------------------------------------------------------------------

Resource::Resource(const char* name, bool imported) noexcept
        : name(name), imported(imported) {
}

Resource::~Resource() noexcept {
    if (!imported) {
        assert(!texture);
    }
}

void Resource::create(DriverApi& driver) noexcept {
    // some sanity check
    if (!imported) {
        if (readerCount) {
            // Don't create the texture handle if this resource is never read
            // (it means it's only used as an attachment for a rendertarget)
            texture = driver.createTexture(desc.type, desc.levels, desc.format, 1,
                    desc.width, desc.height, desc.depth,
                    TextureUsage::COLOR_ATTACHMENT); // FIXME: this should be calculated automatically
        }
    }
}

void Resource::destroy(DriverApi& driver) noexcept {
    // we don't own the handles of imported resources
    if (!imported) {
        if (texture) {
            driver.destroyTexture(texture);
            texture.clear(); // needed because of noop driver
        }
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

FrameGraphResource FrameGraph::Builder::declareTexture(
        const char* name, FrameGraphResource::Descriptor const& desc) noexcept {
    ResourceNode& resource = mFrameGraph.createResource(name, desc, false);
    return { resource.index, resource.version };
}

FrameGraphRenderTarget FrameGraph::Builder::declareRenderTarget(
        const char* name, FrameGraphRenderTarget::Descriptor const& desc) noexcept {
    RenderTarget& renderTarget = mFrameGraph.createRenderTarget(name, desc, false);
    // TODO: check that desc.attachments.* are being written by this pass
    mPass.declareRenderTarget(renderTarget);
    return FrameGraphRenderTarget{ renderTarget.index };
}

FrameGraphRenderTarget FrameGraph::Builder::declareRenderTarget(FrameGraphResource texture) noexcept {
    ResourceNode* resource = mFrameGraph.getResource(texture);
    FrameGraphRenderTarget::Descriptor desc {
        .width = resource->desc.width,
        .height = resource->desc.height,
        .samples = 1,
        .attachments.color = texture
    };
    return declareRenderTarget(resource->name, desc);
}

FrameGraphResource FrameGraph::Builder::read(FrameGraphResource const& input) {
    ResourceNode* resource = mFrameGraph.getResource(input);
    if (!resource) {
        return {};
    }
    return mPass.read(*resource);
}

FrameGraphResource FrameGraph::Builder::blit(FrameGraphResource const& input) {
    ResourceNode* resource = mFrameGraph.getResource(input);
    if (!resource) {
        return {};
    }
    return mPass.read(*resource);
}

FrameGraphResource FrameGraph::Builder::write(FrameGraphResource const& output) {
    ResourceNode* resource = mFrameGraph.getResource(output);
    if (!resource) {
        return {};
    }
    return mPass.write(*resource);
}

FrameGraph::Builder& FrameGraph::Builder::sideEffect() noexcept {
    mPass.hasSideEffect = true;
    return *this;
}

// ------------------------------------------------------------------------------------------------

FrameGraphPassResources::FrameGraphPassResources(FrameGraph& fg, fg::PassNode const& pass) noexcept
        : mFrameGraph(fg), mPass(pass) {
}

Handle <HwTexture> FrameGraphPassResources::getTexture(FrameGraphResource r) const noexcept {
    Resource const* const pResource = mFrameGraph.mResourceNodes[r.index].resource;
    assert(pResource);

    // check that this FrameGraphResource is indeed used by this pass
    ASSERT_POSTCONDITION_NON_FATAL(mPass.isReadingFrom(r),
            "Pass \"%s\" doesn't declare reads to resource \"%s\" -- expect graphic corruptions",
            mPass.name, pResource->name);

    return pResource->texture;
}

FrameGraphPassResources::RenderTargetInfo const&
FrameGraphPassResources::getRenderTarget(FrameGraphRenderTarget r) const noexcept {
    fg::RenderTarget& renderTarget = mFrameGraph.mRenderTargets[r.index];

    // check that this FrameGraphRenderTarget is indeed declared by this pass
    ASSERT_POSTCONDITION_NON_FATAL(mPass.isWritingTo(r),
            "Pass \"%s\" doesn't declare rendertarget \"%s\" -- expect graphic corruptions",
            mPass.name, renderTarget.name);

    assert(renderTarget.targetInfo.target);

//    slog.d << mPass.name << ": resource = \"" << renderTarget.name << "\", flags = "
//        << io::hex
//        << renderTarget.targetInfo.params.discardStart << ", "
//        << renderTarget.targetInfo.params.discardEnd << io::endl;

    return renderTarget.targetInfo;
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
          mRenderTargets(mArena),
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

void FrameGraph::present(FrameGraphResource input) {
    struct Dummy {
    };
    addPass<Dummy>("Present",
            [&](Builder& builder, Dummy& data) {
                builder.read(input);
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

fg::RenderTarget& FrameGraph::createRenderTarget(const char* name,
        FrameGraphRenderTarget::Descriptor const& desc, bool imported) noexcept {
    auto& renderTargets = mRenderTargets;
    const uint16_t id = (uint16_t)renderTargets.size();
    renderTargets.emplace_back(name, desc, imported, id);
    return renderTargets.back();
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
        const char* name, FrameGraphRenderTarget::Descriptor const& descriptor,
        Handle<HwRenderTarget> target) {

    // Importing a render target is a bit involved. We first create the render target as well
    // as a frame graph resource. The frame graph resource references the render target for
    // later use.

    // TODO: can we do better with the discard flags? Maybe pass has parameter.
    RenderTarget& renderTarget = createRenderTarget(name, descriptor, true);
    renderTarget.targetInfo.target = target;
    renderTarget.targetInfo.params.discardStart = TargetBufferFlags::NONE;
    renderTarget.targetInfo.params.discardEnd = TargetBufferFlags::NONE;

    // create the resource that will be returned to the user
    FrameGraphResource::Descriptor desc {
        .width = descriptor.width,
        .height = descriptor.height,
    };
    ResourceNode& node = createResource(name, desc, true);

    // imported resources are created immediately
    auto& resourceRegistry = mResourceRegistry;
    resourceRegistry.emplace_back(name, true);
    Resource& resource = resourceRegistry.back();
    resource.type = Resource::IMPORTED_RENDER_TARGET;
    resource.renderTargetIndex = renderTarget.index;

    // we store the offset into the array (instead of the pointer) because the storage might
    // move between now and compile().
    node.offset = &resource - resourceRegistry.data();

    return { node.index, node.version };
}

FrameGraphResource FrameGraph::importResource(
        const char* name, FrameGraphResource::Descriptor const& descriptor,
        Handle<HwTexture> color) {
    ResourceNode& node = createResource(name, descriptor, true);

    // imported resources are created immediately
    auto& resourceRegistry = mResourceRegistry;
    resourceRegistry.emplace_back(name, true);
    Resource& resource = resourceRegistry.back();
    resource.texture = color;

    // we store the offset into the array (instead of the pointer) because the storage might
    // move between now and compile().
    node.offset = &resource - resourceRegistry.data();

    return { node.index, node.version };
}

FrameGraph& FrameGraph::compile() noexcept {
    auto& passNodes = mPassNodes;
    auto& resourceNodes = mResourceNodes;
    auto const& renderTargets = mRenderTargets;
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
    auto const first = passNodes.data();
    auto const last = passNodes.data() + passNodes.size();
    for (PassNode& pass : passNodes) {
        if (!pass.refCount) {
            assert(!pass.hasSideEffect);
            continue;
        }
        for (FrameGraphResource resource : pass.reads) {
            Resource* subResource = resourceNodes[resource.index].resource;
            // figure out which is the first pass to need this resource
            subResource->first = subResource->first ? subResource->first : &pass;
            // figure out which is the last pass to need this resource
            subResource->last = &pass;
        }
        for (FrameGraphResource resource : pass.writes) {
            Resource* subResource = resourceNodes[resource.index].resource;
            // figure out which is the first pass to need this resource
            subResource->first = subResource->first ? subResource->first : &pass;
            // figure out which is the last pass to need this resource
            subResource->last = &pass;
        }

        pass.targetFlags.resize(pass.renderTargets.size());

        for (size_t i = 0, c = pass.renderTargets.size(); i < c; ++i) {
            RenderTarget const& renderTarget = renderTargets[pass.renderTargets[i]];
            // compute this resource discard flag for this pass for this resource

            // does anyone writes to this resource before us -- if so, don't discard those buffers on enter
            // (i.e. if nobody wrote, no need to load from memory)
            uint8_t discardStart = computeDiscardFlags(
                    DiscardPhase::START, first, &pass, renderTarget);

            // does anyone reads this resource after us -- if so, don't discard those buffers on exit
            // (i.e. if noboddy is going to read, no need to write back to memory)
            uint8_t discardEnd = computeDiscardFlags(
                    DiscardPhase::END, &pass + 1, last, renderTarget);

            TargetFlags& targetFlags = pass.targetFlags[i];
            targetFlags.clear = 0;
            targetFlags.discardStart = discardStart;
            targetFlags.discardEnd = discardEnd;
            targetFlags.dependencies = 0;
        }
    }

    // add resource to devirtualize or destroy to the corresponding list for each active pass
    for (size_t index = 0, c = resourceRegistry.size() ; index < c ; index++) {
        Resource& resource = resourceRegistry[index];
        assert(!resource.first == !resource.last);
        if (resource.readerCount && resource.first && resource.last) {
            resource.first->devirtualize.push_back((uint16_t)index);
            resource.last->destroy.push_back((uint16_t)index);
        }
    }

    return *this;
}

uint8_t FrameGraph::computeDiscardFlags(DiscardPhase phase,
        PassNode const* curr, PassNode const* first,
        RenderTarget const& renderTarget) {
    auto& resourceNodes = mResourceNodes;
    uint8_t discardFlags = TargetBufferFlags::ALL;
    // for each pass...
    while (discardFlags && curr != first) {
        PassNode const& pass = *curr++;
        // TODO: maybe find a more efficient way of figuring this out
        // for each resource written or read...
        for (FrameGraphResource cur : ((phase == DiscardPhase::START) ? pass.writes : pass.reads)) {
            // for all possible attachments...
            if (resourceNodes[cur.index].resource ==
                resourceNodes[renderTarget.desc.attachments.color.index].resource) {
                discardFlags &= ~TargetBufferFlags::COLOR;
            }
            if (resourceNodes[cur.index].resource ==
                resourceNodes[renderTarget.desc.attachments.depth.index].resource) {
                discardFlags &= ~TargetBufferFlags::DEPTH;
            }
            if (!discardFlags) {
                break;
            }
        }
    }
    return discardFlags;
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

        // FIXME: this should work like resource, we need to know their lifetime
        // create the rendertargets
        for (size_t i = 0, c = node.renderTargets.size(); i < c; ++i) {
            mRenderTargets[node.renderTargets[i]].create(*this, driver, node.targetFlags[i]);
        }

        // execute the pass
        FrameGraphPassResources resources(*this, node);
        node.base->execute(resources, driver);

        // FIXME: use a cache or something
        // destroy the rendertargets
        for (size_t id : node.renderTargets) {
            mRenderTargets[id].destroy(driver);
        }

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
