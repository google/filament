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

package com.google.android.filament.utils

import android.view.MotionEvent
import android.view.Surface
import android.view.SurfaceView
import android.view.TextureView
import com.google.android.filament.*
import com.google.android.filament.android.DisplayHelper
import com.google.android.filament.android.UiHelper
import com.google.android.filament.gltfio.*
import kotlinx.coroutines.*
import java.nio.Buffer

private const val kNearPlane = 0.05f     // 5 cm
private const val kFarPlane = 1000.0f    // 1 km
private const val kAperture = 16f
private const val kShutterSpeed = 1f / 125f
private const val kSensitivity = 100f

/**
 * Helps render glTF models into a [SurfaceView] or [TextureView] with an orbit controller.
 *
 * `ModelViewer` owns a Filament engine, renderer, swapchain, view, and scene. It allows clients
 * to access these objects via read-only properties. The viewer can display only one glTF scene
 * at a time, which can be scaled and translated into the viewing frustum by calling
 * [transformToUnitCube]. All ECS entities can be accessed and modified via the [asset] property.
 *
 * For GLB files, clients can call [loadModelGlb] and pass in a [Buffer] with the contents of the
 * GLB file. For glTF files, clients can call [loadModelGltf] and pass in a [Buffer] with the JSON
 * contents, as well as a callback for loading external resources.
 *
 * `ModelViewer` reduces much of the boilerplate required for simple Filament applications, but
 * clients still have the responsibility of adding an [IndirectLight] and [Skybox] to the scene.
 * Additionally, clients should:
 *
 * 1. Pass the model viewer into [SurfaceView.setOnTouchListener] or call its [onTouchEvent]
 *    method from your touch handler.
 * 2. Call [render] and [Animator.applyAnimation] from a `Choreographer` frame callback.
 *
 * NOTE: if its associated SurfaceView or TextureView has become detached from its window, the
 * ModelViewer becomes invalid and must be recreated.
 *
 * See `sample-gltf-viewer` for a usage example.
 */
class ModelViewer(
        val engine: Engine,
        private val uiHelper: UiHelper
) : android.view.View.OnTouchListener {
    var asset: FilamentAsset? = null
        private set

    var animator: Animator? = null
        private set

    @Suppress("unused")
    val progress
        get() = resourceLoader.asyncGetLoadProgress()

    var normalizeSkinningWeights = true

    var cameraFocalLength = 28f
        set(value) {
            field = value
            updateCameraProjection()
        }

    var cameraNear = kNearPlane
        set(value) {
            field = value
            updateCameraProjection()
        }

    var cameraFar = kFarPlane
        set(value) {
            field = value
            updateCameraProjection()
        }

    val scene: Scene
    val view: View
    val camera: Camera
    val renderer: Renderer
    @Entity val light: Int

    private lateinit var displayHelper: DisplayHelper
    private lateinit var cameraManipulator: Manipulator
    private lateinit var gestureDetector: GestureDetector
    private var surfaceView: SurfaceView? = null
    private var textureView: TextureView? = null

    private var fetchResourcesJob: Job? = null

    private var swapChain: SwapChain? = null
    private var assetLoader: AssetLoader
    private var materialProvider: MaterialProvider
    private var resourceLoader: ResourceLoader
    private val readyRenderables = IntArray(128) // add up to 128 entities at a time

    private val eyePos = DoubleArray(3)
    private val target = DoubleArray(3)
    private val upward = DoubleArray(3)

    init {
        renderer = engine.createRenderer()
        scene = engine.createScene()
        camera = engine.createCamera(engine.entityManager.create()).apply { setExposure(kAperture, kShutterSpeed, kSensitivity) }
        view = engine.createView()
        view.scene = scene
        view.camera = camera

        materialProvider = UbershaderProvider(engine)
        assetLoader = AssetLoader(engine, materialProvider, EntityManager.get())
        resourceLoader = ResourceLoader(engine, normalizeSkinningWeights)

        // Always add a direct light source since it is required for shadowing.
        // We highly recommend adding an indirect light as well.

        light = EntityManager.get().create()

        val (r, g, b) = Colors.cct(6_500.0f)
        LightManager.Builder(LightManager.Type.DIRECTIONAL)
                .color(r, g, b)
                .intensity(100_000.0f)
                .direction(0.0f, -1.0f, 0.0f)
                .castShadows(true)
                .build(engine, light)

        scene.addEntity(light)
    }

    constructor(
            surfaceView: SurfaceView,
            engine: Engine = Engine.create(),
            uiHelper: UiHelper = UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK),
            manipulator: Manipulator? = null
    ) : this(engine, uiHelper) {
        cameraManipulator = manipulator ?: Manipulator.Builder()
                .targetPosition(kDefaultObjectPosition.x, kDefaultObjectPosition.y, kDefaultObjectPosition.z)
                .viewport(surfaceView.width, surfaceView.height)
                .build(Manipulator.Mode.ORBIT)

        this.surfaceView = surfaceView
        gestureDetector = GestureDetector(surfaceView, cameraManipulator)
        displayHelper = DisplayHelper(surfaceView.context)
        uiHelper.renderCallback = SurfaceCallback()
        uiHelper.attachTo(surfaceView)
        addDetachListener(surfaceView)
    }

    @Suppress("unused")
    constructor(
            textureView: TextureView,
            engine: Engine = Engine.create(),
            uiHelper: UiHelper = UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK),
            manipulator: Manipulator? = null
    ) : this(engine, uiHelper) {
        cameraManipulator = manipulator ?: Manipulator.Builder()
                .targetPosition(kDefaultObjectPosition.x, kDefaultObjectPosition.y, kDefaultObjectPosition.z)
                .viewport(textureView.width, textureView.height)
                .build(Manipulator.Mode.ORBIT)

        this.textureView = textureView
        gestureDetector = GestureDetector(textureView, cameraManipulator)
        displayHelper = DisplayHelper(textureView.context)
        uiHelper.renderCallback = SurfaceCallback()
        uiHelper.attachTo(textureView)
        addDetachListener(textureView)
    }

    /**
     * Loads a monolithic binary glTF and populates the Filament scene.
     */
    fun loadModelGlb(buffer: Buffer) {
        destroyModel()
        asset = assetLoader.createAsset(buffer)
        asset?.let { asset ->
            resourceLoader.asyncBeginLoad(asset)
            animator = asset.instance.animator
            asset.releaseSourceData()
        }
    }

    /**
     * Loads a JSON-style glTF file and populates the Filament scene.
     *
     * The given callback is triggered for each requested resource.
     */
    fun loadModelGltf(buffer: Buffer, callback: (String) -> Buffer?) {
        destroyModel()
        asset = assetLoader.createAsset(buffer)
        asset?.let { asset ->
            for (uri in asset.resourceUris) {
                val resourceBuffer = callback(uri)
                if (resourceBuffer == null) {
                    this.asset = null
                    return
                }
                resourceLoader.addResourceData(uri, resourceBuffer)
            }
            resourceLoader.asyncBeginLoad(asset)
            animator = asset.instance.animator
            asset.releaseSourceData()
        }
    }

    /**
     * Loads a JSON-style glTF file and populates the Filament scene.
     *
     * The given callback is triggered from a worker thread for each requested resource.
     */
    fun loadModelGltfAsync(buffer: Buffer, callback: (String) -> Buffer) {
        destroyModel()
        asset = assetLoader.createAsset(buffer)
        fetchResourcesJob = CoroutineScope(Dispatchers.IO).launch {
            fetchResources(asset!!, callback)
        }
    }

    /**
     * Sets up a root transform on the current model to make it fit into a unit cube.
     *
     * @param centerPoint Coordinate of center point of unit cube, defaults to < 0, 0, -4 >
     */
    fun transformToUnitCube(centerPoint: Float3 = kDefaultObjectPosition) {
        asset?.let { asset ->
            val tm = engine.transformManager
            var center = asset.boundingBox.center.let { v -> Float3(v[0], v[1], v[2]) }
            val halfExtent = asset.boundingBox.halfExtent.let { v -> Float3(v[0], v[1], v[2]) }
            val maxExtent = 2.0f * max(halfExtent)
            val scaleFactor = 2.0f / maxExtent
            center -= centerPoint / scaleFactor
            val transform = scale(Float3(scaleFactor)) * translation(-center)
            tm.setTransform(tm.getInstance(asset.root), transpose(transform).toFloatArray())
        }
    }

    /**
     * Removes the transformation that was set up via transformToUnitCube.
     */
    fun clearRootTransform() {
        asset?.let {
            val tm = engine.transformManager
            tm.setTransform(tm.getInstance(it.root), Mat4().toFloatArray())
        }
    }

    /**
     * Frees all entities associated with the most recently-loaded model.
     */
    fun destroyModel() {
        fetchResourcesJob?.cancel()
        resourceLoader.asyncCancelLoad()
        resourceLoader.evictResourceData()
        asset?.let { asset ->
            this.scene.removeEntities(asset.entities)
            assetLoader.destroyAsset(asset)
            this.asset = null
            this.animator = null
        }
    }

    /**
     * Renders the model and updates the Filament camera.
     *
     * @param frameTimeNanos time in nanoseconds when the frame started being rendered,
     *                       typically comes from {@link android.view.Choreographer.FrameCallback}
     */
    fun render(frameTimeNanos: Long) {
        if (!uiHelper.isReadyToRender) {
            return
        }

        // Allow the resource loader to finalize textures that have become ready.
        resourceLoader.asyncUpdateLoad()

        // Add renderable entities to the scene as they become ready.
        asset?.let { populateScene(it) }

        // Extract the camera basis from the helper and push it to the Filament camera.
        cameraManipulator.getLookAt(eyePos, target, upward)
        camera.lookAt(
                eyePos[0], eyePos[1], eyePos[2],
                target[0], target[1], target[2],
                upward[0], upward[1], upward[2])

        // Render the scene, unless the renderer wants to skip the frame.
        if (renderer.beginFrame(swapChain!!, frameTimeNanos)) {
            renderer.render(view)
            renderer.endFrame()
        }
    }

    private fun populateScene(asset: FilamentAsset) {
        val rcm = engine.renderableManager
        var count = 0
        val popRenderables = { count = asset.popRenderables(readyRenderables); count != 0 }
        while (popRenderables()) {
            for (i in 0 until count) {
                val ri = rcm.getInstance(readyRenderables[i])
                rcm.setScreenSpaceContactShadows(ri, true)
            }
            scene.addEntities(readyRenderables.take(count).toIntArray())
        }
        scene.addEntities(asset.lightEntities)
    }

    private fun addDetachListener(view: android.view.View) {
        view.addOnAttachStateChangeListener(object : android.view.View.OnAttachStateChangeListener {
            override fun onViewAttachedToWindow(v: android.view.View) {}
            override fun onViewDetachedFromWindow(v: android.view.View) {
                uiHelper.detach()

                destroyModel()
                assetLoader.destroy()
                materialProvider.destroyMaterials()
                materialProvider.destroy()
                resourceLoader.destroy()

                engine.destroyEntity(light)
                engine.destroyRenderer(renderer)
                engine.destroyView(this@ModelViewer.view)
                engine.destroyScene(scene)
                engine.destroyCameraComponent(camera.entity)
                EntityManager.get().destroy(camera.entity)

                EntityManager.get().destroy(light)

                engine.destroy()
            }
        })
    }

    /**
     * Handles a [MotionEvent] to enable one-finger orbit, two-finger pan, and pinch-to-zoom.
     */
    fun onTouchEvent(event: MotionEvent) {
        gestureDetector.onTouchEvent(event)
    }

    @SuppressWarnings("ClickableViewAccessibility")
    override fun onTouch(view: android.view.View, event: MotionEvent): Boolean {
        onTouchEvent(event)
        return true
    }

    private suspend fun fetchResources(asset: FilamentAsset, callback: (String) -> Buffer) {
        val items = HashMap<String, Buffer>()
        val resourceUris = asset.resourceUris
        for (resourceUri in resourceUris) {
            items[resourceUri] = callback(resourceUri)
        }

        withContext(Dispatchers.Main) {
            for ((uri, buffer) in items) {
                resourceLoader.addResourceData(uri, buffer)
            }
            resourceLoader.asyncBeginLoad(asset)
            animator = asset.instance.animator
            asset.releaseSourceData()
        }
    }

    private fun updateCameraProjection() {
        val width = view.viewport.width
        val height = view.viewport.height
        val aspect = width.toDouble() / height.toDouble()
        camera.setLensProjection(cameraFocalLength.toDouble(), aspect,
            cameraNear.toDouble(), cameraFar.toDouble())
    }

    inner class SurfaceCallback : UiHelper.RendererCallback {
        override fun onNativeWindowChanged(surface: Surface) {
            swapChain?.let { engine.destroySwapChain(it) }
            swapChain = engine.createSwapChain(surface)
            surfaceView?.let { displayHelper.attach(renderer, it.display) }
            textureView?.let { displayHelper.attach(renderer, it.display) }
        }

        override fun onDetachedFromSurface() {
            displayHelper.detach()
            swapChain?.let {
                engine.destroySwapChain(it)
                engine.flushAndWait()
                swapChain = null
            }
        }

        override fun onResized(width: Int, height: Int) {
            view.viewport = Viewport(0, 0, width, height)
            cameraManipulator.setViewport(width, height)
            updateCameraProjection()
            synchronizePendingFrames(engine)
        }
    }

    private fun synchronizePendingFrames(engine: Engine) {
        // Wait for all pending frames to be processed before returning. This is to
        // avoid a race between the surface being resized before pending frames are
        // rendered into it.
        val fence = engine.createFence()
        fence.wait(Fence.Mode.FLUSH, Fence.WAIT_FOR_EVER)
        engine.destroyFence(fence)
    }

    companion object {
        private val kDefaultObjectPosition = Float3(0.0f, 0.0f, -4.0f)
    }
}