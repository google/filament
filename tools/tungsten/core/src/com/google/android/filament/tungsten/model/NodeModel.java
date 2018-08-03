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
import com.google.android.filament.tungsten.properties.NodeProperty;
import java.awt.Point;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

public abstract class NodeModel {

    private final List<OutputSlotModel> mOutputSlots = new ArrayList<>();
    private final List<InputSlotModel> mInputSlots = new ArrayList<>();
    private final List<NodeProperty> mProperties = new ArrayList<>();
    private final Map<String, SlotModel> mSlotMapping = new HashMap<>();

    @NotNull
    private Point mPosition = new Point(0, 0);

    /**
     * @return A list of OutputSlotModels that represent the output slots for this node.
     */
    public final List<OutputSlotModel> getOutputSlots() {
        return mOutputSlots;
    }

    /**
     * @return A list of InputSlotModels that represent the input slots for this node.
     */
    public final List<InputSlotModel> getInputSlots() {
        return mInputSlots;
    }

    @Nullable
    public InputSlotModel getInputSlot(String label) {
        SlotModel slot = mSlotMapping.get(label);
        if (slot instanceof InputSlotModel) {
            return (InputSlotModel) slot;
        }
        return null;
    }

    @Nullable
    public OutputSlotModel getOutputSlot(String label) {
        SlotModel slot = mSlotMapping.get(label);
        if (slot instanceof OutputSlotModel) {
            return (OutputSlotModel) slot;
        }
        return null;
    }

    public List<NodeProperty> getProperties() {
        return mProperties;
    }

    /**
     * Invalidate the Variable cache for this node, and recursively for nodes an input slot is
     * connected to.
     */
    public void invalidate() {
        for (OutputSlotModel slot : mOutputSlots) {
            slot.setVariable(null);
        }
        for (InputSlotModel slot : mInputSlots) {
            if (slot.isConnected()) {
                slot.getConnectedSlot().node.invalidate();
            }
        }
    }

    protected OutputSlotModel addOutputSlot(String label) {
        OutputSlotModel newSlot = new OutputSlotModel(label, this);
        mOutputSlots.add(newSlot);
        mSlotMapping.put(label, newSlot);
        return newSlot;
    }

    protected void addProperty(String label, NodeProperty property) {
        mProperties.add(property);
    }

    protected InputSlotModel addInputSlot(String label) {
        InputSlotModel newSlot = new InputSlotModel(label, this);
        mInputSlots.add(newSlot);
        mSlotMapping.put(label, newSlot);
        return newSlot;
    }

    public void setPosition(int x, int y) {
        mPosition.x = x;
        mPosition.y = y;
    }

    @NotNull
    public Point getPosition() {
        return mPosition;
    }

    public abstract void compile(GraphCompiler compiler);
}
