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

import com.google.android.filament.tungsten.compiler.Literal
import org.junit.Assert.assertEquals
import org.junit.Test

class NodeTest {

    @Test
    fun `resolveInputExpressions keeps two float4s the same length`() {
        val (a, b) = resolveInputExpressions(Literal(4), Literal(4))
        assertEquals(4, a.dimensions)
        assertEquals(4, b.dimensions)
    }

    @Test
    fun `resolveInputExpressions keeps two floats the same length`() {
        val (a, b) = resolveInputExpressions(Literal(1), Literal(1))
        assertEquals(1, a.dimensions)
        assertEquals(1, b.dimensions)
    }

    @Test
    fun `resolveInputExpressions trims both a float3 and a float2 to a float2`() {
        val (a, b) = resolveInputExpressions(Literal(3), Literal(2))
        assertEquals(2, a.dimensions)
        assertEquals(2, b.dimensions)
    }

    @Test
    fun `resolveInputExpressions allows a float as input for scalar operations`() {
        val (a, b) = resolveInputExpressions(Literal(3), Literal(1))
        assertEquals(3, a.dimensions)
        assertEquals(1, b.dimensions)
    }

    @Test
    fun `resolveInputExpressions defaults to float4 when a single input is not present`() {
        val (a, b) = resolveInputExpressions(null, Literal(1))
        assertEquals(4, a.dimensions)
        assertEquals(1, b.dimensions)
    }
}