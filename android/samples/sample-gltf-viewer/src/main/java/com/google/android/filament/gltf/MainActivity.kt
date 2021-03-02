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
import android.os.Bundle
import android.util.Log
import android.util.Size
import android.view.*
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import androidx.lifecycle.LifecycleOwner
import com.google.android.filament.utils.KtxLoader
import com.google.android.filament.utils.ModelViewer
import com.google.android.filament.utils.Utils
import com.google.common.util.concurrent.ListenableFuture
import com.google.mlkit.vision.barcode.Barcode
import com.google.mlkit.vision.barcode.BarcodeScannerOptions
import com.google.mlkit.vision.barcode.BarcodeScanning
import com.google.mlkit.vision.common.InputImage
import kotlinx.coroutines.*
import java.io.IOException
import java.net.HttpURLConnection
import java.net.URL
import java.nio.Buffer
import java.nio.ByteBuffer
import java.util.concurrent.Executors
import java.util.zip.ZipInputStream

class MainActivity : AppCompatActivity() {

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
    private lateinit var previewView: androidx.camera.view.PreviewView
    private lateinit var cameraProviderFuture : ListenableFuture<ProcessCameraProvider>

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.layout)

        setSupportActionBar(findViewById(R.id.toolbar))

        // The app has two modes, 3D viewer mode and QR code scanning mode.
        // To toggle quickly between the two modes, we have two Views and collapse
        // one or the other within the linear layout.
        surfaceView = findViewById(R.id.surfaceView)
        previewView = findViewById(R.id.previewView)
        surfaceView.visibility = View.VISIBLE
        previewView.visibility = View.GONE

        cameraProviderFuture = ProcessCameraProvider.getInstance(this)

        cameraProviderFuture.addListener({
            val cameraProvider = cameraProviderFuture.get()
            bindPreview(cameraProvider)
        }, ContextCompat.getMainExecutor(this))

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

        val dynamicResolutionOptions = modelViewer.view.dynamicResolutionOptions
        dynamicResolutionOptions.enabled = true
        modelViewer.view.dynamicResolutionOptions = dynamicResolutionOptions

        val ssaoOptions = modelViewer.view.ambientOcclusionOptions
        ssaoOptions.enabled = true
        modelViewer.view.ambientOcclusionOptions = ssaoOptions

        val bloomOptions = modelViewer.view.bloomOptions
        bloomOptions.enabled = true
        modelViewer.view.bloomOptions = bloomOptions
    }

    private fun bindPreview(cameraProvider: ProcessCameraProvider) {
        val preview = Preview.Builder().build().also {
            it.setSurfaceProvider(previewView.createSurfaceProvider())
        }

        val cameraSelector = CameraSelector.Builder()
                .requireLensFacing(CameraSelector.LENS_FACING_BACK)
                .build()

        val imageAnalysis = ImageAnalysis.Builder()
                .setTargetResolution(Size(1280, 720))
                .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                .build()

        imageAnalysis.setAnalyzer(Executors.newSingleThreadExecutor(), { imageProxy ->
            scanQRCode(imageProxy)
        })

        try {
            cameraProvider.unbindAll()
            cameraProvider.bindToLifecycle(this as LifecycleOwner, cameraSelector, imageAnalysis, preview)
        } catch (exc: Exception) {
            Log.e(TAG, "Preview binding failed", exc)
        }
    }

    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem) = when (item.itemId) {
        R.id.action_qrcode -> {
            if (previewView.visibility == View.GONE) {
                surfaceView.visibility = View.GONE
                previewView.visibility = View.VISIBLE
            } else {
                surfaceView.visibility = View.VISIBLE
                previewView.visibility = View.GONE
            }
            true
        }
        else -> {
            super.onOptionsItemSelected(item)
        }
    }

    @SuppressLint("UnsafeExperimentalUsageError")
    private fun scanQRCode(imageProxy: ImageProxy) {
        val mediaImage = imageProxy.image ?: return
        if (previewView.visibility != View.VISIBLE) {
            imageProxy.close()
            return
        }
        val options = BarcodeScannerOptions.Builder().setBarcodeFormats(Barcode.FORMAT_QR_CODE).build()
        val image = InputImage.fromMediaImage(mediaImage, imageProxy.imageInfo.rotationDegrees)
        val scanner = BarcodeScanning.getClient(options)
        scanner.process(image)
                .addOnSuccessListener { barcodes ->
                    if (barcodes.size > 0) {
                        downloadFromBarcode(barcodes[0])
                    }
                    imageProxy.close()
                }
                .addOnFailureListener {
                    imageProxy.close()
                }
    }

    private fun setStatusText(text: String) {
        runOnUiThread {
            val toast = Toast.makeText(applicationContext, text, Toast.LENGTH_SHORT)
            toast.show()
        }
    }

    private fun downloadFromBarcode(barcode: Barcode) {
        if (barcode.valueType != Barcode.TYPE_URL) return
        val url = barcode.url!!.url!!

        // At this point we successfully scanned a QR Code with a well-formed URL, so we go back to
        // the 3D view immediately, without waiting to see if we can successfully fetch anything
        // from the URL. This lets the user know that we detected a QR code and prevents a stream
        // of repeated fetch requests.
        surfaceView.visibility = View.VISIBLE
        previewView.visibility = View.GONE

        setStatusText("Downloading model data...")
        Log.i(TAG, "Fetching URL $url")

        CoroutineScope(Dispatchers.IO).launch {
            try {
                if (url.endsWith("-zip")) {
                    downloadZip(url)
                } else {
                    downloadGlb(url)
                }
            } catch (exc: IOException) {
                setStatusText("URL fetch failed.")
                Log.e(TAG, "URL fetch failed", exc)
            }
        }
    }

    private suspend fun downloadGlb(url: String) {
        val buffer = withContext(Dispatchers.IO) {
            val connection = URL(url).openConnection() as HttpURLConnection
            connection.readTimeout = 2000 // two second timeout
            val reader = connection.inputStream
            val bytes = reader.readBytes()
            val buffer = ByteBuffer.wrap(bytes)
            setStatusText("Received model data.")
            buffer
        }
        withContext(Dispatchers.Main) {
            modelViewer.destroyModel()
            modelViewer.loadModelGlb(buffer)
            modelViewer.transformToUnitCube()
        }
    }

    private suspend fun downloadZip(url: String) {
        var gltfPath: String? = null

        val pathToBufferMapping = withContext(Dispatchers.IO) {
            val connection = URL(url).openConnection() as HttpURLConnection
            connection.readTimeout = 2000 // two second timeout
            val reader = connection.inputStream
            val deflater = ZipInputStream(reader)
            val mapping = HashMap<String, Buffer>()
            while (true) {
                val entry = deflater.nextEntry ?: break
                if (entry.isDirectory) continue
                if (entry.name.startsWith("__MACOSX")) continue
                val uri = entry.name
                setStatusText("Deflating $uri")
                Log.i(TAG, "Deflating zip entry $uri...")
                val byteArray = deflater.readBytes()
                Log.i(TAG, "Deflated ${byteArray.size} bytes")
                val buffer = ByteBuffer.wrap(byteArray)
                mapping[uri] = buffer
                if (uri.endsWith(".gltf")) {
                    gltfPath = uri
                }
            }
            mapping
        }

        if (gltfPath == null) {
            setStatusText("Could not find .gltf in the zip.")
            return
        }

        setStatusText("Received all model data.")

        val gltfBuffer = pathToBufferMapping[gltfPath]!!

        // The gltf is often not at the root level (e.g. if a folder is zipped) so
        // we need to extract its path in order to resolve the embedded uri strings.
        var gltfPrefix = gltfPath!!.substringBeforeLast('/', "")
        if (gltfPrefix.isNotEmpty()) {
            gltfPrefix += "/"
        }

        withContext(Dispatchers.Main) {
            modelViewer.destroyModel()
            modelViewer.loadModelGltfAsync(gltfBuffer) { uri ->
                val path = gltfPrefix + uri
                Log.i(TAG, "glTF resource: uri = $uri, path = $path")
                if (!pathToBufferMapping.contains(path)) {
                    Log.e(TAG, "Could not find $path in the zip.")
                }
                pathToBufferMapping[path]!!
            }
            modelViewer.transformToUnitCube()
        }
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
