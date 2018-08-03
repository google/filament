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

package com.google.android.filament.tungsten.model.serialization

import com.google.android.filament.tungsten.model.ConnectionModel
import com.google.android.filament.tungsten.model.MaterialGraphModel
import com.google.android.filament.tungsten.model.NodeModel

/**
 * An interface used by GraphSerializer that maps classes of NodeModels into String type identifiers
 * for serialization. Given a type identifier, it can instantiate a new NodeModel of the correct
 * type.
 */
interface INodeFactory {

    fun createNodeForTypeIdentifier(typeIdentifier: String): NodeModel?

    fun typeIdentifierForNode(node: NodeModel): String?
}

const val GRAPH_VERSION = "0.1"
const val ROOT_NODE_ID = 0

object GraphSerializer {

    fun serialize(
        graph: MaterialGraphModel,
        editorData: Map<String, Any>,
        nodeFactory: INodeFactory,
        serializer: ISerializer = JsonSerializer()
    ): String {
        // Assign a unique id to each of the nodes in the graph (starting at 1)
        val nodeToId = mutableMapOf<NodeModel, Int>()

        val nodesToSerialize = mutableListOf<Node>()
        for ((index, node) in graph.nodes.withIndex()) {
            // The root node has a special id of 0
            val nodeId = if (node == graph.rootNode) ROOT_NODE_ID else index + 1

            nodeToId[node] = nodeId

            val typeIdentifier = nodeFactory.typeIdentifierForNode(node)
            if (typeIdentifier != null) {
                nodesToSerialize.add(Node(typeIdentifier, nodeId, node.position))
            }
        }

        val connectionsToSerialize = mutableListOf<Connection>()
        for (connection in graph.connections) {
            val fromId = nodeToId[connection.outputSlot.node]
            val fromSlot = connection.outputSlot.label
            val toId = nodeToId[connection.inputSlot.node]
            val toSlot = connection.inputSlot.label
            if (fromId != null && toId != null && fromSlot != null && toSlot != null) {
                connectionsToSerialize.add(Connection(fromId, fromSlot, toId, toSlot))
            }
        }
        return serializer.serialize(
                Graph(nodesToSerialize, connectionsToSerialize, GRAPH_VERSION, editorData)
        )
    }

    fun deserialize(
        data: String,
        nodeFactory: INodeFactory,
        deserializer: IDeserializer = JsonDeserializer()
    ): Pair<MaterialGraphModel, Map<String, Any>> {
        val graph = MaterialGraphModel()
        val idToNode = HashMap<Int, NodeModel>()
        val (nodes, connections, _, editor) = deserializer.deserialize(data)

        for (node in nodes) {
            val newNode = nodeFactory.createNodeForTypeIdentifier(node.type)
            if (newNode != null) {
                if (node.id == ROOT_NODE_ID) {
                    graph.rootNode = newNode
                }
                node.position?.let { p -> newNode.setPosition(p.x, p.y) }
                graph.addNode(newNode)
                idToNode[node.id] = newNode
            }
        }

        for (connection in connections) {
            val fromNode = idToNode[connection.from.id]
            val outputSlotName = connection.from.name
            val toNode = idToNode[connection.to.id]
            val inputSlotName = connection.to.name

            // Ensure we have two valid nodes
            if (fromNode == null || toNode == null) {
                break
            }

            // Ensure we have two valid slots
            val outputSlot = fromNode.getOutputSlot(outputSlotName)
            val inputSlot = toNode.getInputSlot(inputSlotName)
            if (outputSlot == null || inputSlot == null) {
                break
            }

            inputSlot.connectTo(outputSlot)

            val newConnection = ConnectionModel(outputSlot, inputSlot)
            graph.createConnection(newConnection)
        }

        return Pair(graph, editor)
    }
}
