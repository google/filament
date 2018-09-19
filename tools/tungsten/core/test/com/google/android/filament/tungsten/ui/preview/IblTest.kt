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

package com.google.android.filament.tungsten.ui.preview

import org.junit.Assert.assertArrayEquals
import org.junit.Test

class IblTest {

    @Test
    fun `parseSphereHarmonics parses single row`() {
        val result = parseSphereHarmonics(
                "(0.001, -0.003, -0.005); // L20, irradiance, pre-scaled base\n")
        val expected = floatArrayOf(0.001f, -0.003f, -0.005f)
        assertArrayEquals(expected, result, 0.0f)
    }

    @Test
    fun `parseSphereHarmonics parses multiple rows`() {
        val result = parseSphereHarmonics(
                "(-0.2, -0.24, -0.24); // L2-2, irradiance, pre-scaled base\n" +
                "( 0.05,  0.06,  0.06); // L2-1, irradiance, pre-scaled base\n")
        val expected = floatArrayOf(-0.2f, -0.24f, -0.24f, 0.05f, 0.06f, 0.06f)
        assertArrayEquals(expected, result, 0.0f)
    }
}