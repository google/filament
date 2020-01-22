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
 * [transformToUnitCube]. All ECS entities can be accessed via the [asset] property.
 *
 * For GLB files, clients can call [loadModelGlb] and pass in a [Buffer] with the contents of the
 * GLB file. For glTF files, clients can call [loadModelGltf] and pass in a [Buffer] with the JSON
 * contents, as well as a callback for loading external resources.
 *
 * `ModelViewer` reduces much of the boilerplate required for simple Filament applications, but
 * clients still have the responsibility of adding an [IndirectLight] and [Skybox] to the scene.
 * Additionally, clients should:
 *
 * 1. Call [onTouchEvent] from a touch handler.
 * 2. Call [render] and [Animator.applyAnimation] from a `Choreographer` frame callback.
 * 3. Call [detach] when the model viewer is no longer needed.
 *
 * See `sample-gltf-viewer` for a usage example.
 */
class ModelViewer {

    var asset: FilamentAsset? = null
        private set

    var animator: Animator? = null
        private set

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
    }

    /**
     * Loads a monolithic binary glTF and populates the Filament scene.
     */
    fun loadModelGlb(buffer: Buffer) {
        destroyModel()
        asset = assetLoader.createAssetFromJson(buffer)
        asset?.let { asset ->
            val resourceLoader = ResourceLoader(engine)
            resourceLoader.loadResources(asset)
            resourceLoader.destroy()
            animator = asset.animator
            asset.releaseSourceData()
            scene.addEntities(asset.entities)
        }
    }

    /**
     * Loads a JSON-style glTF file and populates the Filament scene.
     */
    fun loadModelGltf(buffer: Buffer, callback: (String) -> Buffer) {
        destroyModel()
        asset = assetLoader.createAssetFromJson(buffer)
        asset?.let { asset ->
            val resourceLoader = ResourceLoader(engine)
            for (uri in asset.resourceUris) {
                resourceLoader.addResourceData(uri, callback(uri))
            }
            resourceLoader.loadResources(asset)
            resourceLoader.destroy()
            animator = asset.animator
            asset.releaseSourceData()
            scene.addEntities(asset.entities)
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
     *
     * @see detach
     */
    fun destroyModel() {
        asset?.let { asset ->
            assetLoader.destroyAsset(asset)
            this.asset = null
            this.animator = null
        }
    }

    /**
     * Destroys the Filament engine and all related objects.
     *
     * The ModelViewer becomes invalid after this is called.
     *
     * @see destroyModel
     */
    fun detach() {
        uiHelper.detach()

        destroyModel()
        assetLoader.destroy()

        engine.destroyEntity(light)
        engine.destroyRenderer(renderer)
        engine.destroyView(view)
        engine.destroyScene(scene)
        engine.destroyCamera(camera)

        EntityManager.get().destroy(light)

        engine.destroy()
    }

    /**
     * Renders the model and updates the Filament camera.
     */
    fun render() {
        if (!uiHelper.isReadyToRender) {
            return
        }

        cameraManipulator.getLookAt(eyePos, target, upward)
        camera.lookAt(
                eyePos[0], eyePos[1], eyePos[2],
                target[0], target[1], target[2],
                upward[0], upward[1], upward[2])

        if (renderer.beginFrame(swapChain!!)) {
            renderer.render(view)
            renderer.endFrame()
        }
    }

    /**
     * Handles a [MotionEvent] to enable one-finger orbit, two-finger pan, and pinch-to-zoom.
     */
    fun onTouchEvent(event: MotionEvent) {
        gestureDetector.onTouchEvent(event)
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