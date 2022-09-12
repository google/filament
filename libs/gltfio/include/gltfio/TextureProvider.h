/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef GLTFIO_TEXTUREPROVIDER_H
#define GLTFIO_TEXTUREPROVIDER_H

#include <stddef.h>
#include <stdint.h>

#include <utils/compiler.h>
#include <utils/BitmaskEnum.h>

namespace filament {
    class Engine;
    class Texture;
}

namespace filament::gltfio {

/**
 * TextureProvider is an interface that allows clients to implement their own texture decoding
 * facility for JPEG, PNG, or KTX2 content. It constructs Filament Texture objects synchronously,
 * but populates their miplevels asynchronously.
 *
 * gltfio calls all public methods from the foreground thread, i.e. the thread that the Filament
 * engine was created with. However the implementation may create 0 or more background threads to
 * perform decoding work.
 *
 * The following pseudocode illustrates how this interface could be used, but in practice the only
 * client is the gltfio ResourceLoader.
 *
 *     filament::Engine* engine = ...;
 *     TextureProvider* provider = createStbProvider(engine);
 *
 *     for (auto filename : textureFiles) {
 *         std::vector<uint8_t> buf = readEntireFile(filename);
 *         Texture* texture = provider->pushTexture(buf.data(), buf.size(), "image/png", 0);
 *         if (texture == nullptr) { puts(provider->getPushMessage()); exit(1); }
 *     }
 *
 *     // At this point, the returned textures can be bound to material instances, but none of their
 *     // miplevel images have been populated yet.
 *
 *     while (provider->getPoppedCount() < provider->getPushedCount()) {
 *         sleep(200);
 *
 *         // The following call gives the provider an opportunity to reap the results of any
 *         // background decoder work that has been completed (e.g. by calling Texture::setImage).
 *         provider->updateQueue();
 *
 *         // Check for textures that now have all their miplevels initialized.
 *         while (Texture* texture = provider->popTexture()) {
 *             printf("%p has all its miplevels ready.\n", texture);
 *         }
 *     }
 *
 *     delete provider;
 */
class UTILS_PUBLIC TextureProvider {
public:
    using Texture = filament::Texture;

    enum class TextureFlags : uint64_t {
        NONE = 0,
        sRGB = 1 << 0,
    };

    /**
     * Creates a Filament texture and pushes it to the asynchronous decoding queue.
     *
     * The provider synchronously determines the texture dimensions in order to create a Filament
     * texture object, then populates the miplevels asynchronously.
     *
     * If construction fails, nothing is pushed to the queue and null is returned. The failure
     * reason can be obtained with getPushMessage(). The given buffer pointer is not held, so the
     * caller can free it immediately. It is also the caller's responsibility to free the returned
     * Texture object, but it is only safe to do so after it has been popped from the queue.
     */
    virtual Texture* pushTexture(const uint8_t* data, size_t byteCount,
            const char* mimeType, TextureFlags flags) = 0;

    /**
     * Checks if any texture is ready to be removed from the asynchronous decoding queue, and if so
     * pops it off.
     *
     * Unless an error or cancellation occurred during the decoding process, the returned texture
     * should have all its miplevels populated. If the texture is not complete, the reason can be
     * obtained with getPopMessage().
     *
     * Due to concurrency, textures are not necessarily popped off in the same order they were
     * pushed. Returns null if there are no textures that are ready to be popped.
     */
    virtual Texture* popTexture() = 0;

    /**
     * Polls textures in the queue and uploads mipmap images if any have emerged from the decoder.
     *
     * This gives the provider an opportunity to call Texture::setImage() on the foreground thread.
     * If needed, it can also call Texture::generateMipmaps() here.
     *
     * Items in the decoding queue can become "poppable" only during this call.
     */
    virtual void updateQueue() = 0;

    /**
     * Returns a failure message for the most recent call to pushTexture(), or null for success.
     *
     * Note that this method does not pertain to the decoding process. If decoding fails, clients to
     * can pop the incomplete texture off the queue and obtain a failure message using the
     * getPopFailure() method.
     *
     * The returned string is owned by the provider and becomes invalid after the next call to
     * pushTexture().
     */
    virtual const char* getPushMessage() const = 0;

    /**
     * Returns a failure message for the most recent call to popTexture(), or null for success.
     *
     * If the most recent call to popTexture() returned null, then no error occurred and this
     * returns null. If the most recent call to popTexture() returned a "complete" texture (i.e.
     * all miplevels present), then this returns null. This returns non-null only if an error or
     * cancellation occurred while decoding the popped texture.
     *
     * The returned string is owned by the provider and becomes invalid after the next call to
     * popTexture().
     */
    virtual const char* getPopMessage() const = 0;

    /**
     * Waits for all outstanding decoding jobs to complete.
     *
     * Clients should call updateQueue() afterwards if they wish to update the push / pop queue.
     */
    virtual void waitForCompletion() = 0;

    /**
     * Cancels all not-yet-started decoding jobs and waits for all other jobs to complete.
     *
     * Jobs that have already started cannot be canceled. Textures whose decoding process has
     * been cancelled will be made poppable on the subsequent call to updateQueue().
     */
    virtual void cancelDecoding() = 0;

    /** Total number of successful push calls since the provider was created. */
    virtual size_t getPushedCount() const = 0;

    /** Total number of successful pop calls since the provider was created. */
    virtual size_t getPoppedCount() const = 0;

    /** Total number of textures that have become ready-to-pop since the provider was created. */
    virtual size_t getDecodedCount() const = 0;

    virtual ~TextureProvider() = default;
};

/**
 * Creates a simple decoder based on stb_image that can handle "image/png" and "image/jpeg".
 * This works only if your build configuration includes STB.
 */
TextureProvider* createStbProvider(filament::Engine* engine);

/**
 * Creates a decoder that can handle certain types of "image/ktx2" content as specified in
 * the KHR_texture_basisu specification.
 */
TextureProvider* createKtx2Provider(filament::Engine* engine);

} // namespace filament::gltfio

template<> struct utils::EnableBitMaskOperators<filament::gltfio::TextureProvider::TextureFlags>
        : public std::true_type {};

#endif // GLTFIO_TEXTUREPROVIDER_H
