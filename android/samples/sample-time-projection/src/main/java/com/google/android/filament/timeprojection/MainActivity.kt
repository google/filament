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

package com.google.android.filament.timeprojection

import android.app.Activity
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.Surface
import android.view.SurfaceView
import android.widget.Button
import android.widget.PopupMenu
import android.widget.Switch
import com.google.android.filament.*
import com.google.android.filament.RenderableManager.PrimitiveType
import com.google.android.filament.VertexBuffer.AttributeType
import com.google.android.filament.VertexBuffer.VertexAttribute
import com.google.android.filament.android.ChoreographerHelper
import com.google.android.filament.android.DisplayHelper
import com.google.android.filament.android.FilamentHelper
import com.google.android.filament.android.FramePacer
import com.google.android.filament.android.UiHelper
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.channels.Channels

class MainActivity : Activity() {
    companion object {
        init {
            Filament.init()
        }
    }

    private lateinit var surfaceView: SurfaceView
    private lateinit var uiHelper: UiHelper
    private lateinit var displayHelper: DisplayHelper

    private lateinit var engine: Engine
    private lateinit var renderer: Renderer
    private lateinit var scene: Scene
    private lateinit var view: View
    private lateinit var camera: Camera

    private lateinit var material: Material
    private lateinit var vertexBuffer: VertexBuffer
    private lateinit var indexBuffer: IndexBuffer

    @Entity private var renderable = 0
    private var swapChain: SwapChain? = null
    private var framePacer: FramePacer? = null

    private val frameScheduler = FrameCallback()

    private var mUseDesiredPresentationTime = false
    private var mPauseRequested = false
    private var mPacingDemo = false
    private var mTargetFps = 120L
    private val mPatternFrames = 7L

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        surfaceView = findViewById(R.id.surface_view)
        displayHelper = DisplayHelper(this)

        setupSurfaceView()
        setupFilament()
        setupView()
        setupScene()
        setupControls()
    }

    private fun setupControls() {
        findViewById<Switch>(R.id.switch_projection).setOnCheckedChangeListener { _, isChecked ->
            mUseDesiredPresentationTime = isChecked
        }

        findViewById<Switch>(R.id.switch_pause).setOnCheckedChangeListener { _, isChecked ->
            mPauseRequested = isChecked
        }

        findViewById<Switch>(R.id.switch_pacing_demo).setOnCheckedChangeListener { _, isChecked ->
            mPacingDemo = isChecked
            setTargetFps(mTargetFps)
        }

        val btnFps = findViewById<Button>(R.id.btn_fps)
        btnFps.setOnClickListener { view ->
            val popup = PopupMenu(this, view)
            popup.menu.add("30 FPS")
            popup.menu.add("60 FPS")
            popup.menu.add("120 FPS")
            popup.setOnMenuItemClickListener { item ->
                val fps = when (item.title) {
                    "30 FPS" -> 30L
                    "120 FPS" -> 120L
                    else -> 60L
                }
                setTargetFps(fps)
                btnFps.text = item.title
                true
            }
            popup.show()
        }
    }

    private fun setTargetFps(fps: Long) {
        mTargetFps = fps
        if (mPacingDemo) {
            swapChain?.setFrameRate(120f)
            framePacer?.configure(60f, 2)
            material.defaultInstance.setParameter("targetFps", 60f)
        } else {
            swapChain?.setFrameRate(fps.toFloat())
            framePacer?.configure(fps.toFloat(), 2)
            material.defaultInstance.setParameter("targetFps", fps.toFloat())
        }
    }

    private fun setupSurfaceView() {
        uiHelper = UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK)
        uiHelper.renderCallback = SurfaceCallback()
        uiHelper.attachTo(surfaceView)
    }

    private fun setupFilament() {
        val config = Engine.Config()
        engine = Engine.Builder()
            .config(config)
            .featureLevel(Engine.FeatureLevel.FEATURE_LEVEL_0)
            .build()
        renderer = engine.createRenderer()
        framePacer = FramePacer.Builder()
            .targetFrameRate(mTargetFps.toFloat())
            .latencyFrames(2)
            .build(engine)
        frameScheduler.setRenderer(renderer)
        scene = engine.createScene()
        view = engine.createView()
        camera = engine.createCamera(engine.entityManager.create())
    }

    private fun setupView() {
        scene.skybox = Skybox.Builder().color(0.035f, 0.035f, 0.035f, 1.0f).build(engine)

        if (engine.activeFeatureLevel == Engine.FeatureLevel.FEATURE_LEVEL_0) {
            view.isPostProcessingEnabled = false
        }

        view.camera = camera
        view.scene = scene
    }

    private fun setupScene() {
        loadMaterial()
        createMesh()

        material.defaultInstance.setParameter("targetFps", mTargetFps.toFloat())

        renderable = EntityManager.get().create()

        RenderableManager.Builder(1)
                .boundingBox(Box(-0.1f, 0.0f, 0.0f, 0.1f, 0.9f, 0.01f))
                .geometry(0, PrimitiveType.TRIANGLES, vertexBuffer, indexBuffer, 0, 3)
                .material(0, material.defaultInstance)
                .build(engine, renderable)

        scene.addEntity(renderable)
    }

    private fun loadMaterial() {
        readUncompressedAsset("materials/baked_color.filamat").let {
            material = Material.Builder().payload(it, it.remaining()).build(engine)
            material.compile(
                Material.CompilerPriorityQueue.HIGH,
                Material.UserVariantFilterBit.ALL,
                Handler(Looper.getMainLooper())) {
            }
            engine.flush()
        }
    }

    private fun createMesh() {
        val intSize = 4
        val floatSize = 4
        val shortSize = 2
        val vertexSize = 3 * floatSize + intSize

        data class Vertex(val x: Float, val y: Float, val z: Float, val color: Int)
        fun ByteBuffer.put(v: Vertex): ByteBuffer {
            putFloat(v.x)
            putFloat(v.y)
            putFloat(v.z)
            putInt(v.color)
            return this
        }

        val vertexCount = 3
        val vertexData = ByteBuffer.allocate(vertexCount * vertexSize)
                .order(ByteOrder.nativeOrder())
                .put(Vertex(0.0f,  0.8f, 0.0f, 0xff0000ff.toInt())) // Red tip
                .put(Vertex(-0.04f, 0.0f, 0.0f, 0xffffffff.toInt())) // Base left
                .put(Vertex(0.04f,  0.0f, 0.0f, 0xffffffff.toInt())) // Base right
                .flip()

        vertexBuffer = VertexBuffer.Builder()
                .bufferCount(1)
                .vertexCount(vertexCount)
                .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT3, 0,             vertexSize)
                .attribute(VertexAttribute.COLOR,    0, AttributeType.UBYTE4, 3 * floatSize, vertexSize)
                .normalized(VertexAttribute.COLOR)
                .build(engine)

        vertexBuffer.setBufferAt(engine, 0, vertexData)

        val indexData = ByteBuffer.allocate(vertexCount * shortSize)
                .order(ByteOrder.nativeOrder())
                .putShort(0)
                .putShort(1)
                .putShort(2)
                .flip()

        indexBuffer = IndexBuffer.Builder()
                .indexCount(3)
                .bufferType(IndexBuffer.Builder.IndexType.USHORT)
                .build(engine)
        indexBuffer.setBuffer(engine, indexData)
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

        uiHelper.detach()

        framePacer?.destroy(engine)
        engine.destroyEntity(renderable)
        engine.destroyRenderer(renderer)
        engine.destroyVertexBuffer(vertexBuffer)
        engine.destroyIndexBuffer(indexBuffer)
        engine.destroyMaterial(material)
        engine.destroyView(view)
        engine.destroyScene(scene)
        engine.destroyCameraComponent(camera.entity)

        val entityManager = EntityManager.get()
        entityManager.destroy(renderable)
        entityManager.destroy(camera.entity)

        engine.destroy()
    }

    inner class FrameCallback : ChoreographerHelper() {
        private var mFrameCounter = 0L

        override fun onFrame(frameTimeNanos: Long) {
            if (uiHelper.isReadyToRender && swapChain != null) {
                val periodNs = 1_000_000_000L / mTargetFps

                if (framePacer?.setupFrame(frameTimeNanos, periodNs) == false) {
                    return
                }

                if (framePacer?.hasGpuFallenBehind(renderer) == true) {
                    renderer.skipFrame(frameTimeNanos)
                    return
                }

                framePacer?.applyPresentationTime(renderer)

                // Since FramePacer sets the desiredPresentationTime automatically,
                // we override it manually after apply() if the user selects vsync time
                if (!mUseDesiredPresentationTime) {
                    // we need to set it to zero, otherwise it interfers with the frame history
                    renderer.setDesiredPresentationTime(0)
                }

                if (mFrameCounter % mPatternFrames == 0L) {
                    if (mUseDesiredPresentationTime) {
                        framePacer?.expectedPresentationTimeNanos?.let { renderer.setMaterialTimeEpoch(it) }
                    } else {
                        renderer.setMaterialTimeEpoch(frameTimeNanos)
                    }
                }

                if (renderer.beginFrame(swapChain!!, frameTimeNanos)) {
                    mFrameCounter++
                    renderer.render(view)
                    renderer.endFrame()

                    if (mPauseRequested && (mFrameCounter % mPatternFrames == 3L)) {
                        renderer.pauseRenderThread(periodNs * 1L)
                    }
                }
            }
        }
    }

    inner class SurfaceCallback : UiHelper.RendererCallback {
        override fun onNativeWindowChanged(surface: Surface) {
            swapChain?.let { engine.destroySwapChain(it) }

            var flags = uiHelper.swapChainFlags
            if (engine.activeFeatureLevel == Engine.FeatureLevel.FEATURE_LEVEL_0) {
                if (SwapChain.isSRGBSwapChainSupported(engine)) {
                    flags = flags or SwapChainFlags.CONFIG_SRGB_COLORSPACE
                }
            }

            swapChain = engine.createSwapChain(surface, flags)
            swapChain?.setFrameRate(mTargetFps.toFloat())
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
            val zoom = 1.5
            val aspect = width.toDouble() / height.toDouble()
            camera.setProjection(Camera.Projection.ORTHO,
                    -aspect * zoom, aspect * zoom, -zoom, zoom, 0.0, 10.0)

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
