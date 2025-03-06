/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMENT_STREAM_H
#define TNT_FILAMENT_STREAM_H

#include <filament/FilamentAPI.h>

#include <backend/DriverEnums.h>
#include <backend/CallbackHandler.h>

#include <utils/compiler.h>

#include <stdint.h>
#include <math/mat3.h>

namespace filament {

class FStream;

class Engine;

/**
 * Stream is used to attach a video stream to a Filament `Texture`.
 *
 * Note that the `Stream` class is fairly Android centric. It supports two different
 * configurations:
 *
 *   - ACQUIRED.....connects to an Android AHardwareBuffer
 *   - NATIVE.......connects to an Android SurfaceTexture
 *
 * Before explaining these different configurations, let's review the high-level structure of an AR
 * or video application that uses Filament:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * while (true) {
 *
 *     // Misc application work occurs here, such as:
 *     // - Writing the image data for a video frame into a Stream
 *     // - Moving the Filament Camera
 *
 *     if (renderer->beginFrame(swapChain)) {
 *         renderer->render(view);
 *         renderer->endFrame();
 *     }
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Let's say that the video image data at the time of a particular invocation of `beginFrame`
 * becomes visible to users at time A. The 3D scene state (including the camera) at the time of
 * that same invocation becomes apparent to users at time B.
 *
 * - If time A matches time B, we say that the stream is \em{synchronized}.
 * - Filament invokes low-level graphics commands on the \em{driver thread}.
 * - The thread that calls `beginFrame` is called the \em{main thread}.
 *
 * For ACQUIRED streams, there is no need to perform the copy because Filament explictly acquires
 * the stream, then releases it later via a callback function. This configuration is especially
 * useful when the Vulkan backend is enabled.
 *
 * For NATIVE streams, Filament does not make any synchronization guarantee. However they are simple
 * to use and do not incur a copy. These are often appropriate in video applications.
 *
 * Please see `sample-stream-test` and `sample-hello-camera` for usage examples.
 *
 * @see backend::StreamType
 * @see Texture#setExternalStream
 * @see Engine#destroyStream
 */
class UTILS_PUBLIC Stream : public FilamentAPI {
    struct BuilderDetails;

public:
    using Callback = backend::StreamCallback;
    using StreamType = backend::StreamType;

    /**
     * Constructs a Stream object instance.
     *
     * By default, Stream objects are ACQUIRED and must have external images pushed to them via
     * <pre>Stream::setAcquiredImage</pre>.
     *
     * To create a NATIVE stream, call the <pre>stream</pre> method on the builder.
     */
    class Builder : public BuilderBase<BuilderDetails>, public BuilderNameMixin<Builder> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        /**
         * Creates a NATIVE stream. Native streams can sample data directly from an
         * opaque platform object such as a SurfaceTexture on Android.
         *
         * @param stream An opaque native stream handle. e.g.: on Android this is an
         *                     `android/graphics/SurfaceTexture` JNI jobject. The wrap mode must
         *                     be CLAMP_TO_EDGE.
         *
         * @return This Builder, for chaining calls.
         */
        Builder& stream(void* UTILS_NULLABLE stream) noexcept;

        /**
         *
         * @param width initial width of the incoming stream. Whether this value is used is
         *              stream dependent. On Android, it must be set when using
         *              Builder::stream(long externalTextureId).
         *
         * @return This Builder, for chaining calls.
         */
        Builder& width(uint32_t width) noexcept;

        /**
         *
         * @param height initial height of the incoming stream. Whether this value is used is
         *              stream dependent. On Android, it must be set when using
         *              Builder::stream(long externalTextureId).
         *
         * @return This Builder, for chaining calls.
         */
        Builder& height(uint32_t height) noexcept;

        /**
         * Associate an optional name with this Stream for debugging purposes.
         *
         * name will show in error messages and should be kept as short as possible. The name is
         * truncated to a maximum of 128 characters.
         *
         * The name string is copied during this method so clients may free its memory after
         * the function returns.
         *
         * @param name A string to identify this Stream
         * @param len Length of name, should be less than or equal to 128
         * @return This Builder, for chaining calls.
         */
        Builder& name(const char* UTILS_NONNULL name, size_t len) noexcept;

        /**
         * Creates the Stream object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this Stream with.
         *
         * @return pointer to the newly created object.
         */
        Stream* UTILS_NONNULL build(Engine& engine);

    private:
        friend class FStream;
    };

    /**
     * Indicates whether this stream is a NATIVE stream or ACQUIRED stream.
     */
    StreamType getStreamType() const noexcept;

    /**
     * Updates an ACQUIRED stream with an image that is guaranteed to be used in the next frame.
     *
     * This method tells Filament to immediately "acquire" the image and trigger a callback
     * when it is done with it. This should be called by the user outside of beginFrame / endFrame,
     * and should be called only once per frame. If the user pushes images to the same stream
     * multiple times in a single frame, only the final image is honored, but all callbacks are
     * invoked.
     *
     * This method should be called on the same thread that calls Renderer::beginFrame, which is
     * also where the callback is invoked. This method can only be used for streams that were
     * constructed without calling the `stream` method on the builder.
     *
     * @see Stream for more information about NATIVE and ACQUIRED configurations.
     *
     * @param image      Pointer to AHardwareBuffer, casted to void* since this is a public header.
     * @param callback   This is triggered by Filament when it wishes to release the image.
     *                   The callback tales two arguments: the AHardwareBuffer and the userdata.
     * @param userdata   Optional closure data. Filament will pass this into the callback when it
     *                   releases the image.
     * @param transform  Optional transform matrix to apply to the image.
     */
    void setAcquiredImage(void* UTILS_NONNULL image,
            Callback UTILS_NONNULL callback, void* UTILS_NULLABLE userdata, math::mat3f transform = math::mat3f()) noexcept;

    /**
     * @see setAcquiredImage(void*, Callback, void*)
     *
     * @param image      Pointer to AHardwareBuffer, casted to void* since this is a public header.
     * @param handler    Handler to dispatch the AcquiredImage or nullptr for the default handler.
     * @param callback   This is triggered by Filament when it wishes to release the image.
     *                   It callback tales two arguments: the AHardwareBuffer and the userdata.
     * @param userdata   Optional closure data. Filament will pass this into the callback when it
     *                   releases the image.
     * @param transform  Optional transform matrix to apply to the image.
     */
    void setAcquiredImage(void* UTILS_NONNULL image,
            backend::CallbackHandler* UTILS_NULLABLE handler,
            Callback UTILS_NONNULL callback, void* UTILS_NULLABLE userdata, math::mat3f transform = math::mat3f()) noexcept;

    /**
     * Updates the size of the incoming stream. Whether this value is used is
     *              stream dependent. On Android, it must be set when using
     *              Builder::stream(long externalTextureId).
     *
     * @param width     new width of the incoming stream
     * @param height    new height of the incoming stream
     */
    void setDimensions(uint32_t width, uint32_t height) noexcept;

    /**
     * Returns the presentation time of the currently displayed frame in nanosecond.
     *
     * This value can change at any time.
     *
     * @return timestamp in nanosecond.
     */
    int64_t getTimestamp() const noexcept;

protected:
    // prevent heap allocation
    ~Stream() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_STREAM_H
