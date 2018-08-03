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

package com.google.android.filament.tungsten.compiler;

import static org.mockito.Mockito.verify;

import com.google.android.filament.tungsten.model.OutputSlotModel;
import org.junit.Before;
import org.junit.Test;

public class AddNodeTest {

    private AddNode mAddNode;
    private GraphCompiler mGraphCompiler;

    @Before
    public void setUp() {
        mAddNode = new AddNode();
        mGraphCompiler = TestUtils.createMockGraphCompiler("add_AddNode", "adder1");
    }

    @Test
    public void testDisconnectedAddNode() {
        mAddNode.compile(mGraphCompiler);
        String expected = "float3 adder1 = add_AddNode(float3(0.0), float3(0.0));\n";
        verify(mGraphCompiler).addCodeToMaterialFunctionBody(expected);
    }

    @Test
    public void testOneConnection() {
        OutputSlotModel mockSlot = TestUtils.createMockSlot("varA");
        mAddNode.getInputSlot("inputA").connectTo(mockSlot);
        mAddNode.compile(mGraphCompiler);
        String expected = "float3 adder1 = add_AddNode(varA, float3(0.0));\n";
        verify(mGraphCompiler).addCodeToMaterialFunctionBody(expected);
    }

    @Test
    public void testFullyConnected() {
        OutputSlotModel outputA = TestUtils.createMockSlot("varA");
        OutputSlotModel outputB = TestUtils.createMockSlot("varB");
        mAddNode.getInputSlot("inputA").connectTo(outputA);
        mAddNode.getInputSlot("inputB").connectTo(outputB);
        mAddNode.compile(mGraphCompiler);
        String expected = "float3 adder1 = add_AddNode(varA, varB);\n";
        verify(mGraphCompiler).addCodeToMaterialFunctionBody(expected);
    }
}
