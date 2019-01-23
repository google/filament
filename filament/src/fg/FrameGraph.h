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


#include "FrameGraphPass.h"
#include "FrameGraphResource.h"

#include "driver/DriverApiForward.h"
#include "FrameGraphPassResources.h"

#include "details/Allocators.h"

#include <utils/Log.h>

#include <vector>

/*
 * A somewhat generic frame graph API.
 *
 * The design is largely inspired from Yuriy O'Donnell 2017 GDC talk
 * "FrameGraph: Extensible Rendering Architecture in Frostbite"
 *
 */

namespace filament {

namespace fg {
struct Resource;
struct ResourceNode;
struct RenderTarget;
struct PassNode;
struct Alias;
} // namespace fg

class FrameGraphPassResources;

class FrameGraph {
public:

    class Builder {
    public:
        Builder(Builder const&) = delete;
        Builder& operator=(Builder const&) = delete;

        // TODO: should this always register a write? If the GPU doesn't write into the resource
        // what's the point of creating it?
        // Create a virtual resource that can eventually turn into a concrete texture or
        // render target
        FrameGraphResource declareTexture(const char* name,
                FrameGraphResource::Descriptor const& desc = {}) noexcept;

        FrameGraphRenderTarget declareRenderTarget(const char* name,
                FrameGraphRenderTarget::Descriptor const& desc = {}) noexcept;

        FrameGraphRenderTarget declareRenderTarget(FrameGraphResource texture) noexcept;

        // Read from a resource (i.e. add a reference to that resource)
        FrameGraphResource read(FrameGraphResource const& input);

        // The resource will be used as a source of a blit()
        FrameGraphResource blit(FrameGraphResource const& input);

        // TODO: should write be replaced by declareRenderTarget()?
        // Write to a resource (i.e. add a reference to the pass that's doing the writing))
        // Writing to a resource makes its handle invalid.
        // Writing to an imported resources adds a side-effect (see sideEffect() below).
        FrameGraphResource write(FrameGraphResource const& output);

        // Declare that this pass has side effects outside the framegraph (i.e. it can't be culled)
        // Calling write() on an imported resource automatically adds a side-effect.
        Builder& sideEffect() noexcept;

    private:
        friend class FrameGraph;
        Builder(FrameGraph& fg, fg::PassNode& pass) noexcept;
        ~Builder() noexcept;
        FrameGraph& mFrameGraph;
        fg::PassNode& mPass;
    };

    FrameGraph();
    FrameGraph(FrameGraph const&) = delete;
    FrameGraph& operator = (FrameGraph const&) = delete;
    ~FrameGraph();

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

        // create the FrameGraph pass (TODO: use special allocator)
        auto* const pass = new FrameGraphPass<Data, Execute>(std::forward<Execute>(execute));

        // record in our pass list
        fg::PassNode& node = createPass(name, pass);

        // call the setup code, which will declare used resources
        Builder builder(*this, node);
        setup(builder, pass->getData());

        // return a reference to the pass to the user
        return *pass;
    }

    // Adds a reference to 'input', preventing it from being culled.
    void present(FrameGraphResource input);

    // Returns whether the resource handle is valid. A resource handle becomes invalid after
    // it's used to declare a resource write (see Builder::write()).
    bool isValid(FrameGraphResource r) const noexcept;

    // Return the Descriptor associated to this resource handle, or nullptr if the resource
    // handle is invalid.
    FrameGraphResource::Descriptor* getDescriptor(FrameGraphResource r);

    // Import a write-only render target from outside the framegraph and returns a handle to it.
    FrameGraphResource importResource(
            const char* name, FrameGraphRenderTarget::Descriptor const& descriptor,
            Handle<HwRenderTarget> target);

    // Import a read-only render target from outside the framegraph and returns a handle to it.
    FrameGraphResource importResource(
            const char* name, FrameGraphResource::Descriptor const& descriptor,
            Handle<HwTexture> color);


    // Moves the resource associated to the handle 'from' to the handle 'to'. After this call,
    // all handles referring to the resource 'to' are redirected to the resource 'from'
    // (including handles used in the past).
    // All writes to 'from' are disconnected (i.e. these passes loose a reference).
    // Returns true on success, false if one of the handle was invalid.
    bool moveResource(FrameGraphResource from, FrameGraphResource to);

    // allocates concrete resources and culls unreferenced passes
    FrameGraph& compile() noexcept;

    // execute all referenced passes
    void execute(driver::DriverApi& driver) noexcept;

    // for debugging
    void export_graphviz(utils::io::ostream& out);

private:
    friend class FrameGraphPassResources;
    friend struct fg::PassNode;
    friend struct fg::RenderTarget;

    template <typename T>
    using Allocator = utils::STLAllocator<T, details::LinearAllocatorArena>;

    template <typename T>
    using Vector = std::vector<T, Allocator<T>>;

    auto& getArena() noexcept { return mArena; }

    fg::PassNode& createPass(const char* name, FrameGraphPassExecutor* base) noexcept;
    fg::ResourceNode& createResource(const char* name,
            FrameGraphResource::Descriptor const& desc, bool imported) noexcept;
    fg::ResourceNode* getResource(FrameGraphResource r);

    fg::RenderTarget& createRenderTarget(const char* name,
            FrameGraphRenderTarget::Descriptor const& desc, bool imported) noexcept;

    enum class DiscardPhase { START, END };
    uint8_t computeDiscardFlags(DiscardPhase phase,
            fg::PassNode const* curr, fg::PassNode const* first,
            fg::RenderTarget const& renderTarget);

    details::LinearAllocatorArena mArena;

    Vector<fg::PassNode> mPassNodes;           // list of frame graph passes
    Vector<fg::ResourceNode> mResourceNodes;
    Vector<fg::RenderTarget> mRenderTargets;
    Vector<fg::Resource> mResourceRegistry;    // frame graph concrete resources
    Vector<fg::Alias> mAliases;
};

} // namespace filament

#endif //TNT_FILAMENT_FRAMEGRAPH_H
