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

import android.app.Activity
import android.os.Bundle
import android.view.Choreographer
import android.view.Surface
import android.view.SurfaceView
import android.util.Log

import com.google.android.filament.*
import com.google.android.filament.android.UiHelper
import com.google.android.filament.gltfio.*
import com.google.android.filament.utils.*

import java.nio.ByteBuffer
import java.nio.channels.Channels

class MainActivity : Activity() {

    companion object {
        // Load the library for the utility layer,  which includes gltfio and the Filament core.
        init { Utils.init() }
    }

    private lateinit var surfaceView: SurfaceView
    private lateinit var uiHelper: UiHelper
    private lateinit var choreographer: Choreographer
    private val frameScheduler = FrameCallback()

    // gltfio and utils objects
    private lateinit var manipulator: Manipulator
    private lateinit var assetLoader: AssetLoader
    private lateinit var filamentAsset: FilamentAsset
    private lateinit var gestureDetector: GestureDetector

    // core filament objects
    private lateinit var engine: Engine
    private lateinit var renderer: Renderer
    private lateinit var scene: Scene
    private lateinit var view: View
    private lateinit var camera: Camera
    private var swapChain: SwapChain? = null
    @Entity private var light = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        surfaceView = SurfaceView(this).apply { setContentView(this) }

        choreographer = Choreographer.getInstance()

        uiHelper = UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK)
        uiHelper.renderCallback = SurfaceCallback()
        uiHelper.attachTo(surfaceView)

        manipulator = Manipulator.Builder()
                .targetPosition(0.0f, 0.0f, -4.0f)
                .viewport(surfaceView.width, surfaceView.height)
                .build(Manipulator.Mode.ORBIT)

        gestureDetector = GestureDetector(surfaceView, manipulator)

        // Engine, Renderer, Scene, Camera, View
        // -------------------------------------

        engine = Engine.create()
        renderer = engine.createRenderer()
        scene = engine.createScene()
        camera = engine.createCamera().apply { setExposure(16.0f, 1.0f / 125.0f, 100.0f) }
        view = engine.createView()
        view.scene = scene
        view.camera = camera

        // IndirectLight and SkyBox
        // ------------------------

        val ibl = "venetian_crossroads_2k"

        readUncompressedAsset("envs/$ibl/${ibl}_ibl.ktx").let {
            scene.indirectLight = KtxLoader.createIndirectLight(engine, it, KtxLoader.Options())
            scene.indirectLight!!.intensity = 50_000.0f
        }

        readUncompressedAsset("envs/$ibl/${ibl}_skybox.ktx").let {
            scene.skybox = KtxLoader.createSkybox(engine, it, KtxLoader.Options())
        }

        // glTF Entities, Textures, and Materials
        // --------------------------------------

        assetLoader = AssetLoader(engine, MaterialProvider(engine), EntityManager.get())

        filamentAsset = assets.open("models/scene.gltf").use { input ->
            val bytes = ByteArray(input.available())
            input.read(bytes)
            assetLoader.createAssetFromJson(ByteBuffer.wrap(bytes))!!
        }

        val resourceLoader = ResourceLoader(engine)
        for (uri in filamentAsset.resourceUris) {
            val buffer = readUncompressedAsset("models/$uri")
            resourceLoader.addResourceData(uri, buffer)
        }
        resourceLoader.loadResources(filamentAsset)

        scene.addEntities(filamentAsset.entities)

        // Transform to unit cube
        // ----------------------

        val tm = engine.transformManager
        val center = filamentAsset.boundingBox.center.let { Float3(it[0], it[1], it[2]) }
        val halfExtent  = filamentAsset.boundingBox.halfExtent.let { Float3(it[0], it[1], it[2]) }
        val maxExtent = 2.0f * max(halfExtent)
        val scaleFactor = 2.0f / maxExtent
        center.z = center.z + 4.0f / scaleFactor
        val xform = scale(Float3(scaleFactor)) * translation(Float3(-center))
        tm.setTransform(tm.getInstance(filamentAsset.root), transpose(xform).toFloatArray())

        // Light Sources
        // -------------

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

        // Stop the animation and any pending frame
        choreographer.removeFrameCallback(frameScheduler)

        // Always detach the surface before destroying the engine
        uiHelper.detach()

        assetLoader.destroyAsset(filamentAsset)
        assetLoader.destroy()

        engine.destroyEntity(light)
        engine.destroyRenderer(renderer)
        engine.destroyView(view)
        engine.destroyScene(scene)
        engine.destroyCamera(camera)

        // Engine.destroyEntity() destroys Filament components only, not the entity itself.
        val entityManager = EntityManager.get()
        entityManager.destroy(light)

        // Destroying the engine will free up any resource you may have forgotten
        // to destroy, but it's recommended to do the cleanup properly
        engine.destroy()
    }

    inner class FrameCallback : Choreographer.FrameCallback {
        private val eyepos = DoubleArray(3)
        private val target = DoubleArray(3)
        private val upward = DoubleArray(3)
        private val startTime = System.nanoTime()
        override fun doFrame(frameTimeNanos: Long) {
            choreographer.postFrameCallback(this)

            if (!uiHelper.isReadyToRender) {
                return
            }

            manipulator.getLookAt(eyepos, target, upward)
            camera.lookAt(
                    eyepos[0], eyepos[1], eyepos[2],
                    target[0], target[1], target[2],
                    upward[0], upward[1], upward[2])

            val elapsedTimeSeconds = (frameTimeNanos - startTime).toDouble() / 1_000_000_000
            filamentAsset.animator.applyAnimation(0, elapsedTimeSeconds.toFloat())
            filamentAsset.animator.updateBoneMatrices()

            if (renderer.beginFrame(swapChain!!)) {
                renderer.render(view)
                renderer.endFrame()
            }
        }
    }

    inner class SurfaceCallback : UiHelper.RendererCallback {
        override fun onNativeWindowChanged(surface: Surface) {
            swapChain?.let { engine.destroySwapChain(it) }
            swapChain = engine.createSwapChain(surface)
        }

        override fun onDetachedFromSurface() {
            swapChain?.let {
                engine.destroySwapChain(it)
                // Required to ensure we don't return before Filament is done executing the
                // destroySwapChain command, otherwise Android might destroy the Surface
                // too early
                engine.flushAndWait()
                swapChain = null
            }
        }

        override fun onResized(width: Int, height: Int) {
            view.viewport = Viewport(0, 0, width, height)
            val aspect = width.toDouble() / height.toDouble()
            camera.setProjection(45.0, aspect, 0.5, 10000.0, Camera.Fov.VERTICAL)
            manipulator.setViewport(width, height)
        }
    }

    private fun readUncompressedAsset(assetName: String): ByteBuffer {
        Log.i("gltf-viewer", "Reading ${assetName}...")
        assets.openFd(assetName).use { fd ->
            val input = fd.createInputStream()
            val dst = ByteBuffer.allocate(fd.length.toInt())

            val src = Channels.newChannel(input)
            src.read(dst)
            src.close()

            return dst.apply { rewind() }
        }
    }
}
