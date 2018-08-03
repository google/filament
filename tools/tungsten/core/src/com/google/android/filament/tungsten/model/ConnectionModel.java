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

/**
 * ConnectionModel models a connection between two slots in the graph.
 */
public final class ConnectionModel {

    private final OutputSlotModel mOutputSlot;
    private final InputSlotModel mInputSlot;

    public ConnectionModel(OutputSlotModel output, InputSlotModel input) {
        mOutputSlot = output;
        mInputSlot = input;
    }

    @Override
    public String toString() {
        return String.format("Connection: %s -> %s\n", mOutputSlot.label, mInputSlot.label);
    }

    public OutputSlotModel getOutputSlot() {
        return mOutputSlot;
    }

    public InputSlotModel getInputSlot() {
        return mInputSlot;
    }
}
