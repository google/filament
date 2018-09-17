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
import com.google.android.filament.tungsten.model.NodeId
import com.google.android.filament.tungsten.model.Slot
import java.awt.Graphics
import java.awt.Graphics2D
import java.awt.event.MouseEvent
import java.util.ArrayList
import java.util.HashMap
import javax.swing.JComponent

class MaterialGraphComponent : JComponent(), TungstenMouseListener {

    private val mEventManager = TungstenEventManager(this)
    private var mNodeBeingDragged: NodeId? = null
    private var mPreviousSelectionModel: List<Int> = ArrayList()
    private var mIsConnecting = false

    // A connection is "in progress" when a user has started dragging from a slot
    private var mInProgressConnectionLine: ConnectionLine? = null

    // The slot the user started dragging from when forming a connection
    private var mInProgressOriginSlot: Slot? = null

    private val mConnectionLines = ArrayList<ConnectionLine>()
    private var mPresenter: GraphPresenter? = null

    // Stores the current node views being shown
    private val mNodeViews = mutableListOf<NodeView>()

    // Stores the current graph displayed in the component, or null if one hasn't been rendered yet
    private var mGraph: Graph? = null

    internal var graphView: GraphView? = null

    // Maps between the Nodes and the associated NodeViews that represent them
    private val nodeMap = HashMap<NodeId, NodeView>()

    // Maps between the SlotModels we receive and the NodeSlot views
    private val mSlotMap = HashMap<Slot, SlotCircle>()

    // Maps between the ConnectionModels we receive and our views
    private val mConnectionViewMap = HashMap<Connection, ConnectionLine>()

    init {
        addMouseMotionListener(mEventManager)
        addMouseListener(mEventManager)
    }

    fun setPresenter(presenter: GraphPresenter) {
        mPresenter = presenter
    }

    private fun renderGraph(graph: Graph): GraphView {
        mSlotMap.clear()
        nodeMap.clear()

        val renderSlot = { node: Node, slotName: String, isInput: Boolean ->
            val slotHandle = if (isInput) {
                node.getInputSlot(slotName)
            } else {
                node.getOutputSlot(slotName)
            }
            val expression = graph.expressionMap[slotHandle]
            val slotView = SlotView(
                    label = if (expression != null)
                        "$slotName (${expression.dimensions})" else slotName,
                    isInput = isInput,
                    onConnectionDragStart = { this.nodeSlotDragStarted(slotHandle) },
                    onConnectionDragEnd = { this.nodeSlotDragEnded(slotHandle) }
            )
            mSlotMap[slotHandle] = slotView.circle
            slotView
        }

        val renderNode = { node: Node ->
            val nodeView = NodeView(
                    x = node.x,
                    y = node.y,
                    inputSlots = node.inputSlots.map { slotName ->
                        renderSlot(node, slotName, true)
                    },
                    outputSlots = node.outputSlots.map { slotName ->
                        renderSlot(node, slotName, false)
                    },
                    isSelected = graph.isNodeSelected(node),
                    nodeDragStarted = { view ->
                        mNodeBeingDragged = node.id
                        notifySelectionIfChanged(listOf(node.id))
                    },
                    nodeDragStopped = { view ->
                        // We've just finished dragging a node, so alert the presenter
                        mPresenter?.nodeMoved(node, view.x.toInt(), view.y.toInt())
                        mNodeBeingDragged = null
                    },
                    nodeClicked = {
                        notifySelectionIfChanged(listOf(node.id))
                    }
            )
            nodeMap[node.id] = nodeView
            nodeView
        }

        return GraphView(
                nodes = graph.nodes.map { renderNode(it) },
                width = this.width.toFloat(),
                height = this.height.toFloat()
        )
    }

    fun render(graph: Graph) {
        // Because graphs are immutable, if the graph we previously rendered is the same graph
        // object, we have no work to do.
        if (mGraph === graph) {
            return
        }

        val graphView = renderGraph(graph)
        this.graphView = graphView

        mNodeViews.clear()
        mNodeViews.addAll(graphView.nodes)

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

        // Layout the rendered views so they have accurate bounds. This must be done here and not in
        // paintComponent, as paintComponent might not be called immediately.
        layoutHierarchy(graphView)

        revalidate()
        repaint()

        mGraph = graph
    }

    override fun mousePressed(e: MouseEvent) {
        // Check if we've clicked on a MaterialNode.
        val node = graphView?.let { graphView ->
            val view = findViewAt(graphView, e.point)
            view as? NodeView?
        }
        if (node == null) {
            // If we didn't click on a MaterialNode, clear the selection
            deselectNodes()
            return
        }

        node.nodeClicked()
    }

    override fun mouseDragStarted(e: MouseEvent) {
        // If we've started dragging over a slot circle, we've started forming a new connection
        val graphView = graphView ?: return
        val view = findViewAt(graphView, e.point) ?: return

        when (view) {
            is SlotCircle -> view.parent.onConnectionDragStart(view.parent)
            is NodeView -> view.nodeDragStarted(view)
        }
    }

    override fun mouseDragEnded(e: MouseEvent) {
        mNodeBeingDragged?.let { nodeId ->
            val nodeView = nodeMap[nodeId]
            if (nodeView != null) {
                nodeView.nodeDragStopped(nodeView)
            }
        }

        if (!mIsConnecting) {
            return
        }
        mIsConnecting = false

        repaint()

        val graphView = graphView ?: return
        val view = findViewAt(graphView, e.point) ?: return

        when (view) {
            is SlotCircle -> view.parent.onConnectionDragEnd(view.parent)
        }
    }

    override fun mouseDragged(e: MouseEvent) {
        val nodeId = mNodeBeingDragged
        if (nodeId != null) {
            val nodeView = nodeMap[nodeId]
            if (nodeView != null) {
                nodeView.x += mEventManager.mouseDeltaX
                nodeView.y += mEventManager.mouseDeltaY
                repaint()
                return
            }
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
        if (mIsConnecting) {
            mInProgressConnectionLine?.paint(g2d)
        }

        mConnectionLines.forEach { connection -> connection.paint(g2d) }

        // Render graph
        graphView?.let { graphView -> renderHierarchy(graphView, g2d) }
    }

    internal fun getSlotCircleForSlot(slot: Slot) = mSlotMap[slot]

    private fun nodeSlotDragStarted(slot: Slot) {
        // If the user starts dragging on an input slot that already has a connection, remove the
        // previous connection
        val graph = mGraph ?: return
        for (connection in graph.connections) {
            if (connection.inputSlot == slot) {
                // First, remove the connection view
                val connectionLine = mConnectionViewMap.remove(connection) ?: return
                mConnectionLines.remove(connectionLine)
                mInProgressConnectionLine = connectionLine

                // Create an in-progress connection by disconnecting the destination
                mInProgressOriginSlot = connection.outputSlot
                mIsConnecting = true

                mPresenter?.connectionDisconnectedAtInput(connection)

                return
            }
        }

        // Create a new connection line to represent this new "in progress" connection
        mInProgressConnectionLine = ConnectionLine(slotPoint(slot, this),
                slotPoint(slot, this))
        mInProgressOriginSlot = slot
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
            mInProgressConnectionLine?.outputPoint = arbitraryPoint(e.x.toDouble(), e.y.toDouble())
        } else {
            mInProgressConnectionLine?.inputPoint = arbitraryPoint(e.x.toDouble(), e.y.toDouble())
        }
        repaint()
    }

    private fun nodeSlotDragEnded(slot: Slot) {
        var output = mInProgressOriginSlot ?: return
        var input = slot

        mIsConnecting = false

        // If the user started dragging on an input slot, they're connecting the nodes "backwards"
        // from input to output, so swap output and input.
        if (mInProgressOriginSlot is Node.InputSlot) {
            val temp = output
            output = input
            input = temp
        }

        val outputSlot = output as? Node.OutputSlot ?: return
        val inputSlot = input as? Node.InputSlot ?: return
        val newConnectionModel = Connection(outputSlot, inputSlot)

        mPresenter?.connectionCreated(newConnectionModel)

        repaint()
    }

    private fun deselectNodes() {
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
        mPresenter?.selectionChanged(selectedNodes)
        mPreviousSelectionModel = selectedNodes
    }
}
