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

import com.google.android.filament.tungsten.model.Connection
import com.google.android.filament.tungsten.model.Graph
import com.google.android.filament.tungsten.model.Node
import com.google.android.filament.tungsten.model.NodeId
import com.google.android.filament.tungsten.model.copyPropertyWithValue
import java.awt.Point

/**
 * An interface used by GraphSerializer that maps classes of NodeModels into String type identifiers
 * for serialization. Given a type identifier, it can instantiate a new NodeModel of the correct
 * type.
 */
interface INodeFactory {

    fun createNodeForTypeIdentifier(typeIdentifier: String, id: NodeId): Node?
}

const val GRAPH_VERSION = "0.1"

object GraphSerializer {

    fun serialize(
        graph: Graph,
        editorData: Map<String, Any>,
        serializer: ISerializer = JsonSerializer()
    ): String {
        val nodesToSerialize = graph.nodes.map { n ->
            val properties = n.properties.mapNotNull {
                val serialized = it.value.serialize()
                if (serialized == null) null else it.name to serialized
            }.toMap()
            NodeSchema(n.type, n.id, Point(n.x.toInt(), n.y.toInt()), properties)
        }

        val connectionsToSerialize = graph.connections.map { c ->
            val fromId = c.outputSlot.nodeId
            val fromSlot = c.outputSlot.name
            val toId = c.inputSlot.nodeId
            val toSlot = c.inputSlot.name
            ConnectionSchema(fromId, fromSlot, toId, toSlot)
        }

        return serializer.serialize(
                GraphSchema(nodesToSerialize, graph.rootNodeId, connectionsToSerialize,
                        GRAPH_VERSION, editorData)
        )
    }

    fun deserialize(
        data: String,
        nodeFactory: INodeFactory,
        deserializer: IDeserializer = JsonDeserializer()
    ): Pair<Graph, Map<String, Any>> {
        val (nodes, rootNode, connections, _, editor) = deserializer.deserialize(data)

        val nodesInGraph = nodes.mapNotNull { n ->
            val newNode = nodeFactory.createNodeForTypeIdentifier(n.type, n.id)
            val position = n.position ?: Point(0, 0)
            if (newNode == null) return@mapNotNull null
            // Allow properties specified in the serialized data to override default properties
            // of the node.
            val overridenProperties = newNode.properties.map { p ->
                val override = n.properties?.get(p.name) ?: p
                val newValue = p.value.deserialize(override)
                copyPropertyWithValue(p, newValue)
            }
            newNode.copy(
                    x = position.x.toFloat(),
                    y = position.y.toFloat(),
                    properties = overridenProperties)
        }

        val nodeMap = nodesInGraph.associateBy { n -> n.id }

        val connectionsInGraph = connections.mapNotNull { c ->
            // Only add connections if they refer to valid nodes in the graph
            if (c.from.id in nodeMap && c.to.id in nodeMap)
            Connection(Node.OutputSlot(c.from.id, c.from.name),
                    Node.InputSlot(c.to.id, c.to.name)) else null
        }

        val graph = Graph(nodes = nodesInGraph, rootNodeId = rootNode,
                connections = connectionsInGraph)
        return Pair(graph, editor)
    }
}
