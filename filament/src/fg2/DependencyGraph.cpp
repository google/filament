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

#include "fg2/details/DependencyGraph.h"

namespace filament::fg2 {

DependencyGraph::DependencyGraph() noexcept {
    // Some reasonable defaults size for our vectors
    mNodes.reserve(8);
    mEdges.reserve(16);
}

uint32_t DependencyGraph::generateNodeId() noexcept {
    return mNodes.size();
}

void DependencyGraph::registerNode(Node* node, NodeID id) noexcept {
    // node* is not fully constructed here
    assert_invariant(id == mNodes.size());
    mNodes.push_back(node);
}

bool DependencyGraph::isEdgeValid(DependencyGraph::Edge const* edge) const noexcept {
    auto& nodes = mNodes;
    Node const* from = nodes[edge->from];
    Node const* to = nodes[edge->to];
    return !from->isCulled() && !to->isCulled();
}

void DependencyGraph::link(DependencyGraph::Edge* edge) noexcept {
    mEdges.push_back(edge);
}

DependencyGraph::EdgeContainer const& DependencyGraph::getEdges() const noexcept {
    return mEdges;
}


DependencyGraph::NodeContainer const& DependencyGraph::getNodes() const noexcept {
    return mNodes;
}

DependencyGraph::EdgeContainer DependencyGraph::getIncomingEdges(
        DependencyGraph::Node const* node) const noexcept {
    // TODO: we might need something more efficient
    EdgeContainer result;
    result.reserve(mEdges.size());
    for (auto* edge : mEdges) {
        if (edge->to == node->getId()) {
            result.push_back(edge);
        }
    }
    return result;
}

DependencyGraph::EdgeContainer DependencyGraph::getOutgoingEdges(
        DependencyGraph::Node const* node) const noexcept {
    // TODO: we might need something more efficient
    EdgeContainer result;
    result.reserve(mEdges.size());
    for (auto* edge : mEdges) {
        if (edge->from == node->getId()) {
            result.push_back(edge);
        }
    }
    return result;
}

DependencyGraph::Node const* DependencyGraph::getNode(DependencyGraph::NodeID id) const noexcept {
    return mNodes[id];
}

DependencyGraph::Node* DependencyGraph::getNode(DependencyGraph::NodeID id) noexcept {
    return mNodes[id];
}

void DependencyGraph::cull() noexcept {
    auto& nodes = mNodes;
    auto& edges = mEdges;

    // update reference counts
    for (Edge* const pEdge : edges) {
        Node* node = nodes[pEdge->from];
        node->mRefCount++;
    }

    // cull nodes with a 0 reference count
    std::vector<Node*> stack;
    stack.reserve(nodes.size());
    for (Node* const pNode : nodes) {
        if (pNode->getRefCount() == 0) {
            stack.push_back(pNode);
        }
    }
    while (!stack.empty()) {
        Node* const pNode = stack.back();
        stack.pop_back();
        EdgeContainer const& incoming = getIncomingEdges(pNode);
        for (Edge* edge : incoming) {
            Node* pLinkedNode = getNode(edge->from);
            if (--pLinkedNode->mRefCount == 0) {
                stack.push_back(pLinkedNode);
            }
        }
        pNode->onCulled(this);
    }
}

void DependencyGraph::clear() noexcept {
    mEdges.clear();
    mNodes.clear();
}

void DependencyGraph::export_graphviz(utils::io::ostream& out, char const* name) {
#ifndef NDEBUG
    const char* graphName = name ? name : "graph";
    out << "digraph \"" << graphName << "\" {\n";
    out << "rankdir = LR\n";
    out << "bgcolor = black\n";
    out << "node [shape=rectangle, fontname=\"helvetica\", fontsize=10]\n\n";

    auto const& nodes = mNodes;

    for (Node const* node : nodes) {
        uint32_t id = node->getId();
        utils::CString s = node->graphvizify();
        out << "\"N" << id << "\" " << s.c_str() << "\n";
    }

    out << "\n";
    for (Node const* node : nodes) {
        uint32_t id = node->getId();

        auto edges = getOutgoingEdges(node);
        auto first = edges.begin();
        auto pos = std::partition(first, edges.end(),
                [this](auto const& edge) { return isEdgeValid(edge); });

        utils::CString s = node->graphvizifyEdgeColor();

        // render the valid edges
        if (first != pos) {
            out << "N" << id << " -> { ";
            while (first != pos) {
                Node const* ref = getNode((*first++)->to);
                out << "N" << ref->getId() << " ";
            }
            out << "} [color=" << s.c_str() << "2]\n";
        }

        // render the invalid edges
        if (first != edges.end()) {
            out << "N" << id << " -> { ";
            while (first != edges.end()) {
                Node const* ref = getNode((*first++)->to);
                out << "N" << ref->getId() << " ";
            }
            out << "} [color=" << s.c_str() << "4 style=dashed]\n";
        }
    }

    out << "}" << utils::io::endl;
#endif
}

bool DependencyGraph::isAcyclic() const noexcept {
#ifndef NDEBUG
    // We work on a copy of the graph
    DependencyGraph graph;
    graph.mEdges = mEdges;
    graph.mNodes = mNodes;
    return DependencyGraph::isAcyclicInternal(graph);
#else
    return true;
#endif
}
bool DependencyGraph::isAcyclicInternal(DependencyGraph& graph) noexcept {
#ifndef NDEBUG
    while (!graph.mNodes.empty() && !graph.mEdges.empty()) {
        // check if we have at lest one leaf (i.e. nodes that have incoming but no outgoing edges)
        auto pos = std::find_if(graph.mNodes.begin(), graph.mNodes.end(),
                [&graph](Node const* node) {
                    auto pos = std::find_if(graph.mEdges.begin(), graph.mEdges.end(),
                            [node](Edge const* edge) {
                                return edge->from == node->getId();
                            });
                    return pos == graph.mEdges.end();
                });

        if (pos == graph.mNodes.end()) {
            return false;   // cyclic
        }

        // remove the leaf's edges
        auto last = std::remove_if(graph.mEdges.begin(), graph.mEdges.end(),
                [&pos](Edge const* edge) {
            return edge->to == (*pos)->getId() || edge->from == (*pos)->getId();
        });
        graph.mEdges.erase(last, graph.mEdges.end());

        // remove the leaf
        graph.mNodes.erase(pos);
    }
#endif
    return true; // acyclic
}

// ------------------------------------------------------------------------------------------------

DependencyGraph::Node::Node(DependencyGraph& graph) noexcept : mId(graph.generateNodeId()) {
    graph.registerNode(this, mId);
}

DependencyGraph::Node::~Node() noexcept = default;

uint32_t DependencyGraph::Node::getRefCount() const noexcept {
    return (mRefCount & TARGET) ? 1u : mRefCount;
}

uint32_t DependencyGraph::Node::getId() const noexcept {
    return mId;
}

bool DependencyGraph::Node::isCulled() const noexcept {
    return mRefCount == 0;
}

void DependencyGraph::Node::makeTarget() noexcept {
    assert_invariant(mRefCount == 0 || mRefCount == TARGET);
    mRefCount = TARGET;
}

bool DependencyGraph::Node::isTarget() const noexcept {
    return mRefCount >= TARGET;
}

char const* DependencyGraph::Node::getName() const noexcept {
    return "unknown";
}

void DependencyGraph::Node::onCulled(DependencyGraph* graph) noexcept {
}

utils::CString DependencyGraph::Node::graphvizify() const noexcept {
#ifndef NDEBUG
    std::string s;
    s.reserve(128);

    uint32_t id = getId();
    const char* const nodeName = getName();
    uint32_t const refCount = getRefCount();

    s.append("[label=\"");
    s.append(nodeName);
    s.append("\\nrefs: ");
    s.append(std::to_string(refCount));
    s.append(", id: ");
    s.append(std::to_string(id));
    s.append("\", style=filled, fillcolor=");
    s.append(refCount ? "skyblue" : "skyblue4");
    s.append("]");
    s.shrink_to_fit();

    return utils::CString{ s.c_str() };
#else
    return {};
#endif
}

utils::CString DependencyGraph::Node::graphvizifyEdgeColor() const noexcept {
#ifndef NDEBUG
    return utils::CString{ "darkolivegreen" };
#else
    return {};
#endif
}

} // namespace filament::fg2
