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

#include <filament/MaterialEnums.h>

#include <private/filament/DescriptorSets.h>

#include <private/backend/PlatformFactory.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/PrivateImplementation-impl.h>
#include <utils/Systrace.h>
#include <utils/ThreadUtils.h>

#include <algorithm>
#include <memory>
#include <thread>

#include "generated/resources/materials.h"

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;
using namespace filaflat;

struct Engine::BuilderDetails {
    Backend mBackend = Backend::DEFAULT;
    Platform* mPlatform = nullptr;
    Engine::Config mConfig;
    FeatureLevel mFeatureLevel = FeatureLevel::FEATURE_LEVEL_1;
    void* mSharedContext = nullptr;
    bool mPaused = false;
    static Config validateConfig(Config config) noexcept;
};

Engine* FEngine::create(Engine::Builder const& builder) {
    SYSTRACE_ENABLE();
    SYSTRACE_CALL();

    FEngine* instance = new FEngine(builder);

    // initialize all fields that need an instance of FEngine
    // (this cannot be done safely in the ctor)

    // Normally we launch a thread and create the context and Driver from there (see FEngine::loop).
    // In the single-threaded case, we do so in the here and now.
    if (!UTILS_HAS_THREADING) {
        Platform* platform = builder->mPlatform;
        void* const sharedContext = builder->mSharedContext;

        if (platform == nullptr) {
            platform = PlatformFactory::create(&instance->mBackend);
            instance->mPlatform = platform;
            instance->mOwnPlatform = true;
        }
        if (platform == nullptr) {
            slog.e << "Selected backend not supported in this build." << io::endl;
            delete instance;
            return nullptr;
        }
        DriverConfig const driverConfig{
                .handleArenaSize = instance->getRequestedDriverHandleArenaSize(),
                .textureUseAfterFreePoolSize = instance->getConfig().textureUseAfterFreePoolSize,
                .metalUploadBufferSizeBytes = instance->getConfig().metalUploadBufferSizeBytes,
                .disableParallelShaderCompile = instance->getConfig().disableParallelShaderCompile,
                .disableHandleUseAfterFreeCheck = instance->getConfig().disableHandleUseAfterFreeCheck,
                .forceGLES2Context = instance->getConfig().forceGLES2Context,
                .stereoscopicType =  instance->getConfig().stereoscopicType,
        };
        instance->mDriver = platform->createDriver(sharedContext, driverConfig);

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

void FEngine::create(Engine::Builder const& builder, Invocable<void(void*)>&& callback) {
    SYSTRACE_ENABLE();
    SYSTRACE_CALL();

    FEngine* instance = new FEngine(builder);

    // start the driver thread
    instance->mDriverThread = std::thread(&FEngine::loop, instance);

    // launch a thread to call the callback -- so it can't do any damage.
    std::thread callbackThread = std::thread([instance, callback = std::move(callback)]() {
        instance->mDriverBarrier.await();
        callback(instance);
    });

    // let the callback thread die on its own
    callbackThread.detach();
}

FEngine* FEngine::getEngine(void* token) {

    FEngine* instance = static_cast<FEngine*>(token);

    FILAMENT_CHECK_PRECONDITION(ThreadUtils::isThisThread(instance->mMainThreadId))
            << "Engine::createAsync() and Engine::getEngine() must be called on the same thread.";

    if (!instance->mInitialized) {
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

FEngine::FEngine(Engine::Builder const& builder) :
        mBackend(builder->mBackend),
        mActiveFeatureLevel(builder->mFeatureLevel),
        mPlatform(builder->mPlatform),
        mSharedGLContext(builder->mSharedContext),
        mPostProcessManager(*this),
        mEntityManager(EntityManager::get()),
        mRenderableManager(*this),
        mTransformManager(),
        mLightManager(*this),
        mCameraManager(*this),
        mCommandBufferQueue(
                builder->mConfig.minCommandBufferSizeMB * MiB,
                builder->mConfig.commandBufferSizeMB * MiB,
                builder->mPaused),
        mPerRenderPassArena(
                "FEngine::mPerRenderPassAllocator",
                builder->mConfig.perRenderPassArenaSizeMB * MiB),
        mHeapAllocator("FEngine::mHeapAllocator", AreaPolicy::NullArea{}),
        mJobSystem(getJobSystemThreadPoolSize(builder->mConfig)),
        mEngineEpoch(std::chrono::steady_clock::now()),
        mDriverBarrier(1),
        mMainThreadId(ThreadUtils::getThreadId()),
        mConfig(builder->mConfig)
{
    // we're assuming we're on the main thread here.
    // (it may not be the case)
    mJobSystem.adopt();

    slog.i << "FEngine (" << sizeof(void*) * 8 << " bits) created at " << this << " "
           << "(threading is " << (UTILS_HAS_THREADING ? "enabled)" : "disabled)") << io::endl;
}

uint32_t FEngine::getJobSystemThreadPoolSize(Engine::Config const& config) noexcept {
    if (config.jobSystemThreadCount > 0) {
        return config.jobSystemThreadCount;
    }

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

    // this must be first.
    assert_invariant( intptr_t(&mDriverApiStorage) % alignof(DriverApi) == 0 );
    ::new(&mDriverApiStorage) DriverApi(*mDriver, mCommandBufferQueue.getCircularBuffer());

    DriverApi& driverApi = getDriverApi();

    mActiveFeatureLevel = std::min(mActiveFeatureLevel, driverApi.getFeatureLevel());

#ifndef FILAMENT_ENABLE_FEATURE_LEVEL_0
    assert_invariant(mActiveFeatureLevel > FeatureLevel::FEATURE_LEVEL_0);
#endif

    slog.i << "Backend feature level: " << int(driverApi.getFeatureLevel()) << io::endl;
    slog.i << "FEngine feature level: " << int(mActiveFeatureLevel) << io::endl;


    mResourceAllocatorDisposer = std::make_shared<ResourceAllocatorDisposer>(driverApi);

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
            PrimitiveType::TRIANGLES);

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

    // initialize the dummy textures so that their contents are not undefined
    static const uint32_t zeroes[6] = {0};
    static const uint32_t ones = 0xffffffff;

    mDefaultIblTexture = downcast(Texture::Builder()
            .width(1).height(1).levels(1)
            .format(Texture::InternalFormat::RGBA8)
            .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
            .build(*this));

    driverApi.update3DImage(mDefaultIblTexture->getHwHandle(), 0, 0, 0, 0, 1, 1, 6,
            { zeroes, sizeof(zeroes), Texture::Format::RGBA, Texture::Type::UBYTE });

    // 3 bands = 9 float3
    const float sh[9 * 3] = { 0.0f };
    mDefaultIbl = downcast(IndirectLight::Builder()
            .irradiance(3, reinterpret_cast<const float3*>(sh))
            .build(*this));

    mDefaultRenderTarget = driverApi.createDefaultRenderTarget();

    // Create a dummy morph target buffer, without using the builder
    mDummyMorphTargetBuffer = createMorphTargetBuffer(
            FMorphTargetBuffer::EmptyMorphTargetBuilder());

    // create dummy textures we need throughout the engine
    mDummyOneTexture = driverApi.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);

    mDummyZeroTexture = driverApi.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);

    driverApi.update3DImage(mDummyOneTexture, 0, 0, 0, 0, 1, 1, 1,
            { &ones, 4, Texture::Format::RGBA, Texture::Type::UBYTE });

    driverApi.update3DImage(mDummyZeroTexture, 0, 0, 0, 0, 1, 1, 1,
            { zeroes, 4, Texture::Format::RGBA, Texture::Type::UBYTE });


    mPerViewDescriptorSetLayoutSsrVariant = {
            driverApi,
            descriptor_sets::getSsrVariantLayout() };

    mPerViewDescriptorSetLayoutDepthVariant = {
            driverApi,
            descriptor_sets::getDepthVariantLayout() };

    mPerRenderableDescriptorSetLayout = {
            driverApi,
            descriptor_sets::getPerRenderableLayout() };

#ifdef FILAMENT_ENABLE_FEATURE_LEVEL_0
    if (UTILS_UNLIKELY(mActiveFeatureLevel == FeatureLevel::FEATURE_LEVEL_0)) {
        FMaterial::DefaultMaterialBuilder defaultMaterialBuilder;
        defaultMaterialBuilder.package(
                MATERIALS_DEFAULTMATERIAL_FL0_DATA, MATERIALS_DEFAULTMATERIAL_FL0_SIZE);
        mDefaultMaterial = downcast(defaultMaterialBuilder.build(*const_cast<FEngine*>(this)));
    } else
#endif
    {
        mDefaultColorGrading = downcast(ColorGrading::Builder().build(*this));

        FMaterial::DefaultMaterialBuilder defaultMaterialBuilder;
        switch (mConfig.stereoscopicType) {
            case StereoscopicType::NONE:
            case StereoscopicType::INSTANCED:
                defaultMaterialBuilder.package(
                    MATERIALS_DEFAULTMATERIAL_DATA, MATERIALS_DEFAULTMATERIAL_SIZE);
                break;
            case StereoscopicType::MULTIVIEW:
#ifdef FILAMENT_ENABLE_MULTIVIEW
                defaultMaterialBuilder.package(
                    MATERIALS_DEFAULTMATERIAL_MULTIVIEW_DATA, MATERIALS_DEFAULTMATERIAL_MULTIVIEW_SIZE);
#else
                assert_invariant(false);
#endif
                break;
        }
        mDefaultMaterial = downcast(defaultMaterialBuilder.build(*const_cast<FEngine*>(this)));

        float3 dummyPositions[1] = {};
        short4 dummyTangents[1] = {};
        mDummyMorphTargetBuffer->setPositionsAt(*this, 0, dummyPositions, 1, 0);
        mDummyMorphTargetBuffer->setTangentsAt(*this, 0, dummyTangents, 1, 0);

        mDummyOneTextureArray = driverApi.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1,
                TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);

        mDummyZeroTextureArray = driverApi.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1,
                TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);

        driverApi.update3DImage(mDummyOneTextureArray, 0, 0, 0, 0, 1, 1, 1,
                { &ones, 4, Texture::Format::RGBA, Texture::Type::UBYTE });

        driverApi.update3DImage(mDummyZeroTextureArray, 0, 0, 0, 0, 1, 1, 1,
                { zeroes, 4, Texture::Format::RGBA, Texture::Type::UBYTE });

        mLightManager.init(*this);
        mDFG.init(*this);
    }

    mPostProcessManager.init();

    mDebugRegistry.registerProperty("d.shadowmap.debug_directional_shadowmap",
            &debug.shadowmap.debug_directional_shadowmap, [this]() {
                mMaterials.forEach([this](FMaterial* material) {
                    if (material->getMaterialDomain() == MaterialDomain::SURFACE) {

                        material->setConstant(
                                +ReservedSpecializationConstants::CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP,
                                debug.shadowmap.debug_directional_shadowmap);

                        material->invalidate(
                                Variant::DIR | Variant::SRE | Variant::DEP,
                                Variant::DIR | Variant::SRE);
                    }
                });
            });

    mDebugRegistry.registerProperty("d.lighting.debug_froxel_visualization",
            &debug.lighting.debug_froxel_visualization, [this]() {
                mMaterials.forEach([this](FMaterial* material) {
                    if (material->getMaterialDomain() == MaterialDomain::SURFACE) {

                        material->setConstant(
                                +ReservedSpecializationConstants::CONFIG_DEBUG_FROXEL_VISUALIZATION,
                                debug.lighting.debug_froxel_visualization);

                        material->invalidate(
                                Variant::DYN | Variant::DEP,
                                Variant::DYN);
                    }
                });
            });

    mInitialized = true;
}

FEngine::~FEngine() noexcept {
    SYSTRACE_CALL();
    assert_invariant(!mResourceAllocatorDisposer);
    delete mDriver;
    if (mOwnPlatform) {
        PlatformFactory::destroy(&mPlatform);
    }
}

void FEngine::shutdown() {
    SYSTRACE_CALL();

    // by construction this should never be nullptr
    assert_invariant(mResourceAllocatorDisposer);

    FILAMENT_CHECK_PRECONDITION(ThreadUtils::isThisThread(mMainThreadId))
            << "Engine::shutdown() called from the wrong thread!";

#ifndef NDEBUG
    // print out some statistics about this run
    size_t const wm = mCommandBufferQueue.getHighWatermark();
    size_t const wmpct = wm / (getCommandBufferSize() / 100);
    slog.d << "CircularBuffer: High watermark "
           << wm / 1024 << " KiB (" << wmpct << "%)" << io::endl;
#endif

    DriverApi& driver = getDriverApi();

    /*
     * Destroy our own state first
     */

    mPostProcessManager.terminate(driver);  // free-up post-process manager resources
    mResourceAllocatorDisposer->terminate();
    mResourceAllocatorDisposer.reset();
    mDFG.terminate(*this);                  // free-up the DFG
    mRenderableManager.terminate();         // free-up all renderables
    mLightManager.terminate();              // free-up all lights
    mCameraManager.terminate(*this);        // free-up all cameras

    mPerViewDescriptorSetLayoutDepthVariant.terminate(driver);
    mPerViewDescriptorSetLayoutSsrVariant.terminate(driver);
    mPerRenderableDescriptorSetLayout.terminate(driver);

    driver.destroyRenderPrimitive(mFullScreenTriangleRph);

    destroy(mFullScreenTriangleIb);
    destroy(mFullScreenTriangleVb);
    destroy(mDummyMorphTargetBuffer);
    destroy(mDefaultIblTexture);
    destroy(mDefaultIbl);

    destroy(mDefaultColorGrading);

    destroy(mDefaultMaterial);

    destroy(mUnprotectedDummySwapchain);

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
    cleanupResourceList(std::move(mInstanceBuffers));
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

    // There might be commands added by the `terminate()` calls, so we need to flush all commands
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
        // Driver::terminate() has been called here.
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
    // skipped if the UBO hasn't changed. Still we could have a lot of these.
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
    mCameraManager.gc(*this, em);
}

void FEngine::flush() {
    // flush the command buffer
    flushCommandBuffer(mCommandBufferQueue);
}

void FEngine::flushAndWait() {
    FILAMENT_CHECK_PRECONDITION(!mCommandBufferQueue.isPaused())
            << "Cannot call flushAndWait() when rendering thread is paused!";

#if defined(__ANDROID__)

    // first make sure we've not terminated filament
    FILAMENT_CHECK_PRECONDITION(!mCommandBufferQueue.isExitRequested())
            << "calling Engine::flushAndWait() after Engine::shutdown()!";

#endif

    // enqueue finish command -- this will stall in the driver until the GPU is done
    getDriverApi().finish();

#if defined(__ANDROID__) && !defined(NDEBUG)

    // then create a fence that will trigger when we're past the finish() above
    size_t tryCount = 8;
    FFence* fence = FEngine::createFence();
    UTILS_NOUNROLL
    do {
        FenceStatus status = fence->wait(FFence::Mode::FLUSH,250000000u);
        // if the fence didn't trigger after 250ms, check that the command queue thread is still
        // running (otherwise indicating a precondition violation).
        if (UTILS_UNLIKELY(status == FenceStatus::TIMEOUT_EXPIRED)) {
            FILAMENT_CHECK_PRECONDITION(!mCommandBufferQueue.isExitRequested())
                    << "called Engine::shutdown() WHILE in Engine::flushAndWait()!";
            tryCount--;
            FILAMENT_CHECK_POSTCONDITION(tryCount) << "flushAndWait() failed inexplicably after 2s";
            // if the thread is still running, maybe we just need to give it more time
            continue;
        }
        break;
    } while (true);
    destroy(fence);

#else

    FFence::waitAndDestroy(FEngine::createFence(), FFence::Mode::FLUSH);

#endif

    // finally, execute callbacks that might have been scheduled
    getDriver().purge();
}

// -----------------------------------------------------------------------------------------------
// Render thread / command queue
// -----------------------------------------------------------------------------------------------

int FEngine::loop() {
    if (mPlatform == nullptr) {
        mPlatform = PlatformFactory::create(&mBackend);
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
        debug.server = new matdbg::DebugServer(mBackend, port);

        // Sometimes the server can fail to spin up (e.g. if the above port is already in use).
        // When this occurs, carry onward, developers can look at civetweb.txt for details.
        if (!debug.server->isReady()) {
            delete debug.server;
            debug.server = nullptr;
        } else {
            debug.server->setEditCallback(FMaterial::onEditCallback);
            debug.server->setQueryCallback(FMaterial::onQueryCallback);
        }
    }
#endif

    JobSystem::setThreadName("FEngine::loop");
    JobSystem::setThreadPriority(JobSystem::Priority::DISPLAY);

    DriverConfig const driverConfig {
            .handleArenaSize = getRequestedDriverHandleArenaSize(),
            .textureUseAfterFreePoolSize = mConfig.textureUseAfterFreePoolSize,
            .metalUploadBufferSizeBytes = mConfig.metalUploadBufferSizeBytes,
            .disableParallelShaderCompile = mConfig.disableParallelShaderCompile,
            .disableHandleUseAfterFreeCheck = mConfig.disableHandleUseAfterFreeCheck,
            .forceGLES2Context = mConfig.forceGLES2Context,
            .stereoscopicType =  mConfig.stereoscopicType,
    };
    mDriver = mPlatform->createDriver(mSharedGLContext, driverConfig);

    mDriverBarrier.latch();
    if (UTILS_UNLIKELY(!mDriver)) {
        // if we get here, it's because the driver couldn't be initialized and the problem has
        // been logged.
        return 0;
    }

    while (true) {
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

template<typename T, typename ... ARGS>
inline T* FEngine::create(ResourceList<T>& list,
        typename T::Builder const& builder, ARGS&& ... args) noexcept {
    T* p = mHeapAllocator.make<T>(*this, builder, std::forward<ARGS>(args)...);
    if (UTILS_UNLIKELY(p)) { // this should never happen
        list.insert(p);
    }
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

FInstanceBuffer* FEngine::createInstanceBuffer(const InstanceBuffer::Builder& builder) noexcept {
    return create(mInstanceBuffers, builder);
}

FTexture* FEngine::createTexture(const Texture::Builder& builder) noexcept {
    return create(mTextures, builder);
}

FIndirectLight* FEngine::createIndirectLight(const IndirectLight::Builder& builder) noexcept {
    return create(mIndirectLights, builder);
}

FMaterial* FEngine::createMaterial(const Material::Builder& builder,
        std::unique_ptr<MaterialParser> materialParser) noexcept {
    return create(mMaterials, builder, std::move(materialParser));
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
    if (UTILS_UNLIKELY(p)) { // should never happen
        mRenderers.insert(p);
    }
    return p;
}

FMaterialInstance* FEngine::createMaterialInstance(const FMaterial* material,
        const FMaterialInstance* other, const char* name) noexcept {
    FMaterialInstance* p = mHeapAllocator.make<FMaterialInstance>(*this, other, name);
    if (UTILS_UNLIKELY(p)) { // should never happen
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
    if (UTILS_UNLIKELY(p)) { // should never happen
        mScenes.insert(p);
    }
    return p;
}

FView* FEngine::createView() noexcept {
    FView* p = mHeapAllocator.make<FView>(*this);
    if (UTILS_UNLIKELY(p)) { // should never happen
        mViews.insert(p);
    }
    return p;
}

FFence* FEngine::createFence() noexcept {
    FFence* p = mHeapAllocator.make<FFence>(*this);
    if (UTILS_UNLIKELY(p)) { // should never happen
        std::lock_guard const guard(mFenceListLock);
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
    if (UTILS_UNLIKELY(p)) { // should never happen
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
    return mCameraManager.create(*this, entity);
}

FCamera* FEngine::getCameraComponent(Entity entity) noexcept {
    auto ci = mCameraManager.getInstance(entity);
    return ci ? mCameraManager.getCamera(ci) : nullptr;
}

void FEngine::destroyCameraComponent(utils::Entity entity) noexcept {
    mCameraManager.destroy(*this, entity);
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
inline bool FEngine::isValid(const T* ptr, ResourceList<T> const& list) const {
    auto& l = const_cast<ResourceList<T>&>(list);
    return l.find(ptr) != l.end();
}

template<typename T>
UTILS_ALWAYS_INLINE
inline bool FEngine::terminateAndDestroy(const T* ptr, ResourceList<T>& list) {
    if (ptr == nullptr) return true;
    bool const success = list.remove(ptr);

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
    bool const success = list.remove(ptr);
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
bool FEngine::destroy(const FInstanceBuffer* p){
    return terminateAndDestroy(p, mInstanceBuffers);
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
    mCameraManager.destroy(*this, e);
}

bool FEngine::isValid(const FBufferObject* p) const {
    return isValid(p, mBufferObjects);
}

bool FEngine::isValid(const FVertexBuffer* p) const {
    return isValid(p, mVertexBuffers);
}

bool FEngine::isValid(const FFence* p) const {
    return isValid(p, mFences);
}

bool FEngine::isValid(const FIndexBuffer* p) const {
    return isValid(p, mIndexBuffers);
}

bool FEngine::isValid(const FSkinningBuffer* p) const {
    return isValid(p, mSkinningBuffers);
}

bool FEngine::isValid(const FMorphTargetBuffer* p) const {
    return isValid(p, mMorphTargetBuffers);
}

bool FEngine::isValid(const FIndirectLight* p) const {
    return isValid(p, mIndirectLights);
}

bool FEngine::isValid(const FMaterial* p) const {
    return isValid(p, mMaterials);
}

bool FEngine::isValid(const FMaterial* m, const FMaterialInstance* p) const {
    // first make sure the material we're given is valid.
    if (!isValid(m)) {
        return false;
    }

    // then find the material instance list for that material
    auto it = mMaterialInstances.find(m);
    if (it == mMaterialInstances.end()) {
        // this could happen if this material has no material instances at all
        return false;
    }

    // finally validate the material instance
    return isValid(p, it->second);
}

bool FEngine::isValidExpensive(const FMaterialInstance* p) const {
    return std::any_of(mMaterialInstances.cbegin(), mMaterialInstances.cend(),
            [this, p](auto&& entry) {
        return isValid(p, entry.second);
    });
}

bool FEngine::isValid(const FRenderer* p) const {
    return isValid(p, mRenderers);
}

bool FEngine::isValid(const FScene* p) const {
    return isValid(p, mScenes);
}

bool FEngine::isValid(const FSkybox* p) const {
    return isValid(p, mSkyboxes);
}

bool FEngine::isValid(const FColorGrading* p) const {
    return isValid(p, mColorGradings);
}

bool FEngine::isValid(const FSwapChain* p) const {
    return isValid(p, mSwapChains);
}

bool FEngine::isValid(const FStream* p) const {
    return isValid(p, mStreams);
}

bool FEngine::isValid(const FTexture* p) const {
    return isValid(p, mTextures);
}

bool FEngine::isValid(const FRenderTarget* p) const {
    return isValid(p, mRenderTargets);
}

bool FEngine::isValid(const FView* p) const {
    return isValid(p, mViews);
}

bool FEngine::isValid(const FInstanceBuffer* p) const {
    return isValid(p, mInstanceBuffers);
}

size_t FEngine::getBufferObjectCount() const noexcept { return mBufferObjects.size(); }
size_t FEngine::getViewCount() const noexcept { return mViews.size(); }
size_t FEngine::getSceneCount() const noexcept { return mScenes.size(); }
size_t FEngine::getSwapChainCount() const noexcept { return mSwapChains.size(); }
size_t FEngine::getStreamCount() const noexcept { return mStreams.size(); }
size_t FEngine::getIndexBufferCount() const noexcept { return mIndexBuffers.size(); }
size_t FEngine::getSkinningBufferCount() const noexcept { return mSkinningBuffers.size(); }
size_t FEngine::getMorphTargetBufferCount() const noexcept { return mMorphTargetBuffers.size(); }
size_t FEngine::getInstanceBufferCount() const noexcept { return mInstanceBuffers.size(); }
size_t FEngine::getVertexBufferCount() const noexcept { return mVertexBuffers.size(); }
size_t FEngine::getIndirectLightCount() const noexcept { return mIndirectLights.size(); }
size_t FEngine::getMaterialCount() const noexcept { return mMaterials.size(); }
size_t FEngine::getTextureCount() const noexcept { return mTextures.size(); }
size_t FEngine::getSkyboxeCount() const noexcept { return mSkyboxes.size(); }
size_t FEngine::getColorGradingCount() const noexcept { return mColorGradings.size(); }
size_t FEngine::getRenderTargetCount() const noexcept { return mRenderTargets.size(); }

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
    auto& driver = getDriverApi();
    for (auto& item : buffers) {
        if (UTILS_LIKELY(item.begin)) {
            driver.execute(item.begin);
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

bool FEngine::isPaused() const noexcept {
    return mCommandBufferQueue.isPaused();
}

void FEngine::setPaused(bool paused) {
    mCommandBufferQueue.setPaused(paused);
}

Engine::FeatureLevel FEngine::getSupportedFeatureLevel() const noexcept {
    FEngine::DriverApi& driver = const_cast<FEngine*>(this)->getDriverApi();
    return driver.getFeatureLevel();
}

Engine::FeatureLevel FEngine::setActiveFeatureLevel(FeatureLevel featureLevel) {
    FILAMENT_CHECK_PRECONDITION(featureLevel <= getSupportedFeatureLevel())
            << "Feature level " << (unsigned)featureLevel << " not supported";
    FILAMENT_CHECK_PRECONDITION(mActiveFeatureLevel >= FeatureLevel::FEATURE_LEVEL_1)
            << "Cannot adjust feature level beyond 0 at runtime";
    return (mActiveFeatureLevel = std::max(mActiveFeatureLevel, featureLevel));
}

#if defined(__EMSCRIPTEN__)
void FEngine::resetBackendState() noexcept {
    getDriverApi().resetState();
}
#endif

void FEngine::unprotected() noexcept {
    if (UTILS_UNLIKELY(!mUnprotectedDummySwapchain)) {
        mUnprotectedDummySwapchain = createSwapChain(1, 1, 0);
    }
    mUnprotectedDummySwapchain->makeCurrent(getDriverApi());
}

// ------------------------------------------------------------------------------------------------

Engine::Builder::Builder() noexcept = default;
Engine::Builder::~Builder() noexcept = default;
Engine::Builder::Builder(Engine::Builder const& rhs) noexcept = default;
Engine::Builder::Builder(Engine::Builder&& rhs) noexcept = default;
Engine::Builder& Engine::Builder::operator=(Engine::Builder const& rhs) noexcept = default;
Engine::Builder& Engine::Builder::operator=(Engine::Builder&& rhs) noexcept = default;

Engine::Builder& Engine::Builder::backend(Backend backend) noexcept {
    mImpl->mBackend = backend;
    return *this;
}

Engine::Builder& Engine::Builder::platform(Platform* platform) noexcept {
    mImpl->mPlatform = platform;
    return *this;
}

Engine::Builder& Engine::Builder::config(Engine::Config const* config) noexcept {
    mImpl->mConfig = config ? *config : Engine::Config{};
    return *this;
}

Engine::Builder& Engine::Builder::featureLevel(FeatureLevel featureLevel) noexcept {
    mImpl->mFeatureLevel = featureLevel;
    return *this;
}

Engine::Builder& Engine::Builder::sharedContext(void* sharedContext) noexcept {
    mImpl->mSharedContext = sharedContext;
    return *this;
}

Engine::Builder& Engine::Builder::paused(bool paused) noexcept {
    mImpl->mPaused = paused;
    return *this;
}

#if UTILS_HAS_THREADING

void Engine::Builder::build(Invocable<void(void*)>&& callback) const {
    FEngine::create(*this, std::move(callback));
}

#endif

Engine* Engine::Builder::build() const {
    mImpl->mConfig = BuilderDetails::validateConfig(mImpl->mConfig);
    return FEngine::create(*this);
}

Engine::Config Engine::BuilderDetails::validateConfig(Config config) noexcept {
    // Rule of thumb: perRenderPassArenaMB must be roughly 1 MB larger than perFrameCommandsMB
    constexpr uint32_t COMMAND_ARENA_OVERHEAD = 1;
    constexpr uint32_t CONCURRENT_FRAME_COUNT = 3;

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

    config.stereoscopicEyeCount =
            std::clamp(config.stereoscopicEyeCount, uint8_t(1), CONFIG_MAX_STEREOSCOPIC_EYES);

    return config;
}

} // namespace filament
