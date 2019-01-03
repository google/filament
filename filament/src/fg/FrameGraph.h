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

class FrameGraph;

// ------------------------------------------------------------------------------------------------

class FrameGraphPassResources {
public:
};

// ------------------------------------------------------------------------------------------------

class FrameGraphBuilder {
public:
    FrameGraphBuilder(FrameGraph& fg, fg::PassNode& pass) noexcept;
    FrameGraphBuilder(FrameGraphBuilder const&) = delete;
    FrameGraphBuilder& operator = (FrameGraphBuilder const&) = delete;

    // create a resource
    enum CreateFlags : uint32_t {
        UNKNOWN, READ, WRITE
    };
    FrameGraphResource createTexture(const char* name, CreateFlags flags,
            FrameGraphResource::TextureDesc const& desc) noexcept;

    // read from a resource (i.e. add a reference to that resource)
    FrameGraphResource read(FrameGraphResource const& input /*, read-flags*/);

    // write to a resource (i.e. add a reference to the pass that's doing the writing))
    FrameGraphResource write(FrameGraphResource const& output  /*, write-flags*/);

private:
    fg::ResourceNode* getResource(FrameGraphResource handle);
    FrameGraph& mFrameGraph;
    fg::PassNode& mPass;
};

// ------------------------------------------------------------------------------------------------

class FrameGraph {
public:
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
        FrameGraphBuilder builder(*this, node);
        setup(builder, pass->getData());

        // return a reference to the pass to the user
        return *pass;
    }

    void moveResource(FrameGraphResource from, FrameGraphResource to);

    void present(FrameGraphResource input);

    bool isValid(FrameGraphResource handle) const noexcept ;

    FrameGraph& compile() noexcept;

    void execute() noexcept;

    void export_graphviz(utils::io::ostream& out);

private:
    friend class FrameGraphBuilder;

    FrameGraphPassResources mResources;

    fg::PassNode& createPass(const char* name, FrameGraphPassExecutor* base) noexcept;

    fg::ResourceNode& createResource(const char* name) noexcept;

    // list of frame graph passes
    std::vector<fg::PassNode> mPassNodes;

    // frame graph concrete resources
    std::vector<fg::ResourceNode> mResourceNodes;

    std::vector<fg::Resource> mResourceRegistry;

    std::vector<fg::Alias> mAliases;
};


} // namespace filament

#endif //TNT_FILAMENT_FRAMEGRAPH_H
