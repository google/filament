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
import com.curiouscreature.kotlin.math.Float3
import com.curiouscreature.kotlin.math.Float4
import com.curiouscreature.kotlin.math.length
import com.curiouscreature.kotlin.math.normalize
import com.curiouscreature.kotlin.math.rotation
import com.curiouscreature.kotlin.math.translation
import com.curiouscreature.kotlin.math.transpose
import com.google.android.filament.Camera
import kotlin.math.exp

private const val START_RADIUS = 3.0f

private fun rotateVector(rx: Float, ry: Float, v: Float3): Float3 {
    val matrix = rotation(Float3(rx, ry, 0.0f))
    return matrix.times(Float4(v)).xyz
}

internal class CameraManipulator(private val camera: Camera) {

    private var cameraTranslation = Float3(z = START_RADIUS)
    private var cameraRotation = Float3()
    private var centerOfInterest = -START_RADIUS

    fun updateCameraTransform() {
        val view = translation(cameraTranslation) * rotation(cameraRotation)
        camera.setModelMatrix(transpose(view).toFloatArray())
    }

    fun dolly(delta: Float, dollySpeed: Float) {
        val eye = cameraTranslation
        val v = rotateVector(cameraRotation.x, cameraRotation.y,
                Float3(0.0f, 0.0f, centerOfInterest))
        val view = eye + v

        normalize(v)
        val dollyBy = (1.0f - exp(-dollySpeed * delta)) * centerOfInterest

        val newEye = eye + (v * dollyBy)

        cameraTranslation = newEye
        centerOfInterest = -length(newEye - view)
    }

    fun rotate(delta: Float2, rotateSpeed: Float) {
        var rotX = cameraRotation.x
        var rotY = cameraRotation.y
        val eye = cameraTranslation

        val view = eye + rotateVector(rotX, rotY, Float3(0.0f, 0.0f, centerOfInterest))

        rotY += -delta.x * rotateSpeed
        rotX += -delta.y * rotateSpeed

        cameraTranslation = view - rotateVector(rotX, rotY, Float3(0.0f, 0.0f, centerOfInterest))
        cameraRotation = Float3(rotX, rotY, cameraRotation.z)
    }
}