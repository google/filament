/*
 * Copyright (C) 2015 The Android Open Source Project
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

//! \file

#ifndef TNT_FILAMENT_RENDERER_H
#define TNT_FILAMENT_RENDERER_H

#include <filament/FilamentAPI.h>
#include <filament/Viewport.h>

#include <utils/compiler.h>

#include <stdint.h>

namespace filament {

class Engine;
class SwapChain;
class View;

namespace backend {
class PixelBufferDescriptor;
} // namespace backend

/**
 * A Renderer instance represents an operating system's window.
 *
 * Typically, applications create a Renderer per window. The Renderer generates drawing commands
 * for the render thread and manages frame latency.
 *
 * A Renderer generates drawing commands from a View, itself containing a Scene description.
 *
 * Creation and Destruction
 * ========================
 *
 * A Renderer is created using Engine.createRenderer() and destroyed using
 * Engine.destroy(const Renderer*).
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * #include <filament/Renderer.h>
 * #include <filament/Engine.h>
 * using namespace filament;
 *
 * Engine* engine = Engine::create();
 *
 * Renderer* renderer = engine->createRenderer();
 * engine->destroy(&renderer);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @see Engine, View
 */
class UTILS_PUBLIC Renderer : public FilamentAPI {
public:
     /**
      * Get the Engine that created this Renderer.
      *
      * @return A pointer to the Engine instance this Renderer is associated to.
      */
    Engine* getEngine() noexcept;

    /**
     * Get the Engine that created this Renderer.
     *
     * @return A constant pointer to the Engine instance this Renderer is associated to.
     */
    inline Engine const* getEngine() const noexcept {
        return const_cast<Renderer *>(this)->getEngine();
    }

    /**
     * Render a View into this renderer's window.
     *
     * This is filament main rendering method, most of the CPU-side heavy lifting is performed
     * here. render() main function is to generate render commands which are asynchronously
     * executed by the Engine's render thread.
     *
     * render() generates commands for each of the following stages:
     *
     * 1. Shadow map pass, if needed (currently only a single shadow map is supported).
     * 2. Depth pre-pass.
     * 3. Color pass.
     * 4. Post-processing pass.
     *
     * A typical render loop looks like this:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * #include <filament/Renderer.h>
     * #include <filament/View.h>
     * using namespace filament;
     *
     * void renderLoop(Renderer* renderer, SwapChain* swapChain) {
     *     do {
     *         // typically we wait for VSYNC and user input events
     *         if (renderer->beginFrame(swapChain)) {
     *             renderer->render(mView);
     *             renderer->endFrame();
     *         }
     *     } while (!quit());
     * }
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     *
     * @param view A pointer to the view to render.
     *
     * @attention
     * render() must be called *after* beginFrame() and *before* endFrame().
     *
     * @note
     * render() must be called from the Engine's main thread (or external synchronization
     * must be provided). In particular, calls to render() on different Renderer instances
     * **must** be synchronized.
     *
     * @remark
     * render() perform potentially heavy computations and cannot be multi-threaded. However,
     * internally, render() is highly multi-threaded to both improve performance in mitigate
     * the call's latency.
     *
     * @remark
     * render() is typically called once per frame (but not necessarily).
     *
     * @see
     * beginFrame(), endFrame(), View
     *
     */
    void render(View const* view);

    /**
     * Flags used to configure the behavior of mirrorFrame().
     *
     * @see
     * mirrorFrame()
     */
    using MirrorFrameFlag = uint32_t;

    /**
     * Indicates that the dstSwapChain passed into mirrorFrame() should be
     * committed after the frame has been mirrored.
     *
     * @see
     * mirrorFrame()
     */
    static constexpr MirrorFrameFlag COMMIT = 0x1;
    /**
     * Indicates that the presentation time should be set on the dstSwapChain
     * passed into mirrorFrame to the monotonic clock time when the frame is
     * mirrored.
     *
     * @see
     * mirrorFrame()
     */
    static constexpr MirrorFrameFlag SET_PRESENTATION_TIME = 0x2;
    /**
     * Indicates that the dstSwapChain passed into mirrorFrame() should be
     * cleared to black before the frame is mirrored into the specified viewport.
     *
     * @see
     * mirrorFrame()
     */
    static constexpr MirrorFrameFlag CLEAR = 0x4;

    /**
     * Mirror the currently rendered view to the indicated swap chain, using the
     * indicated source and destination rectangle.
     *
     * @param dstSwapChain The swap chain into which the frame should be mirrored.
     * @param dstViewport The destination rectangle in which to draw the view.
     * @param srcViewport The source rectangle to be mirrored.
     * @param flags One or more MirrorFrameFlag behavior configuration flags.
     *
     * @remark
     * mirrorFrame() should be called after a frame is rendered using render()
     * but before endFrame() is called.
     */
    void mirrorFrame(SwapChain* dstSwapChain, Viewport const& dstViewport, Viewport const& srcViewport,
                     uint32_t flags=0);

    /**
     * Read-back the content of the SwapChain associated with this Renderer.
     *
     * @param xoffset   Left offset of the sub-region to read back.
     * @param yoffset   Bottom offset of the sub-region to read back.
     * @param width     Width of the sub-region to read back.
     * @param height    Height of the sub-region to read back.
     * @param buffer    Client-side buffer where the read-back will be written.
     *
     *                  The following format are always supported:
     *                      - backend::PixelDataFormat::RGBA
     *                      - backend::PixelDataFormat::RGBA_INTEGER
     *
     *                  The following types are always supported:
     *                      - backend::PixelDataType::UBYTE
     *                      - backend::PixelDataType::UINT
     *                      - backend::PixelDataType::INT
     *                      - backend::PixelDataType::FLOAT
     *
     *                  Other combination of format/type may be supported. If a combination is
     *                  not supported, this operation may fail silently. Use a DEBUG build
     *                  to get some logs about the failure.
     *
     *
     *  Framebuffer as seen on         User buffer (PixelBufferDescriptor&)
     *  screen
     *  +--------------------+
     *  |                    |                .stride         .alignment
     *  |                    |         ----------------------->-->
     *  |                    |         O----------------------+--+   low addresses
     *  |                    |         |          |           |  |
     *  |             w      |         |          | .top      |  |
     *  |       <--------->  |         |          V           |  |
     *  |       +---------+  |         |     +---------+      |  |
     *  |       |     ^   |  | ======> |     |         |      |  |
     *  |   x   |    h|   |  |         |.left|         |      |  |
     *  +------>|     v   |  |         +---->|         |      |  |
     *  |       +.........+  |         |     +.........+      |  |
     *  |            ^       |         |                      |  |
     *  |          y |       |         +----------------------+--+  high addresses
     *  O------------+-------+
     *
     *
     * Typically readPixels() will be called after render() and before endFrame().
     *
     * After issuing this method, the callback associated with `buffer` will be invoked on the
     * main thread, indicating that the read-back has completed. Typically, this will happen
     * after multiple calls to beginFrame(), render(), endFrame().
     *
     * It is also possible to use a Fence to wait for the read-back.
     *
     * @remark
     * readPixels() is intended for debugging and testing. It will impact performance significantly.
     *
     */
    void readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);

    /**
     * Set-up a frame for this Renderer.
     *
     * beginFrame() manages frame pacing, and returns whether or not a frame should be drawn. The
     * goal of this is to skip frames when the GPU falls behind in order to keep the frame
     * latency low.
     *
     * If a given frame takes too much time in the GPU, the CPU will get ahead of the GPU. The
     * display will draw the same frame twice producing a stutter. At this point, the CPU is
     * ahead of the GPU and depending on how many frames are buffered, latency increases.
     *
     * beginFrame() attempts to detect this situation and returns false in that case, indicating
     * to the caller to skip the current frame.
     *
     * @param swapChain A pointer to the SwapChain instance to use.
     * 
     * @return
     *      *false* the current frame must be skipped,
     *      *true* the current frame can be drawn.
     *
     * @remark
     * When skipping a frame, the whole frame is canceled, and endFrame() must not be called.
     *
     * @note
     * All calls to render() must happen *after* beginFrame().
     *
     * @see
     * endFrame()
     */
    bool beginFrame(SwapChain* swapChain);

    /**
     * Finishes the current frame and schedules it for display.
     *
     * endFrame() schedules the current frame to be displayed on the Renderer's window.
     *
     * @note
     * All calls to render() must happen *before* endFrame().
     *
     * @see
     * beginFrame()
     */
    void endFrame();

    /**
     * Returns the time in second of the last call to beginFrame(). This value is constant for all
     * views rendered during a frame. The epoch is set with resetUserTime().
     *
     * In materials, this value can be queried using `vec4 getUserTime()`. The value returned
     * is a highp vec4 encoded as follows:
     *
     *      time.x = (float)Renderer.getUserTime();
     *      time.y = Renderer.getUserTime() - time.x;
     *
     * It follows that the following invariants are true:
     *
     *      (double)time.x + (double)time.y == Renderer.getUserTime()
     *      time.x == (float)Renderer.getUserTime()
     *
     * This encoding allows the shader code to perform high precision (i.e. double) time
     * calculations when needed despite the lack of double precision in the shader, for e.g.:
     *
     *      To compute (double)time * vertex in the material, use the following construct:
     *
     *              vec3 result = time.x * vertex + time.y * vertex;
     *
     *
     * Most of the time, high precision computations are not required, but be aware that the
     * precision of time.x rapidly diminishes as time passes:
     *
     *          time    | precision
     *          --------+----------
     *          16.7s   |    us
     *          4h39    |    ms
     *         77h      |   1/60s
     *
     *
     * In other words, it only possible to get microsecond accuracy for about 16s or millisecond
     * accuracy for just under 5h.
     *
     * This problem can be mitigated by calling resetUserTime(), or using high precision time as
     * described above.
     *
     * @return The time is seconds since resetUserTime() was last called.
     *
     * @see
     * resetUserTime()
     */
    double getUserTime() const;

    /**
     * Sets the user time epoch to now, i.e. resets the user time to zero.
     *
     * Use this method used to keep the precision of time high in materials, in practice it should
     * be called at least when the application is paused, e.g. Activity.onPause() in Android.
     *
     * @see
     * getUserTime()
     */
    void resetUserTime();
};

} // namespace filament

#endif // TNT_FILAMENT_RENDERER_H
