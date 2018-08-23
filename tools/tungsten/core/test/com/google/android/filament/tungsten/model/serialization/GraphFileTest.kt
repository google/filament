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

package com.google.android.filament.tungsten.model.serialization

import org.junit.Assert.assertEquals
import org.junit.Test

class GraphFileTest {

    @Test
    fun `Extract missing tool block`() {
        assertEquals("", GraphFile.extractToolBlockFromMaterialFile("""
            |material {
            |   ...
            |}
            |
            |fragment {
            |   ...
            |}
           """.trimMargin()))
    }

    @Test
    fun `Extract empty tool block`() {
        assertEquals("", GraphFile.extractToolBlockFromMaterialFile("""
            |material {
            |   ...
            |}
            |
            |fragment {
            |   ...
            |}
            |
            |tool {}
           """.trimMargin()))
    }

    @Test
    fun `Extract tool block with extra whitespace`() {
        assertEquals("\nfoobar\n", GraphFile.extractToolBlockFromMaterialFile("""
            |material {
            |   ...
            |}
            |
            |fragment {
            |   ...
            |}
            |
            |    tool      {
            |foobar
            |}
           """.trimMargin()))
    }

    @Test
    fun `Extract tool block with JSON`() {
        assertEquals("\n  {\"foo\":\"bar\",\"fizz\":[\"buzz\", 1]}\n",
                GraphFile.extractToolBlockFromMaterialFile("""
            |material {
            |   ...
            |}
            |
            |fragment {
            |   ...
            |}
            |
            |tool {
            |  {"foo":"bar","fizz":["buzz", 1]}
            |}
           """.trimMargin()))
    }

    @Test
    fun `Add tool block`() {
        assertEquals("""
            |content
            |
            |tool {
            |tool content
            |}
            """.trimMargin(), GraphFile.addToolBlockToMaterialFile("content", "tool content"))
    }
}