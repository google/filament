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
#include "FrameGraphResource.h"

#include "private/backend/CommandStream.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/Panic.h>
#include <utils/Log.h>

using namespace utils;

namespace filament {

using namespace backend;
using namespace fg;

// ------------------------------------------------------------------------------------------------

namespace fg {

struct Alias { //4
    FrameGraphResource from, to;
};

struct VirtualResource {
    VirtualResource() noexcept = default;
    VirtualResource(VirtualResource const&) = default;
    virtual void create(FrameGraph& fg, DriverApi& driver) noexcept = 0;
    virtual void destroy(FrameGraph& fg, DriverApi& driver) noexcept = 0;
    virtual ~VirtualResource();

    // computed during compile()
    PassNode* first = nullptr;              // pass that needs to instantiate the resource
    PassNode* last = nullptr;               // pass that can destroy the resource
};

VirtualResource::~VirtualResource() = default;

struct Resource final : public VirtualResource { // 72
    enum Type {
        TEXTURE
    };

    Resource(const char* name, uint16_t id,
            Type type, FrameGraphResource::Descriptor desc, bool imported) noexcept;
    Resource(Resource const&) = delete;
    Resource& operator=(Resource const&) = delete;
    ~Resource() noexcept override;

    // concrete resource -- set when the resource is created
    void create(FrameGraph& fg, DriverApi& driver) noexcept override;
    void destroy(FrameGraph& fg, DriverApi& driver) noexcept override;

    // constants
    const char* const name;
    const uint16_t id;            // for debugging and graphing
    const Type type;
    bool imported;

    // updated by builder
    uint8_t version = 0;
    TextureUsage usage = (TextureUsage)0;
    bool needsTexture = false;
    FrameGraphResource::Descriptor desc;

    // computed during compile()
    uint32_t refs = 0;                      // final reference count

    // set during execute(), as needed
    backend::Handle<backend::HwTexture> texture;
};

struct ResourceNode { // 24
    ResourceNode(Resource* resource, uint8_t version) noexcept
            : resource(resource), version(version) { }
    ResourceNode(ResourceNode const&) = delete;
    ResourceNode(ResourceNode&&) noexcept = default;
    ResourceNode& operator=(ResourceNode const&) = delete;

    // updated during compile()
    Resource* resource;             // actual (aliased) resource data
    PassNode* writer = nullptr;     // writer to this node
    uint32_t readerCount = 0;       // # of passes reading from this resource

    // updated by builder
    static constexpr uint16_t UNINITIALIZED = std::numeric_limits<uint16_t>::max();
    uint16_t renderTargetIndex = UNINITIALIZED;      // used to retrieve the RT infos

    // constants
    const uint8_t version;          // version of the resource when the node was created
};

struct RenderTargetResource final : public VirtualResource {  // 104

    RenderTargetResource(
            FrameGraphRenderTarget::Descriptor const& desc, bool imported,
            TargetBufferFlags targets,
            uint32_t width, uint32_t height, TextureFormat format)
            : desc(desc), imported(imported),
              attachments(targets), format(format), width(width), height(height)  {
        targetInfo.params.viewport   = desc.viewport;
        // if Descriptor was initialized with default values, set the viewport to width/height
        if (targetInfo.params.viewport.width == 0 && targetInfo.params.viewport.height == 0) {
            targetInfo.params.viewport.width  = width;
            targetInfo.params.viewport.height = height;
        }
    }

    RenderTargetResource(RenderTargetResource const&) = delete;
    RenderTargetResource(RenderTargetResource&&) noexcept = default;
    RenderTargetResource& operator=(RenderTargetResource const&) = delete;
    ~RenderTargetResource() override;

    // cache key
    const FrameGraphRenderTarget::Descriptor desc;
    const bool imported;

    // render target creation info
    TargetBufferFlags attachments;
    TextureFormat format;
    uint32_t width;
    uint32_t height;

    // updated during execute with the current pass' discard flags
    FrameGraphPassResources::RenderTargetInfo targetInfo;

    void create(FrameGraph& fg, DriverApi& driver) noexcept override {
        if (!imported) {
            if (attachments) {
                FrameGraph::Vector<ResourceNode> const& resourceNodes = fg.mResourceNodes;

                // devirtualize our texture handles. By this point these handles have been
                // remapped to their alias if any.
                backend::Handle<backend::HwTexture> textures[FrameGraphRenderTarget::Attachments::COUNT];
                for (size_t i = 0, c = desc.attachments.textures.size(); i < c; i++) {
                    FrameGraphResource r = desc.attachments.textures[i];
                    if (r.isValid()) {
                        ResourceNode const& node = resourceNodes[r.index];
                        assert(node.resource);
                        textures[i] = node.resource->texture;
                    }
                }

                // create the concrete rendertarget
                targetInfo.target = driver.createRenderTarget(attachments,
                        width, height, desc.samples, format,
                        { textures[0] }, { textures[1] }, {});
            }
        }
    }

    void destroy(FrameGraph&, DriverApi& driver) noexcept override {
        if (!imported) {
            if (targetInfo.target) {
                driver.destroyRenderTarget(targetInfo.target);
                targetInfo.target.clear();
            }
        }
    }
};

RenderTargetResource::~RenderTargetResource() = default;


struct RenderTarget { // 32
    RenderTarget(const char* name,
            FrameGraphRenderTarget::Descriptor const& desc, bool imported, uint16_t index) noexcept
            : name(name), imported(imported), index(index), desc(desc) {
    }
    RenderTarget(RenderTarget const&) = delete;
    RenderTarget(RenderTarget&&) noexcept = default;
    RenderTarget& operator=(RenderTarget const&) = delete;

    // constants
    const char* const name;         // for debugging
    bool imported;
    uint16_t index;

    // set by builder
    FrameGraphRenderTarget::Descriptor desc;
    RenderPassFlags userTargetFlags{};

    // set in compile
    RenderPassFlags targetFlags{};
    RenderTargetResource* cache = nullptr;

    void resolve(FrameGraph& fg) noexcept {
        const auto& resourceNodes = fg.mResourceNodes;
        auto& renderTargetCache = fg.mRenderTargetCache;

        // find a matching rendertarget
        auto pos = std::find_if(renderTargetCache.begin(), renderTargetCache.end(),
                [this, &fg](auto const& rt) {
                    return fg.equals(rt->desc, desc);
                });

        if (pos != renderTargetCache.end()) {
            cache = pos->get();
            cache->targetInfo.params.flags.clear |= userTargetFlags.clear;
        } else {
            uint8_t attachments = 0;
            uint32_t width = 0;
            uint32_t height = 0;
            TextureFormat colorFormat = {};
            backend::Handle<backend::HwTexture> textures[FrameGraphRenderTarget::Attachments::COUNT];

            static constexpr TargetBufferFlags flags[] = {
                    TargetBufferFlags::COLOR,
                    TargetBufferFlags::DEPTH,
                    TargetBufferFlags::STENCIL };

            bool relaxed = true;
            uint32_t minWidth = std::numeric_limits<uint32_t>::max();
            uint32_t maxWidth = 0;
            uint32_t minHeight = std::numeric_limits<uint32_t>::max();
            uint32_t maxHeight = 0;

            for (FrameGraphResource const& attachment : desc.attachments.textures) {
                size_t index = &attachment - desc.attachments.textures.data();
                if (attachment.isValid()) {
                    Resource const* const pResource = resourceNodes[attachment.index].resource;
                    assert(pResource);

                    attachments |= flags[index];

                    // figure out the min/max dimensions across all attachments
                    minWidth  = std::min(minWidth,  pResource->desc.width);
                    maxWidth  = std::max(maxWidth,  pResource->desc.width);
                    minHeight = std::min(minHeight, pResource->desc.height);
                    maxHeight = std::max(maxHeight, pResource->desc.height);

                    textures[index] = pResource->texture;
                    if (index == FrameGraphRenderTarget::Attachments::COLOR) {
                        colorFormat = pResource->desc.format;
                    }

                    // TODO: if the resource doesn't need an actual texture, we may be able to
                    //       increase its size.
                    relaxed = relaxed && pResource->desc.relaxed;
                }
            }

            if (attachments) {
                if (!relaxed && (minWidth == maxWidth && minHeight == maxHeight)) {
                    // All attachments' size match, we're good to go.
                    // We don't take this path if all attachments allow to tweak their size,
                    // as in that case we can round-up to take advantage of caching in the driver.
                    width = maxWidth;
                    height = maxHeight;
                } else {
                    // Some attachments might not allow to be resized (i.e. the non relaxed textures),
                    // use the largest size.
                    // We also round dimensions up to avoid lots of small resizes in the driver.
                    // (this is assuming the driver uses some cache internally).
                    width  = (maxWidth  + 31) & ~31;
                    height = (maxHeight + 31) & ~31;

                    // and update the resource's descriptors that allow it
                    for (FrameGraphResource const& attachment : desc.attachments.textures) {
                        if (attachment.isValid()) {
                            Resource* const pResource = resourceNodes[attachment.index].resource;
                            if (pResource->desc.relaxed || !pResource->needsTexture) {
                                pResource->desc.width = width;
                                pResource->desc.height = height;
                            }
                        }
                    }
                    // at the end of this process, it's possible that some attachments won't match
                    // but there is nothing more we can do, as these attachments require a texture
                    // and don't have the relaxed flag.
                }

                // create the cache entry
                RenderTargetResource* pRenderTargetResource =
                        fg.mArena.make<RenderTargetResource>(desc, imported,
                                TargetBufferFlags(attachments), width, height, colorFormat);
                renderTargetCache.emplace_back(pRenderTargetResource, fg);
                cache = pRenderTargetResource;
                cache->targetInfo.params.flags.clear |= userTargetFlags.clear;
            }
        }
    }
};

struct PassNode { // 200
    template <typename T>
    using Vector = FrameGraph::Vector<T>;

    PassNode(FrameGraph& fg, const char* name, uint32_t id, FrameGraphPassExecutor* base) noexcept
            : name(name), id(id), base(base, fg),
              reads(fg.getArena()),
              writes(fg.getArena()),
              renderTargets(fg.getArena()),
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
        renderTargets.push_back(&renderTarget);
    }

    FrameGraphResource read(FrameGraph& fg, FrameGraphResource const& handle, bool isRenderTarget = false) {
        ResourceNode const& node = fg.getResource(handle);

        if (!isRenderTarget) {
            node.resource->needsTexture = true;
        }

        // don't allow multiple reads of the same resource -- it's just redundant.
        auto pos = std::find_if(reads.begin(), reads.end(),
                [&handle](FrameGraphResource cur) { return handle.index == cur.index; });
        if (pos != reads.end()) {
            return *pos;
        }

        // just record that we're reading from this resource (at the given version)
        FrameGraphResource r{ handle.index };
        reads.push_back(r);
        return r;
    }

    bool isReadingFrom(FrameGraphResource resource) const noexcept {
        auto pos = std::find_if(reads.begin(), reads.end(),
                [resource](FrameGraphResource cur) { return resource.index == cur.index; });
        return (pos != reads.end());
    }

    FrameGraphResource write(FrameGraph& fg, const FrameGraphResource& handle) {
        ResourceNode const& node = fg.getResource(handle);

        // don't allow multiple writes of the same resource -- it's just redundant.
        auto pos = std::find_if(writes.begin(), writes.end(),
                [&handle](FrameGraphResource cur) { return handle.index == cur.index; });
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

        ++node.resource->version;

        // writing to an imported resource should count as a side-effect
        if (node.resource->imported) {
            hasSideEffect = true;
        }

        FrameGraphResource r = fg.createResourceNode(node.resource);

        // record the write
        //FrameGraphResource r{ resource->index, resource->version };
        writes.push_back(r);
        return r;
    }

    // constants
    const char* const name;                             // our name
    const uint32_t id;                                  // a unique id (only for debugging)
    FrameGraph::UniquePtr<FrameGraphPassExecutor> base; // type eraser for calling execute()

    // set by the builder
    Vector<FrameGraphResource> reads;               // resources we're reading from
    Vector<FrameGraphResource> writes;              // resources we're writing to
    Vector<RenderTarget*> renderTargets;

    // computed during compile()
    Vector<VirtualResource*> devirtualize;         // resources we need to create before executing
    Vector<VirtualResource*> destroy;              // resources we need to destroy after executing
    uint32_t refCount = 0;                  // count resources that have a reference to us

    // set by the builder
    bool hasSideEffect = false;             // whether this pass has side effects
};

// ------------------------------------------------------------------------------------------------
// out-of-line definitions
// ------------------------------------------------------------------------------------------------

Resource::Resource(const char* name, uint16_t id,
        Type type, FrameGraphResource::Descriptor desc, bool imported) noexcept
        : name(name), id(id), type(type), imported(imported), desc(desc) {
}

Resource::~Resource() noexcept {
    if (!imported) {
        assert(!texture);
    }
}

void Resource::create(FrameGraph&, DriverApi& driver) noexcept {
    // some sanity check
    if (!imported) {
        if (needsTexture) {
            assert(usage);
            // (it means it's only used as an attachment for a rendertarget)
            texture = driver.createTexture(desc.type, desc.levels, desc.format, 1,
                    desc.width, desc.height, desc.depth, usage);
        }
    }
}

void Resource::destroy(FrameGraph&, DriverApi& driver) noexcept {
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

FrameGraphResource FrameGraph::Builder::createTexture(
        const char* name, FrameGraphResource::Descriptor const& desc) noexcept {
    Resource* resource = mFrameGraph.createResource(name, desc, false);
    return mFrameGraph.createResourceNode(resource);
}

FrameGraph::Builder::Attachments FrameGraph::Builder::useRenderTarget(const char* name,
        FrameGraphRenderTarget::Descriptor const& desc, TargetBufferFlags clearFlags) noexcept {

    // TODO: add support for cubemaps and arrays

    // TODO: enforce that we can't have a resource used in 2 rendertarget in the same pass

    Attachments rt{};
    FrameGraph& fg = mFrameGraph;

    RenderTarget& renderTarget = fg.createRenderTarget(name, desc, false);
    renderTarget.userTargetFlags.clear = clearFlags;

    mPass.declareRenderTarget(renderTarget);

    // update the referenced textures usage flags
    static constexpr TextureUsage usages[] = {
            TextureUsage::COLOR_ATTACHMENT,
            TextureUsage::DEPTH_ATTACHMENT,
            TextureUsage::STENCIL_ATTACHMENT
    };
    for (FrameGraphResource const& attachment : desc.attachments.textures) {
        const size_t index = &attachment - desc.attachments.textures.data();
        if (attachment.isValid()) {
            // using a resource as a RT implies reading (i.e. adds a reference to that resource) from it
            rt.textures[index] = mPass.read(fg, attachment, true);
            // using a resource as a RT implies writing (i.e. adds a reference to the pass) into it
            rt.textures[index] = mPass.write(fg, rt.textures[index]);

            ResourceNode& node = fg.getResource(rt.textures[index]);
            uint8_t usage = node.resource->usage;
            usage |= usages[index];
            node.resource->usage = TextureUsage(usage);

            // renderTargetIndex is used to retrieve the Descriptor
            node.renderTargetIndex = renderTarget.index;
        }
    }
    return rt;
}

FrameGraph::Builder::Attachments FrameGraph::Builder::useRenderTarget(
        FrameGraphResource texture, TargetBufferFlags clearFlags) noexcept {
    uint8_t samples = 1;
    if (isAttachment(texture)) {
        samples = getRenderTargetDescriptor(texture).samples;
    }
    ResourceNode& resourceNode = mFrameGraph.getResource(texture);
    Resource* pResource = resourceNode.resource;
    assert(pResource);
    FrameGraphRenderTarget::Descriptor desc { .attachments.color = texture, .samples = samples };
    return useRenderTarget(pResource->name, desc, clearFlags);
}

FrameGraphResource FrameGraph::Builder::read(FrameGraphResource const& input, bool doesntNeedTexture) {
    return mPass.read(mFrameGraph, input, doesntNeedTexture);
}

FrameGraphResource FrameGraph::Builder::read(FrameGraphResource const& input) {
    return mPass.read(mFrameGraph, input);
}

FrameGraphResource FrameGraph::Builder::write(FrameGraphResource const& output) {
    return mPass.write(mFrameGraph, output);
}

FrameGraph::Builder& FrameGraph::Builder::sideEffect() noexcept {
    mPass.hasSideEffect = true;
    return *this;
}

bool FrameGraph::Builder::isAttachment(FrameGraphResource resource) const noexcept {
    ResourceNode& node = mFrameGraph.getResource(resource);
    return node.renderTargetIndex != ResourceNode::UNINITIALIZED;
}

FrameGraphRenderTarget::Descriptor const&
FrameGraph::Builder::getRenderTargetDescriptor(FrameGraphResource attachment) const {
    FrameGraph& fg = mFrameGraph;
    ResourceNode& node = fg.getResource(attachment);
    ASSERT_POSTCONDITION(node.renderTargetIndex != ResourceNode::UNINITIALIZED,
            "Resource \"%s\" isn't a render target attachment", node.resource->name);
    assert(node.renderTargetIndex < fg.mRenderTargets.size());
    return fg.mRenderTargets[node.renderTargetIndex].desc;
}

// ------------------------------------------------------------------------------------------------

FrameGraphPassResources::FrameGraphPassResources(FrameGraph& fg, fg::PassNode const& pass) noexcept
        : mFrameGraph(fg), mPass(pass) {
}

backend::Handle<backend::HwTexture> FrameGraphPassResources::getTexture(FrameGraphResource r) const noexcept {
    Resource const* const pResource = mFrameGraph.mResourceNodes[r.index].resource;
    assert(pResource);

    // check that this FrameGraphResource is indeed used by this pass
    ASSERT_POSTCONDITION_NON_FATAL(mPass.isReadingFrom(r),
            "Pass \"%s\" doesn't declare reads to resource \"%s\" -- expect graphic corruptions",
            mPass.name, pResource->name);

    return pResource->texture;
}

FrameGraphPassResources::RenderTargetInfo
FrameGraphPassResources::getRenderTarget(FrameGraphResource r) const noexcept {

    FrameGraphPassResources::RenderTargetInfo info{};
    FrameGraph& fg = mFrameGraph;
    auto const& resourceNodes = fg.mResourceNodes;
    Resource const* const pResource = resourceNodes[r.index].resource;

    // find a rendertarget in this pass that has this resource has attachment

    // TODO: for cubemaps/arrays, we'll need to be able to specifyt he face/index

    for (RenderTarget const* renderTarget : mPass.renderTargets) {
        auto const& desc = renderTarget->desc;
        auto pos = std::find_if(
                desc.attachments.textures.begin(),
                desc.attachments.textures.end(),
                [pResource, &resourceNodes](FrameGraphResource const& r) {
                    return r.isValid() && resourceNodes[r.index].resource == pResource;
                });
        if (pos != std::end(desc.attachments.textures)) {
            assert(renderTarget->cache);
            info = renderTarget->cache->targetInfo;
            // overwrite discard flags with the per-rendertarget (per-pass) computed value
            info.params.flags.discardStart = renderTarget->targetFlags.discardStart;
            info.params.flags.discardEnd   = renderTarget->targetFlags.discardEnd;
            info.params.flags.dependencies = renderTarget->targetFlags.dependencies;
            assert(info.target);
            break;
        }
    }
    
    // check that this FrameGraphRenderTarget is indeed declared by this pass
    ASSERT_POSTCONDITION_NON_FATAL(info.target,
            "Pass \"%s\" doesn't declare a rendertarget using \"%s\" -- expect graphic corruptions",
            mPass.name, fg.getResource(r).resource->name);
    
//    slog.d << mPass.name << ": resource = \"" << renderTarget.name << "\", flags = "
//        << io::hex
//        << renderTarget.targetInfo.params.discardStart << ", "
//        << renderTarget.targetInfo.params.discardEnd << io::endl;

    return info;
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
          mAliases(mArena),
          mResourceRegistry(mArena),
          mRenderTargetCache(mArena) {
//    slog.d << "PassNode: " << sizeof(PassNode) << io::endl;
//    slog.d << "ResourceNode: " << sizeof(ResourceNode) << io::endl;
//    slog.d << "Resource: " << sizeof(Resource) << io::endl;
//    slog.d << "RenderTarget: " << sizeof(RenderTarget) << io::endl;
//    slog.d << "RenderTargetResource: " << sizeof(RenderTargetResource) << io::endl;
//    slog.d << "Alias: " << sizeof(Alias) << io::endl;
//    slog.d << "Vector: " << sizeof(Vector<fg::PassNode>) << io::endl;
}

FrameGraph::~FrameGraph() = default;

bool FrameGraph::isValid(FrameGraphResource handle) const noexcept {
    if (!handle.isValid()) return false;
    auto const& registry = mResourceNodes;
    assert(handle.index < registry.size());
    ResourceNode const& node = registry[handle.index];
    return node.version == node.resource->version;
}

FrameGraphResource FrameGraph::createResourceNode(fg::Resource* resource) noexcept {
    auto& resourceNodes = mResourceNodes;
    size_t index = resourceNodes.size();
    resourceNodes.emplace_back(resource, resource->version);
    return FrameGraphResource{ (uint16_t)index };
}

FrameGraphResource FrameGraph::moveResource(FrameGraphResource from, FrameGraphResource to) {
    // this is just used to validate the 'to' handle
    getResource(to);
    // validate and rename the 'from' handle
    ResourceNode const& node = getResource(from);
    ++node.resource->version;
    mAliases.push_back({from, to});
    return createResourceNode(node.resource);
}

void FrameGraph::present(FrameGraphResource input) {
    addPass<std::tuple<>>("Present",
            [&](Builder& builder, auto& data) {
                builder.read(input, true);
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
        FrameGraphRenderTarget::Descriptor const& desc, bool imported) noexcept {
    auto& renderTargets = mRenderTargets;
    const uint16_t id = (uint16_t)renderTargets.size();
    renderTargets.emplace_back(name, desc, imported, id);
    return renderTargets.back();
}

Resource* FrameGraph::createResource(const char* name,
        FrameGraphResource::Descriptor const& desc, bool imported) noexcept {
    Resource* resource = mArena.make<Resource>(name, mId++, Resource::Type::TEXTURE, desc, imported);
    mResourceRegistry.emplace_back(resource, *this);
    return resource;
}

ResourceNode& FrameGraph::getResource(FrameGraphResource r) {
    auto& resourceNodes = mResourceNodes;

    ASSERT_POSTCONDITION(r.isValid(), "using an uninitialized resource handle");

    assert(r.index < resourceNodes.size());
    ResourceNode& node = resourceNodes[r.index];
    assert(node.resource);

    ASSERT_POSTCONDITION(node.resource->version == node.version,
            "using an invalid resource handle (version=%u) for resource=\"%s\" (id=%u, version=%u)",
            node.resource->version, node.resource->name, node.resource->id, node.version);

    return node;
}
FrameGraphResource::Descriptor* FrameGraph::getDescriptor(FrameGraphResource r) {
    ResourceNode& node = getResource(r);
    assert(node.resource);
    return &(node.resource->desc);
}

bool FrameGraph::equals(FrameGraphRenderTarget::Descriptor const& lhs,
        FrameGraphRenderTarget::Descriptor const& rhs) const noexcept {
    const Vector<filament::fg::ResourceNode>& resourceNodes = mResourceNodes;
    return std::equal(
            lhs.attachments.textures.begin(), lhs.attachments.textures.end(),
            rhs.attachments.textures.begin(), rhs.attachments.textures.end(),
            [&resourceNodes](FrameGraphResource lhs, FrameGraphResource rhs) {
                if (lhs.index == rhs.index) {
                    // obviously resources match if they're the same
                    return true;
                }
                if (lhs.isValid() && rhs.isValid()) {
                    if (resourceNodes[lhs.index].resource == resourceNodes[rhs.index].resource) {
                        // they also match if they're the same concrete resource
                        return true;
                    }
                }
                if (!rhs.isValid()) {
                    // it's okay if the cached RT has more attachments than we require
                    return true;
                }
                return false;
            }) && lhs.samples == rhs.samples;
}

FrameGraphResource FrameGraph::importResource(const char* name,
        FrameGraphRenderTarget::Descriptor descriptor,
        backend::Handle<backend::HwRenderTarget> target, uint32_t width, uint32_t height,
        TargetBufferFlags discardStart, TargetBufferFlags discardEnd) {

    // TODO: for now we don't allow imported targets to specify textures
    assert(std::all_of(
            descriptor.attachments.textures.begin(),
            descriptor.attachments.textures.end(),
            [](FrameGraphResource t) { return !t.isValid(); }));

    // create the resource that will be returned to the user
    FrameGraphResource::Descriptor desc{ .width = width, .height = height };

    Resource* resource = createResource(name, desc, true);
    FrameGraphResource rt = createResourceNode(resource);
    descriptor.attachments.textures[0] = rt;

    // Populate the cache with a RenderTargetResource
    // create a cache entry
    RenderTargetResource* pRenderTargetResource = mArena.make<RenderTargetResource>(descriptor, true,
            TargetBufferFlags::COLOR, width, height, TextureFormat{});
    pRenderTargetResource->targetInfo.target = target;
    mRenderTargetCache.emplace_back(pRenderTargetResource, *this);

    // NOTE: we don't even need to create a fg::RenderTarget, all is needed is a cache entry
    // so that, resolve() will find us.

    return rt;
}

FrameGraphResource FrameGraph::importResource(
        const char* name, FrameGraphResource::Descriptor const& descriptor,
        backend::Handle<backend::HwTexture> color) {
    Resource* resource = createResource(name, descriptor, true);
    resource->texture = color;
    return createResourceNode(resource);
}

uint8_t FrameGraph::computeDiscardFlags(DiscardPhase phase,
        PassNode const* curr, PassNode const* first, RenderTarget const& renderTarget) {
    auto& resourceNodes = mResourceNodes;
    uint8_t discardFlags = TargetBufferFlags::ALL;

    static constexpr TargetBufferFlags flags[] = {
            TargetBufferFlags::COLOR,
            TargetBufferFlags::DEPTH,
            TargetBufferFlags::STENCIL };

    auto const& desc = renderTarget.cache->desc;

    // for each pass...
    while (discardFlags && curr != first) {
        PassNode const& pass = *curr++;
        // TODO: maybe find a more efficient way of figuring this out
        // for each resource written or read...
        for (FrameGraphResource cur : ((phase == DiscardPhase::START) ? pass.writes : pass.reads)) {
            // for all possible attachments of our renderTarget...
            Resource const* const pResource = resourceNodes[cur.index].resource;
            for (FrameGraphResource const& attachment : desc.attachments.textures) {
                if (attachment.isValid() && resourceNodes[attachment.index].resource == pResource) {
                    // we can't discard this attachment since it's read/written
                    size_t index = &attachment - desc.attachments.textures.data();
                    discardFlags &= ~flags[index];
                }
            }
            if (!discardFlags) {
                break;
            }
        }
    }

    if (phase == DiscardPhase::START) {
        // clear implies discarding the content of the buffer
        discardFlags |= (renderTarget.userTargetFlags.clear & TargetBufferFlags::ALL);
    }

    if (renderTarget.imported) {
        // we never discard more than the user flags
        if (phase == DiscardPhase::START) {
            discardFlags &= renderTarget.userTargetFlags.discardStart;
        }
        if (phase == DiscardPhase::END) {
            discardFlags &= renderTarget.userTargetFlags.discardEnd;
        }
    }

    return discardFlags;
}

FrameGraph& FrameGraph::compile() noexcept {
    Vector<fg::PassNode>& passNodes = mPassNodes;
    Vector<fg::ResourceNode>& resourceNodes = mResourceNodes;
    Vector<UniquePtr<fg::Resource>>& resourceRegistry = mResourceRegistry;
    Vector<UniquePtr<RenderTargetResource>>& renderTargetCache = mRenderTargetCache;

    /*
     * remap aliased resources
     */

    if (!mAliases.empty()) {
        Vector<filament::fg::RenderTarget>& renderTargets = mRenderTargets;
        Vector<FrameGraphResource> sratch(mArena); // keep out of loops to avoid reallocations
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
                ResourceNode const& node = resourceNodes[textures[0].index];
                if (node.resource->imported && node.resource == from.resource) {
                    for (size_t i = 1; i < textures.size(); ++i) {
                        textures[i] = {};
                    }
                }
            }

            for (PassNode& pass : passNodes) {
                // passes that were reading from "from node", now read from "to node" as well
                for (FrameGraphResource handle : pass.reads) {
                    if (handle.index == alias.from.index) {
                        sratch.push_back(alias.to.index);
                    }
                }
                pass.reads.insert(pass.reads.end(), sratch.begin(), sratch.end());
                sratch.clear();

                // Passes that were writing to "from node", no longer do
                pass.writes.erase(
                        std::remove_if(pass.writes.begin(), pass.writes.end(),
                                [&alias](auto handle) { return handle.index == alias.from.index; }),
                        pass.writes.end());
            }
        }

        // remove duplicates that might have been created when aliasing
        for (PassNode& pass : passNodes) {
            std::sort(pass.reads.begin(), pass.reads.end());
            pass.reads.erase(std::unique(pass.reads.begin(), pass.reads.end()), pass.reads.end());
        }
    }

    /*
     * compute passes and resource reference counts
     */

    for (PassNode& pass : passNodes) {
        // compute passes reference counts (i.e. resources we're writing to)
        pass.refCount = (uint32_t)pass.writes.size() + (uint32_t)pass.hasSideEffect;

        // compute resources reference counts (i.e. resources we're reading from)
        for (FrameGraphResource resource : pass.reads) {
            // add a reference for each pass that reads from this resource
            ResourceNode& node = resourceNodes[resource.index];
            node.readerCount++;
        }

        // set the writers
        for (FrameGraphResource resource : pass.writes) {
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
                for (FrameGraphResource resource : reads) {
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

    // resolve render targets
    for (PassNode& pass : passNodes) {
        for (RenderTarget* pRenderTarget : pass.renderTargets) {
            pRenderTarget->resolve(*this);
        }
    }

    /*
     * compute first/last users for active passes
     */

    auto const first = passNodes.data();
    auto const last = passNodes.data() + passNodes.size();
    for (PassNode& pass : passNodes) {
        if (!pass.refCount) {
            assert(!pass.hasSideEffect);
            continue;
        }
        for (FrameGraphResource resource : pass.reads) {
            VirtualResource* const pResource = resourceNodes[resource.index].resource;
            // figure out which is the first pass to need this resource
            pResource->first = pResource->first ? pResource->first : &pass;
            // figure out which is the last pass to need this resource
            pResource->last = &pass;
        }
        for (FrameGraphResource resource : pass.writes) {
            VirtualResource* const pResource = resourceNodes[resource.index].resource;
            // figure out which is the first pass to need this resource
            pResource->first = pResource->first ? pResource->first : &pass;
            // figure out which is the last pass to need this resource
            pResource->last = &pass;
        }
        for (RenderTarget* const pRenderTarget : pass.renderTargets) {
            VirtualResource* const pResource = pRenderTarget->cache;
            // figure out which is the first pass to need this resource
            pResource->first = pResource->first ? pResource->first : &pass;
            // figure out which is the last pass to need this resource
            pResource->last = &pass;

            // compute this resource discard flag for this pass for this resource

            // does anyone writes to this resource before us -- if so, don't discard those buffers on enter
            // (i.e. if nobody wrote, no need to load from memory)
            uint8_t discardStart = computeDiscardFlags(
                    DiscardPhase::START, first, &pass, *pRenderTarget);

            // does anyone reads this resource after us -- if so, don't discard those buffers on exit
            // (i.e. if nobody is going to read, no need to write back to memory)
            uint8_t discardEnd = computeDiscardFlags(
                    DiscardPhase::END, &pass + 1, last, *pRenderTarget);

            pRenderTarget->targetFlags = {
                    .clear = 0,  // this is eventually set by the user
                    .discardStart = discardStart,
                    .discardEnd = discardEnd,
                    .dependencies = 0
            };
        }
    }

    // add resource to de-virtualize or destroy to the corresponding list for each active pass
    for (UniquePtr<fg::Resource> const& resource : resourceRegistry) {
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

void FrameGraph::execute(DriverApi& driver) noexcept {
    for (PassNode const& node : mPassNodes) {
        if (!node.refCount) continue;
        assert(node.base);

        // create concrete resources and rendertargets
        for (VirtualResource* resource : node.devirtualize) {
            resource->create(*this, driver);
        }

        // execute the pass
        FrameGraphPassResources resources(*this, node);
        node.base->execute(resources, driver);

        // destroy concrete resources
        for (VirtualResource* resource : node.destroy) {
            resource->destroy(*this, driver);
        }
    }

    // reset the frame graph state
    mPassNodes.clear();
    mResourceNodes.clear();
    mResourceRegistry.clear();
    mAliases.clear();
    mRenderTargetCache.clear();
    mId = 0;
}

void FrameGraph::export_graphviz(utils::io::ostream& out) {
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
        Resource const* subresource = node.resource;
        out << "\"R" << node.resource->id << "_" << node.version << "\""
               "[label=\"" << node.resource->name << "\\n(version: " << node.version << ")"
               "\\nid:" << node.resource->id <<
               "\\nrefs:" << node.resource->refs << ", texture: " << node.resource->needsTexture <<
               "\", style=filled, fillcolor="
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
            out << "R" << registry[writer.index].resource->id << "_" << registry[writer.index].version << " ";
        }
        out << "} [color=red2]\n";
    }

    // connect resources to passes
    out << "\n";
    for (ResourceNode const& node : registry) {
        out << "R" << node.resource->id << "_" << node.version << " -> { ";

        // who reads us...
        for (PassNode const& pass : frameGraphPasses) {
            for (FrameGraphResource const& read : pass.reads) {
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
            out << "R" << registry[alias.from.index].resource->id << "_" << registry[alias.from.index].version << " -> ";
            out << "R" << registry[alias.to.index].resource->id << "_" << registry[alias.to.index].version;
            out << " [color=yellow, style=dashed]\n";
        }
    }

    out << "}" << utils::io::endl;
}


} // namespace filament
