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
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.Choreographer
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.SurfaceView
import android.view.WindowManager
import android.widget.TextView
import android.widget.Toast
import com.google.android.filament.Fence
import com.google.android.filament.IndirectLight
import com.google.android.filament.Material
import com.google.android.filament.Skybox
import com.google.android.filament.SwapChain
import com.google.android.filament.View
import com.google.android.filament.android.ChoreographerHelper
import com.google.android.filament.utils.AutomationEngine
import com.google.android.filament.utils.HDRLoader
import com.google.android.filament.utils.IBLPrefilterContext
import com.google.android.filament.utils.KTX1Loader
import com.google.android.filament.utils.ModelViewer
import com.google.android.filament.utils.RemoteServer
import com.google.android.filament.utils.Utils
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.currentCoroutineContext
import kotlinx.coroutines.ensureActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.io.File
import java.io.FileInputStream
import java.io.FileNotFoundException
import java.io.RandomAccessFile
import java.net.URI
import java.nio.Buffer
import java.nio.ByteBuffer
import java.nio.channels.FileChannel
import java.nio.charset.StandardCharsets
import java.util.zip.ZipInputStream

class MainActivity : Activity() {

    companion object {
        // Load the library for the utility layer, which in turn loads gltfio and the Filament core.
        init { Utils.init() }
        private const val TAG = "gltf-viewer"

        // Loads a .glb or .gltf from the device filesystem. Relative paths are resolved against
        // the app-specific external files directory, which adb can write to without root:
        //
        //   adb push model.glb /sdcard/Android/data/com.google.android.filament.gltf/files/
        //   adb shell am start -n com.google.android.filament.gltf/.MainActivity \
        //       --es model-path model.glb
        //
        // The activity is singleTop, so issuing the command again while the app is in the
        // foreground delivers the new path through onNewIntent and replaces the current model.
        //
        // The app creates the directory on launch. Before the first launch, push to an explicit
        // file name (e.g. `.../files/model.glb`) because adb will not push into a missing
        // directory that is specified with a trailing slash.
        private const val MODEL_PATH_TAG = "model-path"
    }

    private lateinit var surfaceView: SurfaceView
    private val frameScheduler = FrameCallback()
    private lateinit var modelViewer: ModelViewer
    private lateinit var titlebarHint: TextView
    private val doubleTapListener = DoubleTapListener()
    private val singleTapListener = SingleTapListener()
    private lateinit var doubleTapDetector: GestureDetector
    private lateinit var singleTapDetector: GestureDetector
    private var remoteServer: RemoteServer? = null
    private var statusToast: Toast? = null
    private var statusText: String? = null
    private var latestDownload: String? = null
    private val automation = AutomationEngine()
    private var loadStartTime = 0L
    private var loadStartFence: Fence? = null
    private val viewerContent = AutomationEngine.ViewerContent()
    private var currentFrameRate = -1.0f
    private val activityScope = CoroutineScope(SupervisorJob() + Dispatchers.Main)
    private var fileLoadJob: Job? = null

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.simple_layout)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        titlebarHint = findViewById(R.id.user_hint)
        surfaceView = findViewById(R.id.main_sv)

        doubleTapDetector = GestureDetector(applicationContext, doubleTapListener)
        singleTapDetector = GestureDetector(applicationContext, singleTapListener)

        modelViewer = ModelViewer(surfaceView)
        viewerContent.view = modelViewer.view
        viewerContent.sunlight = modelViewer.light
        viewerContent.lightManager = modelViewer.engine.lightManager
        viewerContent.scene = modelViewer.scene
        viewerContent.renderer = modelViewer.renderer

        surfaceView.setOnTouchListener { _, event ->
            modelViewer.onTouchEvent(event)
            doubleTapDetector.onTouchEvent(event)
            singleTapDetector.onTouchEvent(event)
            true
        }

        // Creating the app-specific external directory up front gives `adb push` a target.
        Log.i(TAG, "Models can be pushed to ${getExternalFilesDir(null)}")

        val initialModelPath = intent.getStringExtra(MODEL_PATH_TAG)
        if (initialModelPath != null) {
            loadModelFromPath(initialModelPath)
        } else {
            createDefaultRenderables()
        }
        createIndirectLight()

        setStatusText("To load a new model, go to the above URL on your host machine.")

        val view = modelViewer.view

        /*
         * Note: The settings below are overridden when connecting to the remote UI.
         */

        // on mobile, better use lower quality color buffer
        view.renderQuality = view.renderQuality.apply {
            hdrColorBuffer = View.QualityLevel.MEDIUM
        }

        // dynamic resolution often helps a lot
        view.dynamicResolutionOptions = view.dynamicResolutionOptions.apply {
            enabled = true
            quality = View.QualityLevel.MEDIUM
        }

        // MSAA is needed with dynamic resolution MEDIUM
        view.multiSampleAntiAliasingOptions = view.multiSampleAntiAliasingOptions.apply {
            enabled = true
        }

        // FXAA is pretty cheap and helps a lot
        view.antiAliasing = View.AntiAliasing.FXAA

        // ambient occlusion is the cheapest effect that adds a lot of quality
        view.ambientOcclusionOptions = view.ambientOcclusionOptions.apply {
            enabled = true
        }

        // bloom is pretty expensive but adds a fair amount of realism
        view.bloomOptions = view.bloomOptions.apply {
            enabled = true
        }

        // A previous instance may still hold the port when this activity is recreated (for
        // example via `am start --activity-clear-task`), because Android creates the new
        // instance before destroying the old one. Continue without the remote UI in that case.
        remoteServer = try {
            RemoteServer(8082)
        } catch (e: IllegalStateException) {
            Log.w(TAG, "Remote UI is unavailable: ${e.message}")
            null
        }
    }

    private fun createDefaultRenderables() {
        val buffer = assets.open("models/scene.gltf").use { input ->
            val bytes = ByteArray(input.available())
            input.read(bytes)
            ByteBuffer.wrap(bytes)
        }

        modelViewer.loadModelGltfAsync(buffer) { uri -> readCompressedAsset("models/$uri") }
        updateRootTransform()
    }

    private fun createIndirectLight() {
        val engine = modelViewer.engine
        val scene = modelViewer.scene
        val ibl = "default_env"
        readCompressedAsset("envs/$ibl/${ibl}_ibl.ktx").let {
            val bundle = KTX1Loader.createIndirectLight(engine, it)
            scene.indirectLight = bundle.indirectLight
            modelViewer.indirectLightCubemap = bundle.cubemap
            scene.indirectLight!!.intensity = 30_000.0f
            viewerContent.indirectLight = modelViewer.scene.indirectLight
        }
        readCompressedAsset("envs/$ibl/${ibl}_skybox.ktx").let {
            val bundle = KTX1Loader.createSkybox(engine, it)
            scene.skybox = bundle.skybox
            modelViewer.skyboxCubemap = bundle.cubemap
        }
    }

    private fun readCompressedAsset(assetName: String): ByteBuffer {
        val input = assets.open(assetName)
        val bytes = ByteArray(input.available())
        input.read(bytes)
        return ByteBuffer.wrap(bytes)
    }

    private fun clearStatusText() {
        statusToast?.let {
            it.cancel()
            statusText = null
        }
    }

    private fun setStatusText(text: String) {
        runOnUiThread {
            if (statusToast == null || statusText != text) {
                statusText = text
                statusToast = Toast.makeText(applicationContext, text, Toast.LENGTH_SHORT)
                statusToast!!.show()

            }
        }
    }

    private fun resolveModelFile(path: String): File {
        val file = File(path)
        return if (file.isAbsolute) file else File(getExternalFilesDir(null), path)
    }

    /**
     * Replaces the current model with a .glb or .gltf read from the device filesystem.
     *
     * The current model is released before the new file is read in order to lower peak memory
     * usage. File IO runs on a worker thread, while all Filament calls run on the main thread.
     */
    private fun loadModelFromPath(path: String) {
        val file = resolveModelFile(path)
        val isGlb = file.name.endsWith(".glb", ignoreCase = true)
        val isGltf = file.name.endsWith(".gltf", ignoreCase = true)
        if (!isGlb && !isGltf) {
            Log.e(TAG, "Unsupported model type: ${file.path}")
            setStatusText("Only .glb and .gltf files are supported.")
            return
        }
        if (!file.canRead()) {
            Log.e(TAG, "Cannot read ${file.path}")
            setStatusText("Cannot read ${file.path}")
            return
        }

        // Cancel any in-flight file load so that a stale result never replaces this one.
        fileLoadJob?.cancel()
        unloadModel()
        clearStatusText()
        titlebarHint.text = file.name
        Log.i(TAG, "Loading model from ${file.path}")

        fileLoadJob = activityScope.launch {
            try {
                if (isGlb) {
                    val buffer = withContext(Dispatchers.IO) { mapFile(file) }
                    // createAsset() copies the GLB, so the mapping is not used after this call.
                    modelViewer.loadModelGlb(buffer)
                } else {
                    val (gltf, resources) = withContext(Dispatchers.IO) {
                        readGltfWithResources(file)
                    }
                    modelViewer.loadModelGltf(gltf) { uri ->
                        val resource = resources[uri]
                        if (resource == null) {
                            Log.e(TAG, "Resource '$uri' was not found next to ${file.name}")
                        }
                        resource
                    }
                }
                if (modelViewer.asset == null) {
                    setStatusText("Could not load ${file.name}")
                    return@launch
                }
                updateRootTransform()
                beginLoadTiming()
            } catch (e: CancellationException) {
                throw e
            } catch (e: Exception) {
                Log.e(TAG, "Failed to load ${file.path}", e)
                setStatusText("Failed to load ${file.name}: ${e.message}")
            } catch (e: OutOfMemoryError) {
                Log.e(TAG, "Out of memory while loading ${file.path}", e)
                setStatusText("Out of memory while loading ${file.name}")
            }
        }
    }

    /**
     * Reads a JSON-style glTF along with every external buffer and image that it references.
     * Resource paths are resolved relative to the directory that contains the glTF file.
     */
    private suspend fun readGltfWithResources(file: File): Pair<Buffer, Map<String, Buffer>> {
        val bytes = file.readBytes()
        val json = JSONObject(String(bytes, StandardCharsets.UTF_8))
        val baseDir = file.absoluteFile.parentFile
        val resources = HashMap<String, Buffer>()
        for (key in arrayOf("buffers", "images")) {
            val entries = json.optJSONArray(key) ?: continue
            for (i in 0 until entries.length()) {
                val uri = entries.optJSONObject(i)?.optString("uri", "") ?: ""
                if (uri.isEmpty() || uri.startsWith("data:") || resources.containsKey(uri)) {
                    continue
                }
                currentCoroutineContext().ensureActive()
                val resourceFile = File(baseDir, Uri.decode(uri))
                if (!resourceFile.canRead()) {
                    throw FileNotFoundException("missing resource '$uri'")
                }
                resources[uri] = readFileDirect(resourceFile)
            }
        }
        return Pair(ByteBuffer.wrap(bytes), resources)
    }

    // Memory-mapped pages are backed by the file, so they do not count against the Java heap.
    private fun mapFile(file: File): ByteBuffer =
        FileInputStream(file).channel.use { channel ->
            channel.map(FileChannel.MapMode.READ_ONLY, 0, channel.size())
        }

    // Resources are copied rather than mapped because the ResourceLoader holds on to them until
    // the model is destroyed, and a mapped file that is overwritten by `adb push` in the meantime
    // would cause a SIGBUS when the loader reads from it.
    private fun readFileDirect(file: File): ByteBuffer =
        FileInputStream(file).channel.use { channel ->
            val size = channel.size()
            require(size <= Int.MAX_VALUE) { "${file.name} is too large" }
            val buffer = ByteBuffer.allocateDirect(size.toInt())
            while (buffer.hasRemaining()) {
                if (channel.read(buffer) < 0) break
            }
            buffer.flip()
            buffer
        }

    /**
     * Destroys the current model along with any pending load-timing fence.
     */
    private fun unloadModel() {
        loadStartFence?.let {
            modelViewer.engine.destroyFence(it)
            loadStartFence = null
        }
        modelViewer.destroyModel()
    }

    private fun beginLoadTiming() {
        loadStartFence?.let { modelViewer.engine.destroyFence(it) }
        loadStartTime = System.nanoTime()
        loadStartFence = modelViewer.engine.createFence()
    }

    private suspend fun loadGlb(message: RemoteServer.ReceivedMessage) {
        withContext(Dispatchers.Main) {
            unloadModel()
            modelViewer.loadModelGlb(message.buffer)
            updateRootTransform()
            beginLoadTiming()
        }
    }

    private suspend fun loadHdr(message: RemoteServer.ReceivedMessage) {
        withContext(Dispatchers.Main) {
            val engine = modelViewer.engine
            val equirect = HDRLoader.createTexture(engine, message.buffer)
            if (equirect == null) {
                setStatusText("Could not decode HDR file.")
            } else {
                setStatusText("Successfully decoded HDR file.")

                val context = IBLPrefilterContext(engine)
                val equirectToCubemap = IBLPrefilterContext.EquirectangularToCubemap(context)
                val skyboxTexture = equirectToCubemap.run(equirect)!!
                engine.destroyTexture(equirect)

                val specularFilter = IBLPrefilterContext.SpecularFilter(context)
                val reflections = specularFilter.run(skyboxTexture)

                val ibl = IndirectLight.Builder()
                         .reflections(reflections)
                         .intensity(30000.0f)
                         .build(engine)

                val sky = Skybox.Builder().environment(skyboxTexture).build(engine)

                specularFilter.destroy()
                equirectToCubemap.destroy()
                context.destroy()

                // destroy the previous IBl
                engine.destroyIndirectLight(modelViewer.scene.indirectLight!!)
                engine.destroySkybox(modelViewer.scene.skybox!!)

                modelViewer.scene.skybox = sky
                modelViewer.scene.indirectLight = ibl
                viewerContent.indirectLight = ibl

            }
        }
    }

    private suspend fun loadZip(message: RemoteServer.ReceivedMessage) {
        // To alleviate memory pressure, remove the old model before deflating the zip.
        withContext(Dispatchers.Main) {
            unloadModel()
        }

        // Large zip files should first be written to a file to prevent OOM.
        // It is also crucial that we null out the message "buffer" field.
        val (zipStream, zipFile) = withContext(Dispatchers.IO) {
            val file = File.createTempFile("incoming", "zip", cacheDir)
            val raf = RandomAccessFile(file, "rw")
            raf.channel.write(message.buffer)
            message.buffer = null
            raf.seek(0)
            Pair(FileInputStream(file), file)
        }

        // Deflate each resource using the IO dispatcher, one by one.
        var gltfPath: String? = null
        var outOfMemory: String? = null
        val pathToBufferMapping = withContext(Dispatchers.IO) {
            val deflater = ZipInputStream(zipStream)
            val mapping = HashMap<String, Buffer>()
            while (true) {
                val entry = deflater.nextEntry ?: break
                if (entry.isDirectory) continue

                // This isn't strictly required, but as an optimization
                // we ignore common junk that often pollutes ZIP files.
                if (entry.name.startsWith("__MACOSX")) continue
                if (entry.name.startsWith(".DS_Store")) continue

                val uri = entry.name
                val byteArray: ByteArray? = try {
                    deflater.readBytes()
                }
                catch (_: OutOfMemoryError) {
                    outOfMemory = uri
                    break
                }
                Log.i(TAG, "Deflated ${byteArray!!.size} bytes from $uri")
                val buffer = ByteBuffer.wrap(byteArray)
                mapping[uri] = buffer
                if (uri.endsWith(".gltf") || uri.endsWith(".glb")) {
                    gltfPath = uri
                }
            }
            mapping
        }

        zipFile.delete()

        if (gltfPath == null) {
            setStatusText("Could not find .gltf or .glb in the zip.")
            return
        }

        if (outOfMemory != null) {
            setStatusText("Out of memory while deflating $outOfMemory")
            return
        }

        val gltfBuffer = pathToBufferMapping[gltfPath]!!

        // In a zip file, the gltf file might be in the same folder as resources, or in a different
        // folder. It is crucial to test against both of these cases. In any case, the resource
        // paths are all specified relative to the location of the gltf file.
        val prefix = URI(gltfPath).resolve(".")

        withContext(Dispatchers.Main) {
            if (gltfPath.endsWith(".glb")) {
                modelViewer.loadModelGlb(gltfBuffer)
            } else {
                modelViewer.loadModelGltf(gltfBuffer) { uri ->
                    val path = prefix.resolve(uri).toString()
                    if (!pathToBufferMapping.contains(path)) {
                        Log.e(TAG, "Could not find '$uri' in zip using prefix '$prefix' and base path '${gltfPath}'")
                        setStatusText("Zip is missing $path")
                    }
                    pathToBufferMapping[path]
                }
            }
            updateRootTransform()
            beginLoadTiming()
        }
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        setIntent(intent)
        intent.getStringExtra(MODEL_PATH_TAG)?.let { loadModelFromPath(it) }
    }

    override fun onResume() {
        super.onResume()
        frameScheduler.post()
    }

    override fun onPause() {
        super.onPause()
        frameScheduler.remove()
    }

    override fun onDestroy() {
        super.onDestroy()
        frameScheduler.remove()
        // Cancel pending loads before the view detaches and ModelViewer destroys the engine.
        activityScope.cancel()
        remoteServer?.close()
    }

    fun loadModelData(message: RemoteServer.ReceivedMessage) {
        Log.i(TAG, "Downloaded model ${message.label} (${message.buffer.capacity()} bytes)")
        clearStatusText()
        titlebarHint.text = message.label
        fileLoadJob?.cancel()
        CoroutineScope(Dispatchers.IO).launch {
            when {
                message.label.endsWith(".zip") -> loadZip(message)
                message.label.endsWith(".hdr") -> loadHdr(message)
                else -> loadGlb(message)
            }
        }
    }

    fun loadSettings(message: RemoteServer.ReceivedMessage) {
        val json = StandardCharsets.UTF_8.decode(message.buffer).toString()
        viewerContent.assetLights = modelViewer.asset?.lightEntities
        automation.applySettings(modelViewer.engine, json, viewerContent)
        modelViewer.view.colorGrading = automation.getColorGrading(modelViewer.engine)
        modelViewer.cameraFocalLength = automation.cameraSettings.focalLength
        modelViewer.cameraNear = automation.cameraSettings.near
        modelViewer.cameraFar = automation.cameraSettings.far
        updateRootTransform()

        val targetFps = automation.viewerOptions.cameraFrameRate
        if (targetFps != currentFrameRate) {
            modelViewer.framePacer.configure(targetFps, 42_000_000L)
            modelViewer.swapChain?.let { sc ->
                if (sc.isFrameRateChangeSupported) {
                    sc.setFrameRate(
                        targetFps,
                        SwapChain.FrameRateCompatibility.DEFAULT,
                        SwapChain.ChangeFrameRateStrategy.ONLY_IF_SEAMLESS
                    )
                }
            }
            currentFrameRate = targetFps
        }
    }

    private fun updateRootTransform() {
        if (automation.viewerOptions.autoScaleEnabled) {
            modelViewer.transformToUnitCube()
        } else {
            modelViewer.clearRootTransform()
        }
    }

    inner class FrameCallback : ChoreographerHelper() {
        override fun onFrame(frameTimeNanos: Long, frameData: Any?) {
            loadStartFence?.let {
                if (it.wait(Fence.Mode.FLUSH, 0) == Fence.FenceStatus.CONDITION_SATISFIED) {
                    val end = System.nanoTime()
                    val total = (end - loadStartTime) / 1_000_000
                    Log.i(TAG, "The Filament backend took $total ms to load the model geometry.")
                    modelViewer.engine.destroyFence(it)
                    loadStartFence = null

                    val materials = mutableSetOf<Material>()
                    val rcm = modelViewer.engine.renderableManager
                    val em = modelViewer.engine.entityManager
                    modelViewer.scene.forEach { entity ->
                        if (em.isAlive(entity) && rcm.hasComponent(entity)) {
                            val ri = rcm.getInstance(entity)
                            val c = rcm.getPrimitiveCount(ri)
                            for (i in 0 until c) {
                                val mi = rcm.getMaterialInstanceAt(ri, i)
                                mi.material?.let { material ->
                                    materials.add(material)
                                }
                            }
                        }
                    }
                    materials.forEach { material ->
                        material.compile(
                            Material.CompilerPriorityQueue.HIGH,
                            Material.UserVariantFilterBit.DIRECTIONAL_LIGHTING or
                            Material.UserVariantFilterBit.DYNAMIC_LIGHTING or
                            Material.UserVariantFilterBit.SHADOW_RECEIVER,
                            null, null)
                        material.compile(
                            Material.CompilerPriorityQueue.LOW,
                            Material.UserVariantFilterBit.FOG or
                            Material.UserVariantFilterBit.SKINNING or
                            Material.UserVariantFilterBit.SSR or
                            Material.UserVariantFilterBit.VSM,
                            null, null)
                    }
                }
            }


            if (Build.VERSION.SDK_INT >= 33 && frameData is Choreographer.FrameData) {
                modelViewer.render(frameData)
            } else {
                modelViewer.render(frameTimeNanos)
            }

        // Check if a new download is in progress. If so, let the user know with toast.
        val currentDownload = remoteServer?.peekIncomingLabel()
        if (RemoteServer.isBinary(currentDownload) && currentDownload != latestDownload) {
            latestDownload = currentDownload
            Log.i(TAG, "Downloading $currentDownload")
            setStatusText("Downloading $currentDownload")
        }

        // Check if a new message has been fully received from the client.
        val message = remoteServer?.acquireReceivedMessage()
        if (message != null) {
            if (message.label == latestDownload) {
                latestDownload = null
            }
            if (RemoteServer.isJson(message.label)) {
                loadSettings(message)
            } else {
                loadModelData(message)
            }
        }
    }
}

    // Just for testing purposes, this releases the current model and reloads the default model.
    inner class DoubleTapListener : GestureDetector.SimpleOnGestureListener() {
        override fun onDoubleTap(e: MotionEvent): Boolean {
            fileLoadJob?.cancel()
            unloadModel()
            createDefaultRenderables()
            return super.onDoubleTap(e)
        }
    }

    // Just for testing purposes
    inner class SingleTapListener : GestureDetector.SimpleOnGestureListener() {
        override fun onSingleTapUp(event: MotionEvent): Boolean {
            modelViewer.view.pick(
                event.x.toInt(),
                surfaceView.height - event.y.toInt(),
                surfaceView.handler
            ) {
                val name = modelViewer.asset!!.getName(it.renderable)
                Log.v("Filament", "Picked ${it.renderable}: " + name)
            }
            return super.onSingleTapUp(event)
        }
    }
}
