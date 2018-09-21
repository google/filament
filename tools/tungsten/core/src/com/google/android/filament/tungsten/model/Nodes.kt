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
import com.google.android.filament.tungsten.properties.ColorChooser
import com.google.android.filament.tungsten.properties.FloatSlider
import com.google.android.filament.tungsten.properties.MultipleChoice
import com.google.android.filament.tungsten.properties.TextureFileChooser

private val UNLIT_INPUTS = listOf("baseColor", "emissive")
private val LIT_INPUTS = listOf("normal", "baseColor", "metallic", "roughness", "reflectance",
        "clearCoat", "clearCoatRoughness", "anisotropy", "anisotropyDirection", "ambientOcclusion",
        "emissive")

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
        // The "normal" input is special, and its code must go before the call to prepareMaterial().
        if (name == "normal") {
            compiler.setCurrentCodeSection(GraphCompiler.CodeSection.BEFORE_PREPARE_MATERIAL)
        }

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

        compiler.setCurrentCodeSection(GraphCompiler.CodeSection.AFTER_PREPARE_MATERIAL)
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

    if (shadingModel == "lit") {
        // "normal" must be compiled first. If it has any dependencies that are connected to other
        // inputs, we need to ensure their code comes before prepareMaterial()
        compileMaterialInput("normal", 3)
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

private val constantFloatNodeCompile = fun(node: Node, compiler: GraphCompiler): Node {
    val value = (node.properties[0].value as FloatValue)

    val outputVariable = compiler.getNewTemporaryVariableName("floatConstant")
    compiler.addCodeToMaterialFunctionBody("float $outputVariable = ${value.v};\n")

    val outputSlot = node.getOutputSlot("out")
    compiler.setExpressionForSlot(outputSlot, Expression(outputVariable, 1))

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

private val textureSampleCompile = fun(node: Node, compiler: GraphCompiler): Node {
    compiler.requireAttribute("uv0")
    val parameter = compiler.addParameter("sampler2d", "texture")
    compiler.associateParameterWithProperty(parameter, node.getPropertyHandle("textureSource"))

    // If nothing is connected to the UV input, default to getUV0()
    val uvs = compiler.compileAndRetrieveExpression(node.getInputSlot("uv"))?.rg
            ?: Expression("getUV0()", 2)
    compiler.setExpressionForSlot(node.getInputSlot("uv"), uvs)

    val outputVariable = compiler.getNewTemporaryVariableName("textureSample")
    compiler.addCodeToMaterialFunctionBody(
            "float4 $outputVariable = texture(materialParams_${parameter.name}, $uvs);\n", 4)
    compiler.setExpressionForSlot(node.getOutputSlot("out"),
            Expression(outputVariable, 4))

    return node
}

val createTextureSampleNode = fun(id: NodeId) =
        Node(
            id = id,
            type = "textureSample",
            compileFunction = textureSampleCompile,
            inputSlots = listOf("uv"),
            outputSlots = listOf("out"),
            properties = listOf(Property(
                name = "textureSource",
                value = TextureFile(),
                type = PropertyType.MATERIAL_PARAMETER,
                editorFactory = ::TextureFileChooser
            ))
        )

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

val createFloatConstantNode = fun(id: NodeId) =
    Node(
        id = id,
        type = "floatConstant",
        compileFunction = constantFloatNodeCompile,
        outputSlots = listOf("out"),
        properties = listOf(Property(
            name = "value",
            value = FloatValue(),
            editorFactory = ::FloatSlider
        ))
    )

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
