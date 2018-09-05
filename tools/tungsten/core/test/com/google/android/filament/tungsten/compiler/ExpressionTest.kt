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

import org.junit.Assert.assertEquals
import org.junit.Test

class ExpressionTest {

    @Test
    fun `toString returns the symbol`() {
        assertEquals("float3(1.0, 2.0, 3.0)", Expression("float3(1.0, 2.0, 3.0)", 3).toString())
    }

    @Test
    fun `swizzle to a lower dimension`() {
        assertEquals("exp.rg", "${Expression("exp", 4).rg}")
    }

    @Test
    fun `swizzle to a higher dimension`() {
        assertEquals("float4(exp, 0.0, 0.0)", "${Expression("exp", 2).rgba}")
    }

    @Test
    fun `swizzling a literal returns a new literal`() {
        assertEquals("float2(0.0, 0.0)", "${Literal(4).rg}")
        assertEquals("float3(0.0, 0.0, 0.0)", "${Literal(1).rgb}")
    }

    @Test
    fun `create a float literal of dimension 1`() {
        assertEquals("float(0.0)", "${Literal(1).r}")
    }
}