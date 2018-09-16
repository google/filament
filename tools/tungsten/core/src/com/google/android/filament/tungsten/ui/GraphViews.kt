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

import java.awt.Graphics2D
import java.awt.geom.Point2D

private const val SLOT_CIRCLE_RADIUS = 10.0f
private const val SLOT_TEXT_PAD = 5.0f
private const val ARC_RADIUS = 10
private const val BORDER_THICKNESS = 2
private const val NODE_HEIGHT = 100.0f
private const val NODE_WIDTH = 200.0f
private const val SLOT_MARGIN = 10.0f
private const val SLOT_HEIGHT = 20.0f

internal class SlotCircle(val parent: SlotView) : View() {

    val centerPoint = Point2D.Float(SLOT_CIRCLE_RADIUS / 2, SLOT_CIRCLE_RADIUS / 2)

    override fun render(g2d: Graphics2D) {
        g2d.color = ColorScheme.slotCircle
        g2d.fillOval(0, 0, SLOT_CIRCLE_RADIUS.toInt(), SLOT_CIRCLE_RADIUS.toInt())
    }
}

internal data class SlotView(
    val label: String,
    val isInput: Boolean,

    val onConnectionDragStart: (SlotView) -> Unit,
    val onConnectionDragEnd: (SlotView) -> Unit
) : View() {

    val circle = SlotCircle(this)
    override val children = listOf(circle)

    override val reactToMouseEvents = false

    override fun layout() {
        if (isInput) {
            circle.x = 0.0f
            circle.y = 0.0f
        } else {
            circle.x = width - SLOT_CIRCLE_RADIUS
            circle.y = 0.0f
        }
        circle.width = SLOT_CIRCLE_RADIUS
        circle.height = SLOT_CIRCLE_RADIUS
    }

    override fun render(g2d: Graphics2D) {
        g2d.color = ColorScheme.slotLabel
        if (isInput) {
            g2d.drawString(label, SLOT_CIRCLE_RADIUS + SLOT_TEXT_PAD, g2d.fontMetrics.height / 2.0f)
        } else {
            val textWidth = g2d.fontMetrics.stringWidth(label)
            g2d.drawString(label, (width - SLOT_CIRCLE_RADIUS - SLOT_TEXT_PAD) - textWidth,
                    g2d.fontMetrics.height / 2.0f)
        }
    }
}

internal data class NodeView(
    override var x: Float,
    override var y: Float,
    val inputSlots: List<SlotView>,
    val outputSlots: List<SlotView>,
    val isSelected: Boolean,
    val nodeDragStarted: (NodeView) -> Unit,
    val nodeDragStopped: (NodeView) -> Unit,
    val nodeClicked: () -> Unit
) : View() {

    override val children = inputSlots + outputSlots

    override fun layout() {
        width = NODE_WIDTH
        height = maxOf((inputSlots.size + outputSlots.size) * SLOT_HEIGHT + 2 * SLOT_MARGIN,
                NODE_HEIGHT)
        inputSlots.forEachIndexed { index, slot ->
            slot.x = SLOT_MARGIN
            slot.y = SLOT_MARGIN + index * SLOT_HEIGHT
            slot.width = width - SLOT_MARGIN * 2
            slot.height = SLOT_HEIGHT
        }
        outputSlots.forEachIndexed { index, slot ->
            slot.x = SLOT_MARGIN
            slot.y = SLOT_MARGIN + (index + inputSlots.size) * SLOT_HEIGHT
            slot.width = width - SLOT_MARGIN * 2
            slot.height = SLOT_HEIGHT
        }
    }

    override fun render(g2d: Graphics2D) {
        if (isSelected) {
            g2d.color = ColorScheme.nodeSelected
            g2d.fillRoundRect(0, 0, width.toInt(), height.toInt(), ARC_RADIUS, ARC_RADIUS)
        }
        g2d.color = ColorScheme.nodeBackground
        g2d.fillRoundRect(BORDER_THICKNESS, BORDER_THICKNESS,
                (width - 2 * BORDER_THICKNESS).toInt(),
                (height - 2 * BORDER_THICKNESS).toInt(), ARC_RADIUS, ARC_RADIUS)
    }
}

internal data class GraphView(
    val nodes: List<NodeView>,
    override var width: Float,
    override var height: Float
) : View() {

    override val children = nodes
}
