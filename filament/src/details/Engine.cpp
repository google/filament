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

#include "details/Engine.h"

#include "MaterialParser.h"
#include "ResourceAllocator.h"
#include "RenderPrimitive.h"

#include "details/BufferObject.h"
#include "details/Camera.h"
#include "details/Fence.h"
#include "details/IndexBuffer.h"
#include "details/IndirectLight.h"
#include "details/Material.h"
#include "details/Renderer.h"
#include "details/Scene.h"
#include "details/SkinningBuffer.h"
#include "details/Skybox.h"
#include "details/Stream.h"
#include "details/SwapChain.h"
#include "details/Texture.h"
#include "details/VertexBuffer.h"
#include "details/View.h"

#include <private/filament/SibStructs.h>

#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>
#include <utils/ThreadUtils.h>

#include <algorithm>
#include <memory>

#include "generated/resources/materials.h"

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;
using namespace filaflat;

FEngine* FEngine::create(Backend backend, Platform* platform,
        void* sharedGLContext, const Config *pConfig) {
    SYSTRACE_ENABLE();
    SYSTRACE_CALL();

    const Config config{ validateConfig(pConfig) };
    FEngine* instance = new FEngine(backend, platform, config, sharedGLContext);

    // initialize all fields that need an instance of FEngine
    // (this cannot be done safely in the ctor)

    // Normally we launch a thread and create the context and Driver from there (see FEngine::loop).
    // In the single-threaded case, we do so in the here and now.
    if (!UTILS_HAS_THREADING) {
        if (platform == nullptr) {
            platform = DefaultPlatform::create(&instance->mBackend);
            instance->mPlatform = platform;
            instance->mOwnPlatform = true;
        }
        if (platform == nullptr) {
            slog.e << "Selected backend not supported in this build." << io::endl;
            delete instance;
            return nullptr;
        }
        DriverConfig driverConfig{ .handleArenaSize = instance->getRequestedDriverHandleArenaSize() };
        instance->mDriver = platform->createDriver(sharedGLContext, driverConfig);

    } else {
        // start the driver thread
        instance->mDriverThread = std::thread(&FEngine::loop, instance);

        // wait for the driver to be ready
        instance->mDriverBarrier.await();

        if (UTILS_UNLIKELY(!instance->mDriver)) {
            // something went horribly wrong during driver initialization
            instance->mDriverThread.join();
            delete instance;
            return nullptr;
        }
    }

    // now we can initialize the largest part of the engine
    instance->init();

    if (!UTILS_HAS_THREADING) {
        instance->execute();
    }

    return instance;
}

#if UTILS_HAS_THREADING

void FEngine::createAsync(CreateCallback callback, void* user,
        Backend backend, Platform* platform, void* sharedGLContext, const Config* config) {
    SYSTRACE_ENABLE();
    SYSTRACE_CALL();
    Config validConfig = validateConfig(config);
    FEngine* instance = new FEngine(backend, platform, validConfig, sharedGLContext);

    // start the driver thread
    instance->mDriverThread = std::thread(&FEngine::loop, instance);

    // launch a thread to call the callback -- so it can't do any damage.
    std::thread callbackThread = std::thread([instance, callback, user]() {
        instance->mDriverBarrier.await();
        callback(user, instance);
    });

    // let the callback thread die on its own
    callbackThread.detach();
}

FEngine* FEngine::getEngine(void* token) {

    FEngine* instance = static_cast<FEngine*>(token);

    ASSERT_PRECONDITION(ThreadUtils::isThisThread(instance->mMainThreadId),
            "Engine::createAsync() and Engine::getEngine() must be called on the same thread.");

    // we use mResourceAllocator as a proxy for "am I already initialized"
    if (!instance->mResourceAllocator) {
        if (UTILS_UNLIKELY(!instance->mDriver)) {
            // something went horribly wrong during driver initialization
            instance->mDriverThread.join();
            delete instance;
            return nullptr;
        }

        // now we can initialize the largest part of the engine
        instance->init();
    }

    return instance;
}

#endif

Engine::Config FEngine::validateConfig(const Config* const pConfig) noexcept
{
    // Rule of thumb: perRenderPassArenaMB must be roughly 1 MB larger than perFrameCommandsMB
    constexpr uint32_t COMMAND_ARENA_OVERHEAD = 1;
    constexpr uint32_t CONCURRENT_FRAME_COUNT = 3;

    Config config;
    if (!pConfig) {
        return config;
    }

    // make sure to copy all the fields
    config = *pConfig;

    // Use at least the defaults set by the build system
    config.minCommandBufferSizeMB = std::max(
            config.minCommandBufferSizeMB,
            (uint32_t)FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB);

    config.perFrameCommandsSizeMB = std::max(
            config.perFrameCommandsSizeMB,
            (uint32_t)FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB);

    config.perRenderPassArenaSizeMB = std::max(
            config.perRenderPassArenaSizeMB,
            (uint32_t)FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB);

    config.commandBufferSizeMB = std::max(
            config.commandBufferSizeMB,
            config.minCommandBufferSizeMB * CONCURRENT_FRAME_COUNT);

    // Enforce pre-render-pass arena rule-of-thumb
    config.perRenderPassArenaSizeMB = std::max(
            config.perRenderPassArenaSizeMB,
            config.perFrameCommandsSizeMB + COMMAND_ARENA_OVERHEAD);

    // This value gets validated during driver creation, so pass it through
    config.driverHandleArenaSizeMB = config.driverHandleArenaSizeMB;

    return config;
}

// these must be static because only a pointer is copied to the render stream
// Note that these coordinates are specified in OpenGL clip space. Other backends can transform
// these in the vertex shader as needed.
static constexpr float4 sFullScreenTriangleVertices[3] = {
        { -1.0f, -1.0f, 1.0f, 1.0f },
        {  3.0f, -1.0f, 1.0f, 1.0f },
        { -1.0f,  3.0f, 1.0f, 1.0f }
};

// these must be static because only a pointer is copied to the render stream
static const uint16_t sFullScreenTriangleIndices[3] = { 0, 1, 2 };

FEngine::FEngine(Backend backend, Platform* platform, const Config& config, void* sharedGLContext) :
        mBackend(backend),
        mPlatform(platform),
        mSharedGLContext(sharedGLContext),
        mPostProcessManager(*this),
        mEntityManager(EntityManager::get()),
        mRenderableManager(*this),
        mTransformManager(),
        mLightManager(*this),
        mCameraManager(*this),
        mCommandBufferQueue(config.minCommandBufferSizeMB * MiB, config.commandBufferSizeMB * MiB),
        mPerRenderPassAllocator("FEngine::mPerRenderPassAllocator", config.perRenderPassArenaSizeMB * MiB),
        mHeapAllocator("FEngine::mHeapAllocator", AreaPolicy::NullArea{}),
        mJobSystem(getJobSystemThreadPoolSize()),
        mEngineEpoch(std::chrono::steady_clock::now()),
        mDriverBarrier(1),
        mMainThreadId(ThreadUtils::getThreadId()),
        mConfig(config)
{
    // we're assuming we're on the main thread here.
    // (it may not be the case)
    mJobSystem.adopt();

    slog.i << "FEngine (" << sizeof(void*) * 8 << " bits) created at " << this << " "
           << "(threading is " << (UTILS_HAS_THREADING ? "enabled)" : "disabled)") << io::endl;
}

uint32_t FEngine::getJobSystemThreadPoolSize() noexcept {
    // 1 thread for the user, 1 thread for the backend
    int threadCount = (int)std::thread::hardware_concurrency() - 2;
    // make sure we have at least 1 thread though
    threadCount = std::max(1, threadCount);
    return threadCount;
}

/*
 * init() is called just after the driver thread is initialized. Driver commands are therefore
 * possible.
 */

void FEngine::init() {
    SYSTRACE_CALL();

#if FILAMENT_ENABLE_FGDBG
    if (!debug.fgdbg) {
        auto server = std::make_shared<fgdbg::DebugServer>();
        debug.fgdbg = std::make_unique<fgdbg::Session>("sample", server);
    }
#endif

    // this must be first.
    assert_invariant( intptr_t(&mDriverApiStorage) % alignof(DriverApi) == 0 );
    ::new(&mDriverApiStorage) DriverApi(*mDriver, mCommandBufferQueue.getCircularBuffer());

    DriverApi& driverApi = getDriverApi();

    slog.i << "FEngine feature level: " << int(driverApi.getFeatureLevel()) << io::endl;

    mResourceAllocator = new ResourceAllocator(driverApi);

    mFullScreenTriangleVb = downcast(VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT4, 0)
            .build(*this));

    mFullScreenTriangleVb->setBufferAt(*this, 0,
            { sFullScreenTriangleVertices, sizeof(sFullScreenTriangleVertices) });

    mFullScreenTriangleIb = downcast(IndexBuffer::Builder()
            .indexCount(3)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(*this));

    mFullScreenTriangleIb->setBuffer(*this,
            { sFullScreenTriangleIndices, sizeof(sFullScreenTriangleIndices) });

    mFullScreenTriangleRph = driverApi.createRenderPrimitive(
            mFullScreenTriangleVb->getHwHandle(), mFullScreenTriangleIb->getHwHandle(),
            PrimitiveType::TRIANGLES, 0, 0, 2, (uint32_t)mFullScreenTriangleIb->getIndexCount());

    // Compute a clip-space [-1 to 1] to texture space [0 to 1] matrix, taking into account
    // backend differences.
    const bool textureSpaceYFlipped = mBackend == Backend::METAL || mBackend == Backend::VULKAN;
    if (textureSpaceYFlipped) {
        mUvFromClipMatrix = mat4f(mat4f::row_major_init{
                0.5f,  0.0f,   0.0f, 0.5f,
                0.0f, -0.5f,   0.0f, 0.5f,
                0.0f,  0.0f,   1.0f, 0.0f,
                0.0f,  0.0f,   0.0f, 1.0f
        });
    } else {
        mUvFromClipMatrix = mat4f(mat4f::row_major_init{
                0.5f,  0.0f,   0.0f, 0.5f,
                0.0f,  0.5f,   0.0f, 0.5f,
                0.0f,  0.0f,   1.0f, 0.0f,
                0.0f,  0.0f,   0.0f, 1.0f
        });
    }

    mDefaultIblTexture = downcast(Texture::Builder()
            .width(1).height(1).levels(1)
            .format(Texture::InternalFormat::RGBA8)
            .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
            .build(*this));

    // 3 bands = 9 float3
    const float sh[9 * 3] = { 0.0f };
    mDefaultIbl = downcast(IndirectLight::Builder()
            .irradiance(3, reinterpret_cast<const float3*>(sh))
            .build(*this));

    mDefaultColorGrading = downcast(ColorGrading::Builder().build(*this));

    // Always initialize the default material, most materials' depth shaders fallback on it.
    mDefaultMaterial = downcast(
            FMaterial::DefaultMaterialBuilder()
                    .package(MATERIALS_DEFAULTMATERIAL_DATA, MATERIALS_DEFAULTMATERIAL_SIZE)
                    .build(*const_cast<FEngine*>(this)));

    // Create a dummy morph target buffer.
    mDummyMorphTargetBuffer = createMorphTargetBuffer(FMorphTargetBuffer::EmptyMorphTargetBuilder());

    float3 dummyPositions[1] = {};
    short4 dummyTangents[1] = {};
    mDummyMorphTargetBuffer->setPositionsAt(*this, 0, dummyPositions, 1, 0);
    mDummyMorphTargetBuffer->setTangentsAt(*this, 0, dummyTangents, 1, 0);

    // create dummy textures we need throughout the engine

    mDummyOneTexture = driverApi.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);

    mDummyOneTextureArray = driverApi.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);

    mDummyZeroTextureArray = driverApi.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);

    mDummyZeroTexture = driverApi.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);


    // initialize the dummy textures so that their contents are not undefined

    static const uint32_t zeroes[6] = {0};
    static const uint32_t ones = 0xffffffff;

    driverApi.update3DImage(mDefaultIblTexture->getHwHandle(), 0, 0, 0, 0, 1, 1, 6,
            { zeroes, sizeof(zeroes), Texture::Format::RGBA, Texture::Type::UBYTE });

    driverApi.update3DImage(mDummyOneTexture, 0, 0, 0, 0, 1, 1, 1,
            { &ones, 4, Texture::Format::RGBA, Texture::Type::UBYTE });

    driverApi.update3DImage(mDummyOneTextureArray, 0, 0, 0, 0, 1, 1, 1,
            { &ones, 4, Texture::Format::RGBA, Texture::Type::UBYTE });

    driverApi.update3DImage(mDummyZeroTexture, 0, 0, 0, 0, 1, 1, 1,
            { zeroes, 4, Texture::Format::RGBA, Texture::Type::UBYTE });

    driverApi.update3DImage(mDummyZeroTextureArray, 0, 0, 0, 0, 1, 1, 1,
            { zeroes, 4, Texture::Format::RGBA, Texture::Type::UBYTE });

    mDefaultRenderTarget = driverApi.createDefaultRenderTarget();

    mPostProcessManager.init();
    mLightManager.init(*this);
    mDFG.init(*this);
}

FEngine::~FEngine() noexcept {
    SYSTRACE_CALL();
    delete mResourceAllocator;
    delete mDriver;
    if (mOwnPlatform) {
        DefaultPlatform::destroy((DefaultPlatform**)&mPlatform);
    }
}

void FEngine::shutdown() {
    SYSTRACE_CALL();

    // by construction this should never be nullptr
    assert_invariant(mResourceAllocator);

    ASSERT_PRECONDITION(ThreadUtils::isThisThread(mMainThreadId),
            "Engine::shutdown() called from the wrong thread!");

#ifndef NDEBUG
    // print out some statistics about this run
    size_t wm = mCommandBufferQueue.getHighWatermark();
    size_t wmpct = wm / (getCommandBufferSize() / 100);
    slog.d << "CircularBuffer: High watermark "
           << wm / 1024 << " KiB (" << wmpct << "%)" << io::endl;
#endif

    DriverApi& driver = getDriverApi();

    /*
     * Destroy our own state first
     */

    mPostProcessManager.terminate(driver);  // free-up post-process manager resources
    mResourceAllocator->terminate();
    mDFG.terminate(*this);                  // free-up the DFG
    mRenderableManager.terminate();         // free-up all renderables
    mLightManager.terminate();              // free-up all lights
    mCameraManager.terminate();             // free-up all cameras

    driver.destroyRenderPrimitive(mFullScreenTriangleRph);
    destroy(mFullScreenTriangleIb);
    destroy(mFullScreenTriangleVb);
    destroy(mDummyMorphTargetBuffer);
    destroy(mDefaultIblTexture);
    destroy(mDefaultIbl);

    destroy(mDefaultColorGrading);

    destroy(mDefaultMaterial);

    /*
     * clean-up after the user -- we call terminate on each "leaked" object and clear each list.
     *
     * This should free up everything.
     */

    // try to destroy objects in the inverse dependency
    cleanupResourceList(std::move(mRenderers));
    cleanupResourceList(std::move(mViews));
    cleanupResourceList(std::move(mScenes));
    cleanupResourceList(std::move(mSkyboxes));
    cleanupResourceList(std::move(mColorGradings));

    // this must be done after Skyboxes and before materials
    destroy(mSkyboxMaterial);

    cleanupResourceList(std::move(mBufferObjects));
    cleanupResourceList(std::move(mIndexBuffers));
    cleanupResourceList(std::move(mMorphTargetBuffers));
    cleanupResourceList(std::move(mSkinningBuffers));
    cleanupResourceList(std::move(mVertexBuffers));
    cleanupResourceList(std::move(mTextures));
    cleanupResourceList(std::move(mRenderTargets));
    cleanupResourceList(std::move(mMaterials));
    for (auto& item : mMaterialInstances) {
        cleanupResourceList(std::move(item.second));
    }

    cleanupResourceListLocked(mFenceListLock, std::move(mFences));

    driver.destroyTexture(mDummyOneTexture);
    driver.destroyTexture(mDummyOneTextureArray);
    driver.destroyTexture(mDummyZeroTexture);
    driver.destroyTexture(mDummyZeroTextureArray);

    driver.destroyRenderTarget(mDefaultRenderTarget);

    /*
     * Shutdown the backend...
     */

    // There might be commands added by the terminate() calls, so we need to flush all commands
    // up to this point. After flushCommandBuffer() is called, all pending commands are guaranteed
    // to be executed before the driver thread exits.
    flushCommandBuffer(mCommandBufferQueue);

    // now wait for all pending commands to be executed and the thread to exit
    mCommandBufferQueue.requestExit();
    if (!UTILS_HAS_THREADING) {
        execute();
        getDriverApi().terminate();
    } else {
        mDriverThread.join();

    }

    // Finally, call user callbacks that might have been scheduled.
    // These callbacks CANNOT call driver APIs.
    getDriver().purge();

    // and destroy the CommandStream
    std::destroy_at(std::launder(reinterpret_cast<DriverApi*>(&mDriverApiStorage)));

    /*
     * Terminate the JobSystem...
     */

    // detach this thread from the JobSystem
    mJobSystem.emancipate();
}

void FEngine::prepare() {
    SYSTRACE_CALL();
    // prepare() is called once per Renderer frame. Ideally we would upload the content of
    // UBOs that are visible only. It's not such a big issue because the actual upload() is
    // skipped is the UBO hasn't changed. Still we could have a lot of these.
    FEngine::DriverApi& driver = getDriverApi();

    for (auto& materialInstanceList: mMaterialInstances) {
        materialInstanceList.second.forEach([&driver](FMaterialInstance* item) {
            item->commit(driver);
        });
    }

    // Commit default material instances.
    mMaterials.forEach([&driver](FMaterial* material) {
#if FILAMENT_ENABLE_MATDBG
        material->checkProgramEdits();
#endif
        material->getDefaultInstance()->commit(driver);
    });
}

void FEngine::gc() {
    // Note: this runs in a Job
    auto& em = mEntityManager;
    mRenderableManager.gc(em);
    mLightManager.gc(em);
    mTransformManager.gc(em);
    mCameraManager.gc(em);
}

void FEngine::flush() {
    // flush the command buffer
    flushCommandBuffer(mCommandBufferQueue);
}

void FEngine::flushAndWait() {

#if defined(__ANDROID__)

    // first make sure we've not terminated filament
    ASSERT_PRECONDITION(!mCommandBufferQueue.isExitRequested(),
            "calling Engine::flushAndWait() after Engine::shutdown()!");

#endif

    // enqueue finish command -- this will stall in the driver until the GPU is done
    getDriverApi().finish();

#if defined(__ANDROID__) && !defined(NDEBUG)

    // then create a fence that will trigger when we're past the finish() above
    size_t tryCount = 8;
    FFence* fence = FEngine::createFence(FFence::Type::SOFT);
    UTILS_NOUNROLL
    do {
        FenceStatus status = fence->wait(FFence::Mode::FLUSH,250000000u);
        // if the fence didn't trigger after 250ms, check that the command queue thread is still
        // running (otherwise indicating a precondition violation).
        if (UTILS_UNLIKELY(status == FenceStatus::TIMEOUT_EXPIRED)) {
            ASSERT_PRECONDITION(!mCommandBufferQueue.isExitRequested(),
                    "called Engine::shutdown() WHILE in Engine::flushAndWait()!");
            tryCount--;
            ASSERT_POSTCONDITION(tryCount, "flushAndWait() failed inexplicably after 2s");
            // if the thread is still running, maybe we just need to give it more time
            continue;
        }
        break;
    } while (true);
    destroy(fence);

#else

    FFence::waitAndDestroy(
            FEngine::createFence(FFence::Type::SOFT), FFence::Mode::FLUSH);

#endif

    // finally, execute callbacks that might have been scheduled
    getDriver().purge();
}

// -----------------------------------------------------------------------------------------------
// Render thread / command queue
// -----------------------------------------------------------------------------------------------

int FEngine::loop() {
    if (mPlatform == nullptr) {
        mPlatform = DefaultPlatform::create(&mBackend);
        mOwnPlatform = true;
        const char* const backend = backendToString(mBackend);
        slog.i << "FEngine resolved backend: " << backend << io::endl;
        if (mPlatform == nullptr) {
            slog.e << "Selected backend not supported in this build." << io::endl;
            mDriverBarrier.latch();
            return 0;
        }
    }

#if FILAMENT_ENABLE_MATDBG
    #ifdef __ANDROID__
        const char* portString = "8081";
    #else
        const char* portString = getenv("FILAMENT_MATDBG_PORT");
    #endif
    if (portString != nullptr) {
        const int port = atoi(portString);
        debug.matdbg = std::make_unique<matdbg::DebugServer>(mBackend, port);

        // Sometimes the server can fail to spin up (e.g. if the above port is already in use).
        // When this occurs, carry onward, developers can look at civetweb.txt for details.
        if (!debug.matdbg->isReady()) {
            delete debug.matdbg.get();
            debug.matdbg = nullptr;
        } else {
            debug.matdbg->setEditCallback(FMaterial::onEditCallback);
            debug.matdbg->setQueryCallback(FMaterial::onQueryCallback);
        }
    }
#endif

    JobSystem::setThreadName("FEngine::loop");
    JobSystem::setThreadPriority(JobSystem::Priority::DISPLAY);

    DriverConfig driverConfig { .handleArenaSize = getRequestedDriverHandleArenaSize() };
    mDriver = mPlatform->createDriver(mSharedGLContext, driverConfig);

    mDriverBarrier.latch();
    if (UTILS_UNLIKELY(!mDriver)) {
        // if we get here, it's because the driver couldn't be initialized and the problem has
        // been logged.
        return 0;
    }

    // We use the highest affinity bit, assuming this is a Big core in a  big.little
    // configuration. This is also a core not used by the JobSystem.
    // Either way the main reason to do this is to avoid this thread jumping from core to core
    // and loose its caches in the process.
    uint32_t id = std::thread::hardware_concurrency() - 1;

    while (true) {
#if FILAMENT_ENABLE_FGDBG
        // TODO (@feresr): Only for testing purposes, remove
        if (debug.fgdbg) {
            static int fgdbgMessage = 0;
            fgdbgMessage++;
            if (fgdbgMessage % 100 == 0) {
                debug.fgdbg->update();
            }
        }
#endif
        // looks like thread affinity needs to be reset regularly (on Android)
        JobSystem::setThreadAffinityById(id);
        if (!execute()) {
            break;
        }
    }

    // terminate() is a synchronous API
    getDriverApi().terminate();
    return 0;
}

void FEngine::flushCommandBuffer(CommandBufferQueue& commandQueue) {
    getDriver().purge();
    commandQueue.flush();
}

const FMaterial* FEngine::getSkyboxMaterial() const noexcept {
    FMaterial const* material = mSkyboxMaterial;
    if (UTILS_UNLIKELY(material == nullptr)) {
        material = FSkybox::createMaterial(*const_cast<FEngine*>(this));
        mSkyboxMaterial = material;
    }
    return material;
}

// -----------------------------------------------------------------------------------------------
// Resource management
// -----------------------------------------------------------------------------------------------

/*
 * Object created from a Builder
 */

template<typename T>
inline T* FEngine::create(ResourceList<T>& list, typename T::Builder const& builder) noexcept {
    T* p = mHeapAllocator.make<T>(*this, builder);
    list.insert(p);
    return p;
}

FBufferObject* FEngine::createBufferObject(const BufferObject::Builder& builder) noexcept {
    return create(mBufferObjects, builder);
}

FVertexBuffer* FEngine::createVertexBuffer(const VertexBuffer::Builder& builder) noexcept {
    return create(mVertexBuffers, builder);
}

FIndexBuffer* FEngine::createIndexBuffer(const IndexBuffer::Builder& builder) noexcept {
    return create(mIndexBuffers, builder);
}

FSkinningBuffer* FEngine::createSkinningBuffer(const SkinningBuffer::Builder& builder) noexcept {
    return create(mSkinningBuffers, builder);
}

FMorphTargetBuffer* FEngine::createMorphTargetBuffer(const MorphTargetBuffer::Builder& builder) noexcept {
    return create(mMorphTargetBuffers, builder);
}

FTexture* FEngine::createTexture(const Texture::Builder& builder) noexcept {
    return create(mTextures, builder);
}

FIndirectLight* FEngine::createIndirectLight(const IndirectLight::Builder& builder) noexcept {
    return create(mIndirectLights, builder);
}

FMaterial* FEngine::createMaterial(const Material::Builder& builder) noexcept {
    return create(mMaterials, builder);
}

FSkybox* FEngine::createSkybox(const Skybox::Builder& builder) noexcept {
    return create(mSkyboxes, builder);
}

FColorGrading* FEngine::createColorGrading(const ColorGrading::Builder& builder) noexcept {
    return create(mColorGradings, builder);
}

FStream* FEngine::createStream(const Stream::Builder& builder) noexcept {
    return create(mStreams, builder);
}

FRenderTarget* FEngine::createRenderTarget(const RenderTarget::Builder& builder) noexcept {
    return create(mRenderTargets, builder);
}

/*
 * Special cases
 */

FRenderer* FEngine::createRenderer() noexcept {
    FRenderer* p = mHeapAllocator.make<FRenderer>(*this);
    if (p) {
        mRenderers.insert(p);
    }
    return p;
}

FMaterialInstance* FEngine::createMaterialInstance(const FMaterial* material,
        const FMaterialInstance* other, const char* name) noexcept {
    FMaterialInstance* p = mHeapAllocator.make<FMaterialInstance>(*this, other, name);
    if (p) {
        auto pos = mMaterialInstances.emplace(material, "MaterialInstance");
        pos.first->second.insert(p);
    }
    return p;
}

/*
 * Objects created without a Builder
 */

FScene* FEngine::createScene() noexcept {
    FScene* p = mHeapAllocator.make<FScene>(*this);
    if (p) {
        mScenes.insert(p);
    }
    return p;
}

FView* FEngine::createView() noexcept {
    FView* p = mHeapAllocator.make<FView>(*this);
    if (p) {
        mViews.insert(p);
    }
    return p;
}

FFence* FEngine::createFence(FFence::Type type) noexcept {
    FFence* p = mHeapAllocator.make<FFence>(*this, type);
    if (p) {
        std::lock_guard guard(mFenceListLock);
        mFences.insert(p);
    }
    return p;
}

FSwapChain* FEngine::createSwapChain(void* nativeWindow, uint64_t flags) noexcept {
    if (UTILS_UNLIKELY(flags & backend::SWAP_CHAIN_CONFIG_APPLE_CVPIXELBUFFER)) {
        // If this flag is set, then the nativeWindow is a CVPixelBufferRef.
        // The call to setupExternalImage is synchronous, and allows the driver to take ownership of
        // the buffer on this thread.
        // For non-Metal backends, this is a no-op.
        getDriverApi().setupExternalImage(nativeWindow);
    }
    FSwapChain* p = mHeapAllocator.make<FSwapChain>(*this, nativeWindow, flags);
    if (p) {
        mSwapChains.insert(p);
    }
    return p;
}

FSwapChain* FEngine::createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept {
    FSwapChain* p = mHeapAllocator.make<FSwapChain>(*this, width, height, flags);
    if (p) {
        mSwapChains.insert(p);
    }
    return p;
}

/*
 * Objects created with a component manager
 */


FCamera* FEngine::createCamera(Entity entity) noexcept {
    return mCameraManager.create(entity);
}

FCamera* FEngine::getCameraComponent(Entity entity) noexcept {
    auto ci = mCameraManager.getInstance(entity);
    return ci ? mCameraManager.getCamera(ci) : nullptr;
}

void FEngine::destroyCameraComponent(utils::Entity entity) noexcept {
    mCameraManager.destroy(entity);
}


void FEngine::createRenderable(const RenderableManager::Builder& builder, Entity entity) {
    mRenderableManager.create(builder, entity);
    auto& tcm = mTransformManager;
    // if this entity doesn't have a transform component, add one.
    if (!tcm.hasComponent(entity)) {
        tcm.create(entity, 0, mat4f());
    }
}

void FEngine::createLight(const LightManager::Builder& builder, Entity entity) {
    mLightManager.create(builder, entity);
}

// -----------------------------------------------------------------------------------------------

template<typename T>
UTILS_NOINLINE
void FEngine::cleanupResourceList(ResourceList<T>&& list) {
    if (UTILS_UNLIKELY(!list.empty())) {
#ifndef NDEBUG
        slog.d << "cleaning up " << list.size()
               << " leaked " << CallStack::typeName<T>().c_str() << io::endl;
#endif
        list.forEach([this, &allocator = mHeapAllocator](T* item) {
            item->terminate(*this);
            allocator.destroy(item);
        });
        list.clear();
    }
}
template<typename T, typename Lock>
UTILS_NOINLINE
void FEngine::cleanupResourceListLocked(Lock& lock, ResourceList<T>&& list) {
    // copy the list with the lock held, then proceed as usual
    lock.lock();
    auto copy(std::move(list));
    lock.unlock();
    cleanupResourceList(std::move(copy));
}

// -----------------------------------------------------------------------------------------------

template<typename T>
UTILS_ALWAYS_INLINE
inline bool FEngine::terminateAndDestroy(const T* ptr, ResourceList<T>& list) {
    if (ptr == nullptr) return true;
    bool success = list.remove(ptr);

#if UTILS_HAS_RTTI
    auto typeName = CallStack::typeName<T>();
    const char * const typeNameCStr = typeName.c_str();
#else
    const char * const typeNameCStr = "<no-rtti>";
#endif

    if (ASSERT_PRECONDITION_NON_FATAL(success,
            "Object %s at %p doesn't exist (double free?)", typeNameCStr, ptr)) {
        const_cast<T*>(ptr)->terminate(*this);
        mHeapAllocator.destroy(const_cast<T*>(ptr));
    }
    return success;
}

template<typename T, typename Lock>
UTILS_ALWAYS_INLINE
inline bool FEngine::terminateAndDestroyLocked(Lock& lock, const T* ptr, ResourceList<T>& list) {
    if (ptr == nullptr) return true;
    lock.lock();
    bool success = list.remove(ptr);
    lock.unlock();

#if UTILS_HAS_RTTI
    auto typeName = CallStack::typeName<T>();
    const char * const typeNameCStr = typeName.c_str();
#else
    const char * const typeNameCStr = "<no-rtti>";
#endif

    if (ASSERT_PRECONDITION_NON_FATAL(success,
            "Object %s at %p doesn't exist (double free?)", typeNameCStr, ptr)) {
        const_cast<T*>(ptr)->terminate(*this);
        mHeapAllocator.destroy(const_cast<T*>(ptr));
    }
    return success;
}

// -----------------------------------------------------------------------------------------------

UTILS_NOINLINE
bool FEngine::destroy(const FBufferObject* p) {
    return terminateAndDestroy(p, mBufferObjects);
}

UTILS_NOINLINE
bool FEngine::destroy(const FVertexBuffer* p) {
    return terminateAndDestroy(p, mVertexBuffers);
}

UTILS_NOINLINE
bool FEngine::destroy(const FIndexBuffer* p) {
    return terminateAndDestroy(p, mIndexBuffers);
}

UTILS_NOINLINE
bool FEngine::destroy(const FSkinningBuffer* p) {
    return terminateAndDestroy(p, mSkinningBuffers);
}

UTILS_NOINLINE
bool FEngine::destroy(const FMorphTargetBuffer* p) {
    return terminateAndDestroy(p, mMorphTargetBuffers);
}

UTILS_NOINLINE
bool FEngine::destroy(const FRenderer* p) {
    return terminateAndDestroy(p, mRenderers);
}

UTILS_NOINLINE
bool FEngine::destroy(const FScene* p) {
    return terminateAndDestroy(p, mScenes);
}

UTILS_NOINLINE
bool FEngine::destroy(const FSkybox* p) {
    return terminateAndDestroy(p, mSkyboxes);
}

UTILS_NOINLINE
bool FEngine::destroy(const FColorGrading* p) {
    return terminateAndDestroy(p, mColorGradings);
}

UTILS_NOINLINE
bool FEngine::destroy(const FTexture* p) {
    return terminateAndDestroy(p, mTextures);
}

UTILS_NOINLINE
bool FEngine::destroy(const FRenderTarget* p) {
    return terminateAndDestroy(p, mRenderTargets);
}

UTILS_NOINLINE
bool FEngine::destroy(const FView* p) {
    return terminateAndDestroy(p, mViews);
}

UTILS_NOINLINE
bool FEngine::destroy(const FIndirectLight* p) {
    return terminateAndDestroy(p, mIndirectLights);
}

UTILS_NOINLINE
bool FEngine::destroy(const FFence* p) {
    return terminateAndDestroyLocked(mFenceListLock, p, mFences);
}

UTILS_NOINLINE
bool FEngine::destroy(const FSwapChain* p) {
    return terminateAndDestroy(p, mSwapChains);
}

UTILS_NOINLINE
bool FEngine::destroy(const FStream* p) {
    return terminateAndDestroy(p, mStreams);
}


UTILS_NOINLINE
bool FEngine::destroy(const FMaterial* ptr) {
    if (ptr == nullptr) return true;
    auto pos = mMaterialInstances.find(ptr);
    if (pos != mMaterialInstances.cend()) {
        // ensure we've destroyed all instances before destroying the material
        if (!ASSERT_PRECONDITION_NON_FATAL(pos->second.empty(),
                "destroying material \"%s\" but %u instances still alive",
                ptr->getName().c_str(), (*pos).second.size())) {
            return false;
        }
    }
    return terminateAndDestroy(ptr, mMaterials);
}

UTILS_NOINLINE
bool FEngine::destroy(const FMaterialInstance* ptr) {
    if (ptr == nullptr) return true;
    auto pos = mMaterialInstances.find(ptr->getMaterial());
    assert_invariant(pos != mMaterialInstances.cend());
    if (pos != mMaterialInstances.cend()) {
        return terminateAndDestroy(ptr, pos->second);
    }
    // if we don't find this instance's material it might be because it's the default instance
    // in which case it fine to ignore.
    return true;
}

UTILS_NOINLINE
void FEngine::destroy(Entity e) {
    mRenderableManager.destroy(e);
    mLightManager.destroy(e);
    mTransformManager.destroy(e);
    mCameraManager.destroy(e);
}

void* FEngine::streamAlloc(size_t size, size_t alignment) noexcept {
    // we allow this only for small allocations
    if (size > 65536) {
        return nullptr;
    }
    return getDriverApi().allocate(size, alignment);
}

bool FEngine::execute() {

    // wait until we get command buffers to be executed (or thread exit requested)
    auto buffers = mCommandBufferQueue.waitForCommands();
    if (UTILS_UNLIKELY(buffers.empty())) {
        return false;
    }

    // execute all command buffers
    for (auto& item : buffers) {
        if (UTILS_LIKELY(item.begin)) {
            getDriverApi().execute(item.begin);
            mCommandBufferQueue.releaseBuffer(item);
        }
    }

    return true;
}

void FEngine::destroy(FEngine* engine) {
    if (engine) {
        engine->shutdown();
        delete engine;
    }
}

Engine::FeatureLevel FEngine::getSupportedFeatureLevel() const noexcept {
    FEngine::DriverApi& driver = const_cast<FEngine*>(this)->getDriverApi();
    return driver.getFeatureLevel();
}

Engine::FeatureLevel FEngine::setActiveFeatureLevel(FeatureLevel featureLevel) {
    ASSERT_PRECONDITION(featureLevel <= getSupportedFeatureLevel(),
            "Feature level %u not supported", (unsigned)featureLevel);
    return (mActiveFeatureLevel = std::max(mActiveFeatureLevel, featureLevel));
}

#if defined(__EMSCRIPTEN__)
void FEngine::resetBackendState() noexcept {
    getDriverApi().resetState();
}
#endif

} // namespace filament
