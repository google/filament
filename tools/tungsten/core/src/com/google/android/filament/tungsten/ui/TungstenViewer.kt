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

import com.google.android.filament.Camera
import com.google.android.filament.Viewport
import com.google.android.filament.tungsten.Filament

class TungstenViewer(val camera: Camera, val previewMeshPanel: PreviewMeshPanel)
    : Filament.Viewer() {

    private val previewCamera = PreviewCamera(camera)
    private var time: Long = 0

    override fun update(deltaTimeMs: Long) {
        time += deltaTimeMs

        // Update preview camera
        val delta = time / 2000.0 * Math.PI
        val x = 3f * Math.sin(delta)
        val z = 3f * Math.cos(delta)
        camera.lookAt(x, 0.0, z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)

        val width = previewMeshPanel.getWidth()
        val height = previewMeshPanel.getHeight()
        view.setViewport(Viewport(0, 0, width, height))
        previewCamera.setProjection(width, height)
    }
}