/*
 * Copyright (C) 2020 The Android Open Source Project
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

package com.google.android.filament.gltf

import android.annotation.SuppressLint
import android.app.Activity
import android.os.Bundle
import android.view.Choreographer
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.SurfaceView
import com.google.android.filament.utils.KtxLoader
import com.google.android.filament.utils.ModelViewer
import com.google.android.filament.utils.Utils
import java.nio.ByteBuffer
import java.nio.channels.Channels


class MainActivity : Activity() {

    companion object {
        // Load the library for the utility layer,  which includes gltfio and the Filament core.
        init { Utils.init() }
    }

    private lateinit var surfaceView: SurfaceView
    private lateinit var choreographer: Choreographer
    private val frameScheduler = FrameCallback()
    private lateinit var modelViewer: ModelViewer
    private val doubleTapListener = DoubleTapListener()
    private lateinit var doubleTapDetector: GestureDetector

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        surfaceView = SurfaceView(this).apply { setContentView(this) }
        choreographer = Choreographer.getInstance()

        doubleTapDetector = GestureDetector(applicationContext, doubleTapListener)

        modelViewer = ModelViewer(surfaceView)

        surfaceView.setOnTouchListener { _, event ->
            modelViewer.onTouchEvent(event)
            doubleTapDetector.onTouchEvent(event)
            true
        }

        createRenderables()
        createIndirectLight()
    }

    private fun createRenderables() {
        val buffer = assets.open("models/scene.gltf").use { input ->
            val bytes = ByteArray(input.available())
            input.read(bytes)
            ByteBuffer.wrap(bytes)
        }
        modelViewer.loadModelGltf(buffer) { uri ->
            readUncompressedAsset("models/$uri")
        }
        modelViewer.transformToUnitCube()
    }

    private fun createIndirectLight() {
        val engine = modelViewer.engine
        val scene = modelViewer.scene
        val ibl = "venetian_crossroads_2k"
        readUncompressedAsset("envs/$ibl/${ibl}_ibl.ktx").let {
            scene.indirectLight = KtxLoader.createIndirectLight(engine, it, KtxLoader.Options())
            scene.indirectLight!!.intensity = 50_000.0f
        }
        readUncompressedAsset("envs/$ibl/${ibl}_skybox.ktx").let {
            scene.skybox = KtxLoader.createSkybox(engine, it, KtxLoader.Options())
        }
    }

    private fun readUncompressedAsset(assetName: String): ByteBuffer {
        assets.openFd(assetName).use { fd ->
            val input = fd.createInputStream()
            val dst = ByteBuffer.allocate(fd.length.toInt())
            val src = Channels.newChannel(input)
            src.read(dst)
            src.close()
            return dst.apply { rewind() }
        }
    }

    override fun onResume() {
        super.onResume()
        choreographer.postFrameCallback(frameScheduler)
    }

    override fun onPause() {
        super.onPause()
        choreographer.removeFrameCallback(frameScheduler)
    }

    override fun onDestroy() {
        super.onDestroy()
        choreographer.removeFrameCallback(frameScheduler)
        modelViewer.scene.skybox?.let { modelViewer.engine.destroySkybox(it) }
        modelViewer.scene.indirectLight?.let { modelViewer.engine.destroyIndirectLight(it) }
        modelViewer.detach()
    }

    inner class FrameCallback : Choreographer.FrameCallback {
        private val startTime = System.nanoTime()
        override fun doFrame(frameTimeNanos: Long) {
            choreographer.postFrameCallback(this)

            modelViewer.animator?.apply {
                val elapsedTimeSeconds = (frameTimeNanos - startTime).toDouble() / 1_000_000_000
                this.applyAnimation(0, elapsedTimeSeconds.toFloat())
                this.updateBoneMatrices()
            }

            modelViewer.render()
        }
    }

    // Just for testing purposes, this releases the model and reloads it.
    inner class DoubleTapListener : GestureDetector.SimpleOnGestureListener() {
        override fun onDoubleTap(e: MotionEvent?): Boolean {
            modelViewer.destroyModel()
            createRenderables()
            return super.onDoubleTap(e)
        }
    }
}
