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
import com.google.android.filament.tungsten.model.InputSlotModel
import com.google.android.filament.tungsten.model.MaterialGraphModel
import com.google.android.filament.tungsten.model.NodeModel
import com.google.android.filament.tungsten.model.OutputSlotModel
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test
import org.mockito.Mockito.`when`
import org.mockito.Mockito.mock
import org.mockito.Mockito.reset

/**
 * These test cases perform round-trip serialization. A MaterialGraphModel is serialized to a
 * String, then deserialized back to a MaterialGraphModel. The result is verified to contain the
 * correct nodes and connections.
 */
class GraphSerializerTest {

    private val mockNodeModels = ArrayList<NodeModel>()
    private val mockConnections = ArrayList<ConnectionModel>()

    private val graphModel = mock(MaterialGraphModel::class.java)
    private val nodeFactory = mock(INodeFactory::class.java)

    @Before
    fun setUp() {
        `when`(graphModel.nodes).thenReturn(mockNodeModels)
        `when`(graphModel.connections).thenReturn(mockConnections)
        mockNodeModels.clear()
        reset(nodeFactory)
    }

    @Test
    fun test_EmptyModel() {
        val (deserialized, _) = serializeDeserialize()
        assertEquals(0, deserialized.nodes.size)
        assertEquals(0, deserialized.connections.size)
    }

    @Test
    fun test_EditorData() {
        val (_, editor) = serializeDeserialize(editorData = mapOf(Pair("foo", "bar")))
        assertEquals("bar", editor["foo"])
    }

    @Test
    fun test_NodesAndConnections() {
        val first = addMockNodeWithIdentifier("first")
        val second = addMockNodeWithIdentifier("second")
        val root = addMockNodeWithIdentifier("third", isRoot = true)

        connectNodes(first, second)
        connectNodes(second, root)

        val (deserialized, _) = serializeDeserialize()

        // Assert that the nodes are connected
        assertEquals(2, deserialized.connections.size)
        verifyConnection(deserialized, 0, first, second)
        verifyConnection(deserialized, 1, second, root)

        // Assert that the root node has been set correctly
        assertEquals(root, deserialized.rootNode)
    }

    /**
     * If a node type identifier is not recognized, avoid adding the node or connection to the
     * graph.
     */
    @Test
    fun test_ConnectionNotRegistered() {
        val first = addMockNodeWithIdentifier("first")
        val second = addMockNodeWithIdentifier("notRegistered", register = false)
        connectNodes(first, second)

        val (deserialized, _) = serializeDeserialize()

        assertEquals(0, deserialized.connections.size)
        assertEquals(1, deserialized.nodes.size)
        assertEquals(first, deserialized.nodes[0])
    }

    /**
     * Serialize two nodes and a connection from an output slot to an unrecognized input slot. The
     * nodes should still be serialized, but not the connection.
     */
    @Test
    fun test_TwoNodesOneConnectionBadSlotName() {
        val first = addMockNodeWithIdentifier("first")
        val second = addMockNodeWithIdentifier("second")
        connectNodes(first, second)

        val serializer = JsonSerializer()
        val serialized = serializer.serialize(Graph(listOf(Node("first", 1), Node("second", 2)),
                listOf(Connection(1, "output", 2, "bad slot")), "1.0"))

        val (deserialized, _) = GraphSerializer.deserialize(serialized, nodeFactory)

        // There should not be any connections and 2 nodes
        assertEquals(0, deserialized.connections.size)
        assertEquals(2, deserialized.nodes.size)
        assertEquals(first, deserialized.nodes[0])
        assertEquals(second, deserialized.nodes[1])
    }

    private fun serializeDeserialize(
        editorData: Map<String, Any> = emptyMap()
    ): Pair<MaterialGraphModel, Map<String, Any>> {
        val serialized = GraphSerializer.serialize(graphModel, editorData, nodeFactory)
        return GraphSerializer.deserialize(serialized, nodeFactory)
    }

    private fun verifyConnection(
        deserialized: MaterialGraphModel,
        index: Int,
        first: NodeModel,
        second: NodeModel
    ) {
        assertEquals(first, deserialized.connections[index].outputSlot.node)
        assertEquals(second, deserialized.connections[index].inputSlot.node)
        assertEquals("output", deserialized.connections[index].outputSlot.label)
        assertEquals("input", deserialized.connections[index].inputSlot.label)
        assertEquals(deserialized.connections[index].outputSlot,
                deserialized.connections[index].inputSlot.connectedSlot)
    }

    private fun connectNodes(first: NodeModel, second: NodeModel) {
        val conn = ConnectionModel(first.getOutputSlot("output"), second.getInputSlot("input"))
        mockConnections.add(conn)
        graphModel.createConnection(conn)
    }

    private fun addMockNodeWithIdentifier(
        typeIdentifier: String,
        isRoot: Boolean = false,
        register: Boolean = true
    ): NodeModel {
        val node = createMockNode()
        mockNodeModels.add(node)
        if (isRoot) {
            `when`(graphModel.rootNode).thenReturn(node)
        }
        if (register) {
            `when`(nodeFactory.createNodeForTypeIdentifier(typeIdentifier)).thenReturn(node)
            `when`(nodeFactory.typeIdentifierForNode(node)).thenReturn(typeIdentifier)
        }
        return node
    }

    /**
     * Create a mock node with 2 slots: "input" and "output".
     */
    private fun createMockNode(): NodeModel {
        val node = mock(NodeModel::class.java)
        `when`(node.getInputSlot("input")).thenReturn(InputSlotModel("input", node))
        `when`(node.getOutputSlot("output")).thenReturn(OutputSlotModel("output", node))
        return node
    }
}
