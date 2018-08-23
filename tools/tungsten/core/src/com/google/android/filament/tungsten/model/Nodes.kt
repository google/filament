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
import com.google.android.filament.tungsten.compiler.Expression

private val adderNodeCompile = fun(node: Node, compiler: GraphCompiler): Node {
    val outputSlot = node.getOutputSlot("result")

    val a = compiler.compileAndRetrieveVariable(node.getInputSlot("a"))
    val b = compiler.compileAndRetrieveVariable(node.getInputSlot("b"))

    val aExpression = a?.symbol ?: "float3(0.0, 0.0, 0.0)"
    val bExpression = b?.symbol ?: "float3(0.0, 0.0, 0.0)"

    val temp = compiler.getNewTemporaryVariableName("adder")
    compiler.addCodeToMaterialFunctionBody("float3 $temp = $aExpression + $bExpression;\n")

    compiler.setExpressionForOutputSlot(outputSlot, Expression(temp))

    return node
}

private val shaderNodeCompile = fun(node: Node, compiler: GraphCompiler): Node {
    val baseColor = compiler.compileAndRetrieveVariable(node.getInputSlot("baseColor"))
    val metallic = compiler.compileAndRetrieveVariable(node.getInputSlot("metallic"))
    val roughness = compiler.compileAndRetrieveVariable(node.getInputSlot("roughness"))

    if (baseColor != null) {
        compiler.addCodeToMaterialFunctionBody("material.baseColor.rgb = ${baseColor.symbol};\n")
    }

    if (metallic != null) {
        compiler.addCodeToMaterialFunctionBody("material.metallic = ${metallic.symbol};\n")
    }

    if (roughness != null) {
        compiler.addCodeToMaterialFunctionBody("material.roughness = ${roughness.symbol};\n")
    }

    return node
}

private val constantFloat3NodeCompile = fun(node: Node, compiler: GraphCompiler): Node {
    val outputSlot = node.getOutputSlot("result")

    val color = (node.properties[0].value as Float3)

    val outputVariable = compiler.getNewTemporaryVariableName("float3Constant")
    compiler.addCodeToMaterialFunctionBody(
            "float3 $outputVariable = float3(${color.x}, ${color.y}, ${color.z});\n")

    compiler.setExpressionForOutputSlot(outputSlot, Expression(outputVariable))
    return node
}

val createAdderNode = fun(id: NodeId): Node {
    return Node(
        id = id,
        type = "adder",
        compileFunction = adderNodeCompile,
        inputSlots = listOf("a", "b"),
        outputSlots = listOf("result")
    )
}

val createFloat3ConstantNode = fun(id: NodeId): Node {
    return Node(
        id = id,
        type = "float3Constant",
        compileFunction = constantFloat3NodeCompile,
        outputSlots = listOf("result"),
        properties = listOf(Property("value", Float3()))
    )
}

val createShaderNode = fun(id: NodeId): Node {
    return Node(
        id = id,
        type = "shader",
        compileFunction = shaderNodeCompile,
        inputSlots = listOf("baseColor", "metallic", "roughness"))
}

object GraphInitializer {

    fun getInitialGraphState(): Graph {
        val shaderNode = createShaderNode(0)
        return Graph(nodes = listOf(shaderNode), rootNodeId = 0)
    }
}
