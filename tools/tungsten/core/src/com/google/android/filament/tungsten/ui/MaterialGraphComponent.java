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

import static com.google.android.filament.tungsten.ui.ConnectionLineKt.arbitraryPoint;
import static com.google.android.filament.tungsten.ui.ConnectionLineKt.slotPoint;

import com.google.android.filament.tungsten.SwingHelper;
import com.google.android.filament.tungsten.model.ConnectionModel;
import com.google.android.filament.tungsten.model.InputSlotModel;
import com.google.android.filament.tungsten.model.NodeModel;
import com.google.android.filament.tungsten.model.OutputSlotModel;
import com.google.android.filament.tungsten.model.SelectionModel;
import com.google.android.filament.tungsten.model.SlotModel;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import javax.swing.JComponent;

public class MaterialGraphComponent extends JComponent implements TungstenMouseListener {

    private final TungstenEventManager mEventManager;
    private MaterialNode mNodeBeingDragged;
    private SelectionModel mPreviousSelectionModel = new SelectionModel();
    private boolean mIsConnecting = false;

    // A connection is "in progress" when a user has started dragging from a slot
    private ConnectionLine mInProgressConnectionLine = null;

    // The slot the user started dragging from when forming a connection
    private SlotModel mInProgressOriginSlot;

    private final List<ConnectionLine> mConnectionLines;
    private GraphPresenter mPresenter;

    // Maps between the NodeModels we receive and our views
    private final Map<NodeModel, MaterialNode> mModelViewMap;

    // Maps between the SlotModels we receive and the NodeSlot views
    private final Map<SlotModel, NodeSlot> mSlotMap;

    // Maps between the ConnectionModels we receive and our views
    private final Map<ConnectionModel, ConnectionLine> mConnectionViewMap;

    public MaterialGraphComponent() {
        mConnectionLines = new ArrayList<>();
        mModelViewMap = new HashMap<>();
        mConnectionViewMap = new HashMap<>();
        mSlotMap = new HashMap<>();
        mEventManager = new TungstenEventManager(this);
        addMouseMotionListener(mEventManager);
        addMouseListener(mEventManager);
    }

    public void setPresenter(GraphPresenter presenter) {
        mPresenter = presenter;
    }

    public void addNode(NodeModel node) {
        MaterialNode nodeView = new MaterialNode(node);

        for (InputSlotModel slot : node.getInputSlots()) {
            NodeSlot slotView = nodeView.addSlot(slot);
            mSlotMap.put(slot, slotView);
        }
        for (OutputSlotModel slot : node.getOutputSlots()) {
            NodeSlot slotView = nodeView.addSlot(slot);
            mSlotMap.put(slot, slotView);
        }

        mModelViewMap.put(node, nodeView);
        Point position = node.getPosition();
        nodeView.setBounds(position.x, position.y, 200, 100);
        add(nodeView);
        revalidate();
        repaint();
    }

    public void addConnection(ConnectionModel connection) {
        NodeSlot inputSlot =  mSlotMap.get(connection.getInputSlot());
        NodeSlot outputSlot =  mSlotMap.get(connection.getOutputSlot());

        if (inputSlot == null || outputSlot == null) {
            return;
        }

        ConnectionLine line = new ConnectionLine(slotPoint(outputSlot), slotPoint(inputSlot));

        mConnectionLines.add(line);
        mConnectionViewMap.put(connection, line);

        revalidate();
        repaint();
    }

    @Override
    public void mousePressed(MouseEvent e) {
        // Check if we've clicked on a MaterialNode.
        Component componentUnderMouse = getComponentAt(e.getPoint());
        if (!(componentUnderMouse instanceof MaterialNode)) {
            // If we didn't click on a MaterialNode, clear the selection
            deselectNodes();
            return;
        }

        selectNode((MaterialNode) componentUnderMouse);
    }

    @Override
    public void mouseDragStarted(MouseEvent e) {
        // If we've started dragging over a slot circle, we've started forming a new connection
        NodeSlot slot = eventIsOverSlotCircle(e);
        if (slot != null) {
            nodeSlotDragStarted(slot);
            return;
        }

        // If we're not dragging over a SlotCircle, check if we're dragging a MaterialNode
        Component componentUnderMouse = getComponentAt(e.getPoint());
        if (componentUnderMouse instanceof MaterialNode) {
            mNodeBeingDragged = (MaterialNode) componentUnderMouse;
            selectNode(mNodeBeingDragged);
        }
    }

    @Override
    public void mouseDragEnded(MouseEvent e) {
        if (mNodeBeingDragged != null) {
            // We've just finished dragging a node, so alert the presenter
            mPresenter.nodeMoved(mNodeBeingDragged.getModel(), mNodeBeingDragged.getX(),
                    mNodeBeingDragged.getY());
            mNodeBeingDragged = null;
        }

        if (!mIsConnecting) {
            return;
        }
        mIsConnecting = false;

        // If we've finished dragging over a SlotCircle, we might be forming a new connection
        NodeSlot slot = eventIsOverSlotCircle(e);
        if (slot != null) {
            nodeSlotDragEnded(slot);
        }

        repaint();
    }

    @Override
    public void mouseDragged(MouseEvent e) {
        if (mNodeBeingDragged != null) {
            // Update the location of the node being dragged by the mouse
            mNodeBeingDragged.setLocation(mNodeBeingDragged.getX() + mEventManager.getMouseDeltaX(),
                    mNodeBeingDragged.getY() + mEventManager.getMouseDeltaY());
            repaint();
            return;
        }

        nodeSlotDragged(e);
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);

        Graphics2D g2d = (Graphics2D) g;
        SwingHelper.setRenderingHints(g2d);

        // Paint background
        g2d.setColor(ColorScheme.INSTANCE.getBackground());
        g2d.fillRect(0, 0, getWidth(), getHeight());

        // If we're currently connecting two slots, draw the connection
        if (mIsConnecting && mInProgressConnectionLine != null) {
            mInProgressConnectionLine.paint(g2d);
        }

        mConnectionLines.forEach(connection -> connection.paint(g2d));
    }

    private void nodeSlotDragStarted(NodeSlot slot) {
        // If the user starts dragging on an input slot that already has a connection, remove the
        // previous connection
        for (ConnectionModel connection : mConnectionViewMap.keySet()) {
            if (connection.getInputSlot() == slot.getModel()) {
                mPresenter.connectionDisconnectedAtInput(connection);
                return;
            }
        }

        // Create a new connection line to represent this new "in progress" connection
        mInProgressConnectionLine = new ConnectionLine(slotPoint(slot), slotPoint(slot));
        mInProgressOriginSlot = slot.getModel();
        mIsConnecting = true;
        repaint();
    }

    private void nodeSlotDragged(MouseEvent e) {
        if (!mIsConnecting || mInProgressConnectionLine == null) {
            return;
        }
        // Depending on what type of slot the user is dragging from, the mouse is updating the
        // input or output point
        if (mInProgressOriginSlot instanceof InputSlotModel) {
            mInProgressConnectionLine.setOutputPoint(arbitraryPoint(e.getX(), e.getY()));
        } else {
            mInProgressConnectionLine.setInputPoint(arbitraryPoint(e.getX(), e.getY()));
        }
        repaint();
    }

    private void nodeSlotDragEnded(NodeSlot slot) {
        SlotModel output = mInProgressOriginSlot;
        SlotModel input = slot.getModel();

        mIsConnecting = false;

        // If the user started dragging on an input slot, they're connecting the nodes "backwards"
        // from input to output, so swap output and input.
        if (mInProgressOriginSlot instanceof InputSlotModel) {
            SlotModel temp = output;
            output = input;
            input = temp;
        }

        ConnectionModel newConnectionModel = new ConnectionModel((OutputSlotModel) output,
                (InputSlotModel)input);

        mPresenter.connectionCreated(newConnectionModel);

        repaint();
    }

    /**
     * A partially-removed connection means that one end of a connection is not connected. This
     * happens when the user drags on an input slot that already has a connection as if to "move"
     * the connection to a different slot.
     */
    public void disconnectConnectionAtInput(ConnectionModel model) {
        ConnectionLine connection = mConnectionViewMap.get(model);

        if (connection == null) {
            return;
        }

        // First, remove the connection view
        mConnectionViewMap.remove(model);
        mConnectionLines.remove(connection);

        mInProgressConnectionLine = connection;

        // Create an in-progress connection by disconnecting the destination
        mInProgressOriginSlot = model.getOutputSlot();

        mIsConnecting = true;
        repaint();
    }

    /**
     * Check if the event is over a slot circle. If it is, return the corresponding NodeSlot.
     */
    private NodeSlot eventIsOverSlotCircle(MouseEvent e) {
        Component componentUnderEvent = findComponentAt(e.getPoint());
        if (componentUnderEvent == null) {
            return null;
        }
        if (componentUnderEvent instanceof SlotCircle) {
            SlotCircle slotCircle = (SlotCircle) componentUnderEvent;
            return (NodeSlot) slotCircle.getParent();
        }
        return null;
    }

    private void selectNode(MaterialNode node) {
        resetNodeSelections();
        node.setSelected(true);
        notifySelectionIfChanged(new SelectionModel(Collections.singletonList(node.getModel())));
    }

    private void deselectNodes() {
        resetNodeSelections();
        notifySelectionIfChanged(new SelectionModel());
    }

    /**
     * If selectionModel has changed, notify the presenter. Here we compare against the previous
     * selection in order to avoid notifying the presenter twice for the same selection (e.g. if the
     * user continuously clicks on the same node)
     */
    private void notifySelectionIfChanged(SelectionModel selectionModel) {
        if (selectionModel.equals(mPreviousSelectionModel)) {
            return;
        }
        mPresenter.selectionChanged(selectionModel);
        mPreviousSelectionModel = selectionModel;
    }

    private void resetNodeSelections() {
        for (MaterialNode node : mModelViewMap.values()) {
            node.setSelected(false);
        }
    }
}
