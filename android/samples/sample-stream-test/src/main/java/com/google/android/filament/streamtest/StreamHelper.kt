/*
 * Copyright (C) 2019 The Android Open Source Project
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

package com.google.android.filament.streamtest

import android.graphics.LinearGradient
import android.os.Handler
import android.util.Size
import android.view.Surface

import android.graphics.*
import android.media.ImageReader
import android.opengl.Matrix
import android.os.Looper
import android.view.Display

import com.google.android.filament.*


/**
 * Demonstrates Filament's various texture sharing mechanisms.
 */
class StreamHelper(
    private val filamentEngine: Engine,
    private val filamentMaterial: MaterialInstance,
    private val display: Display,
) {
    /**
     * The StreamSource configures the source data for the texture.
     *
     * All tests draw animated test stripes as follows:
     *
     *  - The left stripe uses texture-based animation via Android's 2D drawing API.
     *  - The right stripe uses shader-based animation.
     *
     * Ideally these are perfectly in sync with each other.
     */
    enum class StreamSource {
        CANVAS_STREAM_NATIVE,     // copy-free but does not guarantee synchronization
        CANVAS_STREAM_ACQUIRED,   // synchronized and copy-free
    }

    private var streamSource = StreamSource.CANVAS_STREAM_NATIVE
    private val directImageHandler = Handler(Looper.getMainLooper())
    private var resolution = Size(640, 480)
    private var surfaceTexture: SurfaceTexture? = null
    private var imageReader: ImageReader? = null
    private var frameNumber = 0L
    private var canvasSurface: Surface? = null
    private var filamentTexture: Texture? = null
    private var filamentStream: Stream? = null
    var uvOffset = 0.0f
        private set

    private val kGradientSpeed = 5
    private val kGradientCount = 5

    private val kGradientColors = intArrayOf(
            Color.RED, Color.RED,
            Color.WHITE, Color.WHITE,
            Color.GREEN, Color.GREEN,
            Color.WHITE, Color.WHITE,
            Color.BLUE, Color.BLUE)

    private val kGradientStops = floatArrayOf(
            0.0f, 0.1f,
            0.1f, 0.5f,
            0.5f, 0.6f,
            0.6f, 0.9f,
            0.9f, 1.0f)

    // This seems a little high, but lower values cause occasional "client tried to acquire more than maxImages buffers" on a Pixel 3.
    private val kImageReaderMaxImages = 7

    init {
        startTest()
    }

    fun repaintCanvas() {
        val kGradientScale = resolution.width.toFloat() / kGradientCount
        val kGradientOffset = (frameNumber.toFloat() * kGradientSpeed) % resolution.width
        val surface = canvasSurface
        if (surface != null) {
            val canvas = surface.lockCanvas(null)

            val movingPaint = Paint()
            movingPaint.shader = LinearGradient(
                kGradientOffset,
                0.0f,
                kGradientOffset + kGradientScale,
                0.0f,
                kGradientColors,
                kGradientStops,
                Shader.TileMode.REPEAT
            )
            canvas.drawRect(
                Rect(0, resolution.height / 2, resolution.width, resolution.height),
                movingPaint
            )

            val staticPaint = Paint()
            staticPaint.shader = LinearGradient(
                0.0f,
                0.0f,
                kGradientScale,
                0.0f,
                kGradientColors,
                kGradientStops,
                Shader.TileMode.REPEAT
            )
            canvas.drawRect(Rect(0, 0, resolution.width, resolution.height / 2), staticPaint)

            surface.unlockCanvasAndPost(canvas)

            if (streamSource == StreamSource.CANVAS_STREAM_ACQUIRED) {
                val image = imageReader!!.acquireLatestImage()
                filamentStream!!.setAcquiredImage(
                    image.hardwareBuffer!!,
                    directImageHandler
                ) { image.close() }
            }
        }

        frameNumber++
        uvOffset = 1.0f - kGradientOffset / resolution.width.toFloat()
    }

    fun nextTest() {
        stopTest()
        streamSource = StreamSource.values()[(streamSource.ordinal + 1) % StreamSource.values().size]
        startTest()
    }

    fun getTestName(): String {
        return streamSource.name
    }

    private fun startTest() {

        // Create the Filament Texture and Sampler objects.
        filamentTexture = Texture.Builder()
                .sampler(Texture.Sampler.SAMPLER_EXTERNAL)
                .format(Texture.InternalFormat.RGB8)
                .build(filamentEngine)

        val filamentTexture = this.filamentTexture!!

        val sampler = TextureSampler(
            TextureSampler.MinFilter.LINEAR, TextureSampler.MagFilter.LINEAR,
            TextureSampler.WrapMode.REPEAT)

        // We are texturing a front-facing square shape so we need to generate a matrix that transforms (u, v, 0, 1)
        // into a new UV coordinate according to the screen rotation and the aspect ratio of the camera image.
        val aspectRatio = resolution.width.toFloat() / resolution.height.toFloat()
        val textureTransform = FloatArray(16)
        Matrix.setIdentityM(textureTransform, 0)
        when (display.rotation) {
            Surface.ROTATION_0 -> {
                Matrix.translateM(textureTransform, 0, 1.0f, 0.0f, 0.0f)
                Matrix.rotateM(textureTransform, 0, 90.0f, 0.0f, 0.0f, 1.0f)
                Matrix.translateM(textureTransform, 0, 1.0f, 0.0f, 0.0f)
                Matrix.scaleM(textureTransform, 0, -1.0f, 1.0f / aspectRatio, 1.0f)
            }
            Surface.ROTATION_90 -> {
                Matrix.translateM(textureTransform, 0, 1.0f, 1.0f, 0.0f)
                Matrix.rotateM(textureTransform, 0, 180.0f, 0.0f, 0.0f, 1.0f)
                Matrix.translateM(textureTransform, 0, 1.0f, 0.0f, 0.0f)
                Matrix.scaleM(textureTransform, 0, -1.0f / aspectRatio, 1.0f, 1.0f)
            }
            Surface.ROTATION_270 -> {
                Matrix.translateM(textureTransform, 0, 1.0f, 0.0f, 0.0f)
                Matrix.scaleM(textureTransform, 0, -1.0f / aspectRatio, 1.0f, 1.0f)
            }
        }

        // Connect the Stream to the Texture and the Texture to the MaterialInstance.
        filamentMaterial.setParameter("videoTexture", filamentTexture, sampler)
        filamentMaterial.setParameter("textureTransform",
            MaterialInstance.FloatElement.MAT4, textureTransform, 0, 1)

        if (streamSource == StreamSource.CANVAS_STREAM_NATIVE) {

            // Create the Android surface that will hold the canvas image.
            surfaceTexture = SurfaceTexture(0)
            surfaceTexture!!.setDefaultBufferSize(resolution.width, resolution.height)
            surfaceTexture!!.detachFromGLContext()
            canvasSurface = Surface(surfaceTexture)

            // Create the Filament Stream object that gets bound to the Texture.
            filamentStream = Stream.Builder()
                    .stream(surfaceTexture!!)
                    .build(filamentEngine)

            filamentTexture.setExternalStream(filamentEngine, filamentStream!!)
        }

        if (streamSource == StreamSource.CANVAS_STREAM_ACQUIRED) {
            filamentStream = Stream.Builder()
                    .width(resolution.width)
                    .height(resolution.height)
                    .build(filamentEngine)

            filamentTexture.setExternalStream(filamentEngine, filamentStream!!)

            this.imageReader = ImageReader.newInstance(
                resolution.width, resolution.height, ImageFormat.RGB_565, kImageReaderMaxImages
            ).apply {
                canvasSurface = surface
            }
        }

        // Draw the first frame now.
        frameNumber = 0
        repaintCanvas()
    }

    private fun stopTest() {
        filamentTexture?.let { filamentEngine.destroyTexture(it) }
        filamentStream?.let { filamentEngine.destroyStream(it) }
        surfaceTexture?.release()
    }
}
