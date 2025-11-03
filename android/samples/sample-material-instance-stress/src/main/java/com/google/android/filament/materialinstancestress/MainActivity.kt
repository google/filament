/*
 * Copyright (C) 2025 The Android Open Source Project
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

package com.google.android.filament.materialinstancestress

import android.animation.ValueAnimator
import android.app.Activity
import android.opengl.Matrix
import android.os.Bundle
import android.view.Choreographer
import android.view.Surface
import android.view.SurfaceView
import android.view.animation.LinearInterpolator

import com.google.android.filament.*
import com.google.android.filament.RenderableManager.*
import com.google.android.filament.VertexBuffer.*
import com.google.android.filament.android.DisplayHelper
import com.google.android.filament.android.FilamentHelper
import com.google.android.filament.android.UiHelper

import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.channels.Channels
import kotlin.math.cos
import kotlin.math.sin

class MainActivity : Activity() {
    // Make sure to initialize Filament first
    // This loads the JNI library needed by most API calls
    companion object {
        init {
            Filament.init()
        }

        private const val NUM_CUBES = 1000
        private const val GRID_SIZE = 10
        private const val SPACING = 2.5f
    }

    // The View we want to render into
    private lateinit var surfaceView: SurfaceView
    // UiHelper is provided by Filament to manage SurfaceView and SurfaceTexture
    private lateinit var uiHelper: UiHelper
    // DisplayHelper is provided by Filament to manage the display
    private lateinit var displayHelper: DisplayHelper
    // Choreographer is used to schedule new frames
    private lateinit var choreographer: Choreographer

    // Engine creates and destroys Filament resources
    // Each engine must be accessed from a single thread of your choosing
    // Resources cannot be shared across engines
    private lateinit var engine: Engine
    // A renderer instance is tied to a single surface (SurfaceView, TextureView, etc.)
    private lateinit var renderer: Renderer
    // A scene holds all the renderable, lights, etc. to be drawn
    private lateinit var scene: Scene
    // A view defines a viewport, a scene and a camera for rendering
    private lateinit var view: View
    // Should be pretty obvious :)
    private lateinit var camera: Camera

    private lateinit var material: Material
    private lateinit var vertexBuffer: VertexBuffer
    private lateinit var indexBuffer: IndexBuffer

    // Filament entity representing a renderable object
    @Entity private val renderables = IntArray(NUM_CUBES)
    private val materialInstances = arrayOfNulls<MaterialInstance>(NUM_CUBES)
    private val translationMatrices = Array(NUM_CUBES) { FloatArray(16) }

    @Entity private var light = 0

    // A swap chain is Filament's representation of a surface
    private var swapChain: SwapChain? = null

    private var cameraRadius = 42.7f // Initial distance (calculated from original 15Y, 40Z)
    private var cameraTheta = 0.0f  // Azimuth (horizontal) in radians
    private var cameraPhi = 0.358f  // Elevation (vertical) in radians (from original 15Y, 40Z)

    private var mLastX = 0.0f
    private var mLastY = 0.0f
    private val DRAG_SPEED = 0.005f

    // Performs the rendering and schedules new frames
    private val frameScheduler = FrameCallback()

    private val animator = ValueAnimator.ofFloat(0.0f, 360.0f)

    private val animatorListener = object : ValueAnimator.AnimatorUpdateListener {
        // Pre-allocate matrix to avoid GC churn
        private val transformMatrix = FloatArray(16)

        override fun onAnimationUpdate(a: ValueAnimator) {
            val tcm = engine.transformManager
            val time = a.animatedFraction // 0.0 to 1.0

            // Calculate the sine wave scale factor.
            // sin(time * 2 * PI) oscillates between -1.0 and 1.0.
            // We multiply by 0.5f (so it's -0.5 to 0.5) and add 1.0f.
            // This makes the final 'scale' oscillate between 0.5 and 1.5.
            val scale = 1.0f + sin(time * 2.0 * Math.PI).toFloat() * 0.5f

            val localScale = (scale - 0.5f) * 0.5f + 0.5f

            for (i in 0 until NUM_CUBES) {
                val inst = tcm.getInstance(renderables[i])
                val materialInst = materialInstances[i] ?: continue

                // Get the cube's base position from its stored matrix
                // The translation is in elements 12 (x), 13 (y), and 14 (z)
                val baseX = translationMatrices[i][12]
                val baseY = translationMatrices[i][13]
                val baseZ = translationMatrices[i][14]

                // Set the transformMatrix to a new translation matrix,
                // scaled from the center (0,0,0)
                Matrix.setIdentityM(transformMatrix, 0)
                Matrix.translateM(transformMatrix, 0,
                        baseX * scale,
                        baseY * scale,
                        baseZ * scale)
                Matrix.scaleM(transformMatrix, 0, localScale, localScale, localScale)

                tcm.setTransform(inst, transformMatrix)

                // Vary roughness
                val roughness = 0.5f + 0.5f * sin(time * 2.0 * Math.PI + i * 0.1).toFloat()
                materialInst.setParameter("roughness", roughness)

                // Vary color (using linear RGB)
                val r = 0.5f + 0.5f * cos(time * 2.0 * Math.PI + i * 0.5).toFloat()
                val g = 0.5f + 0.5f * sin(time * 2.0 * Math.PI + i * 0.3).toFloat()
                val b = 0.5f + 0.5f * cos(time * 2.0 * Math.PI + i * 0.1).toFloat()
                materialInst.setParameter("baseColor", Colors.RgbType.LINEAR, r, g, b)
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        surfaceView = SurfaceView(this)
        setContentView(surfaceView)

        surfaceView.setOnTouchListener(touchListener)
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
    }

    private fun setupView() {
        scene.skybox = Skybox.Builder().color(0.035f, 0.035f, 0.035f, 1.0f).build(engine)
        view.camera = camera
        view.scene = scene
    }

    private fun setupScene() {
        loadMaterial()
        createMesh()

        val tcm = engine.transformManager
        val center = (GRID_SIZE - 1) * 0.5f

        for (i in 0 until NUM_CUBES) {
            // Calculate grid position
            val x = (i % GRID_SIZE) - center
            val y = ((i / GRID_SIZE) % GRID_SIZE) - center
            val z = (i / (GRID_SIZE * GRID_SIZE)) - center

            val posX = x * SPACING
            val posY = y * SPACING
            val posZ = z * SPACING

            // Create material instance for this cube
            materialInstances[i] = material.createInstance()

            // Set initial parameters (will be overwritten by animator, but good practice)
            materialInstances[i]?.setParameter("baseColor", Colors.RgbType.SRGB, 1.0f, 0.85f, 0.57f)
            materialInstances[i]?.setParameter("metallic", 0.0f)
            materialInstances[i]?.setParameter("roughness", 0.3f)

            // Create the renderable entity
            renderables[i] = EntityManager.get().create()

            // Create the renderable component
            RenderableManager.Builder(1)
                .boundingBox(Box(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f))
                .geometry(0, PrimitiveType.TRIANGLES, vertexBuffer, indexBuffer, 0, 6 * 6)
                .material(0, materialInstances[i]!!)
                .build(engine, renderables[i])

            // Add to scene
            scene.addEntity(renderables[i])

            // Set its initial transform
            // Store the base translation matrix
            Matrix.setIdentityM(translationMatrices[i], 0)
            Matrix.translateM(translationMatrices[i], 0, posX, posY, posZ)

            // Get the transform component instance
            val inst = tcm.getInstance(renderables[i])
            tcm.setTransform(inst, translationMatrices[i])
        }

        // We now need a light, let's create a directional light
        light = EntityManager.get().create()

        val (r, g, b) = Colors.cct(5_500.0f)
        LightManager.Builder(LightManager.Type.DIRECTIONAL)
                .color(r, g, b)
                .intensity(110_000.0f)
                .direction(0.0f, -0.5f, -1.0f)
                .castShadows(true)
                .build(engine, light)

        scene.addEntity(light)

        camera.setExposure(16.0f, 1.0f / 125.0f, 100.0f)

        updateCamera()

        startAnimation()
    }

    private fun loadMaterial() {
        readUncompressedAsset("materials/lit.filamat").let {
            material = Material.Builder().payload(it, it.remaining()).build(engine)
        }
    }

    private fun createMesh() {
        val floatSize = 4
        val shortSize = 2
        val vertexSize = 3 * floatSize + 4 * floatSize

        // A vertex is a position + a tangent frame:
        // 3 floats for XYZ position, 4 floats for normal+tangents (quaternion)
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
                .put(Vertex(-1.0f, -1.0f, -1.0f, tfNZ))
                .put(Vertex(-1.0f,  1.0f, -1.0f, tfNZ))
                .put(Vertex( 1.0f,  1.0f, -1.0f, tfNZ))
                .put(Vertex( 1.0f, -1.0f, -1.0f, tfNZ))
                // Face +X
                .put(Vertex( 1.0f, -1.0f, -1.0f, tfPX))
                .put(Vertex( 1.0f,  1.0f, -1.0f, tfPX))
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
                .put(Vertex(-1.0f,  1.0f, -1.0f, tfNX))
                .put(Vertex(-1.0f, -1.0f, -1.0f, tfNX))
                // Face -Y
                .put(Vertex(-1.0f, -1.0f,  1.0f, tfNY))
                .put(Vertex(-1.0f, -1.0f, -1.0f, tfNY))
                .put(Vertex( 1.0f, -1.0f, -1.0f, tfNY))
                .put(Vertex( 1.0f, -1.0f,  1.0f, tfNY))
                // Face +Y
                .put(Vertex(-1.0f,  1.0f, -1.0f, tfPY))
                .put(Vertex(-1.0f,  1.0f,  1.0f, tfPY))
                .put(Vertex( 1.0f,  1.0f,  1.0f, tfPY))
                .put(Vertex( 1.0f,  1.0f, -1.0f, tfPY))
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

    private fun updateCamera() {
        // Calculate eye position from spherical coordinates
        val eyeY = cameraRadius * sin(cameraPhi)
        val horizontalRadius = cameraRadius * cos(cameraPhi)
        val eyeX = horizontalRadius * sin(cameraTheta)
        val eyeZ = horizontalRadius * cos(cameraTheta)

        // Point the camera at the center (0,0,0)
        camera.lookAt(eyeX.toDouble(), eyeY.toDouble(), eyeZ.toDouble(), // eye
                0.0, 0.0, 0.0, // center
                0.0, 1.0, 0.0) // up
    }

    private val touchListener = object : android.view.View.OnTouchListener {
        override fun onTouch(v: android.view.View?, event: android.view.MotionEvent?): Boolean {
            if (event == null) return false
            when (event.action) {
                android.view.MotionEvent.ACTION_DOWN -> {
                    mLastX = event.x
                    mLastY = event.y
                    return true
                }
                android.view.MotionEvent.ACTION_MOVE -> {
                    val dx = event.x - mLastX
                    val dy = event.y - mLastY

                    cameraTheta -= dx * DRAG_SPEED
                    cameraPhi += dy * DRAG_SPEED

                    // Clamp vertical angle to avoid flipping over
                    cameraPhi = cameraPhi.coerceIn(-1.5f, 1.5f)

                    updateCamera()

                    mLastX = event.x
                    mLastY = event.y
                    return true
                }
            }
            return v?.onTouchEvent(event) ?: false
        }
    }

    private fun startAnimation() {
        animator.interpolator = LinearInterpolator()
        animator.duration = 6000
        animator.repeatMode = ValueAnimator.RESTART
        animator.repeatCount = ValueAnimator.INFINITE
        animator.addUpdateListener(animatorListener)
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

        // Stop the animation and any pending frame
        choreographer.removeFrameCallback(frameScheduler)
        animator.cancel();

        // Always detach the surface before destroying the engine
        uiHelper.detach()

        // Cleanup all resources
        engine.destroyEntity(light)

        // Destroy all 1000 entities and material instances
        for (i in 0 until NUM_CUBES) {
            engine.destroyEntity(renderables[i])
            materialInstances[i]?.let { engine.destroyMaterialInstance(it) }
        }

        engine.destroyRenderer(renderer)
        engine.destroyVertexBuffer(vertexBuffer)
        engine.destroyIndexBuffer(indexBuffer)
        engine.destroyMaterial(material)
        engine.destroyView(view)
        engine.destroyScene(scene)
        engine.destroyCameraComponent(camera.entity)

        // Engine.destroyEntity() destroys Filament related resources only
        // (components), not the entity itself
        val entityManager = EntityManager.get()
        entityManager.destroy(light)
        for (entity in renderables) {
            entityManager.destroy(entity)
        }
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
            camera.setProjection(45.0, aspect, 0.1, 100.0, Camera.Fov.VERTICAL)

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
