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

import com.google.android.filament.tungsten.model.NodeModel
import com.google.android.filament.tungsten.model.serialization.INodeFactory

class NodeRegistry : INodeFactory {

    /**
     * label is a human-readable name used for menus
     * typeIdentifier is a string that uniquely identifiers the NodeModel class for serialization
     */
    private data class NodeEntry(
        val label: String,
        val typeIdentifier: String,
        val clazz: Class<out NodeModel>
    )

    private val mNodes: List<NodeEntry>

    val nodeLabelsForMenu: List<String>

    init {
        mNodes = listOf(
                NodeEntry("Add", "add", AddNode::class.java),
                NodeEntry("Constant", "constant", Float3ConstantNode::class.java),
                NodeEntry("Shader", "shader", ShaderNode::class.java)
        )
        nodeLabelsForMenu = mNodes
                // The ShaderNode should not be visible in menus
                .filter { node -> node.clazz != ShaderNode::class.java }
                .map { node -> node.label }
    }

    fun createNodeForLabel(label: String): NodeModel? {
        val entry = mNodes.find { node -> node.label == label }
        return entry?.clazz?.getConstructor()?.newInstance()
    }

    override fun createNodeForTypeIdentifier(typeIdentifier: String): NodeModel? {
        val entry = mNodes.find { node -> node.typeIdentifier == typeIdentifier }
        return entry?.clazz?.getConstructor()?.newInstance()
    }

    override fun typeIdentifierForNode(node: NodeModel): String? {
        val entry = mNodes.find { n -> n.clazz == node.javaClass }
        return entry?.typeIdentifier
    }
}
