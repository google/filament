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

#ifndef TNT_FILAMENT_FG2_GRAPH_H
#define TNT_FILAMENT_FG2_GRAPH_H

#include <utils/ostream.h>
#include <utils/debug.h>

#include <utils/CString.h>
#include <utils/vector.h>

namespace filament {

/**
 * A very simple dependency graph (DAG) class that support culling of unused nodes
 */
class DependencyGraph {
public:
    DependencyGraph() noexcept;
    ~DependencyGraph() noexcept;
    DependencyGraph(const DependencyGraph&) noexcept = delete;
    DependencyGraph& operator=(const DependencyGraph&) noexcept = delete;

    using NodeID = uint32_t;

    class Node;

    /**
     * A link between two nodes.
     */
    struct Edge {
        // An Edge can't be modified after it's created (e.g. by copying into it)
        const NodeID from;
        const NodeID to;

        /**
         * Creates an Edge between two nodes. The caller keeps ownership of the Edge object,
         * which is only safe to destroy after calling DependencyGraph::clear(). Use
         * DependencyGraph::isEdgeValid() after culling to check that the edge is still
         * connected on both ends.
         * @param graph reference to the DependencyGraph to add the edge to
         * @param from  Node* existing in graph (no runtime check made)
         * @param to    Node* existing in graph (no runtime check made)
         */
        Edge(DependencyGraph& graph, Node* from, Node* to);

        // Edge can't be copied or moved, this is to allow subclassing safely.
        // Subclasses can hold their own data.
        Edge(Edge const& rhs) noexcept = delete;
        Edge& operator=(Edge const& rhs) noexcept = delete;
    };

    /**
     * A generic node
     */
    class Node {
    public:
        /**
         * Creates a Node and adds it to the graph. The caller keeps ownership of the Node object,
         * which is only safe to destroy after calling DependencyGraph::clear().
         * @param graph DependencyGraph pointer to add the Node to.
         */
        Node(DependencyGraph& graph) noexcept;

        // Nodes can't be copied
        Node(Node const&) noexcept = delete;
        Node& operator=(Node const&) noexcept = delete;

        //! Nodes can be moved
        Node(Node&&) noexcept = default;

        virtual ~Node() noexcept = default;

        //! returns a unique id for this node
        NodeID getId() const noexcept;

        /** Prevents this node from being culled. Must be called before culling. */
        void makeTarget() noexcept;

        /** Returns true if this Node is a target */
        bool isTarget() const noexcept;

        /**
         * Returns whether this node was culled.
         * This is only valid after DependencyGraph::cull() has been called.
         * @return true if the node has been culled, false otherwise.
         */
        bool isCulled() const noexcept;

        /**
         * return the reference count of this node. That is how many other nodes have links to us.
         * This is only valid after DependencyGraph::cull() has been called.
         * @return Number of nodes linking to this one.
         */
        uint32_t getRefCount() const noexcept;

    public:
        //! return the name of this node
        virtual char const* getName() const noexcept;

        //! output itself as a graphviz string
        virtual utils::CString graphvizify() const noexcept;

        //! output a graphviz color string for an Edge from this node
        virtual utils::CString graphvizifyEdgeColor() const noexcept;

    private:
        // nodes that read from us: i.e. we have a reference to them
        friend class DependencyGraph;
        static const constexpr uint32_t TARGET = 0x80000000u;
        uint32_t mRefCount = 0;     // how many references to us
        const NodeID mId;           // unique id
    };

    using EdgeContainer = utils::vector<Edge*>;
    using NodeContainer = utils::vector<Node*>;

    /**
     * Removes all edges and nodes from the graph.
     * Note that these objects are not destroyed (DependencyGraph doesn't own them).
     */
    void clear() noexcept;

    /** return the list of all edges */
    EdgeContainer const& getEdges() const noexcept;

    /** return the list of all nodes */
    NodeContainer const& getNodes() const noexcept;

    /**
     * Returns the list of incoming edges to a node
     * @param node the node to consider
     * @return A list of incoming edges
     */
    EdgeContainer getIncomingEdges(Node const* node) const noexcept;

    /**
     * Returns the list of outgoing edges to a node
     * @param node the node to consider
     * @return A list of outgoing edges
     */
    EdgeContainer getOutgoingEdges(Node const* node) const noexcept;

    Node const* getNode(NodeID id) const noexcept;

    Node* getNode(NodeID id) noexcept;

    //! cull unreferenced nodes. Links ARE NOT removed, only reference counts are updated.
    void cull() noexcept;

    /**
     * Return whether an edge is valid, that is if both ends are connected to nodes
     * that are not culled. Valid only after cull() is called.
     * @param edge to check the validity of.
     */
    bool isEdgeValid(Edge const* edge) const noexcept;

    //! export a graphviz view of the graph
    void export_graphviz(utils::io::ostream& out, const char* name = nullptr);

    bool isAcyclic() const noexcept;

private:
    // id must be the node key in the NodeContainer
    uint32_t generateNodeId() noexcept;
    void registerNode(Node* node, NodeID id) noexcept;
    void link(Edge* edge) noexcept;
    static bool isAcyclicInternal(DependencyGraph& graph) noexcept;
    NodeContainer mNodes;
    EdgeContainer mEdges;
};

inline DependencyGraph::Edge::Edge(DependencyGraph& graph,
        DependencyGraph::Node* from, DependencyGraph::Node* to)
        : from(from->getId()), to(to->getId()) {
    assert_invariant(graph.mNodes[this->from] == from);
    assert_invariant(graph.mNodes[this->to] == to);
    graph.link(this);
}

} // namespace filament

#endif //TNT_FILAMENT_FG2_GRAPH_H
