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

#include <string>
#include <vector>

#include <utils/JobSystem.h>

#include <filament/Engine.h>
#include <filament/Texture.h>

#include <ktxreader/Ktx2Reader.h>

using namespace filament;
using namespace utils;

using std::atomic;
using std::vector;
using std::unique_ptr;

namespace filament::gltfio {

class Ktx2Provider final : public TextureProvider {
public:
    Ktx2Provider(Engine* engine);
    ~Ktx2Provider();

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
    enum class QueueItemState {
        TRANSCODING, // Texture has been pushed, mipmap levels are not yet complete.
        READY,       // Mipmap levels are available but texture has not been popped yet.
        POPPED,      // Client has popped the texture from the queue.
    };

    enum class TranscoderState {
        NOT_STARTED,
        ERROR,
        SUCCESS,
    };

    struct QueueItem {
        ktxreader::Ktx2Reader::Async* async;
        QueueItemState state;
        atomic<TranscoderState> transcoderState;
        JobSystem::Job* job;
    };

    void transcodeSingleTexture();

    size_t mPushedCount = 0;
    size_t mPoppedCount = 0;
    size_t mDecodedCount = 0;
    vector<unique_ptr<QueueItem> > mQueueItems;
    JobSystem::Job* mDecoderRootJob;
    std::string mRecentPushMessage;
    std::string mRecentPopMessage;
    std::unique_ptr<ktxreader::Ktx2Reader> mKtxReader;
    Engine* const mEngine;
};

Texture* Ktx2Provider::pushTexture(const uint8_t* data, size_t byteCount,
            const char* mimeType, TextureProvider::TextureFlags flags) {
    using TransferFunction = ktxreader::Ktx2Reader::TransferFunction;

    auto async = mKtxReader->asyncCreate(data, byteCount,
            any(flags & TextureProvider::TextureFlags::sRGB) ?
            TransferFunction::sRGB : TransferFunction::LINEAR);

    if (async == nullptr) {
        mRecentPushMessage = "Unable to build Texture object.";
        return nullptr;
    }

    mRecentPushMessage.clear();
    QueueItem* item = mQueueItems.emplace_back(new QueueItem).get();
    ++mPushedCount;

    item->async = async;
    item->state = QueueItemState::TRANSCODING;
    item->transcoderState.store(TranscoderState::NOT_STARTED);

    // On single threaded systems, it is usually fine to create jobs because the job system will
    // simply execute serially. However in our case, we wish to amortize the decoder cost across
    // several frames, so we instead use the updateQueue() method to perform decoding.
    if constexpr (!UTILS_HAS_THREADING) {
        item->job = nullptr;
        return async->getTexture();
    }

    JobSystem* js = &mEngine->getJobSystem();
    item->job = jobs::createJob(*js, mDecoderRootJob, [item] {
        using Result = ktxreader::Ktx2Reader::Result;
        const bool success = Result::SUCCESS == item->async->doTranscoding();
        item->transcoderState.store(success ? TranscoderState::SUCCESS : TranscoderState::ERROR);
    });

    js->runAndRetain(item->job);
    return async->getTexture();
}

Texture* Ktx2Provider::popTexture() {
    // We don't bother shrinking the mQueueItems vector here, instead we periodically clean it up in
    // the updateQueue method, since popTexture is typically called more frequently. Textures
    // can become ready in non-deterministic order due to concurrency.
    for (auto& item : mQueueItems) {
        if (item->state == QueueItemState::READY) {
            item->state = QueueItemState::POPPED;
            ++mPoppedCount;
            const TranscoderState state = item->transcoderState.load();
            if (state != TranscoderState::SUCCESS) {
                mRecentPopMessage = "Texture is incomplete";
            } else {
                mRecentPopMessage.clear();
            }
            Texture* texture = item->async->getTexture();
            mKtxReader->asyncDestroy(&item->async);
            item->async = nullptr;
            return texture;
        }
    }
    return nullptr;
}

void Ktx2Provider::updateQueue() {
    if (!UTILS_HAS_THREADING) {
        transcodeSingleTexture();
    }
    JobSystem* js = &mEngine->getJobSystem();
    for (auto& item : mQueueItems) {
        if (item->state != QueueItemState::TRANSCODING) {
            continue;
        }
        item->async->getTexture();
        const TranscoderState state = item->transcoderState.load();
        if (state != TranscoderState::NOT_STARTED) {
            if (item->job) {
                js->waitAndRelease(item->job);
            }
            if (state == TranscoderState::ERROR) {
                item->state = QueueItemState::READY;
                ++mDecodedCount;
                continue;
            }
            item->async->uploadImages();
            item->state = QueueItemState::READY;
            ++mDecodedCount;
        }
    }

    // Here we periodically clean up the "queue" (which is really just a vector) by removing unused
    // items from the front. This might ignore a popped texture that occurs in the middle of the
    // vector, but that's okay, it will be cleaned up eventually.
    decltype(mQueueItems)::iterator last = mQueueItems.begin();
    while (last != mQueueItems.end() && (*last)->state == QueueItemState::POPPED) ++last;
    mQueueItems.erase(mQueueItems.begin(), last);
}

void Ktx2Provider::waitForCompletion() {
    JobSystem& js = mEngine->getJobSystem();
    for (auto& item : mQueueItems) {
        if (item->job) {
            js.waitAndRelease(item->job);
        }
    }
}

void Ktx2Provider::cancelDecoding() {
    // TODO: Currently, Ktx2Provider runs jobs eagerly and JobSystem does not allow cancellation of
    // in-flight jobs. We should consider throttling the number of simultaneous decoder jobs, which
    // would allow for actual cancellation.
    waitForCompletion();

    // For cancelled jobs, we need to set the QueueItemState to POPPED and free the decoded data
    // stored in item->async.
    for (auto& item : mQueueItems) {
        if (item->state != QueueItemState::TRANSCODING) {
            continue;
        }
        mKtxReader->asyncDestroy(&item->async);
        item->async = nullptr;
        item->state = QueueItemState::POPPED;
    }
}

const char* Ktx2Provider::getPushMessage() const {
    return mRecentPushMessage.empty() ? nullptr : mRecentPushMessage.c_str();
}

const char* Ktx2Provider::getPopMessage() const {
    return mRecentPopMessage.empty() ? nullptr : mRecentPopMessage.c_str();
}

void Ktx2Provider::transcodeSingleTexture() {
    assert_invariant(!UTILS_HAS_THREADING);
    for (auto& item : mQueueItems) {
        if (item->state == QueueItemState::TRANSCODING) {
            using Result = ktxreader::Ktx2Reader::Result;
            bool success = Result::SUCCESS == item->async->doTranscoding();
            item->transcoderState.store(success ? TranscoderState::SUCCESS : TranscoderState::ERROR);
            break;
        }
    }
}

Ktx2Provider::Ktx2Provider(Engine* engine) : mEngine(engine) {
    mDecoderRootJob = mEngine->getJobSystem().createJob();
#ifdef NDEBUG
    const bool quiet = true;
#else
    const bool quiet = false;
#endif
    mKtxReader.reset(new ktxreader::Ktx2Reader(*engine, quiet));

    mKtxReader->requestFormat(Texture::InternalFormat::ETC2_EAC_SRGBA8);
    mKtxReader->requestFormat(Texture::InternalFormat::DXT5_SRGBA);
    mKtxReader->requestFormat(Texture::InternalFormat::DXT5_RGBA);

    // Uncompressed formats are lower priority, so they get added last.
    mKtxReader->requestFormat(Texture::InternalFormat::SRGB8_A8);
    mKtxReader->requestFormat(Texture::InternalFormat::RGBA8);
}

Ktx2Provider::~Ktx2Provider() {
    cancelDecoding();
    for (auto& item : mQueueItems) {
        mKtxReader->asyncDestroy(&item->async);
    }
    mEngine->getJobSystem().release(mDecoderRootJob);
}

TextureProvider* createKtx2Provider(Engine* engine) {
    return new Ktx2Provider(engine);
}

} // namespace filament::gltfio
