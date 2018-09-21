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
import com.google.android.filament.tungsten.model.Slot
import java.awt.BasicStroke
import java.awt.Color
import java.awt.Graphics2D
import java.awt.geom.GeneralPath
import java.awt.geom.Point2D

/**
 * Endpoint provides either a start or end point to a ConnectionLine.
 */
typealias Endpoint = () -> Point2D.Double?

internal fun slotPoint(slot: Slot, graph: MaterialGraphComponent): Endpoint {
    return fun (): Point2D.Double? {
        val graphView = graph.graphView ?: return null
        val slotView = graph.getSlotCircleForSlot(slot) ?: return null
        val centerPoint = convertPoint(slotView, slotView.centerPoint, graphView)
        if (centerPoint != null) {
            return Point2D.Double(centerPoint.x, centerPoint.y)
        }
        return null
    }
}

internal fun arbitraryPoint(x: Double, y: Double): Endpoint {
    return {
        Point2D.Double(x, y)
    }
}

/**
 * ConnectionLine is a line drawn between two slots, or if a connection is being formed, between a
 * slot and an arbitrary point.
 */
class ConnectionLine(from: Endpoint, to: Endpoint) {

    var outputPoint = from
    var inputPoint = to

    fun paint(g2d: Graphics2D) {
        SwingHelper.setRenderingHints(g2d)
        g2d.color = ColorScheme.connectionLine

        val originPoint = outputPoint() ?: return
        val destinationPoint = inputPoint() ?: return

        val deltaX = Math.abs(destinationPoint.getX() - originPoint.getX())

        val p2 = Point2D.Double(originPoint.getX() + deltaX / 2.0, originPoint.getY())
        val p3 = Point2D.Double(destinationPoint.getX() - deltaX / 2, destinationPoint.getY())

        drawBezierCurve(g2d, originPoint, p2, p3, destinationPoint)
    }

    /**
     * Draw a 4 control point Bezier curve.
     */
    fun drawBezierCurve(
        g2d: Graphics2D,
        p1: Point2D.Double,
        p2: Point2D.Double,
        p3: Point2D.Double,
        p4: Point2D.Double
    ) {
        val path = GeneralPath()

        path.reset()

        path.moveTo(p1.x, p1.y)
        path.curveTo(p2.x, p2.y, p3.x, p3.y, p4.x, p4.y)

        val actionStroke = BasicStroke(2f)

        g2d.stroke = actionStroke
        g2d.color = Color.WHITE
        g2d.draw(path)
    }
}
