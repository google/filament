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

// TODO: support other data types- expression assumes a float vector

open class Expression(
    open val symbol: String,
    open val dimensions: Int
) {
    val r get() = conform(1)
    val rg get() = conform(2)
    val rgb get() = conform(3)
    val rgba get() = conform(4)

    override fun toString() = symbol

    fun conform(components: Int): Expression {
        if (dimensions == components) return this
        return if (dimensions > components) {
            shorten(components)
        } else {
            lengthen(components)
        }
    }

    open fun lengthen(components: Int): Expression {
        val componentsToAdd = components - dimensions
        val extraComponents = ", 0.0".repeat(componentsToAdd)
        return Expression(symbol = "float$components($symbol$extraComponents)",
                dimensions = components)
    }

    open fun shorten(components: Int): Expression {
        val swizzle = when (components) {
            1 -> ".r"
            2 -> ".rg"
            3 -> ".rgb"
            else -> ""
        }
        return Expression(symbol = "$symbol$swizzle", dimensions = components)
    }
}

fun floatTypeName(dimensions: Int) = if (dimensions > 1) "float$dimensions" else "float"

private fun createFloatLiteral(dimensions: Int, value: Float): String {
    val zeroes = "$value, ".repeat(maxOf(dimensions - 1, 0)) + "$value"
    val typeName = floatTypeName(dimensions)
    return "$typeName($zeroes)"
}

class Literal(dimensions: Int, private val value: Float = 0.0f) : Expression(
        dimensions = dimensions, symbol = createFloatLiteral(dimensions, value)) {

    override fun shorten(components: Int) = Literal(dimensions = components, value = value)

    override fun lengthen(components: Int) = Literal(dimensions = components, value = value)
}

/**
 * Make two expressions the same dimensionality by trimming the higher dimension expression.
 */
fun trim(a: Expression, b: Expression): Pair<Expression, Expression> {
    if (a.dimensions == b.dimensions) return Pair(a, b)
    val dimensions = Math.min(a.dimensions, b.dimensions)
    return if (a.dimensions < b.dimensions) {
        Pair(a, b.shorten(dimensions))
    } else {
        Pair(a.shorten(dimensions), b)
    }
}