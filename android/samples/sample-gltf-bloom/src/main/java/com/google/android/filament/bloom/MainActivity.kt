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

package com.google.android.filament.bloom

import android.animation.ValueAnimator
import android.app.Activity
import android.os.Bundle
import android.view.Choreographer
import android.view.Surface
import android.view.SurfaceView
import android.view.animation.LinearInterpolator

import com.google.android.filament.*
import com.google.android.filament.android.DisplayHelper
import com.google.android.filament.android.UiHelper
import com.google.android.filament.gltfio.*
import com.google.android.filament.utils.*

import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.channels.Channels

import kotlin.math.*

data class Framebuffer(
    val camera: Camera,
    val view: View,
    val scene: Scene,
    var color: Texture? = null,
    var depth: Texture? = null,
    var target: RenderTarget? = null)

class MainActivity : Activity() {

    companion object {
        init {
            Utils.init()
        }
    }

    private lateinit var surfaceView: SurfaceView
    private lateinit var uiHelper: UiHelper
    private lateinit var choreographer: Choreographer

    private lateinit var engine: Engine
    private lateinit var renderer: Renderer

    private lateinit var assetLoader: AssetLoader
    private lateinit var filamentAsset: FilamentAsset

    private lateinit var finalScene: Scene
    private lateinit var finalView: View
    private lateinit var finalCamera: Camera

    private val kGaussianSampleCount = 9
    private val kGaussianFilterSize = 1 + 2 * (kGaussianSampleCount - 1)
    private val kGaussianWeights = FloatArray(kGaussianSampleCount * 4 * 2)

    @Entity private var finalQuad = 0
    @Entity private var hblurQuad = 0
    @Entity private var vblurQuad = 0

    private lateinit var primary: Framebuffer
    private lateinit var hblur: Framebuffer
    private lateinit var vblur: Framebuffer

    private var quadVertBufferGpu: VertexBuffer? = null
    private var quadIndxBufferGpu: IndexBuffer? = null

    enum class ImageOp { MIX, HBLUR, VBLUR}

    private lateinit var mixMaterial: Material
    private lateinit var blurMaterial: Material

    @Entity private var light = 0

    private var swapChain: SwapChain? = null

    private val frameScheduler = FrameCallback()

    private val animator = ValueAnimator.ofFloat(0.0f, (2.0 * PI).toFloat())

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        computeGaussianWeights()

        surfaceView = SurfaceView(this)
        setContentView(surfaceView)

        choreographer = Choreographer.getInstance()

        uiHelper = UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK)
        uiHelper.renderCallback = SurfaceCallback()
        uiHelper.setDesiredSize(720, 1280)
        uiHelper.attachTo(surfaceView)

        engine = Engine.create()
        renderer = engine.createRenderer()

        primary = Framebuffer(engine.createCamera(), engine.createView(), engine.createScene())
        primary.view.setName("primary")
        primary.view.scene = primary.scene
        primary.view.camera = primary.camera
        primary.view.dithering = View.Dithering.NONE
        primary.view.toneMapping = View.ToneMapping.LINEAR

        hblur = Framebuffer(engine.createCamera(), engine.createView(), engine.createScene())
        hblur.view.setName("hblur")
        hblur.view.scene = hblur.scene
        hblur.view.camera = hblur.camera
        hblur.view.sampleCount = 1
        hblur.view.isPostProcessingEnabled = false

        vblur = Framebuffer(engine.createCamera(), engine.createView(), engine.createScene())
        vblur.view.setName("vblur")
        vblur.view.scene = vblur.scene
        vblur.view.camera = vblur.camera
        vblur.view.sampleCount = 1
        vblur.view.isPostProcessingEnabled = false

        finalScene = engine.createScene()
        finalView = engine.createView()
        finalView.setName("final")
        finalCamera = engine.createCamera()
        finalCamera.setExposure(16.0f, 1.0f / 125.0f, 100.0f)
        finalView.scene = finalScene
        finalView.camera = finalCamera
        finalView.sampleCount = 1

        // Materials
        // ---------

        readUncompressedAsset("materials/blur.filamat").let {
            blurMaterial = Material.Builder().payload(it, it.remaining()).build(engine)
        }

        readUncompressedAsset("materials/mix.filamat").let {
            mixMaterial = Material.Builder().payload(it, it.remaining()).build(engine)
        }

        // IndirectLight and SkyBox
        // ------------------------

        val ibl = "venetian_crossroads_2k"

        readUncompressedAsset("envs/$ibl/${ibl}_ibl.ktx").let {
            primary.scene.indirectLight = KtxLoader.createIndirectLight(engine, it, KtxLoader.Options())
            primary.scene.indirectLight!!.intensity = 50_000.0f
        }

        readUncompressedAsset("envs/$ibl/${ibl}_skybox.ktx").let {
            primary.scene.skybox = KtxLoader.createSkybox(engine, it, KtxLoader.Options())
        }

        val theta = PI * 2.7
        val distance = 5.0
        primary.camera.lookAt(
                cos(theta) * distance , 0.0, sin(theta) * distance ,
                0.0, -0.1, 0.0,
                0.0, 1.0, 0.0)

        // glTF Entities, Textures, and Materials
        // --------------------------------------

        assetLoader = AssetLoader(engine, MaterialProvider(engine), EntityManager.get())

        filamentAsset = assets.open("models/lucy.glb").use { input ->
            val bytes = ByteArray(input.available())
            input.read(bytes)
            assetLoader.createAssetFromBinary(ByteBuffer.wrap(bytes))!!
        }

        // Since this is a GLB file, the ResourceLoader does not need any additional files, and
        // we can destroy it immediately.
        ResourceLoader(engine).loadResources(filamentAsset).destroy()

        primary.scene.addEntities(filamentAsset.entities)
        val tm = engine.transformManager
        tm.setTransform(tm.getInstance(filamentAsset.root),
                floatArrayOf(
                        1.0f,  0.0f, 0.0f, 0.0f,
                        0.0f,  1.0f, 0.0f, 0.0f,
                        0.0f,  0.0f, 1.0f, 0.0f,
                        0.0f, -1.7f, 0.0f, 1.0f
                ))

        val rm = engine.renderableManager
        for (entity in filamentAsset.entities) {
            val instance = rm.getInstance(entity)
            val primCount = rm.getPrimitiveCount(instance)
            for (primIndex in 0 until primCount) {
                val mi = rm.getMaterialInstanceAt(instance, primIndex)
                mi.setParameter("roughnessFactor", 0.1f)
                mi.setParameter("baseColorFactor", 0.5f, 0.5f, 0.0f, 1.0f)
            }
        }

        // Light Sources
        // ----------------------

        light = EntityManager.get().create()

        val (r, g, b) = Colors.cct(6_500.0f)
        LightManager.Builder(LightManager.Type.DIRECTIONAL)
                .color(r, g, b)
                .intensity(300_000.0f)
                .direction(0.0f, -1.0f, 0.0f)
                .castShadows(true)
                .build(engine, light)

        primary.scene.addEntity(light)

        // Start Animation
        // ---------------

        animator.interpolator = LinearInterpolator()
        animator.duration = 1_000_000
        animator.repeatMode = ValueAnimator.RESTART
        animator.repeatCount = ValueAnimator.INFINITE
        animator.addUpdateListener { a ->
            val v = 5.0f * PI.toFloat() * (a.animatedValue as Float)

            tm.setTransform(tm.getInstance(filamentAsset.root),
                    floatArrayOf(
                            cos(v),  0.0f, -sin(v), 0.0f,
                            0.0f,    1.0f,  0.0f,   0.0f,
                            sin(v),  0.0f,  cos(v), 0.0f,
                            0.0f,   -1.7f,  0.0f,   1.0f
                    ))


            // The lucy asset does not have animation but we invoke the asset animator for demonstration purposes.
            if (filamentAsset.animator.animationCount > 0) {
                val elapsedTimeInSeconds = a.currentPlayTime.toFloat() / 1000.0f
                filamentAsset.animator.applyAnimation(0, elapsedTimeInSeconds)
                filamentAsset.animator.updateBoneMatrices()
            }
        }
        animator.start()
    }

    private fun initRenderTargets() {
        val width = surfaceView.width
        val height = surfaceView.height
        android.util.Log.i("gltf-bloom", "RenderTarget objects are $width x $height.")

        primary.color = Texture.Builder()
                .width(width).height(height).levels(1)
                .usage(Texture.Usage.COLOR_ATTACHMENT or Texture.Usage.SAMPLEABLE)
                .format(Texture.InternalFormat.RGBA16F)
                .build(engine)

        primary.depth = Texture.Builder()
                .width(width).height(height).levels(1)
                .usage(Texture.Usage.DEPTH_ATTACHMENT)
                .format(Texture.InternalFormat.DEPTH24)
                .build(engine)

        primary.target = RenderTarget.Builder()
                .texture(RenderTarget.AttachmentPoint.COLOR, primary.color)
                .texture(RenderTarget.AttachmentPoint.DEPTH, primary.depth)
                .build(engine)

        hblur.color = Texture.Builder()
                .width(width).height(height).levels(1)
                .usage(Texture.Usage.COLOR_ATTACHMENT or Texture.Usage.SAMPLEABLE)
                .format(Texture.InternalFormat.RGBA16F)
                .build(engine)

        hblur.target = RenderTarget.Builder()
                .texture(RenderTarget.AttachmentPoint.COLOR, hblur.color)
                .build(engine)

        vblur.color = Texture.Builder()
                .width(width).height(height).levels(1)
                .usage(Texture.Usage.COLOR_ATTACHMENT or Texture.Usage.SAMPLEABLE)
                .format(Texture.InternalFormat.RGBA16F)
                .build(engine)

        vblur.target = RenderTarget.Builder()
                .texture(RenderTarget.AttachmentPoint.COLOR, vblur.color)
                .build(engine)

        hblurQuad = createQuad(engine, ImageOp.HBLUR, primary.color!!)
        vblurQuad = createQuad(engine, ImageOp.VBLUR, hblur.color!!)
        finalQuad = createQuad(engine, ImageOp.MIX, primary.color!!, vblur.color)

        hblur.scene.addEntity(hblurQuad)
        vblur.scene.addEntity(vblurQuad)
        finalScene.addEntity(finalQuad)

        primary.view.viewport = Viewport(0, 0, width, height)
        primary.view.renderTarget = primary.target

        hblur.view.viewport = Viewport(0, 0, width, height)
        hblur.view.renderTarget = hblur.target
        hblur.camera.setProjection(Camera.Projection.ORTHO, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0)

        vblur.view.viewport = Viewport(0, 0, width, height)
        vblur.view.renderTarget = vblur.target
        vblur.camera.setProjection(Camera.Projection.ORTHO, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0)

        finalView.viewport = Viewport(0, 0, width, height)
        finalCamera.setProjection(Camera.Projection.ORTHO, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0)
    }

    @Entity private fun createQuad(engine: Engine, op: ImageOp, primary: Texture, secondary: Texture? = null): Int {

        if (quadVertBufferGpu == null) {

            val vb = VertexBuffer.Builder()
                    .vertexCount(4)
                    .bufferCount(1)
                    .attribute(VertexBuffer.VertexAttribute.POSITION, 0, VertexBuffer.AttributeType.FLOAT2, 0, 16)
                    .attribute(VertexBuffer.VertexAttribute.UV0, 0, VertexBuffer.AttributeType.FLOAT2, 8, 16)
                    .build(engine)

            data class Vertex(val x: Float, val y: Float, val u: Float, val v: Float)
            fun ByteBuffer.put(v: Vertex): ByteBuffer {
                putFloat(v.x)
                putFloat(v.y)
                putFloat(v.u)
                putFloat(v.v)
                return this
            }

            val quadVertexBuffer = ByteBuffer.allocateDirect(16 * 4)
                    .order(ByteOrder.nativeOrder())
                    .put(Vertex(0f, 0f, 0f, 0f))
                    .put(Vertex(1f, 0f, 1f, 0f))
                    .put(Vertex(0f, 1f, 0f, 1f))
                    .put(Vertex(1f, 1f, 1f, 1f))
                    .flip()

            vb.setBufferAt(engine, 0, quadVertexBuffer)

            quadVertBufferGpu = vb
        }
        val vb = quadVertBufferGpu!!

        ////////////////////////////////////////////////////////////////////

        if (quadIndxBufferGpu == null) {

            val ib = IndexBuffer.Builder()
                    .indexCount(6)
                    .bufferType(IndexBuffer.Builder.IndexType.USHORT)
                    .build(engine)

            data class Triangle(val x: Short, val y: Short, val z: Short)
            fun ByteBuffer.put(t: Triangle): ByteBuffer {
                putShort(t.x)
                putShort(t.y)
                putShort(t.z)
                return this
            }

            val buf = ByteBuffer.allocateDirect(6 * 2)
                    .order(ByteOrder.LITTLE_ENDIAN)
                    .put(Triangle(2, 1, 0))
                    .put(Triangle(1, 2, 3))
                    .flip()

            ib.setBuffer(engine, buf)
            quadIndxBufferGpu = ib
        }
        val ib = quadIndxBufferGpu!!

        ////////////////////////////////////////////////////////////////////

        val sampler = TextureSampler(TextureSampler.MinFilter.LINEAR, TextureSampler.MagFilter.LINEAR, TextureSampler.WrapMode.CLAMP_TO_EDGE)

        val material = when (op) {
            ImageOp.HBLUR -> {
                // Extract the first half of the weights array for the horizontal pass.
                this.blurMaterial.createInstance().apply {
                    setParameter("weights", MaterialInstance.FloatElement.FLOAT4, kGaussianWeights, 0, kGaussianSampleCount)
                    setParameter("passIndex", 0)
                }
            }
            ImageOp.VBLUR -> {
                // Extract the second half of the weights array for the vertical pass.
                this.blurMaterial.createInstance().apply {
                    setParameter("weights", MaterialInstance.FloatElement.FLOAT4, kGaussianWeights, kGaussianSampleCount, kGaussianSampleCount)
                    setParameter("passIndex", 1)
                }
            }
            ImageOp.MIX -> {
                this.mixMaterial.createInstance().apply {
                    setParameter("secondary", secondary!!, sampler)
                }
            }
        }

        material.setParameter("color", primary, sampler)

        val entity = EntityManager.get().create()

        RenderableManager.Builder(1)
                .boundingBox(Box(0.0f, 0.0f, 0.0f, 9000.0f, 9000.0f, 9000.0f))
                .geometry(0, RenderableManager.PrimitiveType.TRIANGLES, vb, ib)
                .material(0, material)
                .build(engine, entity)

        return entity
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

        // Stop the animation and any pending frame
        choreographer.removeFrameCallback(frameScheduler)
        animator.cancel()

        // Always detach the surface before destroying the engine
        uiHelper.detach()

        assetLoader.destroyAsset(filamentAsset)
        assetLoader.destroy()

        engine.destroyEntity(light)
        engine.destroyRenderer(renderer)
        engine.destroyView(finalView)
        engine.destroyScene(finalScene)
        engine.destroyCamera(finalCamera)

        // Engine.destroyEntity() destroys Filament related resources only
        // (components), not the entity itself
        val entityManager = EntityManager.get()
        entityManager.destroy(light)

        // Destroying the engine will free up any resource you may have forgotten
        // to destroy, but it's recommended to do the cleanup properly
        engine.destroy()
    }

    inner class FrameCallback : Choreographer.FrameCallback {
        override fun doFrame(frameTimeNanos: Long) {
            choreographer.postFrameCallback(this)

            if (uiHelper.isReadyToRender) {
                if (renderer.beginFrame(swapChain!!, frameTimeNanos)) {
                    renderer.render(primary.view)
                    renderer.render(hblur.view)
                    renderer.render(vblur.view)
                    renderer.render(finalView)
                    renderer.endFrame()
                }
            }
        }
    }

    inner class SurfaceCallback : UiHelper.RendererCallback {
        override fun onNativeWindowChanged(surface: Surface) {
            swapChain?.let { engine.destroySwapChain(it) }
            swapChain = engine.createSwapChain(surface)
            renderer.setDisplayInfo(DisplayHelper.getDisplayInfo(surfaceView.display, Renderer.DisplayInfo()))
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

            // Lazily create RenderTarget objects, now that we know the size of the view.
            if (primary.color == null) {
                initRenderTargets()
            }

            val aspect = width.toDouble() / height.toDouble()
            primary.camera.setProjection(45.0, aspect, 0.1, 20.0, Camera.Fov.VERTICAL)

            primary.view.viewport = Viewport(0, 0, width, height)

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

    private fun computeGaussianWeights() {

        val filter = fun(t0: Float): Float {
            val t = t0 / 2.0f
            if (t >= 1.0) return 0.0f
            val scale = 1.0f / sqrt(0.5f * PI.toFloat())
            return exp(-2.0f * t * t) * scale
        }

        val pixelWidth = 2.0f / kGaussianFilterSize.toFloat()
        var sum = 0.0f

        repeat (kGaussianSampleCount) { s ->

            // Determine which two texels to sample from.
            val i: Int
            val j: Int
            when {
                s < kGaussianSampleCount/ 2 -> {
                    i = s * 2
                    j = i + 1
                }
                s == kGaussianSampleCount/ 2 -> {
                    j = s * 2
                    i = j
                }
                else -> {
                    j = s * 2
                    i = j - 1
                }
            }

            // Determine the normalized (x,y) along the Gaussian curve for each of the two samples.
            val weighti = filter(abs(-1.0f + pixelWidth / 2.0f + pixelWidth * i))
            val weightj = filter(abs(-1.0f + pixelWidth / 2.0f + pixelWidth * j))
            val offseti = i - (kGaussianFilterSize - 1) / 2

            // Leverage hardware interpolation by sampling between the texel centers.
            // Nudge the left sample rightward by an amount proportional to its weight.
            val offset = offseti + weightj / (weighti + weightj)
            val weight = weighti + weightj

            kGaussianWeights[s * 4 + 0] = weight
            kGaussianWeights[s * 4 + 1] = offset
            kGaussianWeights[s * 4 + 2] = 0.0f

            kGaussianWeights[s * 4 + kGaussianSampleCount * 4 + 0] = weight
            kGaussianWeights[s * 4 + kGaussianSampleCount * 4 + 2] = offset
            kGaussianWeights[s * 4 + kGaussianSampleCount * 4 + 1] = 0.0f

            sum += kGaussianWeights[s * 4]
        }

        repeat (kGaussianSampleCount) { s ->
            kGaussianWeights[s * 4 + 0] /= sum
            kGaussianWeights[s * 4 + kGaussianSampleCount * 4 + 0] /= sum
        }

    }
}
