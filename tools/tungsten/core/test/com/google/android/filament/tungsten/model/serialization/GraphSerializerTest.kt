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

import com.google.android.filament.tungsten.model.Float3
import com.google.android.filament.tungsten.model.Graph
import com.google.android.filament.tungsten.model.Node
import com.google.android.filament.tungsten.model.NodeId
import com.google.android.filament.tungsten.model.Property
import com.google.android.filament.tungsten.model.copyPropertyWithValue
import com.google.android.filament.tungsten.properties.ColorChooser
import org.junit.Assert.assertEquals
import org.junit.Test

/**
 * These test cases perform round-trip serialization. A Graph is serialized to a String, then
 * deserialized back to a Graph. The result is verified to equal the original.
 */
class GraphSerializerTest {

    private val mockNodeFactory = object : INodeFactory {

        override fun createNodeForTypeIdentifier(typeIdentifier: String, id: NodeId): Node? {
            return if (typeIdentifier == "node") createMockNode(id) else null
        }
    }

    @Test
    fun `Serialize empty model`() {
        val graph = Graph()
        val serialized = GraphSerializer.serialize(graph, emptyMap())
        val (deserialized, _) = GraphSerializer.deserialize(serialized, mockNodeFactory)
        assertEquals(graph, deserialized)
    }

    @Test
    fun `Serialize editor data`() {
        val serialized = GraphSerializer.serialize(Graph(), mapOf("foo" to "bar"))
        val (_, editorData) = GraphSerializer.deserialize(serialized, mockNodeFactory)
        assertEquals(mapOf("foo" to "bar"), editorData)
    }

    @Test
    fun `Serialize nodes and connections`() {
        val first = createMockNode(0)
        val second = first.copy(id = 1)
        val root = first.copy(id = 2)

        val graph = Graph(
                nodes = listOf(first, second, root),
                rootNodeId = root.id,
                connections = listOf(
                        first.getOutputSlot("output") to second.getInputSlot("input"),
                        second.getOutputSlot("output") to root.getInputSlot("input")
                )
        )

        val serialized = GraphSerializer.serialize(graph, emptyMap())
        val (deserialized, _) = GraphSerializer.deserialize(serialized, mockNodeFactory)

        assertEquals(graph, deserialized)
    }

    /**
     * If a node type identifier is not recognized, avoid adding the node or connection to the
     * graph.
     */
    @Test
    fun `Serialize connection with unknown node type`() {
        val first = createMockNode(0)
        val root = createMockNode(1).copy(type = "unknown type")

        val graph = Graph(
                nodes = listOf(first, root),
                rootNodeId = root.id,
                connections = listOf(first.getOutputSlot("output") to root.getInputSlot("input")))

        val serialized = GraphSerializer.serialize(graph, emptyMap())
        val (deserialized, _) = GraphSerializer.deserialize(serialized, mockNodeFactory)

        assertEquals(1, deserialized.nodes.size)
        assertEquals(graph.nodes[0], deserialized.nodes[0])
        assertEquals(0, deserialized.connections.size)
    }

    @Test
    fun `Serialize a property`() {
        val node = createMockNode(0).copy()

        // Create a graph with a non-default property value
        val graph = Graph(
                nodes = listOf(node),
                rootNodeId = 0
        ).graphByChangingProperty(node.getPropertyHandle("mockProperty"),
                copyPropertyWithValue(mockProperty, Float3(1.0f, 2.0f, 3.0f)))

        val serialized = GraphSerializer.serialize(graph, emptyMap())
        val (deserialized, _) = GraphSerializer.deserialize(serialized, mockNodeFactory)

        assertEquals(graph, deserialized)
    }

    private val mockProperty = Property(
        name = "mockProperty",
        value = Float3(),
        editorFactory = ::ColorChooser
    )

    private fun createMockNode(id: NodeId) = Node(
        id = id,
        type = "node",
        inputSlots = listOf("input"),
        outputSlots = listOf("output"),
        properties = listOf(mockProperty)
    )
}
