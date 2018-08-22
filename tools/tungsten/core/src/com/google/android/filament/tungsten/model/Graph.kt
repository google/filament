/*
 * Copyright (C) 2018 The Android Open Source Project
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

package com.google.android.filament.tungsten.model

import com.google.android.filament.tungsten.compiler.GraphCompiler
import kotlin.reflect.KProperty

data class Connection(
    val outputSlot: Node.OutputSlot,
    val inputSlot: Node.InputSlot
)

abstract class Slot

typealias NodeId = Int

/**
 * An immutable Node in the graph.
 * To modify properties of the node, use the automatically-generated .copy function.
 */
data class Node(
    // A unique node id to identify this node in a graph
    val id: NodeId,
    val x: Float = 0.0f,
    val y: Float = 0.0f,
    val type: String,

    // A node's compile function generates code by calling methods on a GraphCompiler. The function
    // can return a new node if compilation has changed properties of the node itself.
    val compileFunction: (Node, GraphCompiler) -> Node = { n, _ -> n },

    // Input and output slots of a node are represented by a String name.
    val outputSlots: List<String> = emptyList(),
    val inputSlots: List<String> = emptyList(),

    val properties: List<Property> = emptyList()
) {

    data class InputSlot(val nodeId: NodeId, val name: String) : Slot()

    data class OutputSlot(val nodeId: NodeId, val name: String) : Slot() {

        /**
         * This allows some semantic sugar to construct a Connection:
         * outputSlot to inputSlot
         */
        infix fun to(other: InputSlot): Connection {
            return Connection(this, other)
        }
    }

    fun getInputSlot(name: String): InputSlot {
        return InputSlot(id, name)
    }

    fun getOutputSlot(name: String): OutputSlot {
        return OutputSlot(id, name)
    }
}

class ConnectionMapper {

    operator fun getValue(thisRef: Any?, property: KProperty<*>):
            Map<Node.InputSlot, Node.OutputSlot> {
        val graph = thisRef as Graph
        return graph.connections.associate { c ->
            c.inputSlot to c.outputSlot
        }
    }
}

class NodeMapper {

    operator fun getValue(thisRef: Any?, property: KProperty<*>): Map<NodeId, Node> {
        val graph = thisRef as Graph
        return graph.nodes.associateBy { n -> n.id }
    }
}

/**
 * An immutable graph.
 * To modify, use the generated .copy function or one of the graphBy* convenience functions.
 */
data class Graph(
    val nodes: List<Node> = emptyList(),
    val rootNodeId: NodeId? = null,
    val selection: List<NodeId> = emptyList(),
    val connections: List<Connection> = emptyList()
) {

    /**
     * connectionMap maps InputSlots to connected OutputSlots and aids graph compilation by
     * efficiently answering the question: "is this input connected to any output?"
     * It is lazily-computed by ConnectionMapper when needed.
     */
    private val connectionMap: Map<Node.InputSlot, Node.OutputSlot> by ConnectionMapper()

    /**
     * Lazily computes a map from NodeId to Node used for efficient Node lookup.
     */
    private val nodeMap: Map<NodeId, Node> by NodeMapper()

    /**
     * Returns the next available NodeId for this graph.
     */
    fun getNewNodeId(): NodeId {
        return nodes.size
    }

    fun getRootNode(): Node? {
        return nodeMap[rootNodeId]
    }

    fun getNodeWithId(id: NodeId): Node? {
        return nodeMap[id]
    }

    fun getOutputSlotConnectedToInput(slot: Node.InputSlot): Node.OutputSlot? {
        return connectionMap[slot]
    }

    fun isNodeSelected(node: Node): Boolean {
        return selection.contains(node.id)
    }

    fun getNodeForOutputSlot(slot: Node.OutputSlot): Node? {
        return nodeMap[slot.nodeId]
    }

    fun getSelectedNodes(): List<Node> {
        return nodes.filter { n -> selection.contains(n.id) }
    }

    /**
     * The following convenience methods return a new copy of the graph with certain attributes
     * modified.
     */

    fun graphByAddingNode(node: Node): Graph {
        return this.copy(nodes = nodes + node)
    }

    fun graphByChangingSelection(selection: List<NodeId>): Graph {
        return this.copy(selection = selection)
    }

    fun graphByAddingNodeAtLocation(node: Node, x: Float, y: Float): Graph {
        return this.copy(nodes = nodes + node.copy(x = x, y = y))
    }

    fun graphByReplacingNode(oldNode: Node, newNode: Node): Graph {
        if (nodes.contains(oldNode)) {
            return this.copy(nodes = nodes - oldNode + newNode)
        }
        return this
    }

    fun graphByChangingProperty(id: NodeId, propertyName: String, value: PropertyValue): Graph {
        val node = nodeMap[id] ?: return this
        val newProperties = node.properties.map {
            p -> if (p.name == propertyName) Property(propertyName, value) else p
        }
        return graphByReplacingNode(node, node.copy(properties = newProperties))
    }

    fun graphByMovingNode(node: Node, x: Float, y: Float): Graph {
        return graphByReplacingNode(node, node.copy(x = x, y = y))
    }

    fun graphByFormingConnection(connection: Connection): Graph {
        return this.copy(connections = connections + connection)
    }

    fun graphByRemovingConnection(connection: Connection): Graph {
        return this.copy(connections = connections - connection)
    }
}
