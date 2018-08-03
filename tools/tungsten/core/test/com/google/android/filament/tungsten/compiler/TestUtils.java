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

import static org.mockito.Matchers.anyString;
import static org.mockito.Matchers.anyVararg;
import static org.mockito.Mockito.doCallRealMethod;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.mock;

import com.google.android.filament.tungsten.model.OutputSlotModel;

final class TestUtils {

    private TestUtils() {

    }

    static OutputSlotModel createMockSlot(String variableSymbol) {
        OutputSlotModel mockSlot = mock(OutputSlotModel.class);
        doReturn(new Variable(variableSymbol)).when(mockSlot).getVariable();
        return mockSlot;
    }

    static GraphCompiler createMockGraphCompiler(String functionName, String variableName) {
        GraphCompiler mockGraphCompiler = mock(GraphCompiler.class);
        doReturn(functionName).when(mockGraphCompiler)
                .allocateGlobalFunction(anyString(),anyString());
        doReturn(variableName).when(mockGraphCompiler)
                .getNewTemporaryVariableName(anyString());
        // addCodeToMaterialFunctionBody has a convenience overload, so call the single argument
        // version and we'll verify calls to that one in tests
        doCallRealMethod().when(mockGraphCompiler).addCodeToMaterialFunctionBody(anyString(),
                anyVararg());
        return mockGraphCompiler;
    }
}
