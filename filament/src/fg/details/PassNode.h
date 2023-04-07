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

#ifndef TNT_FILAMENT_FG_DETAILS_PASSNODE_H
#define TNT_FILAMENT_FG_DETAILS_PASSNODE_H

#include "fg/details/DependencyGraph.h"
#include "fg/details/Utilities.h"
#include "fg/FrameGraph.h"
#include "fg/FrameGraphRenderPass.h"

#include "backend/DriverApiForward.h"

#include <backend/TargetBufferInfo.h>

#include <unordered_set>

namespace utils {
class CString;
} // namespace utils

namespace filament {

class FrameGraph;
class FrameGraphResources;
class FrameGraphPassExecutor;
class ResourceNode;

class PassNode : public DependencyGraph::Node {
protected:
    friend class FrameGraphResources;
    FrameGraph& mFrameGraph;
    std::unordered_set<FrameGraphHandle::Index> mDeclaredHandles;
public:
    explicit PassNode(FrameGraph& fg) noexcept;
    PassNode(PassNode&& rhs) noexcept;
    PassNode(PassNode const&) = delete;
    PassNode& operator=(PassNode const&) = delete;
    ~PassNode() noexcept override;
    using NodeID = DependencyGraph::NodeID;

    void registerResource(FrameGraphHandle resourceHandle) noexcept;

    virtual void execute(FrameGraphResources const& resources, backend::DriverApi& driver) noexcept = 0;
    virtual void resolve() noexcept = 0;
    utils::CString graphvizifyEdgeColor() const noexcept override;

    Vector<VirtualResource*> devirtualize;         // resources we need to create before executing
    Vector<VirtualResource*> destroy;              // resources we need to destroy after executing
};

class RenderPassNode : public PassNode {
public:
    class RenderPassData {
    public:
        static constexpr size_t ATTACHMENT_COUNT = backend::MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 2;
        const char* name = {};
        FrameGraphRenderPass::Descriptor descriptor;
        bool imported = false;
        backend::TargetBufferFlags targetBufferFlags = {};
        FrameGraphId<FrameGraphTexture> attachmentInfo[ATTACHMENT_COUNT] = {};
        ResourceNode* incoming[ATTACHMENT_COUNT] = {};  // nodes of the incoming attachments
        ResourceNode* outgoing[ATTACHMENT_COUNT] = {};  // nodes of the outgoing attachments
        struct {
            backend::Handle<backend::HwRenderTarget> target;
            backend::RenderPassParams params;
        } backend;

        void devirtualize(FrameGraph& fg, ResourceAllocatorInterface& resourceAllocator) noexcept;
        void destroy(ResourceAllocatorInterface& resourceAllocator) const noexcept;
    };

    RenderPassNode(FrameGraph& fg, const char* name, FrameGraphPassBase* base) noexcept;
    RenderPassNode(RenderPassNode&& rhs) noexcept;
    ~RenderPassNode() noexcept override;

    uint32_t declareRenderTarget(FrameGraph& fg, FrameGraph::Builder& builder,
            const char* name, FrameGraphRenderPass::Descriptor const& descriptor);

    RenderPassData const* getRenderPassData(uint32_t id) const noexcept;

private:
    // virtuals from DependencyGraph::Node
    char const* getName() const noexcept override { return mName; }
    utils::CString graphvizify() const noexcept override;
    void execute(FrameGraphResources const& resources, backend::DriverApi& driver) noexcept override;
    void resolve() noexcept override;

    // constants
    const char* const mName = nullptr;
    UniquePtr<FrameGraphPassBase, LinearAllocatorArena> mPassBase;

    // set during setup
    std::vector<RenderPassData> mRenderTargetData;
};

class PresentPassNode : public PassNode {
public:
    explicit PresentPassNode(FrameGraph& fg) noexcept;
    PresentPassNode(PresentPassNode&& rhs) noexcept;
    ~PresentPassNode() noexcept override;
    PresentPassNode(PresentPassNode const&) = delete;
    PresentPassNode& operator=(PresentPassNode const&) = delete;
    void execute(FrameGraphResources const& resources, backend::DriverApi& driver) noexcept override;
    void resolve() noexcept override;
private:
    // virtuals from DependencyGraph::Node
    char const* getName() const noexcept override;
    utils::CString graphvizify() const noexcept override;
};

} // namespace filament

#endif // TNT_FILAMENT_FG_DETAILS_PASSNODE_H
