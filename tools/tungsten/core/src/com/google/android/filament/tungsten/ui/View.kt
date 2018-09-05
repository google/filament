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
import java.awt.geom.Rectangle2D

abstract class View {

    /**
     * x, y, width, and height are specified in this View's parent's coordinate system.
     */

    open var x: Float = 0.0f
    open var y: Float = 0.0f
    open var width: Float = 0.0f
    open var height: Float = 0.0f

    /**
     * True, if the View should participate in calls to findViewAt.
     */
    open val reactToMouseEvents: Boolean = true

    /**
     * Bounds of the View with respect to its parent's coordinate system.
     */
    val bounds: Rectangle2D.Float
        get() = Rectangle2D.Float(x, y, width, height)

    open val children: List<View> = emptyList()

    open fun layout() { }

    open fun render(g2d: Graphics2D) { }
}

fun layoutHierarchy(root: View) {
    root.layout()
    root.children.forEach { child -> layoutHierarchy(child) }
}

fun renderHierarchy(root: View, g2d: Graphics2D) {
    g2d.translate(root.x.toDouble(), root.y.toDouble())
    root.render(g2d)
    root.children.forEach { child -> renderHierarchy(child, g2d) }
    g2d.translate(-root.x.toDouble(), -root.y.toDouble())
}

/**
 * Finds the deepest nested view that contains point p. The point p should be in root's coordinate
 * system.
 * This assumes views at the same level of the view hierarchy do not overlap each other.
 */
fun findViewAt(root: View, p: Point2D): View? {
    val child = root.children.firstOrNull { v ->
        v.bounds.contains(p)
    } ?: return null
    p.setLocation(p.x - child.x, p.y - child.y)
    return findViewAt(child, p) ?: (if (child.reactToMouseEvents) child else null)
}

/**
 * Converts from source's coordinates to destination's coordinates.
 * source view must be a descendant of destination view.
 */
fun convertPoint(source: View, point: Point2D, destination: View): Point2D? {
    if (source === destination) return point
    for (child in destination.children) {
        val result = convertPoint(source, Point2D.Float(point.x.toFloat() + child.x,
                point.y.toFloat() + child.y), child)
        if (result != null) return result
    }
    return null
}
