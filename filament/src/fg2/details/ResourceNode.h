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

#ifndef TNT_FILAMENT_FG2_RESOURCENODE_H
#define TNT_FILAMENT_FG2_RESOURCENODE_H

#include "fg2/details/DependencyGraph.h"
#include "fg2/details/Utilities.h"

#include <utils/vector.h>

namespace utils {
class CString;
} // namespace utils

namespace filament {

class FrameGraph;
class ResourceEdgeBase;

class ResourceNode : public DependencyGraph::Node {
public:
    ResourceNode(FrameGraph& fg, FrameGraphHandle h, FrameGraphHandle parent) noexcept;
    ~ResourceNode() noexcept override;

    ResourceNode(ResourceNode const&) = delete;
    ResourceNode& operator=(ResourceNode const&) = delete;

    void addOutgoingEdge(ResourceEdgeBase* edge) noexcept;
    void setIncomingEdge(ResourceEdgeBase* edge) noexcept;

    // constants
    const FrameGraphHandle resourceHandle;


    // is a PassNode writing to this ResourceNode
    bool hasWriterPass() const noexcept {
        return mWriterPass != nullptr;
    }

    // is any non culled Node (of any type) writing to this ResourceNode
    bool hasActiveWriters() const noexcept;

    // is the specified PassNode writing to this resource, if so return the corresponding edge.
    ResourceEdgeBase* getWriterEdgeForPass(PassNode const* node) const noexcept;
    bool hasWriteFrom(PassNode const* node) const noexcept;


    // is at least one PassNode reading from this ResourceNode
    bool hasReaders() const noexcept {
        return !mReaderPasses.empty();
    }

    // is any non culled Node (of any type) reading from this ResourceNode
    bool hasActiveReaders() const noexcept;

    // is the specified PassNode reading this resource, if so return the corresponding edge.
    ResourceEdgeBase* getReaderEdgeForPass(PassNode const* node) const noexcept;


    void resolveResourceUsage(DependencyGraph& graph) noexcept;

    ResourceNode* getParentNode() noexcept;

    // this is the parent resource we're reading from, as a propagating effect of
    // us being read from.
    void setParentReadDependency(ResourceNode* parent) noexcept;

    // this is the parent resource we're writing to, as a propagating effect of
    // us being writen to.
    void setParentWriteDependency(ResourceNode* parent) noexcept;

    void setForwardResourceDependency(ResourceNode* source) noexcept;

    // virtuals from DependencyGraph::Node
    char const* getName() const noexcept override;

private:
    FrameGraph& mFrameGraph;
    Vector<ResourceEdgeBase *> mReaderPasses;
    ResourceEdgeBase* mWriterPass = nullptr;
    FrameGraphHandle mParentHandle;
    DependencyGraph::Edge* mParentReadEdge = nullptr;
    DependencyGraph::Edge* mParentWriteEdge = nullptr;
    DependencyGraph::Edge* mForwardedEdge = nullptr;

    // virtuals from DependencyGraph::Node
    utils::CString graphvizify() const noexcept override;
    utils::CString graphvizifyEdgeColor() const noexcept override;
};

} // namespace filament

#endif //TNT_FILAMENT_FG2_RESOURCENODE_H
