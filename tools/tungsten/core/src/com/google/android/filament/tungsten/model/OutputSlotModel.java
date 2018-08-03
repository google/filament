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

package com.google.android.filament.tungsten.model;

import com.google.android.filament.tungsten.compiler.Variable;

public class OutputSlotModel extends SlotModel {

    // Once this slot is compiled, its Variable will be cached here
    private Variable mVariable;

    public OutputSlotModel(String label, NodeModel node) {
        super(label, node);
    }

    public Variable getVariable() {
        return mVariable;
    }

    public void setVariable(Variable variable) {
        mVariable = variable;
    }
}
