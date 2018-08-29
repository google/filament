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

package com.google.android.filament.tungsten.ui;

import com.google.android.filament.tungsten.model.Node;
import com.google.android.filament.tungsten.model.Slot;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Point;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JComponent;
import javax.swing.JLabel;

/**
 * NodeSlot is a component added to a MaterialNode that contains a slot circle and a label.
 */
class NodeSlot extends JComponent {

    private static final int TEXT_PAD = 5;
    private static final int SLOT_CIRCLE_RADIUS = 10;

    private final Slot mModel;
    private final MaterialNode mParent;
    private final SlotCircle mSlotCircle;
    private final String mLabel;

    NodeSlot(Slot model, String label, MaterialNode parent) {
        mModel = model;
        mParent = parent;
        mSlotCircle = new SlotCircle();
        mSlotCircle.setMaximumSize(new Dimension(SLOT_CIRCLE_RADIUS, SLOT_CIRCLE_RADIUS));
        mLabel = label;
        setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
        setOpaque(false);
        addComponents();
    }

    Slot getModel() {
        return mModel;
    }

    boolean isOutputSlot() {
        return (mModel instanceof Node.OutputSlot);
    }

    boolean isInputSlot() {
        return (mModel instanceof Node.InputSlot);
    }

    MaterialNode getParentMaterialNode() {
        return mParent;
    }

    /**
     * Returns the center point of the slot in this NodeSlot's own coordinate system.
     * This is the point at which a connection line should originate or terminate.
     */
    Point getCenterPoint() {
        int x = mSlotCircle.getX() + mSlotCircle.getWidth() / 2;
        int y = mSlotCircle.getY() + mSlotCircle.getHeight() / 2;
        return new Point(x, y);
    }

    /**
     * Adds the slot circle and label to this NodeSlot.
     */
    private void addComponents() {
        for (Component component : createComponents()) {
            add(component);
        }
        revalidate();
        repaint();
    }

    private List<Component> createComponents() {
        JLabel label = new JLabel(mLabel);
        label.setForeground(ColorScheme.INSTANCE.getSlotLabel());
        List<Component> components = Arrays.asList(
                mSlotCircle,
                Box.createRigidArea(new Dimension(TEXT_PAD, 0)),
                label,
                Box.createHorizontalGlue());
        // Output slots have their slots right-aligned, so reverse the order of components.
        if (isOutputSlot()) {
            Collections.reverse(components);
        }
        return components;
    }
}
