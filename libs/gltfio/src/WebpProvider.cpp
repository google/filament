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

#include <gltfio/TextureProvider.h>

#if !defined(FILAMENT_SUPPORTS_WEBP_TEXTURES)

namespace filament::gltfio {
    bool isWebpSupported() {
        return false;
    }

    TextureProvider* createWebpProvider(Engine* engine) {
        return nullptr;        
    }
}

#else // FILAMENT_SUPPORTS_WEBP_TEXTURES

#include <string>
#include <vector>

#include <utils/JobSystem.h>
#include <utils/Log.h>

#include <filament/Engine.h>
#include <filament/Texture.h>

#include <webp/decode.h>

using namespace filament;
using namespace utils;

using std::atomic;
using std::vector;
using std::unique_ptr;

namespace filament::gltfio {

class WebpProvider final : public TextureProvider {
public:
    WebpProvider(Engine* engine);
    ~WebpProvider();

    Texture* pushTexture(const uint8_t* data, size_t byteCount,
            const char* mimeType, TextureFlags flags) final;

    Texture* popTexture() final;
    void updateQueue() final;
    void waitForCompletion() final;
    void cancelDecoding() final;
    const char* getPushMessage() const final;
    const char* getPopMessage() const final;
    size_t getPushedCount() const final { return mPushedCount; }
    size_t getPoppedCount() const final { return mPoppedCount; }
    size_t getDecodedCount() const final { return mDecodedCount; }

private:
    enum class TextureState {
        DECODING, // Texture has been pushed, mipmap levels are not yet complete.
        READY,    // Mipmap levels are available but texture has not been popped yet.
        POPPED,   // Client has popped the texture from the queue.
    };

    struct TextureInfo {
        Texture* texture;
        TextureState state;
        atomic<intptr_t> decodedTexelsBaseMipmap;
        vector<uint8_t> sourceBuffer;
        JobSystem::Job*  decoderJob;
    };

    // Declare some sentinel values for the "decodedTexelsBaseMipmap" field.
    // Note that the "state" field can be modified only on the foreground thread.
    static const intptr_t DECODING_NOT_READY = 0x0;
    static const intptr_t DECODING_ERROR = 0x1;

    void decodeSingleTexture();

    size_t mPushedCount = 0;
    size_t mPoppedCount = 0;
    size_t mDecodedCount = 0;
    vector<unique_ptr<TextureInfo> > mTextures;
    JobSystem::Job* mDecoderRootJob;
    std::string mRecentPushMessage;
    std::string mRecentPopMessage;
    Engine* const mEngine;
};

Texture* WebpProvider::pushTexture(const uint8_t* data, size_t byteCount,
            const char* mimeType, TextureFlags flags) {
    int width, height;

    if (!WebPGetInfo(data, byteCount, &width, &height)) {
        mRecentPushMessage = "Unable to parse webp texture";
        return nullptr;
    }

    using InternalFormat = Texture::InternalFormat;

    Texture* texture = Texture::Builder()
            .width(width)
            .height(height)
            .levels(0xff)
            .format(any(flags & TextureFlags::sRGB) ? InternalFormat::SRGB8_A8 : InternalFormat::RGBA8)
            .usage(Texture::Usage::DEFAULT | Texture::Usage::GEN_MIPMAPPABLE)
            .build(*mEngine);

    if (texture == nullptr) {
        mRecentPushMessage = "Unable to build Texture object for webp image.";
        return nullptr;
    }

    mRecentPushMessage.clear();
    TextureInfo* info = mTextures.emplace_back(new TextureInfo).get();
    ++mPushedCount;

    info->texture = texture;
    info->state = TextureState::DECODING;
    info->sourceBuffer.assign(data, data + byteCount);
    info->decodedTexelsBaseMipmap.store(DECODING_NOT_READY);

    // On single threaded systems, it is usually fine to create jobs because the job system will
    // simply execute serially. However in our case, we wish to amortize the decoder cost across
    // several frames, so we instead use the updateQueue() method to perform decoding.
    if constexpr (!UTILS_HAS_THREADING) {
        info->decoderJob = nullptr;
        return texture;
    }

    JobSystem* js = &mEngine->getJobSystem();
    info->decoderJob = jobs::createJob(*js, mDecoderRootJob, [info] {
        auto& source = info->sourceBuffer;
        int width, height;

        uint8_t* texels = WebPDecodeRGBA(source.data(), source.size(), &width, &height);

        // Test asynchronous loading by uncommenting this line.
        // std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10000));

        source.clear();
        source.shrink_to_fit();
        info->decodedTexelsBaseMipmap.store(texels ? intptr_t(texels) : DECODING_ERROR);
    });

    js->runAndRetain(info->decoderJob);
    return texture;
}

Texture* WebpProvider::popTexture() {
    // We don't bother shrinking the mTextures vector here, instead we periodically clean it up in
    // the updateQueue method, since popTexture is typically called more frequently. Textures
    // can become ready in non-deterministic order due to concurrency.
    for (auto& texture : mTextures) {
        if (texture->state == TextureState::READY) {
            texture->state = TextureState::POPPED;
            ++mPoppedCount;
            const intptr_t ptr = texture->decodedTexelsBaseMipmap.load();
            if (ptr == DECODING_ERROR || ptr == DECODING_NOT_READY) {
                mRecentPopMessage = "Texture is incomplete";
            } else {
                mRecentPopMessage.clear();
            }
            return texture->texture;
        }
    }
    return nullptr;
}

void WebpProvider::updateQueue() {
    if (!UTILS_HAS_THREADING) {
        decodeSingleTexture();
    }
    JobSystem* js = &mEngine->getJobSystem();
    for (auto& info : mTextures) {
        if (info->state != TextureState::DECODING) {
            continue;
        }
        Texture* texture = info->texture;
        if (intptr_t data = info->decodedTexelsBaseMipmap.load()) {
            if (info->decoderJob) {
                js->waitAndRelease(info->decoderJob);
            }
            if (data == DECODING_ERROR) {
                info->state = TextureState::READY;
                ++mDecodedCount;
                continue;
            }
            Texture::PixelBufferDescriptor pbd((uint8_t*) data,
                    texture->getWidth() * texture->getHeight() * 4, Texture::Format::RGBA,
                    Texture::Type::UBYTE, [](void* mem, size_t, void*) { WebPFree(mem); });
            texture->setImage(*mEngine, 0, std::move(pbd));

            // Call generateMipmaps unconditionally to fulfill the promise of the TextureProvider
            // interface. Providers of hierarchical images (e.g. KTX) call this only if needed.
            texture->generateMipmaps(*mEngine);

            info->state = TextureState::READY;
            ++mDecodedCount;
        }
    }

    // Here we periodically clean up the "queue" (which is really just a vector) by removing unused
    // items from the front. This might ignore a popped texture that occurs in the middle of the
    // vector, but that's okay, it will be cleaned up eventually.
    decltype(mTextures)::iterator last = mTextures.begin();
    while (last != mTextures.end() && (*last)->state == TextureState::POPPED) ++last;
    mTextures.erase(mTextures.begin(), last);
}

void WebpProvider::waitForCompletion() {
    JobSystem& js = mEngine->getJobSystem();
    for (auto& info : mTextures) {
        if (info->decoderJob) {
            js.waitAndRelease(info->decoderJob);
        }
    }
}

void WebpProvider::cancelDecoding() {
    // TODO: Currently, WebpProvider runs jobs eagerly and JobSystem does not allow cancellation of
    // in-flight jobs. We should consider throttling the number of simultaneous decoder jobs, which
    // would allow for actual cancellation.
    waitForCompletion();

    // For cancelled jobs, we need to set the TextureInfo to the popped state and free the decoded
    // data.
    for (auto& info : mTextures) {
        if (info->state != TextureState::DECODING) {
            continue;
        }
        // Deleting data here should be safe thread-wise as the only other place where
        // decodedTexelsBaseMipmap is loaded is in the job threads, and we have waited them to
        // completion above. We also expect the TextureProvider API calls to be made only from one
        // thread.
        if (intptr_t data = info->decodedTexelsBaseMipmap.load()) {
            WebPFree(data);
        }
        info->state = TextureState::POPPED;
    }
}

const char* WebpProvider::getPushMessage() const {
    return mRecentPushMessage.empty() ? nullptr : mRecentPushMessage.c_str();
}

const char* WebpProvider::getPopMessage() const {
    return mRecentPopMessage.empty() ? nullptr : mRecentPopMessage.c_str();
}

void WebpProvider::decodeSingleTexture() {
    assert_invariant(!UTILS_HAS_THREADING);
    for (auto& info : mTextures) {
        if (info->state == TextureState::DECODING) {
            auto& source = info->sourceBuffer;
            int width, height;
            uint8_t* texels = WebPDecodeRGBA(source.data(), source.size(), &width, &height);
            source.clear();
            source.shrink_to_fit();
            info->decodedTexelsBaseMipmap.store(texels ? intptr_t(texels) : DECODING_ERROR);
            break;
        }
    }
}

WebpProvider::WebpProvider(Engine* engine) : mEngine(engine) {
    mDecoderRootJob = mEngine->getJobSystem().createJob();
#ifndef NDEBUG
    slog.i << "Texture Decoder has "
            << mEngine->getJobSystem().getThreadCount()
            << " background threads." << io::endl;
#endif
}

WebpProvider::~WebpProvider() {
    cancelDecoding();
    mEngine->getJobSystem().release(mDecoderRootJob);
}

bool isWebpSupported() {
    return true;
}

TextureProvider* createWebpProvider(Engine* engine) {
    return new WebpProvider(engine);
}

} // namespace filament::gltfio

#endif // FILAMENT_SUPPORTS_WEBP_TEXTURES
