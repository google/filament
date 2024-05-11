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

package com.google.android.filament.livewallpaper

import android.animation.ValueAnimator
import android.app.Service
import android.content.Context
import android.graphics.Color
import android.graphics.PixelFormat
import android.os.Build
import android.service.wallpaper.WallpaperService
import android.view.Choreographer
import android.view.Surface
import android.view.SurfaceHolder
import android.view.WindowManager
import android.view.animation.LinearInterpolator
import androidx.annotation.RequiresApi
import com.google.android.filament.*
import com.google.android.filament.android.DisplayHelper
import com.google.android.filament.android.FilamentHelper
import com.google.android.filament.android.UiHelper

class FilamentLiveWallpaper : WallpaperService() {
    // Make sure to initialize Filament first
    // This loads the JNI library needed by most API calls
    companion object {
        init {
            Filament.init()
        }
    }

    override fun onCreateEngine(): Engine {
        return FilamentWallpaperEngine()
    }

    private inner class FilamentWallpaperEngine : Engine() {

        // UiHelper is provided by Filament to manage SurfaceHolder
        private lateinit var uiHelper: UiHelper
        // DisplayHelper is provided by Filament to manage the display
        private lateinit var displayHelper: DisplayHelper
        // Choreographer is used to schedule new frames
        private lateinit var choreographer: Choreographer

        // Engine creates and destroys Filament resources
        // Each engine must be accessed from a single thread of your choosing
        // Resources cannot be shared across engines
        private lateinit var engine: com.google.android.filament.Engine
        // A renderer instance is tied to a single surface (SurfaceView, TextureView, etc.)
        private lateinit var renderer: Renderer
        // A scene holds all the renderable, lights, etc. to be drawn
        private lateinit var scene: Scene
        // A view defines a viewport, a scene and a camera for rendering
        private lateinit var view: View
        // Should be pretty obvious :)
        private lateinit var camera: Camera

        // A swap chain is Filament's representation of a surface
        private var swapChain: SwapChain? = null

        // Performs the rendering and schedules new frames
        private val frameScheduler = FrameCallback()

        // We'll use this ValueAnimator to smoothly cycle the background between hues.
        private val animator = ValueAnimator.ofFloat(0.0f, 360.0f)

        override fun onCreate(surfaceHolder: SurfaceHolder) {
            super.onCreate(surfaceHolder)
            surfaceHolder.setSizeFromLayout()
            surfaceHolder.setFormat(PixelFormat.RGBA_8888)

            choreographer = Choreographer.getInstance()

            displayHelper = DisplayHelper(this@FilamentLiveWallpaper)

            setupUiHelper()
            setupFilament()
            setupView()
            setupScene()
        }

        private fun setupUiHelper() {
            uiHelper = UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK)
            uiHelper.renderCallback = SurfaceCallback()
            uiHelper.attachTo(surfaceHolder)
        }

        private fun setupFilament() {
            engine = com.google.android.filament.Engine.create()
            renderer = engine.createRenderer()
            scene = engine.createScene()
            view = engine.createView()
            camera = engine.createCamera(engine.entityManager.create())
        }

        private fun setupView() {
            scene.skybox = Skybox.Builder().build(engine)

            // NOTE: Try to disable post-processing (tone-mapping, etc.) to see the difference
            // view.isPostProcessingEnabled = false

            // Tell the view which camera we want to use
            view.camera = camera

            // Tell the view which scene we want to render
            view.scene = scene
        }

        private fun setupScene() {
            // Set the exposure on the camera, this exposure follows the sunny f/16 rule
            camera.setExposure(16.0f, 1.0f / 125.0f, 100.0f)

            startAnimation()
        }

        private fun startAnimation() {
            // Animate the color of the Skybox.
            animator.interpolator = LinearInterpolator()
            animator.duration = 10000
            animator.repeatMode = ValueAnimator.RESTART
            animator.repeatCount = ValueAnimator.INFINITE
            animator.addUpdateListener { a ->
                val hue = a.animatedValue as Float
                val color = Color.HSVToColor(floatArrayOf(hue, 1.0f, 1.0f))
                scene.skybox?.setColor(floatArrayOf(
                        Color.red(color)   / 255.0f,
                        Color.green(color) / 255.0f,
                        Color.blue(color)  / 255.0f,
                        1.0f))
            }
            animator.start()
        }

        override fun onVisibilityChanged(visible: Boolean) {
            super.onVisibilityChanged(visible)
            if (visible) {
                choreographer.postFrameCallback(frameScheduler)
                animator.start()
            } else {
                choreographer.removeFrameCallback(frameScheduler)
                animator.cancel()
            }
        }

        override fun onDestroy() {
            super.onDestroy()

            // Stop the animation and any pending frame
            choreographer.removeFrameCallback(frameScheduler)
            animator.cancel()

            // Always detach the surface before destroying the engine
            uiHelper.detach()

            // Cleanup all resources
            engine.destroyRenderer(renderer)
            engine.destroyView(view)
            engine.destroyScene(scene)
            engine.destroyCameraComponent(camera.entity)
            EntityManager.get().destroy(camera.entity)

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

                @Suppress("deprecation")
                val display = if (Build.VERSION.SDK_INT >= 30) {
                    Api30Impl.getDisplay(displayContext!!)
                } else {
                    (getSystemService(Service.WINDOW_SERVICE) as WindowManager).defaultDisplay
                }

                displayHelper.attach(renderer, display)
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
    }

    @RequiresApi(30)
    class Api30Impl {
        companion object {
            fun getDisplay(context: Context) = context.display!!
        }
    }
}
