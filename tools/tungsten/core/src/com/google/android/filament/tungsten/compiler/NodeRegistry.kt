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
import com.google.android.filament.tungsten.model.createAddNode
import com.google.android.filament.tungsten.model.createDivideNode
import com.google.android.filament.tungsten.model.createFloat2ConstantNode
import com.google.android.filament.tungsten.model.createFloat3ConstantNode
import com.google.android.filament.tungsten.model.createFloat3ParameterNode
import com.google.android.filament.tungsten.model.createFloatConstantNode
import com.google.android.filament.tungsten.model.createMultiplyNode
import com.google.android.filament.tungsten.model.createPannerNode
import com.google.android.filament.tungsten.model.createShaderNode
import com.google.android.filament.tungsten.model.createSubtractNode
import com.google.android.filament.tungsten.model.createTexCoordNode
import com.google.android.filament.tungsten.model.createTextureSampleNode
import com.google.android.filament.tungsten.model.createTimeNode
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
                NodeEntry("Add", "add", createAddNode),
                NodeEntry("Subtract", "subtract", createSubtractNode),
                NodeEntry("Multiply", "multiply", createMultiplyNode),
                NodeEntry("Divide", "divide", createDivideNode),
                NodeEntry("Constant float", "floatConstant", createFloatConstantNode),
                NodeEntry("Constant float2", "float2Constant", createFloat2ConstantNode),
                NodeEntry("Constant float3", "float3Constant", createFloat3ConstantNode),
                NodeEntry("Float3 parameter", "float3Parameter", createFloat3ParameterNode),
                NodeEntry("Texture sampler", "textureSample", createTextureSampleNode),
                NodeEntry("Time", "time", createTimeNode),
                NodeEntry("Texture coordinates", "texCoord", createTexCoordNode),
                NodeEntry("Texture panner", "panner", createPannerNode),
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
