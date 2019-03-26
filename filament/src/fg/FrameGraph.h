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
#include "FrameGraphPassResources.h"
#include "FrameGraphResource.h"

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

namespace fg {
struct Resource;
struct ResourceNode;
struct RenderTarget;
struct RenderTargetResource;
struct PassNode;
struct Alias;
} // namespace fg

class FrameGraphPassResources;

class FrameGraph {
public:

    class Builder {
    public:
        using Attachments = FrameGraphRenderTarget::Attachments;

        Builder(Builder const&) = delete;
        Builder& operator=(Builder const&) = delete;

        // Create a virtual resource that can eventually turn into a concrete texture or
        // render target
        FrameGraphResource createTexture(const char* name,
                FrameGraphResource::Descriptor const& desc = {}) noexcept;

        // Read from a resource (i.e. add a reference to that resource)
        FrameGraphResource read(FrameGraphResource const& input);

        /*
         * Use this resource as a render target.
         * This implies both reading and writing to the resource -- but unlike Builder::read()
         * this doesn't allow reading with a texture sampler.
         * Writing to a resource:
         *   - adds a reference to the pass that's doing the writing
         *   - makes its handle invalid.
         *   - [imported resource only] adds a side-effect (see sideEffect() below
         */

        Attachments useRenderTarget(const char* name,
                FrameGraphRenderTarget::Descriptor const& desc,
                backend::TargetBufferFlags clearFlags = {}) noexcept;

        // helper for single color attachment
        Attachments useRenderTarget(FrameGraphResource texture,
                backend::TargetBufferFlags clearFlags = {}) noexcept;

        // Declare that this pass has side effects outside the framegraph (i.e. it can't be culled)
        // Calling write() on an imported resource automatically adds a side-effect.
        Builder& sideEffect() noexcept;

        // returns whether this resource is an attachment to some rendertarget
        bool isAttachment(FrameGraphResource resource) const noexcept;

        // returns the descriptor of the render target this attachment belongs to
        FrameGraphRenderTarget::Descriptor const& getRenderTargetDescriptor(
                FrameGraphResource attachment) const;

    private:
        // this is private for now because we only have textures, and this is for regular buffers
        FrameGraphResource write(FrameGraphResource const& output);
        FrameGraphResource read(FrameGraphResource const& input, bool doesntNeedTexture);

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

    // Adds a reference to 'input', preventing it from being culled.
    void present(FrameGraphResource input);

    // Returns whether the resource handle is valid. A resource handle becomes invalid after
    // it's used to declare a resource write (see Builder::write()).
    bool isValid(FrameGraphResource r) const noexcept;

    // Return the Descriptor associated to this resource handle, or nullptr if the resource
    // handle is invalid.
    FrameGraphResource::Descriptor* getDescriptor(FrameGraphResource r);

    // Import a write-only render target from outside the framegraph and returns a handle to it.
    FrameGraphResource importResource(const char* name,
            FrameGraphRenderTarget::Descriptor descriptor,
            backend::Handle<backend::HwRenderTarget> target, uint32_t width, uint32_t height,
            backend::TargetBufferFlags discardStart = backend::TargetBufferFlags::NONE,
            backend::TargetBufferFlags discardEnd = backend::TargetBufferFlags::NONE);

    // Import a read-only render target from outside the framegraph and returns a handle to it.
    FrameGraphResource importResource(
            const char* name, FrameGraphResource::Descriptor const& descriptor,
            backend::Handle<backend::HwTexture> color);


    // Moves the resource associated to the handle 'from' to the handle 'to'. After this call,
    // all handles referring to the resource 'to' are redirected to the resource 'from'
    // (including handles used in the past).
    // All writes to 'from' are disconnected (i.e. these passes loose a reference).
    // Return a new handle for the 'from' resource and makes the 'from' handle invalid (i.e. it's
    // similar to if we had written to the 'from' resource)
    FrameGraphResource moveResource(FrameGraphResource from, FrameGraphResource to);

    // allocates concrete resources and culls unreferenced passes
    FrameGraph& compile() noexcept;

    // execute all referenced passes
    void execute(backend::DriverApi& driver) noexcept;

    // for debugging
    void export_graphviz(utils::io::ostream& out);

private:
    friend class FrameGraphPassResources;
    friend struct fg::PassNode;
    friend struct fg::RenderTarget;
    friend struct fg::RenderTargetResource;

    template <typename T>
    struct Deleter {
        FrameGraph& fg;
        Deleter(FrameGraph& fg) noexcept : fg(fg) {} // NOLINT(google-explicit-constructor)
        void operator()(T* object) noexcept { fg.mArena.destroy(object); }
    };

    template<typename T> using UniquePtr = std::unique_ptr<T, Deleter<T>>;
    template<typename T> using Allocator = utils::STLAllocator<T, details::LinearAllocatorArena>;
    template<typename T> using Vector = std::vector<T, Allocator<T>>; // 32 bytes

    auto& getArena() noexcept { return mArena; }

    fg::PassNode& createPass(const char* name, FrameGraphPassExecutor* base) noexcept;

    fg::Resource* createResource(const char* name,
            FrameGraphResource::Descriptor const& desc, bool imported) noexcept;

    fg::ResourceNode& getResource(FrameGraphResource r);

    fg::RenderTarget& createRenderTarget(const char* name,
            FrameGraphRenderTarget::Descriptor const& desc, bool imported) noexcept;

    FrameGraphResource createResourceNode(fg::Resource* resource) noexcept;

    enum class DiscardPhase { START, END };
    uint8_t computeDiscardFlags(DiscardPhase phase,
            fg::PassNode const* curr, fg::PassNode const* first,
            fg::RenderTarget const& renderTarget);

    bool equals(FrameGraphRenderTarget::Descriptor const& lhs,
            FrameGraphRenderTarget::Descriptor const& rhs) const noexcept;

    details::LinearAllocatorArena mArena;
    Vector<fg::PassNode> mPassNodes;                    // list of frame graph passes
    Vector<fg::ResourceNode> mResourceNodes;            // list of resource nodes
    Vector<fg::RenderTarget> mRenderTargets;            // list of rendertarget
    Vector<fg::Alias> mAliases;                         // list of aliases
    Vector<UniquePtr<fg::Resource>> mResourceRegistry;  // list of actual textures
    Vector<UniquePtr<fg::RenderTargetResource>> mRenderTargetCache; // list of actual rendertargets

    uint16_t mId = 0;
};

} // namespace filament

#endif //TNT_FILAMENT_FRAMEGRAPH_H
