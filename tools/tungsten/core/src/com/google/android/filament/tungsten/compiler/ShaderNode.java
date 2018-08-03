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

public class ShaderNode extends NodeModel {

    private final InputSlotModel mBaseColor;
    private final InputSlotModel mMetallic;
    private final InputSlotModel mRoughness;

    public ShaderNode() {
        mBaseColor = addInputSlot("baseColor");
        mMetallic = addInputSlot("metallic");
        mRoughness = addInputSlot("roughness");
    }

    @Override
    public void compile(GraphCompiler compiler) {
        if (mBaseColor.isConnected()) {
            Variable result = mBaseColor.retrieveVariable(compiler);
            compiler.addCodeToMaterialFunctionBody("material.baseColor.rgb = %s;\n",
                    result.symbol);
        }
        if (mMetallic.isConnected()) {
            Variable result = mMetallic.retrieveVariable(compiler);
            compiler.addCodeToMaterialFunctionBody("material.metallic = %s;\n", result.symbol);
        }
        if (mRoughness.isConnected()) {
            Variable result = mRoughness.retrieveVariable(compiler);
            compiler.addCodeToMaterialFunctionBody("material.roughness = %s;\n", result.symbol);
        }
    }
}
