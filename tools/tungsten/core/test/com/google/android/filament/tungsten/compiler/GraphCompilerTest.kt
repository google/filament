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

package com.google.android.filament.tungsten.compiler

import com.google.android.filament.tungsten.model.Graph
import com.google.android.filament.tungsten.model.Node
import com.google.android.filament.tungsten.model.createAddNode
import com.google.android.filament.tungsten.model.createFloat3ConstantNode
import com.google.android.filament.tungsten.model.createShaderNode
import org.junit.Assert.assertNotEquals
import org.junit.Test

class GraphCompilerTest {

    /**
     * For now, we just check that the graphs can compile without throwing any errors.
     * TODO: add more robust checks
     */

    @Test
    fun `Compile single node`() {
        val adderNode = createAddNode(0)
        val graph = Graph(nodes = listOf(adderNode), rootNodeId = adderNode.id)
        val compiler = GraphCompiler(graph)
        compiler.compileGraph()
    }

    @Test
    fun `Compile two nodes with connections`() {
        val constantNodeA = createFloat3ConstantNode(0)
        val constantNodeB = createFloat3ConstantNode(1)
        val adderNode = createAddNode(2)
        val shaderNode = createShaderNode(3)
        val graph = Graph(
                nodes = listOf(shaderNode, adderNode, constantNodeA, constantNodeB),
                rootNodeId = shaderNode.id,
                connections = listOf(
                        adderNode.getOutputSlot("out") to shaderNode.getInputSlot("baseColor"),
                        constantNodeA.getOutputSlot("result") to adderNode.getInputSlot("a"),
                        constantNodeB.getOutputSlot("result") to adderNode.getInputSlot("b")
                )
        )

        val compiler = GraphCompiler(graph)
        compiler.compileGraph()
    }

    @Test
    fun `multiple material parameters have unique names`() {
        val compiler = GraphCompiler(createMockGraph())
        val first = compiler.addParameter("float3", "parameter")
        val second = compiler.addParameter("float3", "parameter")
        assertNotEquals(first, second)
    }

    private fun createMockGraph() = Graph(
            nodes = listOf(Node(id = 0, type = "node")),
            rootNodeId = 0)
}