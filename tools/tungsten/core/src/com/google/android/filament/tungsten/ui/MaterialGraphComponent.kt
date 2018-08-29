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

package com.google.android.filament.tungsten.ui

import com.google.android.filament.tungsten.SwingHelper
import com.google.android.filament.tungsten.model.Connection
import com.google.android.filament.tungsten.model.Graph
import com.google.android.filament.tungsten.model.Node
import com.google.android.filament.tungsten.model.Slot
import java.awt.Graphics
import java.awt.Graphics2D
import java.awt.event.MouseEvent
import java.util.ArrayList
import java.util.HashMap
import java.util.HashSet
import javax.swing.JComponent

class MaterialGraphComponent : JComponent(), TungstenMouseListener {

    private val mEventManager = TungstenEventManager(this)
    private var mNodeBeingDragged: MaterialNode? = null
    private var mPreviousSelectionModel: List<Int> = ArrayList()
    private var mIsConnecting = false

    // A connection is "in progress" when a user has started dragging from a slot
    private var mInProgressConnectionLine: ConnectionLine? = null

    // The slot the user started dragging from when forming a connection
    private var mInProgressOriginSlot: Slot? = null

    private val mConnectionLines = ArrayList<ConnectionLine>()
    private var mPresenter: GraphPresenter? = null

    // Stores the current node views being shown
    private val mNodeViews = HashMap<Node, MaterialNode>()

    // Stores the current graph displayed in the component, or null if one hasn't been rendered yet
    private var mGraph: Graph? = null

    // Maps between the SlotModels we receive and the NodeSlot views
    private val mSlotMap = HashMap<Slot, NodeSlot>()

    // Maps between the ConnectionModels we receive and our views
    private val mConnectionViewMap = HashMap<Connection, ConnectionLine>()

    init {
        addMouseMotionListener(mEventManager)
        addMouseListener(mEventManager)
    }

    fun setPresenter(presenter: GraphPresenter) {
        mPresenter = presenter
    }

    fun render(graph: Graph) {
        // Because graphs are immutable, if the graph we previously rendered is the same graph
        // object, we have no work to do.
        if (mGraph === graph) {
            return
        }

        // The strategy here is to create new MaterialNodes for any new nodes in the graph, and
        // remove any MaterialNodes that are no longer present in the graph.
        val nodesToRemove = HashSet(mNodeViews.values)

        for (node in graph.nodes) {
            val currentNodeView = mNodeViews[node]
            if (currentNodeView != null) {
                nodesToRemove.remove(currentNodeView)
                continue
            }
            // If we don't have a view for this node yet, create a new one by calling renderNode
            val nodeView = renderNode(node, mSlotMap)
            nodeView.setSelected(graph.isNodeSelected(node))
            mNodeViews[node] = nodeView
            add(nodeView)
        }

        // Remove whichever nodes are no longer present in the graph.
        for (node in nodesToRemove) {
            mNodeViews.remove(node.model)
            remove(node)
        }

        // Create a connection line for each connection in the graph.
        mConnectionLines.clear()
        mConnectionViewMap.clear()
        for (connection in graph.connections) {
            val outputSlot = connection.outputSlot
            val inputSlot = connection.inputSlot
            val line = ConnectionLine(slotPoint(outputSlot, this),
                    slotPoint(inputSlot, this))
            mConnectionLines.add(line)
            mConnectionViewMap[connection] = line
        }

        revalidate()
        repaint()

        mGraph = graph
    }

    override fun mousePressed(e: MouseEvent) {
        // Check if we've clicked on a MaterialNode.
        val componentUnderMouse = getComponentAt(e.point)
        if (componentUnderMouse !is MaterialNode) {
            // If we didn't click on a MaterialNode, clear the selection
            deselectNodes()
            return
        }

        selectNode(componentUnderMouse)
    }

    override fun mouseDragStarted(e: MouseEvent) {
        // If we've started dragging over a slot circle, we've started forming a new connection
        val slot = eventIsOverSlotCircle(e)
        if (slot != null) {
            nodeSlotDragStarted(slot)
            return
        }

        // If we're not dragging over a SlotCircle, check if we're dragging a MaterialNode
        val componentUnderMouse = getComponentAt(e.point)
        if (componentUnderMouse is MaterialNode) {
            mNodeBeingDragged = componentUnderMouse
            selectNode(mNodeBeingDragged!!)
        }
    }

    override fun mouseDragEnded(e: MouseEvent) {
        if (mNodeBeingDragged != null) {
            // We've just finished dragging a node, so alert the presenter
            mPresenter!!.nodeMoved(mNodeBeingDragged!!.model, mNodeBeingDragged!!.x,
                    mNodeBeingDragged!!.y)
            mNodeBeingDragged = null
        }

        if (!mIsConnecting) {
            return
        }
        mIsConnecting = false

        // If we've finished dragging over a SlotCircle, we might be forming a new connection
        val slot = eventIsOverSlotCircle(e)
        if (slot != null) {
            nodeSlotDragEnded(slot)
        }

        repaint()
    }

    override fun mouseDragged(e: MouseEvent) {
        if (mNodeBeingDragged != null) {
            // Update the location of the node being dragged by the mouse
            mNodeBeingDragged!!.setLocation(mNodeBeingDragged!!.x + mEventManager.mouseDeltaX,
                    mNodeBeingDragged!!.y + mEventManager.mouseDeltaY)
            repaint()
            return
        }

        nodeSlotDragged(e)
    }

    override fun paintComponent(g: Graphics) {
        super.paintComponent(g)

        val g2d = g as Graphics2D
        SwingHelper.setRenderingHints(g2d)

        // Paint background
        g2d.color = ColorScheme.background
        g2d.fillRect(0, 0, width, height)

        // If we're currently connecting two slots, draw the connection
        if (mIsConnecting && mInProgressConnectionLine != null) {
            mInProgressConnectionLine!!.paint(g2d)
        }

        mConnectionLines.forEach { connection -> connection.paint(g2d) }
    }

    internal fun getSlotViewForSlot(slot: Slot): NodeSlot? {
        return mSlotMap[slot]
    }

    private fun nodeSlotDragStarted(slot: NodeSlot) {
        // If the user starts dragging on an input slot that already has a connection, remove the
        // previous connection
        assert(mGraph != null)
        for (connection in mGraph!!.connections) {
            if (connection.inputSlot == slot.model) {
                // First, remove the connection view
                val connectionLine = mConnectionViewMap.remove(connection) ?: return
                mConnectionLines.remove(connectionLine)
                mInProgressConnectionLine = connectionLine

                // Create an in-progress connection by disconnecting the destination
                mInProgressOriginSlot = connection.outputSlot
                mIsConnecting = true

                mPresenter!!.connectionDisconnectedAtInput(connection)

                return
            }
        }

        // Create a new connection line to represent this new "in progress" connection
        mInProgressConnectionLine = ConnectionLine(slotPoint(slot.model, this),
                slotPoint(slot.model, this))
        mInProgressOriginSlot = slot.model
        mIsConnecting = true
        repaint()
    }

    private fun nodeSlotDragged(e: MouseEvent) {
        if (!mIsConnecting || mInProgressConnectionLine == null) {
            return
        }
        // Depending on what type of slot the user is dragging from, the mouse is updating the
        // input or output point
        if (mInProgressOriginSlot is Node.InputSlot) {
            mInProgressConnectionLine!!.outputPoint = arbitraryPoint(e.x.toDouble(), e.y.toDouble())
        } else {
            mInProgressConnectionLine!!.inputPoint = arbitraryPoint(e.x.toDouble(), e.y.toDouble())
        }
        repaint()
    }

    private fun nodeSlotDragEnded(slot: NodeSlot) {
        var output = mInProgressOriginSlot
        var input = slot.model

        mIsConnecting = false

        // If the user started dragging on an input slot, they're connecting the nodes "backwards"
        // from input to output, so swap output and input.
        if (mInProgressOriginSlot is Node.InputSlot) {
            val temp = output
            output = input
            input = temp
        }

        val outputSlot = output as Node.OutputSlot?
        val inputSlot = input as Node.InputSlot
        val newConnectionModel = Connection(outputSlot!!, inputSlot)

        mPresenter!!.connectionCreated(newConnectionModel)

        repaint()
    }

    /**
     * Check if the event is over a slot circle. If it is, return the corresponding NodeSlot.
     */
    private fun eventIsOverSlotCircle(e: MouseEvent): NodeSlot? {
        val componentUnderEvent = findComponentAt(e.point) ?: return null
        return if (componentUnderEvent is SlotCircle) {
            componentUnderEvent.parent as NodeSlot
        } else null
    }

    private fun selectNode(node: MaterialNode) {
        resetNodeSelections()
        node.setSelected(true)
        notifySelectionIfChanged(listOf(node.model.id))
    }

    private fun deselectNodes() {
        resetNodeSelections()
        notifySelectionIfChanged(emptyList())
    }

    /**
     * If selectionModel has changed, notify the presenter. Here we compare against the previous
     * selection in order to avoid notifying the presenter twice for the same selection (e.g. if the
     * user continuously clicks on the same node)
     */
    private fun notifySelectionIfChanged(selectedNodes: List<Int>) {
        if (selectedNodes == mPreviousSelectionModel) {
            return
        }
        mPresenter!!.selectionChanged(selectedNodes)
        mPreviousSelectionModel = selectedNodes
    }

    private fun resetNodeSelections() {
        for (node in mNodeViews.values) {
            node.setSelected(false)
        }
    }
}
