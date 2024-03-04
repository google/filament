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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_OPENGLPLATFORM_H
#define TNT_FILAMENT_BACKEND_PRIVATE_OPENGLPLATFORM_H

#include <backend/AcquiredImage.h>
#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#include <utils/compiler.h>

#include <stdint.h>

namespace filament::backend {

class Driver;

/**
 * A Platform interface that creates an OpenGL backend.
 *
 * WARNING: None of the methods below are allowed to change the GL state and must restore it
 *          upon return.
 *
 */
class OpenGLPlatform : public Platform {
protected:

    /*
     * Derived classes can use this to instantiate the default OpenGLDriver backend.
     * This is typically called from your implementation of createDriver()
     */
    static Driver* UTILS_NULLABLE createDefaultDriver(OpenGLPlatform* UTILS_NONNULL platform,
            void* UTILS_NULLABLE sharedContext, const DriverConfig& driverConfig);

    ~OpenGLPlatform() noexcept override;

public:

    struct ExternalTexture {
        unsigned int target;            // GLenum target
        unsigned int id;                // GLuint id
    };

    /**
     * Called by the driver to destroy the OpenGL context. This should clean up any windows
     * or buffers from initialization. This is for instance where `eglDestroyContext` would be
     * called.
     */
    virtual void terminate() noexcept = 0;

    /**
     * Return whether createSwapChain supports the SWAP_CHAIN_CONFIG_SRGB_COLORSPACE flag.
     * The default implementation returns false.
     *
     * @return true if SWAP_CHAIN_CONFIG_SRGB_COLORSPACE is supported, false otherwise.
     */
    virtual bool isSRGBSwapChainSupported() const noexcept;

    /**
     * Return whether protected contexts are supported by this backend.
     * If protected context are supported, the SWAP_CHAIN_CONFIG_PROTECTED_CONTENT flag can be
     * used when creating a SwapChain.
     * The default implementation returns false.
     */
    virtual bool isProtectedContextSupported() const noexcept;

    /**
     * Called by the driver to create a SwapChain for this driver.
     *
     * @param nativeWindow  a token representing the native window. See concrete implementation
     *                      for details.
     * @param flags         extra flags used by the implementation, see filament::SwapChain
     * @return              The driver's SwapChain object.
     *
     */
    virtual SwapChain* UTILS_NONNULL createSwapChain(
            void* UTILS_NULLABLE nativeWindow, uint64_t flags) noexcept = 0;

    /**
     * Called by the driver create a headless SwapChain.
     *
     * @param width     width of the buffer
     * @param height    height of the buffer
     * @param flags     extra flags used by the implementation, see filament::SwapChain
     * @return          The driver's SwapChain object.
     *
     * TODO: we need a more generic way of passing construction parameters
     *       A void* might be enough.
     */
    virtual SwapChain* UTILS_NULLABLE createSwapChain(
            uint32_t width, uint32_t height, uint64_t flags) noexcept = 0;

    /**
     * Called by the driver to destroys the SwapChain
     * @param swapChain SwapChain to be destroyed.
     */
    virtual void destroySwapChain(SwapChain* UTILS_NONNULL swapChain) noexcept = 0;

    /**
     * Returns the set of buffers that must be preserved up to the call to commit().
     * The default value is TargetBufferFlags::NONE.
     * The color buffer is always preserved, however ancillary buffers, such as the depth buffer
     * are generally discarded. The preserve flags can be used to make sure those ancillary
     * buffers are preserved until the call to commit.
     *
     * @param swapChain
     * @return buffer that must be preserved
     * @see commit()
     */
    virtual TargetBufferFlags getPreservedFlags(SwapChain* UTILS_NONNULL swapChain) noexcept;

    /**
     * Called by the driver to establish the default FBO. The default implementation returns 0.
      * @return a GLuint casted to a uint32_t that is an OpenGL framebuffer object.
     */
    virtual uint32_t createDefaultRenderTarget() noexcept;

    /**
     * Called by the driver to make the OpenGL context active on the calling thread and bind
     * the drawSwapChain to the default render target (FBO) created with createDefaultRenderTarget.
     * @param drawSwapChain SwapChain to draw to. It must be bound to the default FBO.
     * @param readSwapChain SwapChain to read from (for operation like `glBlitFramebuffer`)
     */
    virtual void makeCurrent(
            SwapChain* UTILS_NONNULL drawSwapChain,
            SwapChain* UTILS_NONNULL readSwapChain) noexcept = 0;

    /**
     * Called by the driver once the current frame finishes drawing. Typically, this should present
     * the drawSwapChain. This is for example where `eglMakeCurrent()` would be called.
     * @param swapChain the SwapChain to present.
     */
    virtual void commit(SwapChain* UTILS_NONNULL swapChain) noexcept = 0;

    /**
     * Set the time the next committed buffer should be presented to the user at.
     *
     * @param presentationTimeInNanosecond  time in the future in nanosecond. The clock used depends
     *                                      on the concrete platform implementation.
     */
    virtual void setPresentationTime(int64_t presentationTimeInNanosecond) noexcept;

    // --------------------------------------------------------------------------------------------
    // Fence support

    /**
     * Can this implementation create a Fence.
     * @return true if supported, false otherwise. The default implementation returns false.
     */
    virtual bool canCreateFence() noexcept;

    /**
     * Creates a Fence (e.g. eglCreateSyncKHR). This must be implemented if `canCreateFence`
     * returns true. Fences are used for frame pacing.
     *
     * @return A Fence object. The default implementation returns nullptr.
     */
    virtual Fence* UTILS_NULLABLE createFence() noexcept;

    /**
     * Destroys a Fence object. The default implementation does nothing.
     *
     * @param fence Fence to destroy.
     */
    virtual void destroyFence(Fence* UTILS_NONNULL fence) noexcept;

    /**
     * Waits on a Fence.
     *
     * @param fence   Fence to wait on.
     * @param timeout Timeout.
     * @return Whether the fence signaled or timed out. See backend::FenceStatus.
     *         The default implementation always return backend::FenceStatus::ERROR.
     */
    virtual backend::FenceStatus waitFence(Fence* UTILS_NONNULL fence, uint64_t timeout) noexcept;


    // --------------------------------------------------------------------------------------------
    // Streaming support

    /**
     * Creates a Stream from a native Stream.
     *
     * WARNING: This is called synchronously from the application thread (NOT the Driver thread)
     *
     * @param nativeStream The native stream, this parameter depends on the concrete implementation.
     * @return A new Stream object.
     */
    virtual Stream* UTILS_NULLABLE createStream(void* UTILS_NULLABLE nativeStream) noexcept;

    /**
     * Destroys a Stream.
     * @param stream Stream to destroy.
     */
    virtual void destroyStream(Stream* UTILS_NONNULL stream) noexcept;

    /**
     * The specified stream takes ownership of the texture (tname) object
     * Once attached, the texture is automatically updated with the Stream's content, which
     * could be a video stream for instance.
     *
     * @param stream Stream to take ownership of the texture
     * @param tname  GL texture id to "bind" to the Stream.
     */
    virtual void attach(Stream* UTILS_NONNULL stream, intptr_t tname) noexcept;

    /**
     * Destroys the texture associated to the stream
     * @param stream Stream to detach from its texture
     */
    virtual void detach(Stream* UTILS_NONNULL stream) noexcept;

    /**
     * Updates the content of the texture attached to the stream.
     * @param stream Stream to update
     * @param timestamp Output parameter: Timestamp of the image bound to the texture.
     */
    virtual void updateTexImage(Stream* UTILS_NONNULL stream,
            int64_t* UTILS_NONNULL timestamp) noexcept;


    // --------------------------------------------------------------------------------------------
    // External Image support

    /**
     * Creates an external texture handle. External textures don't have any parameters because
     * these are undefined until setExternalImage() is called.
     * @return a pointer to an ExternalTexture structure filled with valid token. However, the
     *         implementation could just return { 0, GL_TEXTURE_2D } at this point. The actual
     *         values can be delayed until setExternalImage.
     */
    virtual ExternalTexture* UTILS_NULLABLE createExternalImageTexture() noexcept;

    /**
     * Destroys an external texture handle and associated data.
     * @param texture a pointer to the handle to destroy.
     */
    virtual void destroyExternalImage(ExternalTexture* UTILS_NONNULL texture) noexcept;

    // called on the application thread to allow Filament to take ownership of the image

    /**
     * Takes ownership of the externalImage. The externalImage parameter depends on the Platform's
     * concrete implementation. Ownership is released when destroyExternalImage() is called.
     *
     * WARNING: This is called synchronously from the application thread (NOT the Driver thread)
     *
     * @param externalImage A token representing the platform's external image.
     * @see destroyExternalImage
     */
    virtual void retainExternalImage(void* UTILS_NONNULL externalImage) noexcept;

    /**
     * Called to bind the platform-specific externalImage to an ExternalTexture.
     * ExternalTexture::id is guaranteed to be bound when this method is called and ExternalTexture
     * is updated with new values for id/target if necessary.
     *
     * WARNING: this method is not allowed to change the bound texture, or must restore the previous
     * binding upon return. This is to avoid problem with a backend doing state caching.
     *
     * @param externalImage The platform-specific external image.
     * @param texture an in/out pointer to ExternalTexture, id and target can be updated if necessary.
     * @return true on success, false on error.
     */
    virtual bool setExternalImage(void* UTILS_NONNULL externalImage,
            ExternalTexture* UTILS_NONNULL texture) noexcept;

    /**
     * The method allows platforms to convert a user-supplied external image object into a new type
     * (e.g. HardwareBuffer => EGLImage). The default implementation returns source.
     * @param source Image to transform.
     * @return Transformed image.
     */
    virtual AcquiredImage transformAcquiredImage(AcquiredImage source) noexcept;

    // --------------------------------------------------------------------------------------------

    /**
     * Returns true if additional OpenGL contexts can be created. Default: false.
     * @return true if additional OpenGL contexts can be created.
     * @see createContext
     */
    virtual bool isExtraContextSupported() const noexcept;

    /**
     * Creates an OpenGL context with the same configuration than the main context and makes it
     * current to the current thread. Must not be called from the main driver thread.
     * createContext() is only supported if isExtraContextSupported() returns true.
     * These additional contexts will be automatically terminated in terminate.
     *
     * @param shared whether the new context is shared with the main context.
     * @see isExtraContextSupported()
     * @see terminate()
     */
    virtual void createContext(bool shared);

    /**
     * Detach and destroy the current context if any and releases all resources associated to
     * this thread.
     */
    virtual void releaseContext() noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_BACKEND_PRIVATE_OPENGLPLATFORM_H
