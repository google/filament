/*
 * Copyright (C) 2024 The Android Open Source Project
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

package com.google.android.filament.texturetarget

import android.animation.ValueAnimator
import android.app.Activity
import android.hardware.HardwareBuffer
import android.opengl.Matrix
import android.os.Bundle
import android.view.Choreographer
import android.view.Surface
import android.view.SurfaceView
import android.view.animation.LinearInterpolator
import com.google.android.filament.*
import com.google.android.filament.RenderableManager.PrimitiveType
import com.google.android.filament.VertexBuffer.AttributeType
import com.google.android.filament.VertexBuffer.VertexAttribute
import com.google.android.filament.android.DisplayHelper
import com.google.android.filament.android.FilamentHelper
import com.google.android.filament.android.UiHelper
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.channels.Channels
import kotlin.math.PI
import kotlin.math.cos
import kotlin.math.sin

class MainActivity : Activity() {
    companion object {
        init {
            Filament.init()
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

    private lateinit var triangleMaterial: Material
    private lateinit var texturedMaterial: Material
    private lateinit var triangleVertexBuffer: VertexBuffer
    private lateinit var triangleIndexBuffer: IndexBuffer
    private lateinit var quadVertexBuffer: VertexBuffer
    private lateinit var quadIndexBuffer: IndexBuffer

    @Entity private var triangleRenderable = 0
    @Entity private var quadRenderable = 0

    private var swapChain: SwapChain? = null

    private val frameScheduler = FrameCallback()
    private val animator = ValueAnimator.ofFloat(0.0f, 360.0f)

    private var hardwareBuffer: HardwareBuffer? = null
    private var texture: Texture? = null
    private var renderTarget: RenderTarget? = null

    private var useExternalTexture = true

    private lateinit var offscreenView: View
    private lateinit var offscreenCamera: Camera

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // To use set this flag with adb, run
        // adb shell am start -n com.google.android.filament.texturetarget/.MainActivity --ez useExternalTexture false
        useExternalTexture = intent.getBooleanExtra("useExternalTexture", true)
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
        engine = Engine.create()
        renderer = engine.createRenderer()
        scene = engine.createScene()
        view = engine.createView()
        camera = engine.createCamera(engine.entityManager.create())
        offscreenView = engine.createView()
        offscreenCamera = engine.createCamera(engine.entityManager.create())
    }

    private fun setupView() {
        scene.skybox = Skybox.Builder()
            .priority(0)
            .color(0.0f, 0.0f, 1.0f, 1.0f).build(engine)

        // This is the view that will be drawn on screen.
        view.camera = camera
        view.scene = scene
        view.isPostProcessingEnabled = false

        // This is the view that will be rendered off-screen.
        offscreenView.camera = offscreenCamera
        offscreenView.scene = scene
        offscreenView.isPostProcessingEnabled = false
    }

    private fun setupScene() {
        loadMaterials()
        createTriangleMesh()
        createQuadMesh()

        // layer 1: skybox
        // layer 2: triangle
        // layer 3: quad

        triangleMaterial.defaultInstance.cullingMode = Material.CullingMode.NONE;
        texturedMaterial.defaultInstance.cullingMode = Material.CullingMode.NONE;

        // The triangle is a regular renderable.
        triangleRenderable = EntityManager.get().create()
        RenderableManager.Builder(1)
            .geometry(0, PrimitiveType.TRIANGLES, triangleVertexBuffer, triangleIndexBuffer, 0, 3)
            .material(0, triangleMaterial.defaultInstance)
            .culling(false)
            .castShadows(false)
            .receiveShadows(false)
            .layerMask(7, 2)
            .build(engine, triangleRenderable)

        // The quad is a regular renderable.
        quadRenderable = EntityManager.get().create()
        RenderableManager.Builder(1)
            .geometry(0, PrimitiveType.TRIANGLES, quadVertexBuffer, quadIndexBuffer, 0, 6)
            .material(0, texturedMaterial.defaultInstance)
            .culling(false)
            .castShadows(false)
            .receiveShadows(false)
            .layerMask(7, 4)
            .build(engine, quadRenderable)

        // We only want to render the triangle in the offscreen view.
        offscreenView.setVisibleLayers(7, 3)    // render skybox + triangle

        // We only want to render the quad in the on-screen view.
        view.setVisibleLayers(7, 4)             // render quad only

        scene.addEntity(triangleRenderable)
        scene.addEntity(quadRenderable)

        startAnimation()
    }

    private fun loadMaterials() {
        readUncompressedAsset("materials/baked_color.filamat").let {
            triangleMaterial = Material.Builder().payload(it, it.remaining()).build(engine)
        }

        if (useExternalTexture) {
            readUncompressedAsset("materials/texturedExternal.filamat").let {
                texturedMaterial = Material.Builder().payload(it, it.remaining()).build(engine)
            }
        } else {
            readUncompressedAsset("materials/textured.filamat").let {
                texturedMaterial = Material.Builder().payload(it, it.remaining()).build(engine)
            }
        }
    }

    private fun createTriangleMesh() {
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
        val a1 = PI * 2.0 / 3.0
        val a2 = PI * 4.0 / 3.0

        val vertexData = ByteBuffer.allocate(vertexCount * vertexSize)
            .order(ByteOrder.nativeOrder())
            .put(Vertex(1.0f, 0.0f, 0.0f, 0xffff0000.toInt()))
            .put(Vertex(cos(a1).toFloat(), sin(a1).toFloat(), 0.0f, 0xff00ff00.toInt()))
            .put(Vertex(cos(a2).toFloat(), sin(a2).toFloat(), 0.0f, 0xff0000ff.toInt()))
            .flip()

        triangleVertexBuffer = VertexBuffer.Builder()
            .bufferCount(1)
            .vertexCount(vertexCount)
            .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT3, 0, vertexSize)
            .attribute(VertexAttribute.COLOR, 0, AttributeType.UBYTE4, 3 * floatSize, vertexSize)
            .normalized(VertexAttribute.COLOR)
            .build(engine)
        triangleVertexBuffer.setBufferAt(engine, 0, vertexData)

        val indexData = ByteBuffer.allocate(vertexCount * shortSize)
            .order(ByteOrder.nativeOrder())
            .putShort(0)
            .putShort(1)
            .putShort(2)
            .flip()

        triangleIndexBuffer = IndexBuffer.Builder()
            .indexCount(3)
            .bufferType(IndexBuffer.Builder.IndexType.USHORT)
            .build(engine)
        triangleIndexBuffer.setBuffer(engine, indexData)
    }

    private fun createQuadMesh() {
        val floatSize = 4
        val shortSize = 2
        val vertexSize = (2 * floatSize) + (2 * floatSize) // position + UV

        data class Vertex(val x: Float, val y: Float, val u: Float, val v: Float)
        fun ByteBuffer.put(v: Vertex): ByteBuffer {
            putFloat(v.x)
            putFloat(v.y)
            putFloat(v.u)
            putFloat(v.v)
            return this
        }

        val vertexCount = 4
        val vertexData = ByteBuffer.allocate(vertexCount * vertexSize)
            .order(ByteOrder.nativeOrder())
            .put(Vertex(-1.0f, -1.0f, 0.0f, 0.0f))
            .put(Vertex( 1.0f, -1.0f, 1.0f, 0.0f))
            .put(Vertex( 1.0f,  1.0f, 1.0f, 1.0f))
            .put(Vertex(-1.0f,  1.0f, 0.0f, 1.0f))
            .flip()

        quadVertexBuffer = VertexBuffer.Builder()
            .bufferCount(1)
            .vertexCount(vertexCount)
            .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT2, 0, vertexSize)
            .attribute(VertexAttribute.UV0, 0, AttributeType.FLOAT2, 2 * floatSize, vertexSize)
            .build(engine)
        quadVertexBuffer.setBufferAt(engine, 0, vertexData)

        val indexData = ByteBuffer.allocate(6 * shortSize)
            .order(ByteOrder.nativeOrder())
            .putShort(0).putShort(1).putShort(2)
            .putShort(0).putShort(2).putShort(3)
            .flip()

        quadIndexBuffer = IndexBuffer.Builder()
            .indexCount(6)
            .bufferType(IndexBuffer.Builder.IndexType.USHORT)
            .build(engine)
        quadIndexBuffer.setBuffer(engine, indexData)
    }

    private fun startAnimation() {
        animator.interpolator = LinearInterpolator()
        animator.duration = 4000
        animator.repeatMode = ValueAnimator.RESTART
        animator.repeatCount = ValueAnimator.INFINITE
        animator.addUpdateListener { a ->
            val transformMatrix = FloatArray(16)
            Matrix.setRotateM(transformMatrix, 0, -(a.animatedValue as Float), 0.0f, 0.0f, 1.0f)
            val tcm = engine.transformManager
            tcm.setTransform(tcm.getInstance(triangleRenderable), transformMatrix)
        }
        animator.start()
    }

    override fun onResume() {
        super.onResume()
        choreographer.postFrameCallback(frameScheduler)
        animator.start()
    }

    override fun onPause() {
        super.onPause()
        choreographer.removeFrameCallback(frameScheduler)
        animator.cancel()
    }

    override fun onDestroy() {
        super.onDestroy()
        choreographer.removeFrameCallback(frameScheduler)
        animator.cancel()
        uiHelper.detach()

        // Destroy all renderables.
        scene.remove(triangleRenderable)
        scene.remove(quadRenderable)

        // Destroy all resources.
        engine.destroyEntity(triangleRenderable)
        engine.destroyEntity(quadRenderable)
        engine.destroyRenderer(renderer)
        engine.destroyVertexBuffer(triangleVertexBuffer)
        engine.destroyIndexBuffer(triangleIndexBuffer)
        engine.destroyVertexBuffer(quadVertexBuffer)
        engine.destroyIndexBuffer(quadIndexBuffer)
        engine.destroyMaterial(triangleMaterial)
        engine.destroyMaterial(texturedMaterial)
        engine.destroyView(view)
        engine.destroyView(offscreenView)
        engine.destroyScene(scene)
        engine.destroyCameraComponent(camera.entity)
        engine.destroyCameraComponent(offscreenCamera.entity)
        renderTarget?.let { engine.destroyRenderTarget(it) }
        texture?.let { engine.destroyTexture(it) }
        hardwareBuffer?.close()

        val entityManager = EntityManager.get()
        entityManager.destroy(triangleRenderable)
        entityManager.destroy(quadRenderable)
        entityManager.destroy(camera.entity)
        entityManager.destroy(offscreenCamera.entity)

        engine.destroy()
    }

    inner class FrameCallback : Choreographer.FrameCallback {
        override fun doFrame(frameTimeNanos: Long) {
            choreographer.postFrameCallback(this)
            if (uiHelper.isReadyToRender) {
                if (renderer.beginFrame(swapChain!!, frameTimeNanos)) {
                    // Render the triangle to the texture.
                    renderer.render(offscreenView)
                    // Render the quad to the screen.
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
            // On-screen camera
            val zoom = 1.0
            val aspect = width.toDouble() / height.toDouble()
            camera.setProjection(Camera.Projection.ORTHO, -aspect * zoom, aspect * zoom, -zoom, zoom, 0.0, 10.0)
            view.viewport = Viewport(0, 0, width, height)

            // Off-screen camera
            val offscreenZoom = 1.5
            offscreenCamera.setProjection(Camera.Projection.ORTHO,
                -aspect * offscreenZoom, aspect * offscreenZoom,
                -offscreenZoom, offscreenZoom, 0.0, 10.0)
            offscreenView.viewport = Viewport(0, 0, width, height)

            // If we have a render target, destroy it.
            renderTarget?.let { engine.destroyRenderTarget(it) }
            texture?.let { engine.destroyTexture(it) }
            hardwareBuffer?.close()

            if (useExternalTexture) {
                // Create a new render target.
                hardwareBuffer = HardwareBuffer.create(width, height,
                    HardwareBuffer.RGBA_8888, 1,
                    HardwareBuffer.USAGE_GPU_SAMPLED_IMAGE or HardwareBuffer.USAGE_GPU_COLOR_OUTPUT)

                texture = Texture.Builder()
                    .width(width)
                    .height(height)
                    .usage(Texture.Usage.COLOR_ATTACHMENT or Texture.Usage.SAMPLEABLE)
                    .sampler(Texture.Sampler.SAMPLER_EXTERNAL)
                    .format(Texture.InternalFormat.RGBA8)
                    .external()
                    .build(engine)

                texture!!.setExternalImage(engine, hardwareBuffer!!)
            } else {
                texture = Texture.Builder()
                    .width(width)
                    .height(height)
                    .levels(1)
                    .usage(Texture.Usage.COLOR_ATTACHMENT or Texture.Usage.SAMPLEABLE)
                    .format(Texture.InternalFormat.RGBA8)
                    .build(engine)
            }

            renderTarget = RenderTarget.Builder()
                .texture(RenderTarget.AttachmentPoint.COLOR, texture!!)
                .build(engine)

            offscreenView.renderTarget = renderTarget
            // Set the texture on the quad material.
            texturedMaterial.defaultInstance.setParameter("texture", texture!!,
                TextureSampler(TextureSampler.MinFilter.LINEAR, TextureSampler.MagFilter.LINEAR,
                               TextureSampler.WrapMode.CLAMP_TO_EDGE))
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
