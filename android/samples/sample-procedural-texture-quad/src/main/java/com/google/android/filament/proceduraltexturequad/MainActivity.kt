/*
 * Copyright (C) 2026 The Android Open Source Project
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

package com.google.android.filament.proceduraltexturequad

import android.app.Activity
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.Choreographer
import android.view.Surface
import android.view.SurfaceView
import com.google.android.filament.*
import com.google.android.filament.RenderableManager.PrimitiveType
import com.google.android.filament.android.DisplayHelper
import com.google.android.filament.android.FilamentHelper
import com.google.android.filament.android.UiHelper
import com.google.android.filament.utils.*
import java.nio.ByteBuffer
import java.nio.channels.Channels

class MainActivity : Activity() {
    companion object {
        init {
            Filament.init()
            Utils.init()
        }
    }

    private lateinit var surfaceView: SurfaceView
    private lateinit var uiHelper: UiHelper
    private lateinit var displayHelper: DisplayHelper
    private lateinit var choreographer: Choreographer

    private lateinit var engine: Engine
    private lateinit var renderer: Renderer
    private lateinit var scene: Scene
    private lateinit var view: View
    private lateinit var camera: Camera

    private lateinit var material: Material
    private lateinit var materialInstance: MaterialInstance
    private lateinit var vertexBuffer: VertexBuffer
    private lateinit var texture: Texture

    @Entity private var renderable = 0
    private var swapChain: SwapChain? = null
    private val frameScheduler = FrameCallback()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        surfaceView = SurfaceView(this)
        setContentView(surfaceView)

        choreographer = Choreographer.getInstance()
        displayHelper = DisplayHelper(this)

        setupSurfaceView()
        setupFilament()
        setupView()
        setupScene()
    }

    private fun setupSurfaceView() {
        uiHelper = UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK)
        uiHelper.renderCallback = SurfaceCallback()
        uiHelper.attachTo(surfaceView)
    }

    private fun setupFilament() {
        val config = Engine.Config()

        // Attribute-less/procedural rendering requires gl_VertexID support, which is
        // available in FEATURE_LEVEL_1 (OpenGL ES 3.0+) and above.
        engine = Engine.Builder()
            .config(config)
            .featureLevel(Engine.FeatureLevel.FEATURE_LEVEL_1)
            .build()

        renderer = engine.createRenderer()
        scene = engine.createScene()
        view = engine.createView()
        camera = engine.createCamera(engine.entityManager.create())
    }

    private fun setupView() {
        scene.skybox = Skybox.Builder().color(0.1f, 0.125f, 0.25f, 1.0f).build(engine)

        view.camera = camera
        view.scene = scene

        view.isPostProcessingEnabled = false
    }

    private fun setupScene() {
        loadMaterial()
        loadTextureResource()

        // Create an attribute-less VertexBuffer: 0 buffer slots, 6 vertices.
        // The vertex shader will procedurally generate a quad from the vertex index.
        vertexBuffer = VertexBuffer.Builder()
            .bufferCount(0)
            .vertexCount(6)
            .build(engine)

        renderable = EntityManager.get().create()

        // We define an object-space bounding box big enough to prevent culling issues for our quad.
        val aabb = Box(0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.01f)

        RenderableManager.Builder(1)
            .boundingBox(aabb)
            // Use the non-indexed geometry overload that omits the IndexBuffer parameter
            .geometry(0, PrimitiveType.TRIANGLES, vertexBuffer)
            .material(0, materialInstance)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .build(engine, renderable)

        scene.addEntity(renderable)
    }

    private fun loadMaterial() {
        // Read our compiled material filamat from assets
        readUncompressedAsset("materials/proceduralTextureQuad.filamat").let {
            material = Material.Builder().payload(it, it.remaining()).build(engine)
            materialInstance = material.createInstance()

            // Optional: asynchronously compile material variants to prevent frame hiccups
            material.compile(
                Material.CompilerPriorityQueue.HIGH,
                Material.UserVariantFilterBit.ALL,
                Handler(Looper.getMainLooper())
            ) {
                android.util.Log.i("ProceduralTextureQuad", "Material ${material.name} compiled.")
            }
            engine.flush()
        }
    }

    private fun loadTextureResource() {
        texture = loadTexture(engine, resources, R.drawable.moss_01_color, TextureType.COLOR)
        val sampler = TextureSampler(
            TextureSampler.MinFilter.LINEAR,
            TextureSampler.MagFilter.LINEAR,
            TextureSampler.WrapMode.CLAMP_TO_EDGE
        )
        materialInstance.setParameter("albedo", texture, sampler)
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
        uiHelper.detach()

        engine.destroyEntity(renderable)
        engine.destroyRenderer(renderer)
        engine.destroyVertexBuffer(vertexBuffer)
        engine.destroyTexture(texture)
        engine.destroyMaterialInstance(materialInstance)
        engine.destroyMaterial(material)
        engine.destroyView(view)
        engine.destroyScene(scene)
        engine.destroyCameraComponent(camera.entity)

        val entityManager = EntityManager.get()
        entityManager.destroy(renderable)
        entityManager.destroy(camera.entity)

        engine.destroy()
    }

    inner class FrameCallback : Choreographer.FrameCallback {
        override fun doFrame(frameTimeNanos: Long) {
            choreographer.postFrameCallback(this)

            if (uiHelper.isReadyToRender) {
                if (renderer.beginFrame(swapChain!!, frameTimeNanos)) {
                    renderer.render(view)
                    renderer.endFrame()
                }
            }
        }
    }

    inner class SurfaceCallback : UiHelper.RendererCallback {
        override fun onNativeWindowChanged(surface: Surface) {
            swapChain?.let { engine.destroySwapChain(it) }
            swapChain = engine.createSwapChain(surface, uiHelper.swapChainFlags)
            displayHelper.attach(renderer, surfaceView.display)
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
            // Set up an orthographic projection since we are rendering a full-screen/procedural quad
            val aspect = width.toDouble() / height.toDouble()
            camera.setProjection(Camera.Projection.ORTHO, -aspect, aspect, -1.0, 1.0, -1.0, 1.0)

            view.viewport = Viewport(0, 0, width, height)
            FilamentHelper.synchronizePendingFrames(engine)
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
}
