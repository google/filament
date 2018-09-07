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
import com.google.android.filament.tungsten.compiler.Literal
import com.google.android.filament.tungsten.compiler.trim
import com.google.android.filament.tungsten.properties.ColorChooser
import com.google.android.filament.tungsten.properties.MultipleChoice

private val adderNodeCompile = fun(node: Node, compiler: GraphCompiler): Node {
    val outputSlot = node.getOutputSlot("result")

    val a = compiler.compileAndRetrieveExpression(node.getInputSlot("a"))
    val b = compiler.compileAndRetrieveExpression(node.getInputSlot("b"))

    // Trim the expressions so that they're the same dimension.
    val (aExpression, bExpression) = trim(a ?: Literal(4), b ?: Literal(4))

    compiler.setExpressionForSlot(node.getInputSlot("a"), aExpression)
    compiler.setExpressionForSlot(node.getInputSlot("b"), bExpression)

    val temp = compiler.getNewTemporaryVariableName("adder")
    val resultDimensions = aExpression.dimensions
    compiler.addCodeToMaterialFunctionBody(
            "float$resultDimensions $temp = $aExpression + $bExpression;\n")

    compiler.setExpressionForSlot(outputSlot, Expression(temp, resultDimensions))

    return node
}

private val UNLIT_INPUTS = listOf("baseColor", "emissive")
private val LIT_INPUTS = listOf("baseColor", "metallic", "roughness", "reflectance", "clearCoat",
        "clearCoatRoughness", "anisotropy", "anisotropyDirection", "ambientOcclusion", "emissive")

private val inputSlotsForShadingModel = { shadingModel: String ->
    when (shadingModel) {
        "unlit" -> UNLIT_INPUTS
        "lit" -> LIT_INPUTS
        else -> emptyList()
    }
}

private val shaderNodeCompile = fun(node: Node, compiler: GraphCompiler): Node {
    val shadingModel = (node.properties[0].value as StringValue).value
    compiler.setShadingModel(shadingModel)

    val compileMaterialInput = { name: String, dimensions: Int ->
        val inputSlot = node.getInputSlot(name)
        val connectedExpression = compiler.compileAndRetrieveExpression(inputSlot)
        val isConnected = connectedExpression != null
        val expression = connectedExpression ?: Literal(dimensions)

        // Conform the expression to match the dimensionality of the material input.
        val conformedExpression = expression.conform(dimensions)

        if (isConnected) {
            compiler.addCodeToMaterialFunctionBody("material.$name = $conformedExpression;\n")
        }
        compiler.setExpressionForSlot(inputSlot, conformedExpression)
    }

    // Compile inputs common to all shading models.
    val compileCommonInputs = {
        compileMaterialInput("baseColor", 4)
        compileMaterialInput("emissive", 4)
    }

    // Compile inputs unique to the "lit" shading model.
    val compileLitInputs = {
        compileMaterialInput("metallic", 1)
        compileMaterialInput("roughness", 1)
        compileMaterialInput("reflectance", 1)
        compileMaterialInput("clearCoat", 1)
        compileMaterialInput("clearCoatRoughness", 1)
        compileMaterialInput("anisotropy", 1)
        compileMaterialInput("anisotropyDirection", 3)
        compileMaterialInput("ambientOcclusion", 1)
    }

    compileCommonInputs()
    when (shadingModel) {
        "lit" -> compileLitInputs()
    }

    // If the input slots have changed, return a new node.
    return node.nodeBySettingInputSlots(inputSlotsForShadingModel(shadingModel))
}

private val constantFloat3NodeCompile = fun(node: Node, compiler: GraphCompiler): Node {
    val outputSlot = node.getOutputSlot("result")

    val color = (node.properties[0].value as Float3)

    val outputVariable = compiler.getNewTemporaryVariableName("float3Constant")
    compiler.addCodeToMaterialFunctionBody(
            "float3 $outputVariable = float3(${color.x}, ${color.y}, ${color.z});\n")

    compiler.setExpressionForSlot(outputSlot, Expression(outputVariable, 3))

    return node
}

private val float3ParameterNodeCompile = fun(node: Node, compiler: GraphCompiler): Node {
    val parameter = compiler.addParameter("float3", "float3Parameter")
    compiler.associateParameterWithProperty(parameter, node.getPropertyHandle("value"))
    compiler.setExpressionForSlot(node.getOutputSlot("result"),
            Expression("materialParams.${parameter.name}", 3))

    return node
}

private val constantFloat2NodeCompile = fun(node: Node, compiler: GraphCompiler): Node {
    val outputSlot = node.getOutputSlot("result")

    val color = (node.properties[0].value as Float3)

    val outputVariable = compiler.getNewTemporaryVariableName("float2Constant")
    compiler.addCodeToMaterialFunctionBody(
            "float2 $outputVariable = float2(${color.x}, ${color.y});\n")

    compiler.setExpressionForSlot(outputSlot, Expression(outputVariable, 2))

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
        properties = listOf(Property(
            name = "value",
            value = Float3(),
            editorFactory = ::ColorChooser))
    )
}

val createFloat3ParameterNode = fun(id: NodeId): Node {
    return Node(
        id = id,
        type = "float3Parameter",
        compileFunction = float3ParameterNodeCompile,
        outputSlots = listOf("result"),
        properties = listOf(Property(
            name = "value",
            value = Float3(),
            type = PropertyType.MATERIAL_PARAMETER,
            editorFactory = ::ColorChooser))
    )
}

val createFloat2ConstantNode = fun(id: NodeId): Node {
    return Node(
        id = id,
        type = "float2Constant",
        compileFunction = constantFloat2NodeCompile,
        outputSlots = listOf("result"),
        properties = listOf(Property(
            name = "value",
            value = Float3(),
            editorFactory = ::ColorChooser))
    )
}

val createShaderNode = fun(id: NodeId): Node {
    return Node(
        id = id,
        type = "shader",
        compileFunction = shaderNodeCompile,
        inputSlots = inputSlotsForShadingModel("unlit"),
        properties = listOf(Property(
            name = "materialModel",
            value = StringValue("unlit"),
            editorFactory = { MultipleChoice(it, listOf("lit", "unlit")) })))
}

object GraphInitializer {

    fun getInitialGraphState(): Graph {
        val shaderNode = createShaderNode(0)
        return Graph(nodes = listOf(shaderNode), rootNodeId = 0)
    }
}
