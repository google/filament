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
import com.google.android.filament.android.UiHelper
import com.google.android.filament.gltfio.*
import java.nio.Buffer

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
class ModelViewer : android.view.View.OnTouchListener {

    var asset: FilamentAsset? = null
        private set

    var animator: Animator? = null
        private set

    @Suppress("unused")
    val progress
        get() = resourceLoader.asyncGetLoadProgress()

    val engine: Engine
    val scene: Scene
    val view: View
    val camera: Camera
    @Entity val light: Int

    private val uiHelper: UiHelper = UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK)
    private val cameraManipulator: Manipulator
    private val gestureDetector: GestureDetector
    private val renderer: Renderer
    private var swapChain: SwapChain? = null
    private var assetLoader: AssetLoader
    private var resourceLoader: ResourceLoader
    private val readyRenderables = IntArray(128) // add up to 128 entities at a time

    private val eyePos = DoubleArray(3)
    private val target = DoubleArray(3)
    private val upward = DoubleArray(3)

    private val kNearPlane = 0.5
    private val kFarPlane = 10000.0
    private val kFovDegrees = 45.0
    private val kAperture = 16f
    private val kShutterSpeed = 1f / 125f
    private val kSensitivity = 100f

    init {
        engine = Engine.create()
        renderer = engine.createRenderer()
        scene = engine.createScene()
        camera = engine.createCamera().apply { setExposure(kAperture, kShutterSpeed, kSensitivity) }
        view = engine.createView()
        view.scene = scene
        view.camera = camera

        assetLoader = AssetLoader(engine, MaterialProvider(engine), EntityManager.get())
        resourceLoader = ResourceLoader(engine)

        // Always add a direct light source since it is required for shadowing.
        // We highly recommend adding an indirect light as well.

        light = EntityManager.get().create()

        val (r, g, b) = Colors.cct(6_500.0f)
        LightManager.Builder(LightManager.Type.DIRECTIONAL)
                .color(r, g, b)
                .intensity(300_000.0f)
                .direction(0.0f, -1.0f, 0.0f)
                .castShadows(true)
                .build(engine, light)

        scene.addEntity(light)
    }

    constructor(surfaceView: SurfaceView) {
        cameraManipulator = Manipulator.Builder()
                .targetPosition(0.0f, 0.0f, -4.0f)
                .viewport(surfaceView.width, surfaceView.height)
                .build(Manipulator.Mode.ORBIT)

        gestureDetector = GestureDetector(surfaceView, cameraManipulator)
        uiHelper.renderCallback = SurfaceCallback()
        uiHelper.attachTo(surfaceView)
        addDetachListener(surfaceView)
    }

    @Suppress("unused")
    constructor(textureView: TextureView) {
        cameraManipulator = Manipulator.Builder()
                .targetPosition(0.0f, 0.0f, -4.0f)
                .viewport(textureView.width, textureView.height)
                .build(Manipulator.Mode.ORBIT)

        gestureDetector = GestureDetector(textureView, cameraManipulator)
        uiHelper.renderCallback = SurfaceCallback()
        uiHelper.attachTo(textureView)
        addDetachListener(textureView)
    }

    /**
     * Loads a monolithic binary glTF and populates the Filament scene.
     */
    fun loadModelGlb(buffer: Buffer) {
        destroyModel()
        asset = assetLoader.createAssetFromJson(buffer)
        asset?.let { asset ->
            resourceLoader.asyncBeginLoad(asset)
            animator = asset.animator
            asset.releaseSourceData()
        }
    }

    /**
     * Loads a JSON-style glTF file and populates the Filament scene.
     */
    fun loadModelGltf(buffer: Buffer, callback: (String) -> Buffer) {
        destroyModel()
        asset = assetLoader.createAssetFromJson(buffer)
        asset?.let { asset ->
            for (uri in asset.resourceUris) {
                resourceLoader.addResourceData(uri, callback(uri))
            }
            resourceLoader.asyncBeginLoad(asset)
            animator = asset.animator
            asset.releaseSourceData()
        }
    }

    /**
     * Sets up a root transform on the current model to make it fit into the viewing frustum.
     */
    fun transformToUnitCube() {
        asset?.let { asset ->
            val tm = engine.transformManager
            val center = asset.boundingBox.center.let { v-> Float3(v[0], v[1], v[2]) }
            val halfExtent = asset.boundingBox.halfExtent.let { v-> Float3(v[0], v[1], v[2]) }
            val maxExtent = 2.0f * max(halfExtent)
            val scaleFactor = 2.0f / maxExtent
            center.z = center.z + 4.0f / scaleFactor
            val transform = scale(Float3(scaleFactor)) * translation(Float3(-center))
            tm.setTransform(tm.getInstance(asset.root), transpose(transform).toFloatArray())
        }
    }

    /**
     * Frees all entities associated with the most recently-loaded model.
     */
    fun destroyModel() {
        asset?.let { asset ->
            assetLoader.destroyAsset(asset)
            this.asset = null
            this.animator = null
        }
    }

    /**
     * Renders the model and updates the Filament camera.
     */
    fun render() {
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
        if (renderer.beginFrame(swapChain!!)) {
            renderer.render(view)
            renderer.endFrame()
        }
    }

    private fun populateScene(asset: FilamentAsset) {
        var count = 0
        val popRenderables = {count = asset.popRenderables(readyRenderables); count != 0}
        while (popRenderables()) {
            scene.addEntities(readyRenderables.take(count).toIntArray())
        }
    }

    private fun addDetachListener(view: android.view.View) {
        view.addOnAttachStateChangeListener(object : android.view.View.OnAttachStateChangeListener {
            override fun onViewAttachedToWindow(v: android.view.View?) {}
            override fun onViewDetachedFromWindow(v: android.view.View?) {
                uiHelper.detach()

                destroyModel()
                assetLoader.destroy()
                resourceLoader.destroy()

                engine.destroyEntity(light)
                engine.destroyRenderer(renderer)
                engine.destroyView(this@ModelViewer.view)
                engine.destroyScene(scene)
                engine.destroyCamera(camera)

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

    inner class SurfaceCallback : UiHelper.RendererCallback {
        override fun onNativeWindowChanged(surface: Surface) {
            swapChain?.let { engine.destroySwapChain(it) }
            swapChain = engine.createSwapChain(surface)
        }

        override fun onDetachedFromSurface() {
            swapChain?.let {
                engine.destroySwapChain(it)
                engine.flushAndWait()
                swapChain = null
            }
        }

        override fun onResized(width: Int, height: Int) {
            view.viewport = Viewport(0, 0, width, height)
            val aspect = width.toDouble() / height.toDouble()
            camera.setProjection(kFovDegrees, aspect, kNearPlane, kFarPlane, Camera.Fov.VERTICAL)
            cameraManipulator.setViewport(width, height)
        }
    }
}