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

import com.google.android.filament.tungsten.model.InputSlotModel;
import com.google.android.filament.tungsten.model.NodeModel;
import com.google.android.filament.tungsten.model.OutputSlotModel;

public class AddNode extends NodeModel {

    private final InputSlotModel mInputA;
    private final InputSlotModel mInputB;
    private final OutputSlotModel mResult;

    private static final String addFunctionDefinition = "float3 %s(float3 a, float3 b) {\n"
            + "    return a + b;\n"
            + "}\n";

    public AddNode() {
        mInputA = addInputSlot("inputA");
        mInputB = addInputSlot("inputB");
        mResult = addOutputSlot("result");
    }

    @Override
    public void compile(GraphCompiler compiler) {
        String expressionA = "float3(0.0)";
        String expressionB = "float3(0.0)";
        if (mInputA.isConnected()) {
            expressionA = mInputA.retrieveVariable(compiler).symbol;
        }
        if (mInputB.isConnected()) {
            expressionB = mInputB.retrieveVariable(compiler).symbol;
        }

        // Add global "add" function
        String addSymbol = compiler.allocateGlobalFunction("add", "AddNode");
        compiler.provideFunctionDefinition(addSymbol, addFunctionDefinition, addSymbol);

        String outputVariableName = compiler.getNewTemporaryVariableName("addResult");
        compiler.addCodeToMaterialFunctionBody("float3 %s = %s(%s, %s);\n",
                outputVariableName, addSymbol, expressionA, expressionB);
        mResult.setVariable(new Variable(outputVariableName));
    }
}
