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
import android.util.Log
import android.view.*
import android.widget.Toast
import com.google.android.filament.utils.KtxLoader
import com.google.android.filament.utils.ModelViewer
import com.google.android.filament.utils.RemoteServer
import com.google.android.filament.utils.Utils
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.ByteArrayInputStream
import java.io.IOException
import java.nio.Buffer
import java.nio.ByteBuffer
import java.util.zip.ZipInputStream

class MainActivity : Activity() {

    companion object {
        // Load the library for the utility layer, which in turn loads gltfio and the Filament core.
        init { Utils.init() }
        private const val TAG = "gltf-viewer"
    }

    private lateinit var surfaceView: SurfaceView
    private lateinit var choreographer: Choreographer
    private val frameScheduler = FrameCallback()
    private lateinit var modelViewer: ModelViewer
    private val doubleTapListener = DoubleTapListener()
    private lateinit var doubleTapDetector: GestureDetector
    private var remoteServer: RemoteServer? = null

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.simple_layout)

        surfaceView = findViewById(R.id.main_sv)
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

        val msg = "To load a new model, go to the above URL on your host machine."
        Toast.makeText(applicationContext, msg, Toast.LENGTH_LONG).show()

        val dynamicResolutionOptions = modelViewer.view.dynamicResolutionOptions
        dynamicResolutionOptions.enabled = true
        modelViewer.view.dynamicResolutionOptions = dynamicResolutionOptions

        val ssaoOptions = modelViewer.view.ambientOcclusionOptions
        ssaoOptions.enabled = true
        modelViewer.view.ambientOcclusionOptions = ssaoOptions

        val bloomOptions = modelViewer.view.bloomOptions
        bloomOptions.enabled = true
        modelViewer.view.bloomOptions = bloomOptions

        remoteServer = RemoteServer(8082)
    }

    private fun createRenderables() {
        val buffer = assets.open("models/scene.gltf").use { input ->
            val bytes = ByteArray(input.available())
            input.read(bytes)
            ByteBuffer.wrap(bytes)
        }

        modelViewer.loadModelGltfAsync(buffer) { uri -> readCompressedAsset("models/$uri") }
        modelViewer.transformToUnitCube()
    }

    private fun createIndirectLight() {
        val engine = modelViewer.engine
        val scene = modelViewer.scene
        val ibl = "default_env"
        readCompressedAsset("envs/$ibl/${ibl}_ibl.ktx").let {
            scene.indirectLight = KtxLoader.createIndirectLight(engine, it)
            scene.indirectLight!!.intensity = 30_000.0f
        }
        readCompressedAsset("envs/$ibl/${ibl}_skybox.ktx").let {
            scene.skybox = KtxLoader.createSkybox(engine, it)
        }
    }

    private fun readCompressedAsset(assetName: String): ByteBuffer {
        val input = assets.open(assetName)
        val bytes = ByteArray(input.available())
        input.read(bytes)
        return ByteBuffer.wrap(bytes)
    }

    private fun setStatusText(text: String) {
        runOnUiThread {
            Toast.makeText(applicationContext, text, Toast.LENGTH_SHORT).show()
        }
    }

    private suspend fun loadGlb(message: RemoteServer.IncomingMessage) {
        withContext(Dispatchers.Main) {
            modelViewer.destroyModel()
            modelViewer.loadModelGlb(message.buffer)
            modelViewer.transformToUnitCube()
        }
    }

    private suspend fun loadZip(message: RemoteServer.IncomingMessage) {
        val zipfileBytes = ByteArray(message.buffer.remaining())
        message.buffer.get(zipfileBytes)

        var gltfPath: String? = null
        val pathToBufferMapping = withContext(Dispatchers.IO) {
            val deflater = ZipInputStream(ByteArrayInputStream(zipfileBytes))
            val mapping = HashMap<String, Buffer>()
            while (true) {
                val entry = deflater.nextEntry ?: break
                if (entry.isDirectory) continue
                if (entry.name.startsWith("__MACOSX")) continue
                val uri = entry.name
                val byteArray = deflater.readBytes()
                Log.i(TAG, "Deflated ${byteArray.size} bytes from $uri")
                val buffer = ByteBuffer.wrap(byteArray)
                mapping[uri] = buffer
                if (uri.endsWith(".gltf")) {
                    gltfPath = uri
                }
            }
            mapping
        }

        if (gltfPath == null) {
            setStatusText( "Could not find .gltf in the zip.")
            return
        }

        setStatusText( "Received all model data.")

        val gltfBuffer = pathToBufferMapping[gltfPath]!!

        // The gltf is often not at the root level (e.g. if a folder is zipped) so
        // we need to extract its path in order to resolve the embedded uri strings.
        var gltfPrefix = gltfPath!!.substringBeforeLast('/', "")
        if (gltfPrefix.isNotEmpty()) {
            gltfPrefix += "/"
        }

        withContext(Dispatchers.Main) {
            modelViewer.destroyModel()
            modelViewer.loadModelGltf(gltfBuffer) { uri ->
                val path = gltfPrefix + uri
                if (!pathToBufferMapping.contains(path)) {
                    Log.e(TAG, "Could not find $path in the zip.")
                }
                pathToBufferMapping[path]!!
            }
            modelViewer.transformToUnitCube()
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
    }

    inner class FrameCallback : Choreographer.FrameCallback {
        private val startTime = System.nanoTime()
        override fun doFrame(frameTimeNanos: Long) {
            choreographer.postFrameCallback(this)

            modelViewer.animator?.apply {
                if (animationCount > 0) {
                    val elapsedTimeSeconds = (frameTimeNanos - startTime).toDouble() / 1_000_000_000
                    applyAnimation(0, elapsedTimeSeconds.toFloat())
                }
                updateBoneMatrices()
            }

            modelViewer.render(frameTimeNanos)

            val message = remoteServer?.acquireIncomingMessage()
            if (message != null) {
                Log.i("Filament", "Downloaded ${message.label} ${message.buffer.capacity()}")

                CoroutineScope(Dispatchers.IO).launch {
                    try {
                        if (message.label.endsWith(".zip")) {
                            loadZip(message)
                        } else {
                            loadGlb(message)
                        }
                    } catch (exc: IOException) {
                        setStatusText( "URL fetch failed.")
                        Log.e(TAG, "URL fetch failed", exc)
                    }
                }

            }
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
