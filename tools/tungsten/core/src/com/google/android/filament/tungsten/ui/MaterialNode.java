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

import com.google.android.filament.tungsten.SwingHelper;
import com.google.android.filament.tungsten.model.Node;
import com.google.android.filament.tungsten.model.Slot;
import java.awt.Graphics;
import java.awt.Graphics2D;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JPanel;

/**
 * MaterialNode is the base view class for all Nodes in a Material graph. It's declared public
 * so clients may subclass specific Node types outside of this package.
 */
public class MaterialNode extends JPanel {

    private static final int PADDING = 5;
    private static final int ARC_RADIUS = 10;
    private static final int BORDER_THICKNESS = 2;

    private final Node mModel;
    private boolean mSelected = false;

    public MaterialNode(Node model) {
        mModel = model;
        setOpaque(false);
        setBorder(BorderFactory.createEmptyBorder(PADDING, PADDING, PADDING, PADDING));
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
    }

    public void setSelected(boolean selected) {
        mSelected = selected;
        repaint();
    }

    Node getModel() {
        return mModel;
    }

    NodeSlot addSlot(Slot model, String label) {
        NodeSlot nodeSlot = new NodeSlot(model, label, this);
        add(nodeSlot);
        validate();
        repaint();
        return nodeSlot;
    }

    @Override
    protected void paintComponent(Graphics g) {
        Graphics2D g2d = (Graphics2D)g;
        SwingHelper.setRenderingHints(g2d);
        if (mSelected) {
            g2d.setColor(ColorScheme.INSTANCE.getNodeSelected());
            g2d.fillRoundRect(0, 0, getWidth(), getHeight(), ARC_RADIUS, ARC_RADIUS);
        }
        g2d.setColor(ColorScheme.INSTANCE.getNodeBackground());
        g2d.fillRoundRect(BORDER_THICKNESS, BORDER_THICKNESS, getWidth() - 2 * BORDER_THICKNESS,
                getHeight() - 2 * BORDER_THICKNESS, ARC_RADIUS, ARC_RADIUS);
    }
}
