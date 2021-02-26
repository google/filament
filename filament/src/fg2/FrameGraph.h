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

#ifndef TNT_FILAMENT_FG2_FRAMEGRAPH_H
#define TNT_FILAMENT_FG2_FRAMEGRAPH_H

#include "fg2/Blackboard.h"
#include "fg2/FrameGraphId.h"
#include "fg2/FrameGraphPass.h"
#include "fg2/FrameGraphRenderPass.h"
#include "fg2/FrameGraphTexture.h"

#include "fg2/details/DependencyGraph.h"
#include "fg2/details/Resource.h"
#include "fg2/details/Utilities.h"

#include "details/Allocators.h"

#include "private/backend/DriverApiForward.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <functional>
#include <vector>

namespace filament {

class ResourceAllocatorInterface;

class FrameGraphPassExecutor;
class PassNode;
class ResourceNode;
class VirtualResource;

class FrameGraph {
public:

    class Builder {
    public:
        Builder(Builder const&) = delete;
        Builder& operator=(Builder const&) = delete;

        /**
         * Declare a FrameGraphRenderPass for this pass. All subresource handles get new versions
         * after this call. The new values are available in the returned FrameGraphRenderPass
         * structure.
         * declareRenderPass() doesn't assume a read() or write() for its attachments, these must
         * be issued separately before calling declareRenderPass().
         *
         * @param name  A pointer to a null terminated string.
         *              The pointer lifetime must extend beyond execute()
         * @param desc  Descriptor for the FrameGraphRenderPass.
         * @return      An index to retrieve the concrete FrameGraphRenderPass in the execute phase.
         */
        uint32_t declareRenderPass(const char* name,
                FrameGraphRenderPass::Descriptor const& desc);

        /**
         * Helper to easily declare a FrameGraphRenderPass with a single color destination
         * attachment.
         * This is equivalent to:
         * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         *      color = write(color, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
         *      auto id = declareRenderPass(getName(color),
         *              {.attachments = {.color = {color}}});
         *      if (index) *index = id;
         *      return color;
         * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         *
         * @param color handle to the color attachment subresource
         * @param index index of the FrameGraphRenderPass
         * @return handle to the new version of the color attachment subresource
         */
        FrameGraphId<FrameGraphTexture> declareRenderPass(
                FrameGraphId<FrameGraphTexture> color, uint32_t* index = nullptr);

        /**
         * Creates a virtual resource of type RESOURCE
         * @tparam RESOURCE Type of the resource to create
         * @param name      A pointer to a null terminated string.
         *                  The pointer lifetime must extend beyond execute().
         * @param desc      Descriptor for this resources
         * @return          A typed resource handle
         */
        template<typename RESOURCE>
        FrameGraphId<RESOURCE> create(const char* name,
                typename RESOURCE::Descriptor const& desc = {}) noexcept {
            return mFrameGraph.create<RESOURCE>(name, desc);
        }


        /**
         * Creates a subresource of the virtual resource of type RESOURCE. This adds a reference
         * from the subresource to the resource.
         *
         * @tparam RESOURCE     Type of the virtual resource
         * @param parent        Pointer to the handle of parent resource. This will be updated.
         * @param name          A name for the subresource
         * @param desc          Descriptor of the subresource
         * @return              A handle to the subresource
         */
        template<typename RESOURCE>
        inline FrameGraphId<RESOURCE> createSubresource(FrameGraphId<RESOURCE> parent,
                const char* name,
                typename RESOURCE::SubResourceDescriptor const& desc = {}) noexcept {
            return mFrameGraph.createSubresource<RESOURCE>(parent, name, desc);
        }


        /**
         * Declares a read access by this pass to a virtual resource. This adds a reference from
         * the pass to the resource.
         * @tparam RESOURCE Type of the resource
         * @param input     Handle to the resource
         * @param usage     How is this resource used. e.g.: sample vs. upload for textures. This is resource dependant.
         * @return          A new handle to the resource. The input handle is no-longer valid.
         */
        template<typename RESOURCE>
        inline FrameGraphId<RESOURCE> read(FrameGraphId<RESOURCE> input,
                typename RESOURCE::Usage usage = RESOURCE::DEFAULT_R_USAGE) {
            return mFrameGraph.read<RESOURCE>(mPassNode, input, usage);
        }

        /**
         * Declares a write access by this pass to a virtual resource. This adds a reference from
         * the resource to the pass.
         * @tparam RESOURCE Type of the resource
         * @param input     Handle to the resource
         * @param usage     How is this resource used. This is resource dependant.
         * @return          A new handle to the resource. The input handle is no-longer valid.
         */
        template<typename RESOURCE>
        [[nodiscard]] FrameGraphId<RESOURCE> write(FrameGraphId<RESOURCE> input,
                typename RESOURCE::Usage usage = RESOURCE::DEFAULT_W_USAGE) {
            return mFrameGraph.write<RESOURCE>(mPassNode, input, usage);
        }

        /**
         * Marks the current pass as a leaf. Adds a reference to it, so it's not culled.
         * Calling write() on an imported resource automatically adds a side-effect.
         */
        void sideEffect() noexcept;

        /**
         * Retrieves the descriptor associated to a resource
         * @tparam RESOURCE Type of the resource
         * @param handle    Handle to a virtual resource
         * @return          Reference to the descriptor
         */
        template<typename RESOURCE>
        typename RESOURCE::Descriptor const& getDescriptor(FrameGraphId<RESOURCE> handle) const {
            return static_cast<Resource<RESOURCE> const*>(
                    mFrameGraph.getResource(handle))->descriptor;
        }

        /**
         * Retrieves the name of a resource
         * @param handle    Handle to a virtual resource
         * @return          C string to the name of the resource
         */
        const char* getName(FrameGraphHandle handle) const noexcept;


        /**
         * Helper to creates a FrameGraphTexture resource.
         * @param name      A pointer to a null terminated string.
         *                  The pointer lifetime must extend beyond execute().
         * @param desc      Descriptor for this resources
         * @return          A typed resource handle
         */
        FrameGraphId<FrameGraphTexture> createTexture(const char* name,
                FrameGraphTexture::Descriptor const& desc = {}) noexcept {
            return create<FrameGraphTexture>(name, desc);
        }

        /**
         * Helper for the common case of sampling textures
         * @param input     Handle to the FrameGraphTexture
         * @return          A new handle to the FrameGraphTexture.
         *                  The input handle is no-longer valid.
         */
        FrameGraphId<FrameGraphTexture> sample(FrameGraphId<FrameGraphTexture> input) {
            return read(input, FrameGraphTexture::Usage::SAMPLEABLE);
        }

    private:
        friend class FrameGraph;
        Builder(FrameGraph& fg, PassNode* passNode) noexcept;
        ~Builder() noexcept = default;
        FrameGraph& mFrameGraph;
        PassNode* const mPassNode;
    };

    // --------------------------------------------------------------------------------------------

    explicit FrameGraph(ResourceAllocatorInterface& resourceAllocator);
    FrameGraph(FrameGraph const&) = delete;
    FrameGraph& operator=(FrameGraph const&) = delete;
    ~FrameGraph() noexcept;

    /** returns the default Blackboard */
    Blackboard& getBlackboard() noexcept { return mBlackboard; }

    /** returns the default Blackboard */
    Blackboard const& getBlackboard() const noexcept { return mBlackboard; }

    /** Empty struct to use for passes with no data */
    struct Empty { };

    /**
     * Add a pass to the frame graph. Typically:
     *
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * struct PassData {
     * };
     * auto& pass = addPass<PassData>("Pass Name",
     *      [&](Builder& builder, auto& data) {
     *          // synchronously declare resources here
     *      },
     *      [=](FrameGraphResources const& resources, auto const&, DriverApi& driver) {
     *          // issue backend drawing commands here
     *      }
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     * @tparam Data     A user-defined structure containing this pass data
     * @tparam Setup    A lambda of type [](Builder&, Data&).
     * @tparam Execute  A lambda of type [](FrameGraphResources const&, Data const&, DriverApi&)
     *
     * @param name      A name for this pass. Used for debugging only.
     * @param setup     lambda called synchronously, used to declare which and how resources are
     *                  used by this pass. Captures should be done by reference.
     * @param execute   lambda called asynchronously from FrameGraph::execute(),
     *                  where immediate drawing commands can be issued.
     *                  Captures must be done by copy.
     *
     * @return          A reference to a Pass object
     */
    template<typename Data, typename Setup, typename Execute>
    FrameGraphPass<Data, Execute>& addPass(const char* name, Setup setup, Execute&& execute);

    /**
     * Adds a simple execute-only pass with side-effect. Use with caution as such a pass is never
     * culled.
     *
     * @tparam Execute  A lambda of type [](DriverApi&)
     * @param name      A name for this pass. Used for debugging only.
     * @param execute   lambda called asynchronously from FrameGraph::execute(),
     *                  where immediate drawing commands can be issued.
     *                  Captures must be done by copy.
     */
    template<typename Execute>
    void addTrivialSideEffectPass(const char* name, Execute&& execute);

    /**
     * Allocates concrete resources and culls unreferenced passes.
     * @return a reference to the FrameGraph, for chaining calls.
     */
    FrameGraph& compile() noexcept;

    /**
     * Execute all referenced passes
     *
     * @param driver a reference to the backend to execute the commands
     */
    void execute(backend::DriverApi& driver) noexcept;

    /**
     * Forwards a resource to another one which gets replaced.
     * The replaced resource's handle becomes forever invalid.
     *
     * @tparam RESOURCE             Type of the resources
     * @param resource              Handle to the subresource being forwarded
     * @param replacedResource      Handle of the subresource being replaced
     *                              This handle becomes invalid after this call
     * @return                      Handle to a new version of the forwarded resource
     */
    template<typename RESOURCE>
    FrameGraphId<RESOURCE> forwardResource(FrameGraphId<RESOURCE> resource,
            FrameGraphId<RESOURCE> replacedResource);

    /**
     * Adds a reference to 'input', preventing it from being culled
     *
     * @param input a resource handle
     */
    template<typename RESOURCE>
    void present(FrameGraphId<RESOURCE> input);

    /**
     * Imports a concrete resource to the frame graph. The lifetime management is not transferred
     * to the frame graph.
     *
     * @tparam RESOURCE     Type of the resource to import
     * @param name          A name for this resource
     * @param desc          The descriptor for this resource
     * @param resource      A reference to the resource itself
     * @return              A handle that can be used normally in the frame graph
     */
    template<typename RESOURCE>
    FrameGraphId<RESOURCE> import(const char* name,
            typename RESOURCE::Descriptor const& desc,
            typename RESOURCE::Usage usage,
            const RESOURCE& resource) noexcept;

    /**
     * Imports a RenderTarget as a FrameGraphTexture into the frame graph. Later, this
     * FrameGraphTexture can be used with declareRenderPass(), the resulting concrete
     * FrameGraphRenderPass will be the one passed as argument here, instead of being
     * dynamically created.
     *
     * @param name      A name for the FrameGraphRenderPass
     * @param desc      ImportDescriptor for the imported FrameGraphRenderPass
     * @param target    handle to the concrete FrameGraphRenderPass to import
     * @return          A handle to a FrameGraphTexture
     */
    FrameGraphId<FrameGraphTexture> import(const char* name,
            FrameGraphRenderPass::ImportDescriptor const& desc,
            backend::Handle<backend::HwRenderTarget> target);


    /**
     * Check that a handle is initialized and valid.
     * @param handle handle to test validity for
     * @return true of the handle is valid, false otherwise.
     */
    bool isValid(FrameGraphHandle handle) const;

    /**
     * Returns whether a pass has been culled after FrameGraph::compile()
     * @param pass
     * @return true if the pass has been culled
     */
    bool isCulled(FrameGraphPassBase const& pass) const noexcept;

    /**
     * Retrieves the descriptor associated to a resource
     * @tparam RESOURCE Type of the resource
     * @param handle    Handle to a virtual resource
     * @return          Reference to the descriptor
     */
    template<typename RESOURCE>
    typename RESOURCE::Descriptor const& getDescriptor(FrameGraphId<RESOURCE> handle) const {
        return static_cast<Resource<RESOURCE> const*>(getResource(handle))->descriptor;
    }

    /**
     * Checks if the FrameGraph is acyclic. This is intended for testing only.
     * Performance is not expected to be good. Might always return true in Release builds.
     * @return True if the frame graph is acyclic.
     */
    bool isAcyclic() const noexcept;

    //! export a graphviz view of the graph
    void export_graphviz(utils::io::ostream& out, const char* name = nullptr);

private:
    friend class FrameGraphResources;
    friend class PassNode;
    friend class ResourceNode;
    friend class RenderPassNode;

    LinearAllocatorArena& getArena() noexcept { return mArena; }
    DependencyGraph& getGraph() noexcept { return mGraph; }
    ResourceAllocatorInterface& getResourceAllocator() noexcept { return mResourceAllocator; }

    struct ResourceSlot {
        using Version = FrameGraphHandle::Version;
        using Index = int16_t;
        Index rid = 0;    // VirtualResource* index in mResources
        Index nid = 0;    // ResourceNode* index in mResourceNodes
        Index sid =-1;    // ResourceNode* index in mResourceNodes for reading subresource's parent
        Version version = 0;
    };
    void reset() noexcept;
    void addPresentPass(std::function<void(Builder&)> setup) noexcept;
    Builder addPassInternal(const char* name, FrameGraphPassBase* base) noexcept;
    FrameGraphHandle createNewVersion(FrameGraphHandle handle, FrameGraphHandle parent = {}) noexcept;
    FrameGraphHandle createNewVersionForSubresourceIfNeeded(FrameGraphHandle handle) noexcept;
    FrameGraphHandle addResourceInternal(VirtualResource* resource) noexcept;
    FrameGraphHandle addSubResourceInternal(FrameGraphHandle parent, VirtualResource* resource) noexcept;
    FrameGraphHandle readInternal(FrameGraphHandle handle, PassNode* passNode,
            std::function<bool(ResourceNode*, VirtualResource*)> connect);
    FrameGraphHandle writeInternal(FrameGraphHandle handle, PassNode* passNode,
            std::function<bool(ResourceNode*, VirtualResource*)> connect);
    FrameGraphHandle forwardResourceInternal(FrameGraphHandle resourceHandle,
            FrameGraphHandle replaceResourceHandle);

    bool assertValid(FrameGraphHandle handle) const;

    template<typename RESOURCE>
    FrameGraphId<RESOURCE> create(char const* name,
            typename RESOURCE::Descriptor const& desc) noexcept;

    template<typename RESOURCE>
    FrameGraphId<RESOURCE> createSubresource(FrameGraphId<RESOURCE> parent,
            char const* name, typename RESOURCE::SubResourceDescriptor const& desc) noexcept;

        template<typename RESOURCE>
    FrameGraphId<RESOURCE> read(PassNode* passNode,
            FrameGraphId<RESOURCE> input, typename RESOURCE::Usage usage);

    template<typename RESOURCE>
    FrameGraphId<RESOURCE> write(PassNode* passNode,
            FrameGraphId<RESOURCE> input, typename RESOURCE::Usage usage);

    ResourceSlot& getResourceSlot(FrameGraphHandle handle) noexcept {
        assert_invariant((size_t)handle.index < mResourceSlots.size());
        assert_invariant((size_t)mResourceSlots[handle.index].rid < mResources.size());
        assert_invariant((size_t)mResourceSlots[handle.index].nid < mResourceNodes.size());
        return mResourceSlots[handle.index];
    }

    ResourceSlot const& getResourceSlot(FrameGraphHandle handle) const noexcept {
        return const_cast<FrameGraph*>(this)->getResourceSlot(handle);
    }

    VirtualResource* getResource(FrameGraphHandle handle) noexcept {
        assert_invariant(handle.isInitialized());
        ResourceSlot const& slot = getResourceSlot(handle);
        assert_invariant((size_t)slot.rid < mResources.size());
        return mResources[slot.rid];
    }

    ResourceNode* getActiveResourceNode(FrameGraphHandle handle) noexcept {
        ResourceSlot const& slot = getResourceSlot(handle);
        assert_invariant((size_t)slot.nid < mResourceNodes.size());
        return mResourceNodes[slot.nid];
    }

    VirtualResource const* getResource(FrameGraphHandle handle) const noexcept {
        return const_cast<FrameGraph*>(this)->getResource(handle);
    }

    ResourceNode const* getResourceNode(FrameGraphHandle handle) const noexcept {
        return const_cast<FrameGraph*>(this)->getActiveResourceNode(handle);
    }

    void destroyInternal() noexcept;

    Blackboard mBlackboard;
    ResourceAllocatorInterface& mResourceAllocator;
    LinearAllocatorArena mArena;
    DependencyGraph mGraph;

    Vector<ResourceSlot> mResourceSlots;
    Vector<VirtualResource*> mResources;
    Vector<ResourceNode*> mResourceNodes;
    Vector<PassNode*> mPassNodes;
    Vector<PassNode*>::iterator mActivePassNodesEnd;
};

template<typename Data, typename Setup, typename Execute>
FrameGraphPass<Data, Execute>& FrameGraph::addPass(char const* name, Setup setup, Execute&& execute) {
    static_assert(sizeof(Execute) < 1024, "Execute() lambda is capturing too much data.");

    // create the FrameGraph pass
    auto* const pass = mArena.make<FrameGraphPass<Data, Execute>>(std::forward<Execute>(execute));

    Builder builder(addPassInternal(name, pass));
    setup(builder, const_cast<Data&>(pass->getData()));

    // return a reference to the pass to the user
    return *pass;
}

template<typename Execute>
void FrameGraph::addTrivialSideEffectPass(char const* name, Execute&& execute) {
    addPass<Empty>(name, [](FrameGraph::Builder& builder, auto&) { builder.sideEffect(); },
            [execute](FrameGraphResources const&, auto const&, backend::DriverApi& driver) {
                execute(driver);
            });
}

template<typename RESOURCE>
void FrameGraph::present(FrameGraphId<RESOURCE> input) {
    // present doesn't add any usage flags, only a dependency
    addPresentPass([&](Builder& builder) { builder.read(input, {}); });
}

template<typename RESOURCE>
FrameGraphId<RESOURCE> FrameGraph::create(char const* name,
        typename RESOURCE::Descriptor const& desc) noexcept {
    VirtualResource* vresource(mArena.make<Resource<RESOURCE>>(name, desc));
    return FrameGraphId<RESOURCE>(addResourceInternal(vresource));
}

template<typename RESOURCE>
FrameGraphId<RESOURCE> FrameGraph::createSubresource(FrameGraphId<RESOURCE> parent,
        char const* name, typename RESOURCE::SubResourceDescriptor const& desc) noexcept {
    auto* parentResource = static_cast<Resource<RESOURCE>*>(getResource(parent));
    VirtualResource* vresource(mArena.make<Resource<RESOURCE>>(parentResource, name, desc));
    return FrameGraphId<RESOURCE>(addSubResourceInternal(parent, vresource));
}

template<typename RESOURCE>
FrameGraphId<RESOURCE> FrameGraph::import(char const* name,
        typename RESOURCE::Descriptor const& desc,
        typename RESOURCE::Usage usage,
        RESOURCE const& resource) noexcept {
    VirtualResource* vresource(mArena.make<ImportedResource<RESOURCE>>(name, desc, usage, resource));
    return FrameGraphId<RESOURCE>(addResourceInternal(vresource));
}

template<typename RESOURCE>
FrameGraphId<RESOURCE> FrameGraph::read(PassNode* passNode, FrameGraphId<RESOURCE> input,
        typename RESOURCE::Usage usage) {
    FrameGraphId<RESOURCE> result(readInternal(input, passNode,
            [this, passNode, usage](ResourceNode* node, VirtualResource* vrsrc) {
                Resource<RESOURCE>* resource = static_cast<Resource<RESOURCE>*>(vrsrc);
                return resource->connect(mGraph, node, passNode, usage);
            }));
    return result;
}

template<typename RESOURCE>
FrameGraphId<RESOURCE> FrameGraph::write(PassNode* passNode, FrameGraphId<RESOURCE> input,
        typename RESOURCE::Usage usage) {
    FrameGraphId<RESOURCE> result(writeInternal(input, passNode,
            [this, passNode, usage](ResourceNode* node, VirtualResource* vrsrc) {
                Resource<RESOURCE>* resource = static_cast<Resource<RESOURCE>*>(vrsrc);
                return resource->connect(mGraph, passNode, node, usage);
            }));
    return result;
}

template<typename RESOURCE>
FrameGraphId<RESOURCE> FrameGraph::forwardResource(FrameGraphId<RESOURCE> resource,
        FrameGraphId<RESOURCE> replacedResource) {
    return FrameGraphId<RESOURCE>(forwardResourceInternal(resource, replacedResource));
}

// ------------------------------------------------------------------------------------------------

/*
 * Prevent implicit instantiation of methods involving FrameGraphTexture which is a known type
 * these are explicitly instantiated in the .cpp file.
 */

extern template void FrameGraph::present(FrameGraphId<FrameGraphTexture> input);

extern template FrameGraphId<FrameGraphTexture> FrameGraph::create(char const* name,
        FrameGraphTexture::Descriptor const& desc) noexcept;

extern template FrameGraphId<FrameGraphTexture> FrameGraph::createSubresource(FrameGraphId<FrameGraphTexture> parent,
        char const* name, FrameGraphTexture::SubResourceDescriptor const& desc) noexcept;

extern template FrameGraphId<FrameGraphTexture> FrameGraph::import(char const* name,
        FrameGraphTexture::Descriptor const& desc, FrameGraphTexture::Usage usage, FrameGraphTexture const& resource) noexcept;

extern template FrameGraphId<FrameGraphTexture> FrameGraph::read(PassNode* passNode,
        FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Usage usage);

extern template FrameGraphId<FrameGraphTexture> FrameGraph::write(PassNode* passNode,
        FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Usage usage);

extern template FrameGraphId<FrameGraphTexture> FrameGraph::forwardResource(
        FrameGraphId<FrameGraphTexture> resource, FrameGraphId<FrameGraphTexture> replacedResource);

} // namespace filament

#endif //TNT_FILAMENT_FG2_FRAMEGRAPH_H
