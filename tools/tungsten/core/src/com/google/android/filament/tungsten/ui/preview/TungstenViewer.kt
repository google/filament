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

package com.google.android.filament.tungsten.ui.preview

import com.curiouscreature.kotlin.math.Float2
import com.google.android.filament.Camera
import com.google.android.filament.Viewport
import com.google.android.filament.tungsten.Filament
import java.awt.event.MouseEvent
import java.awt.event.MouseListener
import java.awt.event.MouseMotionListener

private const val SPEED_MULTIPLIER = 100.0f
private const val DOLLY_MULTIPLIER = 5.0f

private fun MouseEvent.toFloat2() = Float2(this.x.toFloat(), this.y.toFloat())

internal class TungstenViewer(camera: Camera, val previewMeshPanel: PreviewMeshPanel)
    : Filament.Viewer() {

    private val previewCamera = PreviewCamera(camera)
    private val manipulator = CameraManipulator(camera)

    private var lastMouse: Float2? = null

    init {
        previewMeshPanel.addMouseMotionListener(object : MouseMotionListener {
            override fun mouseMoved(e: MouseEvent?) { }

            override fun mouseDragged(e: MouseEvent?) {
                e ?: return
                lastMouse?.let {
                    val delta = e.toFloat2() - it
                    val scaledDelta = delta / Float2(previewMeshPanel.width.toFloat(),
                            previewMeshPanel.height.toFloat())
                    manipulator.rotate(scaledDelta, SPEED_MULTIPLIER)
                }
                lastMouse = e.toFloat2()
            }
        })
        previewMeshPanel.addMouseListener(object : MouseListener {
            override fun mouseReleased(e: MouseEvent?) { }

            override fun mouseEntered(e: MouseEvent?) { }

            override fun mouseClicked(e: MouseEvent?) { }

            override fun mouseExited(e: MouseEvent?) { }

            override fun mousePressed(e: MouseEvent?) {
                e ?: return
                lastMouse = e.toFloat2()
            }
        })
        previewMeshPanel.addMouseWheelListener { e ->
            e?.let { mouseWheelEvent ->
                // Swing reports wheel events caused by a physical wheel (opposed to a trackpad)
                // with an inverted sign. Flip it so it feels more intuitive.
                val causedByWheel = mouseWheelEvent.wheelRotation != 0
                val multiplier = if (causedByWheel) 1 else -1
                manipulator.dolly(multiplier * mouseWheelEvent.preciseWheelRotation.toFloat() /
                        previewMeshPanel.width, DOLLY_MULTIPLIER)
            }
        }
    }

    override fun update(deltaTimeMs: Long) {
        manipulator.updateCameraTransform()

        val width = previewMeshPanel.width
        val height = previewMeshPanel.height
        view.setViewport(Viewport(0, 0, width, height))
        previewCamera.setProjection(width, height)
    }
}