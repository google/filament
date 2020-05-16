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

#ifndef TNT_FILAMENT_FRAMEGRAPH_H
#define TNT_FILAMENT_FRAMEGRAPH_H


#include "Blackboard.h"
#include "FrameGraphPass.h"
#include "FrameGraphHandle.h"

#include <fg/fg/ResourceEntry.h>
#include <fg/fg/RenderTargetResourceEntry.h>

#include "details/Allocators.h"

#include "private/backend/DriverApiForward.h"

#include <backend/DriverEnums.h>

#include <utils/Log.h>

#include <vector>
#include <memory>

/*
 * A somewhat generic frame graph API.
 *
 * The design is largely inspired from Yuriy O'Donnell 2017 GDC talk
 * "FrameGraph: Extensible Rendering Architecture in Frostbite"
 *
 */

namespace filament {

class FEngine;

namespace fg {
struct ResourceNode;
struct PassNode;
struct Alias;
class ResourceAllocatorInterface;
} // namespace fg

class FrameGraphPassResources;

class FrameGraph {
public:

    class Builder {
    public:
        Builder(Builder const&) = delete;
        Builder& operator=(Builder const&) = delete;

        // Create virtual resource that can eventually turn into a concrete resource (typically
        // a GPU buffer).
        template<typename T>
        FrameGraphId<T> create(const char* name,
                typename T::Descriptor const& desc = {}) noexcept {
            return mFrameGraph.create<T>(name, desc);
        }

        template<>
        FrameGraphId<FrameGraphRenderTarget> create(const char* name,
                typename FrameGraphRenderTarget::Descriptor const& desc) noexcept {
            return createRenderTargetImpl(name, desc);
        }

        // Helper to create a Texture resource
        FrameGraphId<FrameGraphTexture> createTexture(const char* name,
                FrameGraphTexture::Descriptor const& desc = {}) noexcept {
            return create<FrameGraphTexture>(name, desc);
        }

        // Helper to create a RenderTarget resource
        FrameGraphId<FrameGraphRenderTarget> createRenderTarget(const char* name,
                FrameGraphRenderTarget::Descriptor const& desc) noexcept {
            return create<FrameGraphRenderTarget>(name, desc);
        }

        // Read from a resource (i.e. add a reference to that resource)
        // Note: FrameGraphRenderTarget resource are special in that they can't be read from
        //       explicitly (it is implicit though, when creating one).
        template<typename T,
                typename = std::enable_if_t<!std::is_same<T, FrameGraphRenderTarget>::value>>
        FrameGraphId<T> read(FrameGraphId<T> input) {
            return FrameGraphId<T>(readImpl(FrameGraphHandle(input)));
        }

        // Write to a resource (i.e. add a reference to that pass)
        // Note: FrameGraphRenderTarget resource are special in that they can't be written to
        //       (i.e. they can't add a reference to a pass, or create a new version of
        //        themselves)
        template<typename T,
                typename = std::enable_if_t<!std::is_same<T, FrameGraphRenderTarget>::value>>
        [[nodiscard]] FrameGraphId<T> write(FrameGraphId<T> output) {
            return FrameGraphId<T>(writeImpl(FrameGraphHandle(output)));
        }

        // Sample from a texture resource (implies read())
        FrameGraphId<FrameGraphTexture> sample(FrameGraphId<FrameGraphTexture> input);

        // Declare that this pass has side effects outside the framegraph (i.e. it can't be culled)
        // Calling write() on an imported resource automatically adds a side-effect.
        Builder& sideEffect() noexcept;

        // Helpers --------------------------------------------------------------------

        // Return the name of the pass being built
        const char* getPassName() const noexcept;

        // helper to get resource's name
        const char* getName(FrameGraphHandle const& r) const noexcept;

        // helper to get a resource's descriptor
        template<typename T>
        typename T::Descriptor& getDescriptor(FrameGraphId<T> r) {
            return mFrameGraph.getDescriptor<T>(r);
        }

    private:
        friend class FrameGraph;
        Builder(FrameGraph& fg, fg::PassNode& pass) noexcept;
        ~Builder() noexcept;
        FrameGraphHandle readImpl(FrameGraphHandle input);
        [[nodiscard]] FrameGraphHandle writeImpl(FrameGraphHandle output);
        FrameGraphId<FrameGraphRenderTarget> createRenderTargetImpl(
                const char* name, typename FrameGraphRenderTarget::Descriptor const& desc) noexcept;
        FrameGraph& mFrameGraph;
        fg::PassNode& mPass;
    };

    explicit FrameGraph(fg::ResourceAllocatorInterface& resourceAllocator);
    FrameGraph(FrameGraph const&) = delete;
    FrameGraph& operator = (FrameGraph const&) = delete;
    ~FrameGraph();

    // returns the default Blackboard
    Blackboard& getBlackboard() noexcept { return mBlackboard; }

    // returns the default Blackboard
    Blackboard const& getBlackboard() const noexcept { return mBlackboard; }

    struct Empty{};

    /*
     * Add a pass to the framegraph.
     * The Setup lambda is called synchronously and used to declare which and how resources are
     *   used by this pass. Captures should be done by reference.
     * The Execute lambda is called asynchronously from FrameGraph::execute(), and this is where
     *   immediate drawing commands can be issued. Captures must be done by copy.
     */
    template <typename Data, typename Setup, typename Execute>
    FrameGraphPass<Data, Execute>& addPass(const char* name, Setup setup, Execute&& execute) {
        static_assert(sizeof(Execute) < 1024, "Execute() lambda is capturing too much data.");

        // create the FrameGraph pass
        auto* const pass = mArena.make<FrameGraphPass<Data, Execute>>(std::forward<Execute>(execute));

        // record in our pass list
        fg::PassNode& node = createPass(name, pass);

        // call the setup code, which will declare used resources
        Builder builder(*this, node);
        setup(builder, pass->getData());

        // return a reference to the pass to the user
        return *pass;
    }

    // Adds a simple execute-only pass with side effect (so it's not culled)
    template<typename Execute>
    void addTrivialSideEffectPass(const char* name, Execute&& execute) {
        addPass<Empty>(name, [](FrameGraph::Builder& builder, auto&) { builder.sideEffect(); },
                [execute](FrameGraphPassResources const&, auto const&,
                        backend::DriverApi& driver) {
                    execute(driver);
                });
    }

    // Adds a reference to 'input', preventing it from being culled.
    void present(FrameGraphHandle input);

    // Returns whether the resource handle is valid. A resource handle becomes invalid after
    // it's used to declare a resource write (see Builder::write()).
    bool isValid(FrameGraphHandle r) const noexcept;

    bool equal(FrameGraphHandle lhs, FrameGraphHandle rhs) const noexcept;

    // Return the Descriptor associated to this resource handle. The handle must be valid.
    template<typename T>
    typename T::Descriptor& getDescriptor(FrameGraphId<T> r) {
        fg::ResourceEntry<T>& entry = getResourceEntryUnchecked(r);
        return entry.descriptor;
    }

    template<typename T>
    FrameGraphId<T> import(const char* name,
            typename T::Descriptor const& desc, const T& resource) noexcept {
        fg::ResourceEntryBase* pBase = mArena.make<fg::ResourceEntry<T>>(name, desc, resource, mId++, 0);
        return FrameGraphId<T>(create(pBase));
    }

    template<>
    FrameGraphId<FrameGraphRenderTarget> import(const char* name,
            typename FrameGraphRenderTarget::Descriptor const& desc,
            const FrameGraphRenderTarget& resource) noexcept {
        fg::ResourceEntryBase* pBase = mArena.make<fg::RenderTargetResourceEntry>(
                name, desc, resource, mId++, 1);
        return FrameGraphId<FrameGraphRenderTarget>(create(pBase));
    }

    // Moves the resource associated to the handle 'from' to the handle 'to'. After this call,
    // all handles referring to the resource 'to' are redirected to the resource 'from'
    // (including handles used in the past).
    // All writes to 'from' are disconnected (i.e. these passes loose a reference).
    // Return a new handle for the 'from' resource and makes the 'from' handle invalid (i.e. it's
    // similar to if we had written to the 'from' resource)
    template<typename T>
    void moveResource(FrameGraphId<T> from, FrameGraphId<T> to) {
        return moveResourceBase(from, to);
    }

    void moveResource(FrameGraphId<FrameGraphRenderTarget> from, FrameGraphId<FrameGraphTexture> to);

    // allocates concrete resources and culls unreferenced passes
    FrameGraph& compile() noexcept;

    // execute all referenced passes and flush the command queue after each pass
    void execute(FEngine& engine, backend::DriverApi& driver) noexcept;


    /*
     * Debugging...
     */

    // execute all referenced passes -- this version is for unit-testing, where we don't have
    // an engine necessarily.
    void execute(backend::DriverApi& driver) noexcept;

    // print the frame graph as a graphviz file in the log
    void export_graphviz(utils::io::ostream& out);

private:
    friend class FrameGraphPassResources;
    friend struct FrameGraphTexture;
    friend struct fg::PassNode;
    friend class fg::RenderTargetResourceEntry;

    template <typename T>
    struct Deleter {
        FrameGraph& fg;
        Deleter(FrameGraph& fg) noexcept : fg(fg) {} // NOLINT
        void operator()(T* object) noexcept { fg.mArena.destroy(object); }
    };

    template<typename T> using UniquePtr = std::unique_ptr<T, Deleter<T>>;
    template<typename T> using Allocator = utils::STLAllocator<T, LinearAllocatorArena>;
    template<typename T> using Vector = std::vector<T, Allocator<T>>; // 32 bytes

    LinearAllocatorArena& getArena() noexcept { return mArena; }

    fg::PassNode& createPass(const char* name, FrameGraphPassExecutor* base) noexcept;

    FrameGraphHandle createResourceNode(fg::ResourceEntryBase* resource) noexcept;

    void executeInternal(fg::PassNode const& node, backend::DriverApi& driver) noexcept;

    fg::ResourceAllocatorInterface& getResourceAllocator() noexcept { return mResourceAllocator; }

    void reset() noexcept;

    void moveResourceBase(FrameGraphHandle from, FrameGraphHandle to);

    FrameGraphHandle create(fg::ResourceEntryBase* pResourceEntry) noexcept;

    template<typename T>
    FrameGraphId<T> create(const char* name, typename T::Descriptor const& desc) noexcept {
        fg::ResourceEntryBase* pBase = mArena.make<fg::ResourceEntry<T>>(name, desc, mId++, 0);
        FrameGraphId<T> r(create(pBase));
        return r;
    }

    template<>
    FrameGraphId<FrameGraphRenderTarget> create(const char* name,
            typename FrameGraphRenderTarget::Descriptor const& desc) noexcept {
        fg::RenderTargetResourceEntry* pBase = mArena.make<fg::RenderTargetResourceEntry>(name, desc, mId++, 1);
        FrameGraphId<FrameGraphRenderTarget> r(create(pBase));
        return r;
    }

    fg::ResourceNode& getResourceNode(FrameGraphHandle r);
    fg::ResourceNode& getResourceNodeUnchecked(FrameGraphHandle r);

    fg::ResourceEntryBase& getResourceEntryBase(FrameGraphHandle r) noexcept;
    fg::ResourceEntryBase& getResourceEntryBaseUnchecked(FrameGraphHandle r) noexcept;

    template<typename T>
    fg::ResourceEntry<T>& getResourceEntry(FrameGraphId<T> r) noexcept {
        return static_cast<fg::ResourceEntry<T>&>(getResourceEntryBase(r));
    }

    template<typename T>
    fg::ResourceEntry<T>& getResourceEntryUnchecked(FrameGraphId<T> r) noexcept {
        return static_cast<fg::ResourceEntry<T>&>(getResourceEntryBaseUnchecked(r));
    }

    Blackboard mBlackboard;
    fg::ResourceAllocatorInterface& mResourceAllocator;
    LinearAllocatorArena mArena;
    Vector<fg::PassNode> mPassNodes;                    // list of frame graph passes
    Vector<fg::ResourceNode *> mResourceNodes;          // list of resource nodes
    Vector<UniquePtr<fg::ResourceNode>> mResourceNodeEntries;
    Vector<UniquePtr<fg::ResourceEntryBase>> mResourceEntries;
    uint16_t mId = 0;
};

} // namespace filament

#endif //TNT_FILAMENT_FRAMEGRAPH_H
