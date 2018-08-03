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

import com.google.android.filament.tungsten.compiler.GraphCompiler;
import com.google.android.filament.tungsten.compiler.Variable;

public class InputSlotModel extends SlotModel {

    // Holds the connection this slot has if connected to an output slot.
    private ConnectionModel mConnection;

    public InputSlotModel(String label, NodeModel node) {
        super(label, node);
    }

    /**
     * Connects this InputSlotModel to an OutputSlotModel.
     */
    public void connectTo(OutputSlotModel outputSlot) {
        mConnection = new ConnectionModel(outputSlot, this);
    }

    /**
     * Get the Variable for the output slot connected to this input slot, or make a call to its node
     * to compile itself.
     * The Variable is cached so that further calls don't recompile the node.
     */
    public Variable retrieveVariable(GraphCompiler compiler) {
        assert isConnected();
        OutputSlotModel outputSlot = mConnection.getOutputSlot();
        if (outputSlot.getVariable() == null) {
            outputSlot.node.compile(compiler);
        }
        // The node should compile and set this slot's Variable.
        assert outputSlot.getVariable() != null;
        return outputSlot.getVariable();
    }

    /**
     * @return true if this slot is connected to an output slot.
     */
    public boolean isConnected() {
        return mConnection != null;
    }

    public OutputSlotModel getConnectedSlot() {
        assert isConnected();
        return mConnection.getOutputSlot();
    }

    public void removeConnection() {
        mConnection = null;
    }
}
