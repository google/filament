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

import com.google.android.filament.tungsten.model.Node
import com.google.android.filament.tungsten.model.NodeId
import com.google.android.filament.tungsten.model.createAdderNode
import com.google.android.filament.tungsten.model.createFloat3ConstantNode
import com.google.android.filament.tungsten.model.createShaderNode
import com.google.android.filament.tungsten.model.serialization.INodeFactory

class NodeRegistry : INodeFactory {

    /**
     * label is a human-readable name used for menus
     * typeIdentifier is a string that uniquely identifiers the NodeModel class for serialization
     */
    private data class NodeEntry(
        val label: String,
        val typeIdentifier: String,
        val factoryFunction: (id: NodeId) -> Node
    )

    private val mNodes: List<NodeEntry>

    val nodeLabelsForMenu: List<String>

    init {
        mNodes = listOf(
                NodeEntry("Add", "adder", createAdderNode),
                NodeEntry("Constant", "float3Constant", createFloat3ConstantNode),
                NodeEntry("Shader", "shader", createShaderNode)
        )
        nodeLabelsForMenu = mNodes
                // The ShaderNode should not be visible in menus
                .filter { node -> node.factoryFunction != createShaderNode }
                .map { node -> node.label }
    }

    fun createNodeForLabel(label: String, id: NodeId): Node? {
        val entry = mNodes.find { node -> node.label == label }
        return entry?.factoryFunction?.invoke(id)
    }

    override fun createNodeForTypeIdentifier(typeIdentifier: String, id: NodeId): Node? {
        val entry = mNodes.find { node -> node.typeIdentifier == typeIdentifier }
        return entry?.factoryFunction?.invoke(id)
    }
}
