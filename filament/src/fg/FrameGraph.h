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

        // create a resource
        using RWFlags = uint32_t;
        static constexpr RWFlags NONE  = 0x0;
        static constexpr RWFlags COLOR = 0x1;
        static constexpr RWFlags DEPTH = 0x2;

        FrameGraphResource createTexture(const char* name,
                FrameGraphResource::Descriptor const& desc = {}) noexcept;

        // read from a resource (i.e. add a reference to that resource)
        FrameGraphResource read(FrameGraphResource const& input, RWFlags readFlags = COLOR);

        // write to a resource (i.e. add a reference to the pass that's doing the writing))
        FrameGraphResource write(FrameGraphResource const& output, RWFlags writeFlags = COLOR);

        // declare that this pass has side effects
        Builder& sideEffect() noexcept;

    private:
        friend class FrameGraph;
        Builder(FrameGraph& fg, fg::PassNode& pass) noexcept;
        FrameGraph& mFrameGraph;
        fg::PassNode& mPass;
    };

    FrameGraph();
    FrameGraph(FrameGraph const&) = delete;
    FrameGraph& operator = (FrameGraph const&) = delete;
    ~FrameGraph();

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

    void present(FrameGraphResource input);

    bool isValid(FrameGraphResource r) const noexcept;

    FrameGraphResource::Descriptor* getDescriptor(FrameGraphResource r);

    FrameGraphResource importResource(const char* name, Handle <HwRenderTarget> target);

    FrameGraph& compile() noexcept;

    void execute(driver::DriverApi& driver) noexcept;


    void moveResource(FrameGraphResource from, FrameGraphResource to);
    void export_graphviz(utils::io::ostream& out);

private:
    friend class FrameGraphPassResources;

    fg::PassNode& createPass(const char* name, FrameGraphPassExecutor* base) noexcept;
    fg::ResourceNode& createResource(const char* name, bool imported) noexcept;
    fg::ResourceNode* getResource(FrameGraphResource r);

    std::vector<fg::PassNode> mPassNodes;           // list of frame graph passes
    std::vector<fg::ResourceNode> mResourceNodes;
    std::vector<fg::Resource> mResourceRegistry;    // frame graph concrete resources
    std::vector<fg::Alias> mAliases;
};

} // namespace filament

#endif //TNT_FILAMENT_FRAMEGRAPH_H
