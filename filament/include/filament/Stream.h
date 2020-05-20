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

#include <backend/PixelBufferDescriptor.h>

#include <utils/compiler.h>

namespace filament {

class FStream;

class Engine;

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
     * To create a NATIVE or TEXTURE_ID stream, call one of the <pre>stream</pre> methods
     * on the builder.
     */
    class Builder : public BuilderBase<BuilderDetails> {
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
         *                     `android/graphics/SurfaceTexture` JNI jobject.
         *
         * @return This Builder, for chaining calls.
         */
        Builder& stream(void* stream) noexcept;

        /**
         * Creates a TEXTURE_ID stream. This will sample data from the supplied
         * external texture and copy it into an internal private texture.
         *
         * @param externalTextureId An opaque texture id (typically a GLuint created with glGenTextures)
         *                          In a context shared with filament. In that case this texture's
         *                          target must be GL_TEXTURE_EXTERNAL_OES.
         *
         * @return This Builder, for chaining calls.
         *
         * @see Texture::setExternalStream()
         */
        Builder& stream(intptr_t externalTextureId) noexcept;

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
         * Creates the Stream object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this Stream with.
         *
         * @return pointer to the newly created object, or nullptr if the stream couldn't be created.
         */
        Stream* build(Engine& engine);

    private:
        friend class FStream;
    };

    /**
     * Indicates whether this stream is a NATIVE stream, TEXTURE_ID stream, or ACQUIRED stream.
     */
    StreamType getStreamType() const noexcept;

    /**
     * Updates an ACQUIRED stream with an image that is guaranteed to be used in the next frame.
     *
     * This method should be called on the same thread that calls Renderer::beginFrame, which is
     * also where the callback is invoked.
     */
    void setAcquiredImage(void* image, Callback callback, void* userdata) noexcept;

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
     * Read-back the content of the last frame of a Stream since the last call to
     * Renderer.beginFrame().
     *
     * The Stream must be of type externalTextureId. This function is a no-op otherwise.
     *
     * @param xoffset   Left offset of the sub-region to read back.
     * @param yoffset   Bottom offset of the sub-region to read back.
     * @param width     Width of the sub-region to read back.
     * @param height    Height of the sub-region to read back.
     * @param buffer    Client-side buffer where the read-back will be written.
     *
     *                  The following format are always supported:
     *                      - PixelBufferDescriptor::PixelDataFormat::RGBA
     *                      - PixelBufferDescriptor::PixelDataFormat::RGBA_INTEGER
     *
     *                  The following types are always supported:
     *                      - PixelBufferDescriptor::PixelDataType::UBYTE
     *                      - PixelBufferDescriptor::PixelDataType::UINT
     *                      - PixelBufferDescriptor::PixelDataType::INT
     *                      - PixelBufferDescriptor::PixelDataType::FLOAT
     *
     *                  Other combination of format/type may be supported. If a combination is
     *                  not supported, this operation may fail silently. Use a DEBUG build
     *                  to get some logs about the failure.
     *
     *  Stream buffer                  User buffer (PixelBufferDescriptor&)
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
     * Typically readPixels() will be called after Renderer.beginFrame().
     *
     * After issuing this method, the callback associated with `buffer` will be invoked on the
     * main thread, indicating that the read-back has completed. Typically, this will happen
     * after multiple calls to beginFrame(), render(), endFrame().
     *
     * It is also possible to use a Fence to wait for the read-back.
     *
     * @remark
     * readPixels() is intended for debugging and testing. It will impact performance significantly.
     */
    void readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer) noexcept;

    /**
     * Returns the presentation time of the currently displayed frame in nanosecond.
     *
     * This value can change at any time.
     *
     * @return timestamp in nanosecond.
     */
    int64_t getTimestamp() const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_STREAM_H
