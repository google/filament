/*
 * Copyright (C) 2019 The Android Open Source Project
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

package com.google.android.filament.streamtest

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.os.Bundle
import android.view.Choreographer
import android.view.Surface
import android.view.SurfaceView
import androidx.core.app.ActivityCompat

import com.google.android.filament.*
import com.google.android.filament.RenderableManager.*
import com.google.android.filament.VertexBuffer.*
import com.google.android.filament.android.UiHelper

import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.channels.Channels
import android.opengl.*
import android.os.Build
import android.view.MotionEvent
import androidx.annotation.RequiresApi
import com.google.android.filament.android.DisplayHelper
import com.google.android.filament.android.FilamentHelper


class MainActivity : Activity(), ActivityCompat.OnRequestPermissionsResultCallback {
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

    // This helper wraps the Android camera2 API and connects it to a Filament material.
    private lateinit var streamHelper: StreamHelper

    // This is the Filament camera, not the phone camera. :)
    private lateinit var camera: Camera

    // Other Filament objects:
    private lateinit var material: Material
    private lateinit var materialInstance: MaterialInstance
    private lateinit var vertexBuffer: VertexBuffer
    private lateinit var indexBuffer: IndexBuffer

    // Filament entity representing a renderable object
    @Entity private var renderable = 0
    @Entity private var light = 0

    // A swap chain is Filament's representation of a surface
    private var swapChain: SwapChain? = null

    // Performs the rendering and schedules new frames
    private val frameScheduler = FrameCallback()

    @RequiresApi(30)
    class Api30Impl {
        companion object {
            fun getDisplay(context: Context) = context.display!!
        }
    }

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

        @Suppress("deprecation")
        val display = if (Build.VERSION.SDK_INT >= 30) {
            Api30Impl.getDisplay(this)
        } else {
            windowManager.defaultDisplay!!
        }

        streamHelper = StreamHelper(engine, materialInstance, display)
        this.title = streamHelper.getTestName()
    }

    @SuppressLint("ClickableViewAccessibility")
    private fun setupSurfaceView() {
        uiHelper = UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK)
        uiHelper.renderCallback = SurfaceCallback()
        uiHelper.attachTo(surfaceView)

        surfaceView.setOnTouchListener { _, event ->
            when(event.action){
                MotionEvent.ACTION_DOWN -> {
                    streamHelper.nextTest()
                    this.title = streamHelper.getTestName()
                }
            }
            super.onTouchEvent(event)
        }
    }

    private fun setupFilament() {
        val eglContext = createEGLContext()
        engine = Engine.create(eglContext)
        renderer = engine.createRenderer()
        scene = engine.createScene()
        view = engine.createView()
        camera = engine.createCamera(engine.entityManager.create())
    }

    private fun setupView() {
        scene.skybox = Skybox.Builder().color(0.035f, 0.035f, 0.035f, 1.0f).build(engine)
        view.camera = camera
        view.scene = scene
    }

    private fun setupScene() {
        loadMaterial()
        setupMaterial()
        createMesh()

        // To create a renderable we first create a generic entity
        renderable = EntityManager.get().create()

        // We then create a renderable component on that entity
        // A renderable is made of several primitives; in this case we declare only 1
        // If we wanted each face of the cube to have a different material, we could
        // declare 6 primitives (1 per face) and give each of them a different material
        // instance, setup with different parameters
        RenderableManager.Builder(1)
                // Overall bounding box of the renderable
                .boundingBox(Box(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f))
                // Sets the mesh data of the first primitive, 6 faces of 6 indices each
                .geometry(0, PrimitiveType.TRIANGLES, vertexBuffer, indexBuffer, 0, 6 * 6)
                // Sets the material of the first primitive
                .material(0, materialInstance)
                .build(engine, renderable)

        // Add the entity to the scene to render it
        scene.addEntity(renderable)

        // We now need a light, let's create a directional light
        light = EntityManager.get().create()

        // Create a color from a temperature (5,500K)
        val (r, g, b) = Colors.cct(5_500.0f)
        LightManager.Builder(LightManager.Type.DIRECTIONAL)
                .color(r, g, b)
                // Intensity of the sun in lux on a clear day
                .intensity(110_000.0f)
                // The direction is normalized on our behalf
                .direction(0.0f, -0.5f, -1.0f)
                .castShadows(true)
                .build(engine, light)

        // Add the entity to the scene to light it
        scene.addEntity(light)

        // Set the exposure on the camera, this exposure follows the sunny f/16 rule
        // Since we've defined a light that has the same intensity as the sun, it
        // guarantees a proper exposure
        camera.setExposure(16.0f, 1.0f / 125.0f, 100.0f)

        // Move the camera back to see the object
        camera.lookAt(0.0, 0.0, 6.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)
    }

    private fun loadMaterial() {
        readUncompressedAsset("materials/lit.filamat").let {
            material = Material.Builder().payload(it, it.remaining()).build(engine)
        }
    }

    private fun setupMaterial() {
        materialInstance = material.createInstance()
        materialInstance.setParameter("baseColor", Colors.RgbType.SRGB, 1.0f, 0.85f, 0.57f)
        materialInstance.setParameter("roughness", 0.3f)
    }

    private fun createMesh() {
        val floatSize = 4
        val shortSize = 2
        // A vertex is a position + a tangent frame:
        // 3 floats for XYZ position, 4 floats for normal+tangents (quaternion)
        val vertexSize = 3 * floatSize + 4 * floatSize

        // Define a vertex and a function to put a vertex in a ByteBuffer
        @Suppress("ArrayInDataClass")
        data class Vertex(val x: Float, val y: Float, val z: Float, val tangents: FloatArray)
        fun ByteBuffer.put(v: Vertex): ByteBuffer {
            putFloat(v.x)
            putFloat(v.y)
            putFloat(v.z)
            v.tangents.forEach { putFloat(it) }
            return this
        }

        // 6 faces, 4 vertices per face
        val vertexCount = 6 * 4

        // Create tangent frames, one per face
        val tfPX = FloatArray(4)
        val tfNX = FloatArray(4)
        val tfPY = FloatArray(4)
        val tfNY = FloatArray(4)
        val tfPZ = FloatArray(4)
        val tfNZ = FloatArray(4)

        MathUtils.packTangentFrame( 0.0f,  1.0f, 0.0f, 0.0f, 0.0f, -1.0f,  1.0f,  0.0f,  0.0f, tfPX)
        MathUtils.packTangentFrame( 0.0f,  1.0f, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f,  0.0f,  0.0f, tfNX)
        MathUtils.packTangentFrame(-1.0f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,  0.0f,  1.0f,  0.0f, tfPY)
        MathUtils.packTangentFrame(-1.0f,  0.0f, 0.0f, 0.0f, 0.0f,  1.0f,  0.0f, -1.0f,  0.0f, tfNY)
        MathUtils.packTangentFrame( 0.0f,  1.0f, 0.0f, 1.0f, 0.0f,  0.0f,  0.0f,  0.0f,  1.0f, tfPZ)
        MathUtils.packTangentFrame( 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,  0.0f,  0.0f,  0.0f, -1.0f, tfNZ)

        val vertexData = ByteBuffer.allocate(vertexCount * vertexSize)
                // It is important to respect the native byte order
                .order(ByteOrder.nativeOrder())
                // Face -Z
                .put(Vertex(-1.5f, -1.5f, -1.0f, tfNZ))
                .put(Vertex(-1.5f,  1.5f, -1.0f, tfNZ))
                .put(Vertex( 1.5f,  1.5f, -1.0f, tfNZ))
                .put(Vertex( 1.5f, -1.5f, -1.0f, tfNZ))
                // Face +X
                .put(Vertex( 1.5f, -1.5f, -1.0f, tfPX))
                .put(Vertex( 1.5f,  1.5f, -1.0f, tfPX))
                .put(Vertex( 1.0f,  1.0f,  1.0f, tfPX))
                .put(Vertex( 1.0f, -1.0f,  1.0f, tfPX))
                // Face +Z
                .put(Vertex(-1.0f, -1.0f,  1.0f, tfPZ))
                .put(Vertex( 1.0f, -1.0f,  1.0f, tfPZ))
                .put(Vertex( 1.0f,  1.0f,  1.0f, tfPZ))
                .put(Vertex(-1.0f,  1.0f,  1.0f, tfPZ))
                // Face -X
                .put(Vertex(-1.0f, -1.0f,  1.0f, tfNX))
                .put(Vertex(-1.0f,  1.0f,  1.0f, tfNX))
                .put(Vertex(-1.5f,  1.5f, -1.0f, tfNX))
                .put(Vertex(-1.5f, -1.5f, -1.0f, tfNX))
                // Face -Y
                .put(Vertex(-1.0f, -1.0f,  1.0f, tfNY))
                .put(Vertex(-1.5f, -1.5f, -1.0f, tfNY))
                .put(Vertex( 1.5f, -1.5f, -1.0f, tfNY))
                .put(Vertex( 1.0f, -1.0f,  1.0f, tfNY))
                // Face +Y
                .put(Vertex(-1.5f,  1.5f, -1.0f, tfPY))
                .put(Vertex(-1.0f,  1.0f,  1.0f, tfPY))
                .put(Vertex( 1.0f,  1.0f,  1.0f, tfPY))
                .put(Vertex( 1.5f,  1.5f, -1.0f, tfPY))
                // Make sure the cursor is pointing in the right place in the byte buffer
                .flip()

        // Declare the layout of our mesh
        vertexBuffer = VertexBuffer.Builder()
                .bufferCount(1)
                .vertexCount(vertexCount)
                // Because we interleave position and color data we must specify offset and stride
                // We could use de-interleaved data by declaring two buffers and giving each
                // attribute a different buffer index
                .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT3, 0,             vertexSize)
                .attribute(VertexAttribute.TANGENTS, 0, AttributeType.FLOAT4, 3 * floatSize, vertexSize)
                .build(engine)

        // Feed the vertex data to the mesh
        // We only set 1 buffer because the data is interleaved
        vertexBuffer.setBufferAt(engine, 0, vertexData)

        // Create the indices
        val indexData = ByteBuffer.allocate(6 * 2 * 3 * shortSize)
                .order(ByteOrder.nativeOrder())
        repeat(6) {
            val i = (it * 4).toShort()
            indexData
                    .putShort(i).putShort((i + 1).toShort()).putShort((i + 2).toShort())
                    .putShort(i).putShort((i + 2).toShort()).putShort((i + 3).toShort())
        }
        indexData.flip()

        // 6 faces, 2 triangles per face,
        indexBuffer = IndexBuffer.Builder()
                .indexCount(vertexCount * 2)
                .bufferType(IndexBuffer.Builder.IndexType.USHORT)
                .build(engine)
        indexBuffer.setBuffer(engine, indexData)
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

        // Cleanup all resources
        engine.destroyEntity(light)
        engine.destroyEntity(renderable)
        engine.destroyRenderer(renderer)
        engine.destroyVertexBuffer(vertexBuffer)
        engine.destroyIndexBuffer(indexBuffer)
        engine.destroyMaterialInstance(materialInstance)
        engine.destroyMaterial(material)
        engine.destroyView(view)
        engine.destroyScene(scene)
        engine.destroyCameraComponent(camera.entity)

        // Engine.destroyEntity() destroys Filament related resources only
        // (components), not the entity itself
        val entityManager = EntityManager.get()
        entityManager.destroy(light)
        entityManager.destroy(renderable)
        entityManager.destroy(camera.entity)

        // Destroying the engine will free up any resource you may have forgotten
        // to destroy, but it's recommended to do the cleanup properly
        engine.destroy()
    }

    inner class FrameCallback : Choreographer.FrameCallback {
        override fun doFrame(frameTimeNanos: Long) {
            // Schedule the next frame
            choreographer.postFrameCallback(this)

            // This check guarantees that we have a swap chain
            if (uiHelper.isReadyToRender) {

                // If beginFrame() returns false you should skip the frame
                // This means you are sending frames too quickly to the GPU
                if (renderer.beginFrame(swapChain!!, frameTimeNanos)) {
                    streamHelper.repaintCanvas()
                    materialInstance.setParameter("uvOffset", streamHelper.uvOffset)

                    renderer.render(view)
                    renderer.endFrame()
                }
            }
        }
    }

    inner class SurfaceCallback : UiHelper.RendererCallback {
        override fun onNativeWindowChanged(surface: Surface) {
            swapChain?.let { engine.destroySwapChain(it) }
            swapChain = engine.createSwapChain(surface)
            displayHelper.attach(renderer, surfaceView.display)
        }

        override fun onDetachedFromSurface() {
            displayHelper.detach()
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
            val aspect = width.toDouble() / height.toDouble()
            camera.setProjection(45.0, aspect, 0.1, 20.0, Camera.Fov.VERTICAL)

            view.viewport = Viewport(0, 0, width, height)

            FilamentHelper.synchronizePendingFrames(engine)
        }
    }

    private fun readUncompressedAsset(@Suppress("SameParameterValue") assetName: String): ByteBuffer {
        assets.openFd(assetName).use { fd ->
            val input = fd.createInputStream()
            val dst = ByteBuffer.allocate(fd.length.toInt())

            val src = Channels.newChannel(input)
            src.read(dst)
            src.close()

            return dst.apply { rewind() }
        }
    }

    private fun createEGLContext(): EGLContext {
        // Providing this constant here (rather than using EGL_OPENGL_ES3_BIT ) allows us to use a lower target API for this project.
        val kEGLOpenGLES3Bit = 64

        val shareContext = EGL14.EGL_NO_CONTEXT
        val display = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY)

        val minorMajor: IntArray = IntArray(2)
        EGL14.eglInitialize(display, minorMajor, 0, minorMajor, 1)
        val configs = arrayOfNulls<EGLConfig>(1)
        val numConfig = intArrayOf(0)
        val attribs = intArrayOf(EGL14.EGL_RENDERABLE_TYPE, kEGLOpenGLES3Bit, EGL14.EGL_NONE)
        EGL14.eglChooseConfig(display, attribs, 0, configs, 0, 1, numConfig, 0)

        val contextAttribs = intArrayOf(EGL14.EGL_CONTEXT_CLIENT_VERSION, 3, EGL14.EGL_NONE)
        val context = EGL14.eglCreateContext(display, configs[0], shareContext, contextAttribs, 0)

        val surfaceAttribs = intArrayOf(EGL14.EGL_WIDTH, 1, EGL14.EGL_HEIGHT, 1, EGL14.EGL_NONE)

        val surface = EGL14.eglCreatePbufferSurface(display, configs[0], surfaceAttribs, 0)

        check(EGL14.eglMakeCurrent(display, surface, surface, context)) { "Error making GL context." }
        return context
    }
}
