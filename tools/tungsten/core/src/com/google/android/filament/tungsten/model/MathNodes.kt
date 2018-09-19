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

import com.google.android.filament.tungsten.compiler.Expression
import com.google.android.filament.tungsten.compiler.Literal
import com.google.android.filament.tungsten.compiler.floatTypeName
import com.google.android.filament.tungsten.compiler.trim

data class ResolvedExpressions(
    val a: Expression,
    val b: Expression,
    val resultDimensionality: Int
)

// A function that takes two expressions and prepares them for a math operation.
typealias ExpressionResolver = (Expression?, Expression?) -> ResolvedExpressions

typealias Operation = (Expression, Expression) -> String

/**
 * Create a math node compile function with a given operation.
 * Assumes two input slots, "a" and "b", and a single output, "out".
 */
private fun createMathCompileFunction(
    expressionResolver: ExpressionResolver,
    operation: Operation,
    temporaryVariable: String
): CompileFunction = { node, compiler ->
        val a = compiler.compileAndRetrieveExpression(node.getInputSlot("a"))
        val b = compiler.compileAndRetrieveExpression(node.getInputSlot("b"))

        val (aExpr, bExpr, resultDimensionality) = expressionResolver(a, b)

        compiler.setExpressionForSlot(node.getInputSlot("a"), aExpr)
        compiler.setExpressionForSlot(node.getInputSlot("b"), bExpr)

        val temp = compiler.getNewTemporaryVariableName(temporaryVariable)
        compiler.addCodeToMaterialFunctionBody(
                "${floatTypeName(resultDimensionality)} $temp = ${operation(aExpr, bExpr)};\n")

        val output = node.getOutputSlot("out")
        compiler.setExpressionForSlot(output, Expression(temp, resultDimensionality))

        node
    }

/**
 * Trims two expressions to be the same dimensionality, preferring the one with fewer dimensions.
 * If one of the inputs is a single float, neither input is modified, allowing for a scalar
 * operations.
 */
internal fun resolveInputExpressions(a: Expression?, b: Expression?): ResolvedExpressions {
    val aExpr = a ?: Literal(4)
    val bExpr = b ?: Literal(4)
    val (trimmedA, trimmedB) = if (aExpr.dimensions > 1 && bExpr.dimensions > 1) {
        trim(aExpr, bExpr)
    } else {
        Pair(aExpr, bExpr)
    }
    return ResolvedExpressions(trimmedA, trimmedB, maxOf(trimmedA.dimensions, trimmedB.dimensions))
}

private val subtractNodeCompile = createMathCompileFunction(
    ::resolveInputExpressions,
    { a, b -> "$a - $b" },
    "subtract"
)

private val addNodeCompile = createMathCompileFunction(
    ::resolveInputExpressions,
    { a, b -> "$a + $b" },
    "add"
)

private val multiplyNodeCompile = createMathCompileFunction(
    ::resolveInputExpressions,
    { a, b -> "$a * $b" },
    "multiply"
)

private val divideNodeCompile = createMathCompileFunction(
    ::resolveInputExpressions,
    { a, b -> "$a / $b" },
    "divide"
)

val createAddNode = fun(id: NodeId) =
    Node(
        id = id,
        type = "add",
        compileFunction = addNodeCompile,
        inputSlots = listOf("a", "b"),
        outputSlots = listOf("out")
    )

val createSubtractNode = fun(id: NodeId) =
    Node(
        id = id,
        type = "subtract",
        compileFunction = subtractNodeCompile,
        inputSlots = listOf("a", "b"),
        outputSlots = listOf("out")
    )

val createMultiplyNode = fun(id: NodeId) =
    Node(
        id = id,
        type = "multiply",
        compileFunction = multiplyNodeCompile,
        inputSlots = listOf("a", "b"),
        outputSlots = listOf("out")
    )

val createDivideNode = fun(id: NodeId) =
    Node(
        id = id,
        type = "divide",
        compileFunction = divideNodeCompile,
        inputSlots = listOf("a", "b"),
        outputSlots = listOf("out")
    )
