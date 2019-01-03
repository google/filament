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

#include <utils/Panic.h>
#include <utils/Log.h>

using namespace utils;

namespace filament {

using namespace fg;

// ------------------------------------------------------------------------------------------------

namespace fg {

struct Resource {
    explicit Resource(const char* name) noexcept : name(name) { }
    // constants
    const char* const name;         // for debugging
    // computed during compile()
    PassNode* writer = nullptr;     // last writer to this resource
    PassNode* first = nullptr;      // pass that needs to instantiate the resource
    PassNode* last = nullptr;       // pass that can destroy the resource
    uint32_t writerCount = 0;       // # of passes writing to this resource
    uint32_t readerCount = 0;       // # of passes reading from this resource
};

struct ResourceNode {
    ResourceNode(const char* name, uint16_t index) noexcept : name(name), index(index) { }
    ResourceNode(ResourceNode const&) = delete;
    ResourceNode(ResourceNode&&) noexcept = default;

    // constants
    const char* const name;
    uint16_t const index;

    // updated by the builder
    uint16_t version = 0;

    // set during compile()
    Resource* resource = nullptr;
};

struct PassNode {
    // TODO: use something less heavy than a std::vector<>
    using ResourceList = std::vector<FrameGraphResource>;

    PassNode(const char* name, uint32_t id, FrameGraphPassExecutor* base) noexcept
            : name(name), id(id), base(base) {
    }
    PassNode(PassNode const&) = delete;
    PassNode(PassNode&& rhs) noexcept
            : name(rhs.name), id(rhs.id), base(rhs.base),
              reads(std::move(rhs.reads)),
              writes(std::move(rhs.writes)),
              devirtualize(std::move(rhs.devirtualize)),
              destroy(std::move(rhs.destroy)),
              refCount(rhs.refCount) {
        rhs.base = nullptr;
    }

    PassNode& operator=(PassNode const&) = delete;
    PassNode& operator=(PassNode&&) = delete;

    ~PassNode() { delete base; }

    // for FrameGraphBuilder
    void read(ResourceNode const& resource) {
        // just record that we're reading from this resource (at the given version)
        reads.push_back({ resource.index, resource.version });
    }

    void write(ResourceNode& resource) {
        // invalidate existing handles to this resource
        ++resource.version;
        // record the write
        writes.push_back({ resource.index, resource.version });
    }

    // constants
    const char* const name;                     // our name
    const uint32_t id;                          // a unique id (only for debugging)
    FrameGraphPassExecutor* base = nullptr;     // type eraser for calling execute()

    // set by the builder
    ResourceList reads;                     // resources we're reading from
    ResourceList writes;                    // resources we're writing to

    // computed during compile()
    std::vector<uint16_t> devirtualize;     // resources we need to create before executing
    std::vector<uint16_t> destroy;          // resources we need to destroy after executing
    uint32_t refCount = 0;                  // count resources that have a reference to us
};

struct Alias {
    FrameGraphResource from, to;
};

} // namespace fg

// ------------------------------------------------------------------------------------------------

FrameGraphPassExecutor::FrameGraphPassExecutor() = default;
FrameGraphPassExecutor::~FrameGraphPassExecutor() = default;

// ------------------------------------------------------------------------------------------------

FrameGraphBuilder::FrameGraphBuilder(FrameGraph& fg, PassNode& pass) noexcept
    : mFrameGraph(fg), mPass(pass) {
}

ResourceNode* FrameGraphBuilder::getResource(FrameGraphResource input) {
    FrameGraph& frameGraph = mFrameGraph;
    auto& registry = frameGraph.mResourceNodes;

    if (!ASSERT_POSTCONDITION_NON_FATAL(input.isValid(),
            "using an uninitialized resource handle")) {
        return nullptr;
    }

    assert(input.index < registry.size());
    auto& resource = registry[input.index];

    if (!ASSERT_POSTCONDITION_NON_FATAL(input.version == resource.version,
            "using an invalid resource handle (version=%u) for resource=\"%s\" (id=%u, version=%u)",
            input.version, resource.name, resource.index, resource.version)) {
        return nullptr;
    }

    return &resource;
}

FrameGraphResource FrameGraphBuilder::createTexture(
        const char* name, CreateFlags flags, FrameGraphResource::TextureDesc const& desc) noexcept {
    FrameGraph& frameGraph = mFrameGraph;
    ResourceNode& resource = frameGraph.createResource(name);
    switch (flags) {
        case UNKNOWN:
            // we just create a resource, but we don't use it in this pass
            break;
        case READ:
            mPass.read(resource);
            break;
        case WRITE:
            mPass.write(resource);
            break;
    }
    return { resource.index, resource.version };
}

FrameGraphResource FrameGraphBuilder::read(FrameGraphResource const& input) {
    ResourceNode* resource = getResource(input);
    if (!resource) {
        return {};
    }
    mPass.read(*resource);
    return input;
}

FrameGraphResource FrameGraphBuilder::write(FrameGraphResource const& output) {
    ResourceNode* resource = getResource(output);
    if (!resource) {
        return {};
    }

    mPass.write(*resource);

    /*
     * We invalidate and rename handles that are writen into, to avoid undefined order
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

    return { resource->index, resource->version };
}

// ------------------------------------------------------------------------------------------------

FrameGraph::FrameGraph() = default;

FrameGraph::~FrameGraph() = default;

bool FrameGraph::isValid(FrameGraphResource handle) const noexcept {
    if (!handle.isValid()) return false;
    auto const& registry = mResourceNodes;
    assert(handle.index < registry.size());
    auto& resource = registry[handle.index];
    return handle.version == resource.version;
}

void FrameGraph::moveResource(FrameGraphResource from, FrameGraphResource to) {
    // FIXME: what checks need to happen?
    mAliases.push_back({from, to});
}

void FrameGraph::present(FrameGraphResource input) {
    struct Dummy {
    };
    addPass<Dummy>("Present",
            [&input](FrameGraphBuilder& builder, Dummy& data) {
                builder.read(input);
            },
            [](FrameGraphPassResources const& resources, Dummy const& data) {
            });
}

PassNode& FrameGraph::createPass(const char* name, FrameGraphPassExecutor* base) noexcept {
    auto& frameGraphPasses = mPassNodes;
    const uint32_t id = (uint32_t)frameGraphPasses.size();
    frameGraphPasses.emplace_back(name, id, base);
    return frameGraphPasses.back();
}

ResourceNode& FrameGraph::createResource(const char* name) noexcept {
    auto& registry = mResourceNodes;
    uint16_t id = (uint16_t)registry.size();
    registry.emplace_back(name, id);
    return registry.back();
}

FrameGraph& FrameGraph::compile() noexcept {
    auto& registry = mResourceNodes;
    mResourceRegistry.reserve(registry.size());

    // create the sub-resources
    for (ResourceNode& node : registry) {
        mResourceRegistry.emplace_back(node.name);
        node.resource = &mResourceRegistry.back();
    }

    // remap them
    for (auto const& alias : mAliases) {
        registry[alias.to.index].resource = registry[alias.from.index].resource;
    }

    for (PassNode& pass : mPassNodes) {
        // compute passes reference counts (i.e. resources we're writing to)
        pass.refCount = (uint32_t)pass.writes.size();

        // compute resources reference counts (i.e. resources we're reading from), and first/last users
        for (FrameGraphResource resource : pass.reads) {
            Resource* subResource = registry[resource.index].resource;

            // add a reference for each pass that reads from this resource
            subResource->readerCount++;

            // figure out which is the first pass to need this resource
            subResource->first = subResource->first ? subResource->first : &pass;

            // figure out which is the last pass to need this resource
            subResource->last = &pass;
        }

        // set the writers
        for (FrameGraphResource resource : pass.writes) {
            Resource* subResource = registry[resource.index].resource;
            subResource->writer = &pass;
            subResource->writerCount++;
        }
    }

    // cull passes and resources...
    std::vector<Resource*> stack;
    stack.reserve(registry.size());
    for (Resource& resource : mResourceRegistry) {
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

        // if a resource doesn't have a writer and is not imported, then we the graph is not
        // set correctly. For now we ignore this.
        if (!pSubResource->writer) {
            slog.d << "resource \"" << pSubResource->name << "\" is never written!" << io::endl;
            continue; // TODO: this shouldn't happen in a valid graph
        }

        PassNode* const writer = pSubResource->writer;
        assert(writer->refCount >= 1);
        if (--writer->refCount == 0) {
            // this pass is culled
            auto const& reads = writer->reads;
            for (FrameGraphResource resource : reads) {
                Resource* r = registry[resource.index].resource;
                if (--r->readerCount == 0) {
                    stack.push_back(r);
                }
            }
        }
    }

    for (size_t index = 0, c = mResourceRegistry.size() ; index < c ; index++) {
        auto& resource = mResourceRegistry[index];
        assert(!resource.first == !resource.last);
        if (resource.readerCount && resource.first && resource.last) {
            resource.first->devirtualize.push_back((uint16_t)index);
            resource.last->destroy.push_back((uint16_t)index);
        }
    }

    return *this;
}

void FrameGraph::execute() noexcept {
    auto const& resources = mResources;
    for (PassNode const& node : mPassNodes) {
        if (!node.refCount) continue;

        for (size_t id : node.devirtualize) {
            // TODO: create concrete resources
        }

        assert(node.base);
        node.base->execute(resources);

        for (uint32_t id : node.destroy) {
            // TODO: delete concrete resources
        }
    }

    // reset the frame graph state
    mPassNodes.clear();
    mResourceNodes.clear();
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
                   << (subresource->readerCount ? "skyblue" : "skyblue4") << "]\n";
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
