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

import com.google.android.filament.tungsten.model.NodeModel;
import com.google.android.filament.tungsten.model.OutputSlotModel;
import com.google.android.filament.tungsten.properties.Float3Property;

public class Float3ConstantNode extends NodeModel {

    private final OutputSlotModel mValueOutput;
    private final Float3Property mValueProperty;

    public Float3ConstantNode() {
        this(0.0f, 0.0f, 0.0f);
    }

    public Float3ConstantNode(float x, float y, float z) {
        mValueOutput = addOutputSlot("value");
        mValueProperty = new Float3Property(x, y, z);
        addProperty("Constant value", mValueProperty);
    }

    @Override
    public void compile(GraphCompiler compiler) {
        String outputVariableName = compiler.getNewTemporaryVariableName("float3Constant");
        compiler.addCodeToMaterialFunctionBody("float3 %s = float3(%s, %s, %s);\n",
                outputVariableName, mValueProperty.getX(), mValueProperty.getY(),
                mValueProperty.getZ());
        mValueOutput.setVariable(new Variable(outputVariableName));
    }
}
