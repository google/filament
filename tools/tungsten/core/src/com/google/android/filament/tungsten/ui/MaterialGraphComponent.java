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
import static com.google.android.filament.tungsten.ui.RenderNodeKt.renderNode;

import com.google.android.filament.tungsten.SwingHelper;
import com.google.android.filament.tungsten.model.Connection;
import com.google.android.filament.tungsten.model.Graph;
import com.google.android.filament.tungsten.model.Node;
import com.google.android.filament.tungsten.model.Slot;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import javax.swing.JComponent;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

public class MaterialGraphComponent extends JComponent implements TungstenMouseListener {

    private final TungstenEventManager mEventManager = new TungstenEventManager(this);
    private MaterialNode mNodeBeingDragged;
    private List<Integer> mPreviousSelectionModel = new ArrayList<>();
    private boolean mIsConnecting = false;

    // A connection is "in progress" when a user has started dragging from a slot
    private ConnectionLine mInProgressConnectionLine = null;

    // The slot the user started dragging from when forming a connection
    @Nullable
    private Slot mInProgressOriginSlot = null;

    private final List<ConnectionLine> mConnectionLines = new ArrayList<>();
    private GraphPresenter mPresenter;

    // Stores the current node views being shown
    private final Map<Node, MaterialNode> mNodeViews = new HashMap<>();

    // Stores the current graph displayed in the component, or null if one hasn't been rendered yet
    @Nullable
    private Graph mGraph = null;

    // Maps between the SlotModels we receive and the NodeSlot views
    private Map<Slot, NodeSlot> mSlotMap = new HashMap<>();

    // Maps between the ConnectionModels we receive and our views
    private final Map<Connection, ConnectionLine> mConnectionViewMap = new HashMap<>();

    public MaterialGraphComponent() {
        addMouseMotionListener(mEventManager);
        addMouseListener(mEventManager);
    }

    public void setPresenter(GraphPresenter presenter) {
        mPresenter = presenter;
    }

    public void render(@NotNull Graph graph) {
        // Because graphs are immutable, if the graph we previously rendered is the same graph
        // object, we have no work to do.
        if (mGraph == graph) {
            return;
        }

        // The strategy here is to create new MaterialNodes for any new nodes in the graph, and
        // remove any MaterialNodes that are no longer present in the graph.
        Set<MaterialNode> nodesToRemove = new HashSet<>(mNodeViews.values());

        for (Node node : graph.getNodes()) {
            MaterialNode currentNodeView = mNodeViews.get(node);
            if (currentNodeView != null) {
                nodesToRemove.remove(currentNodeView);
                continue;
            }
            // If we don't have a view for this node yet, create a new one by calling renderNode
            MaterialNode nodeView = renderNode(node, mSlotMap);
            nodeView.setSelected(graph.isNodeSelected(node));
            mNodeViews.put(node, nodeView);
            add(nodeView);
        }

        // Remove whichever nodes are no longer present in the graph.
        for (MaterialNode node : nodesToRemove) {
            mNodeViews.remove(node.getModel());
            remove(node);
        }

        // Create a connection line for each connection in the graph.
        mConnectionLines.clear();
        mConnectionViewMap.clear();
        for (Connection connection : graph.getConnections()) {
            Node.OutputSlot outputSlot = connection.getOutputSlot();
            Node.InputSlot inputSlot = connection.getInputSlot();
            ConnectionLine line = new ConnectionLine(slotPoint(outputSlot, this),
                    slotPoint(inputSlot, this));
            mConnectionLines.add(line);
            mConnectionViewMap.put(connection, line);
        }

        revalidate();
        repaint();

        mGraph = graph;
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

    @Nullable
    NodeSlot getSlotViewForSlot(@NotNull Slot slot) {
        return mSlotMap.get(slot);
    }

    private void nodeSlotDragStarted(NodeSlot slot) {
        // If the user starts dragging on an input slot that already has a connection, remove the
        // previous connection
        assert mGraph != null;
        for (Connection connection : mGraph.getConnections()) {
            if (connection.getInputSlot().equals(slot.getModel())) {
                // First, remove the connection view
                ConnectionLine connectionLine = mConnectionViewMap.remove(connection);
                if (connectionLine == null) {
                    return;
                }
                mConnectionLines.remove(connectionLine);
                mInProgressConnectionLine = connectionLine;

                // Create an in-progress connection by disconnecting the destination
                mInProgressOriginSlot = connection.getOutputSlot();
                mIsConnecting = true;

                mPresenter.connectionDisconnectedAtInput(connection);

                return;
            }
        }

        // Create a new connection line to represent this new "in progress" connection
        mInProgressConnectionLine = new ConnectionLine(slotPoint(slot.getModel(), this),
                slotPoint(slot.getModel(), this));
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
        if (mInProgressOriginSlot instanceof Node.InputSlot) {
            mInProgressConnectionLine.setOutputPoint(arbitraryPoint(e.getX(), e.getY()));
        } else {
            mInProgressConnectionLine.setInputPoint(arbitraryPoint(e.getX(), e.getY()));
        }
        repaint();
    }

    private void nodeSlotDragEnded(NodeSlot slot) {
        Slot output = mInProgressOriginSlot;
        Slot input = slot.getModel();

        mIsConnecting = false;

        // If the user started dragging on an input slot, they're connecting the nodes "backwards"
        // from input to output, so swap output and input.
        if (mInProgressOriginSlot instanceof Node.InputSlot) {
            Slot temp = output;
            output = input;
            input = temp;
        }

        Node.OutputSlot outputSlot = (Node.OutputSlot) output;
        Node.InputSlot inputSlot = (Node.InputSlot) input;
        Connection newConnectionModel = new Connection(outputSlot, inputSlot);

        mPresenter.connectionCreated(newConnectionModel);

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
        notifySelectionIfChanged(Collections.singletonList(node.getModel().getId()));
    }

    private void deselectNodes() {
        resetNodeSelections();
        notifySelectionIfChanged(Collections.emptyList());
    }

    /**
     * If selectionModel has changed, notify the presenter. Here we compare against the previous
     * selection in order to avoid notifying the presenter twice for the same selection (e.g. if the
     * user continuously clicks on the same node)
     */
    private void notifySelectionIfChanged(List<Integer> selectedNodes) {
        if (selectedNodes.equals(mPreviousSelectionModel)) {
            return;
        }
        mPresenter.selectionChanged(selectedNodes);
        mPreviousSelectionModel = selectedNodes;
    }

    private void resetNodeSelections() {
        for (MaterialNode node : mNodeViews.values()) {
            node.setSelected(false);
        }
    }
}
