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

#include "details/DFG.h"
#include "details/VertexBuffer.h"
#include "details/Fence.h"
#include "details/Camera.h"
#include "details/IndexBuffer.h"
#include "details/IndirectLight.h"
#include "details/Material.h"
#include "details/Renderer.h"
#include "details/RenderPrimitive.h"
#include "details/Scene.h"
#include "details/Skybox.h"
#include "details/Stream.h"
#include "details/SwapChain.h"
#include "details/Texture.h"
#include "details/View.h"

#include "fg/ResourceAllocator.h"


#include <private/filament/SibGenerator.h>

#include <filament/MaterialEnums.h>

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>

#include <memory>

#include "generated/resources/materials.h"

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;
using namespace filaflat;

FEngine* FEngine::create(Backend backend, Platform* platform, void* sharedGLContext) {
    SYSTRACE_ENABLE();
    SYSTRACE_CALL();

    FEngine* instance = new FEngine(backend, platform, sharedGLContext);

    slog.i << "FEngine (" << sizeof(void*) * 8 << " bits) created at " << instance << " "
            << "(threading is " << (UTILS_HAS_THREADING ? "enabled)" : "disabled)") << io::endl;

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
            return nullptr;
        }
        instance->mDriver = platform->createDriver(sharedGLContext);
    } else {
        // start the driver thread
        instance->mDriverThread = std::thread(&FEngine::loop, instance);

        // wait for the driver to be ready
        instance->mDriverBarrier.await();

        if (UTILS_UNLIKELY(!instance->mDriver)) {
            // something went horribly wrong during driver initialization
            instance->mDriverThread.join();
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

FEngine::FEngine(Backend backend, Platform* platform, void* sharedGLContext) :
        mBackend(backend),
        mPlatform(platform),
        mSharedGLContext(sharedGLContext),
        mPostProcessManager(*this),
        mEntityManager(EntityManager::get()),
        mRenderableManager(*this),
        mTransformManager(),
        mLightManager(*this),
        mCameraManager(*this),
        mCommandBufferQueue(CONFIG_MIN_COMMAND_BUFFERS_SIZE, CONFIG_COMMAND_BUFFERS_SIZE),
        mPerRenderPassAllocator("per-renderpass allocator", CONFIG_PER_RENDER_PASS_ARENA_SIZE),
        mEngineEpoch(std::chrono::steady_clock::now()),
        mDriverBarrier(1)
{
    // we're assuming we're on the main thread here.
    // (it may not be the case)
    mJobSystem.adopt();
}

/*
 * init() is called just after the driver thread is initialized. Driver commands are therefore
 * possible.
 */

void FEngine::init() {
    SYSTRACE_CALL();

    // this must be first.
    mCommandStream = CommandStream(*mDriver, mCommandBufferQueue.getCircularBuffer());
    DriverApi& driverApi = getDriverApi();

    mResourceAllocator = new fg::ResourceAllocator(driverApi);

    mFullScreenTriangleVb = upcast(VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT4, 0)
            .build(*this));

    mFullScreenTriangleVb->setBufferAt(*this, 0,
            { sFullScreenTriangleVertices, sizeof(sFullScreenTriangleVertices) });

    mFullScreenTriangleIb = upcast(IndexBuffer::Builder()
            .indexCount(3)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(*this));

    mFullScreenTriangleIb->setBuffer(*this,
            { sFullScreenTriangleIndices, sizeof(sFullScreenTriangleIndices) });

    mFullScreenTriangleRph = driverApi.createRenderPrimitive();
    driverApi.setRenderPrimitiveBuffer(mFullScreenTriangleRph,
            mFullScreenTriangleVb->getHwHandle(), mFullScreenTriangleIb->getHwHandle(),
            mFullScreenTriangleVb->getDeclaredAttributes().getValue());
    driverApi.setRenderPrimitiveRange(mFullScreenTriangleRph, PrimitiveType::TRIANGLES,
            0, 0, 2, (uint32_t)mFullScreenTriangleIb->getIndexCount());

    mDefaultIblTexture = upcast(Texture::Builder()
            .width(1).height(1).levels(1)
            .format(Texture::InternalFormat::RGBA8)
            .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
            .build(*this));
    static uint32_t pixel = 0;
    Texture::PixelBufferDescriptor buffer(
            &pixel, 4, // 4 bytes in 1 RGBA pixel
            Texture::Format::RGBA, Texture::Type::UBYTE);
    Texture::FaceOffsets offsets = {};
    mDefaultIblTexture->setImage(*this, 0, std::move(buffer), offsets);

    // 3 bands = 9 float3
    const float sh[9 * 3] = { 0.0f };
    mDefaultIbl = upcast(IndirectLight::Builder()
            .reflections(mDefaultIblTexture)
            .irradiance(3, reinterpret_cast<const float3*>(sh))
            .build(*this));

    mDefaultColorGrading = upcast(ColorGrading::Builder().build(*this));

    // Always initialize the default material, most materials' depth shaders fallback on it.
    mDefaultMaterial = upcast(
            FMaterial::DefaultMaterialBuilder()
                    .package(MATERIALS_DEFAULTMATERIAL_DATA, MATERIALS_DEFAULTMATERIAL_SIZE)
                    .build(*const_cast<FEngine*>(this)));

    mPostProcessManager.init();
    mLightManager.init(*this);
    mDFG = std::make_unique<DFG>(*this);

    mMainThreadId = std::this_thread::get_id();
}

FEngine::~FEngine() noexcept {
    SYSTRACE_CALL();

    ASSERT_DESTRUCTOR(mTerminated, "Engine destroyed but not terminated!");
    delete mResourceAllocator;
    delete mDriver;
    if (mOwnPlatform) {
        DefaultPlatform::destroy((DefaultPlatform**)&mPlatform);
    }
}

void FEngine::shutdown() {
    SYSTRACE_CALL();

    ASSERT_PRECONDITION(std::this_thread::get_id() == mMainThreadId,
            "Engine::shutdown() called from the wrong thread!");

#ifndef NDEBUG
    // print out some statistics about this run
    size_t wm = mCommandBufferQueue.getHigWatermark();
    size_t wmpct = wm / (CONFIG_COMMAND_BUFFERS_SIZE / 100);
    slog.d << "CircularBuffer: High watermark "
           << wm / 1024 << " KiB (" << wmpct << "%)" << io::endl;
#endif

    DriverApi& driver = getDriverApi();

    /*
     * Destroy our own state first
     */

    mPostProcessManager.terminate(driver);  // free-up post-process manager resources
    mResourceAllocator->terminate();
    mDFG->terminate();                      // free-up the DFG
    mRenderableManager.terminate();         // free-up all renderables
    mLightManager.terminate();              // free-up all lights
    mCameraManager.terminate();             // free-up all cameras

    driver.destroyRenderPrimitive(mFullScreenTriangleRph);
    destroy(mFullScreenTriangleIb);
    destroy(mFullScreenTriangleVb);

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
    cleanupResourceList(mRenderers);
    cleanupResourceList(mViews);
    cleanupResourceList(mScenes);
    cleanupResourceList(mSkyboxes);
    cleanupResourceList(mColorGradings);

    // this must be done after Skyboxes and before materials
    destroy(mSkyboxMaterial);

    cleanupResourceList(mIndexBuffers);
    cleanupResourceList(mVertexBuffers);
    cleanupResourceList(mTextures);
    cleanupResourceList(mRenderTargets);
    cleanupResourceList(mMaterials);
    for (auto& item : mMaterialInstances) {
        cleanupResourceList(item.second);
    }
    cleanupResourceList(mFences);

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

    /*
     * Terminate the JobSystem...
     */

    // detach this thread from the jobsystem
    mJobSystem.emancipate();

    mTerminated = true;
}

void FEngine::prepare() {
    SYSTRACE_CALL();
    // prepare() is called once per Renderer frame. Ideally we would upload the content of
    // UBOs that are visible only. It's not such a big issue because the actual upload() is
    // skipped is the UBO hasn't changed. Still we could have a lot of these.
    FEngine::DriverApi& driver = getDriverApi();
    for (auto& materialInstanceList : mMaterialInstances) {
        for (auto& item : materialInstanceList.second) {
            item->commit(driver);
        }
    }

    // Commit default material instances.
    for (auto& material : mMaterials) {
        material->getDefaultInstance()->commit(driver);
    }
}

void FEngine::gc() {
    // Note: this runs in a Job

    JobSystem& js = mJobSystem;
    auto parent = js.createJob();
    auto em = std::ref(mEntityManager);

    js.run(jobs::createJob(js, parent, &FRenderableManager::gc, &mRenderableManager, em),
            JobSystem::DONT_SIGNAL);
    js.run(jobs::createJob(js, parent, &FLightManager::gc, &mLightManager, em),
            JobSystem::DONT_SIGNAL);
    js.run(jobs::createJob(js, parent, &FTransformManager::gc, &mTransformManager, em),
            JobSystem::DONT_SIGNAL);
    js.run(jobs::createJob(js, parent, &FCameraManager::gc, &mCameraManager, em),
            JobSystem::DONT_SIGNAL);

    js.runAndWait(parent);
}

void FEngine::flush() {
    // flush the command buffer
    flushCommandBuffer(mCommandBufferQueue);
}

void FEngine::flushAndWait() {

#if defined(ANDROID)

    // first make sure we've not terminated filament
    ASSERT_PRECONDITION(!mCommandBufferQueue.isExitRequested(),
            "calling Engine::flushAndWait() after Engine::shutdown()!");

#endif

    // enqueue finish command -- this will stall in the driver until the GPU is done
    getDriverApi().finish();

#if defined(ANDROID)

    // then create a fence that will trigger when we're past the finish() above
    size_t tryCount = 8;
    FFence* fence = FEngine::createFence(FFence::Type::SOFT);
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
        const char* backend = nullptr;
        switch (mBackend) {
            case backend::Backend::NOOP:
                backend = "Noop";
                break;
            case backend::Backend::OPENGL:
                backend = "OpenGL";
                break;
            case backend::Backend::VULKAN:
                backend = "Vulkan";
                break;
            case backend::Backend::METAL:
                backend = "Metal";
                break;
            default:
                backend = "Unknown";
                break;
        }
        slog.d << "FEngine resolved backend: " << backend << io::endl;
        if (mPlatform == nullptr) {
            slog.e << "Selected backend not supported in this build." << io::endl;
            mDriverBarrier.latch();
            return 0;
        }
    }

#if FILAMENT_ENABLE_MATDBG
    #ifdef ANDROID
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

    mDriver = mPlatform->createDriver(mSharedGLContext);
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

template <typename T>
inline T* FEngine::create(ResourceList<T>& list, typename T::Builder const& builder) noexcept {
    T* p = mHeapAllocator.make<T>(*this, builder);
    list.insert(p);
    return p;
}

FVertexBuffer* FEngine::createVertexBuffer(const VertexBuffer::Builder& builder) noexcept {
    return create(mVertexBuffers, builder);
}

FIndexBuffer* FEngine::createIndexBuffer(const IndexBuffer::Builder& builder) noexcept {
    return create(mIndexBuffers, builder);
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
        p->init();
    }
    return p;
}

FMaterialInstance* FEngine::createMaterialInstance(const FMaterial* material,
        const char* name) noexcept {
    FMaterialInstance* p = mHeapAllocator.make<FMaterialInstance>(*this, material, name);
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
        mFences.insert(p);
    }
    return p;
}

FSwapChain* FEngine::createSwapChain(void* nativeWindow, uint64_t flags) noexcept {
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

template<typename T, typename L>
void FEngine::cleanupResourceList(ResourceList<T, L>& list) {
    if (!list.empty()) {
#ifndef NDEBUG
        slog.d << "cleaning up " << list.size()
               << " leaked " << CallStack::typeName<T>().c_str() << io::endl;
#endif
        // Move the list (copy-and-clear). We can only modify/access the list from this
        // thread, because it's not thread-safe.
        auto copy(list.getListAndClear());
        for (T* item : copy) {
            item->terminate(*this);
            mHeapAllocator.destroy(item);
        }
    }
}

// -----------------------------------------------------------------------------------------------

template<typename T, typename L>
bool FEngine::terminateAndDestroy(const T* ptr, ResourceList<T, L>& list) {
    if (ptr == nullptr) return true;
    bool success = list.remove(ptr);
    if (ASSERT_PRECONDITION_NON_FATAL(success,
            "Object %s at %p doesn't exist (double free?)",
            CallStack::typeName<T>().c_str(), ptr)) {
        const_cast<T*>(ptr)->terminate(*this);
        mHeapAllocator.destroy(const_cast<T*>(ptr));
    }
    return success;
}

// -----------------------------------------------------------------------------------------------

bool FEngine::destroy(const FVertexBuffer* p) {
    return terminateAndDestroy(p, mVertexBuffers);
}

bool FEngine::destroy(const FIndexBuffer* p) {
    return terminateAndDestroy(p, mIndexBuffers);
}

inline bool FEngine::destroy(const FRenderer* p) {
    return terminateAndDestroy(p, mRenderers);
}

inline bool FEngine::destroy(const FScene* p) {
    return terminateAndDestroy(p, mScenes);
}

inline bool FEngine::destroy(const FSkybox* p) {
    return terminateAndDestroy(p, mSkyboxes);
}

inline bool FEngine::destroy(const FColorGrading* p) {
    return terminateAndDestroy(p, mColorGradings);
}

UTILS_NOINLINE
bool FEngine::destroy(const FTexture* p) {
    return terminateAndDestroy(p, mTextures);
}

bool FEngine::destroy(const FRenderTarget* p) {
    return terminateAndDestroy(p, mRenderTargets);
}

inline bool FEngine::destroy(const FView* p) {
    return terminateAndDestroy(p, mViews);
}

inline bool FEngine::destroy(const FIndirectLight* p) {
    return terminateAndDestroy(p, mIndirectLights);
}

UTILS_NOINLINE
bool FEngine::destroy(const FFence* p) {
    return terminateAndDestroy(p, mFences);
}

bool FEngine::destroy(const FSwapChain* p) {
    return terminateAndDestroy(p, mSwapChains);
}

bool FEngine::destroy(const FStream* p) {
    return terminateAndDestroy(p, mStreams);
}


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

bool FEngine::destroy(const FMaterialInstance* ptr) {
    if (ptr == nullptr) return true;
    auto pos = mMaterialInstances.find(ptr->getMaterial());
    assert(pos != mMaterialInstances.cend());
    if (pos != mMaterialInstances.cend()) {
        return terminateAndDestroy(ptr, pos->second);
    }
    // if we don't find this instance's material it might be because it's the default instance
    // in which case it fine to ignore.
    return true;
}

void FEngine::destroy(Entity e) {
    mRenderableManager.destroy(e);
    mLightManager.destroy(e);
    mTransformManager.destroy(e);
    mCameraManager.destroy(e);
}

void* FEngine::streamAlloc(size_t size, size_t alignment) noexcept {
    // we allow this only for small allocations
    if (size > 1024) {
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
            mCommandStream.execute(item.begin);
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

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

Engine* Engine::create(Backend backend, Platform* platform, void* sharedGLContext) {
    return FEngine::create(backend, platform, sharedGLContext);
}

void Engine::destroy(Engine* engine) {
    FEngine::destroy(upcast(engine));
}

void Engine::destroy(Engine** pEngine) {
    if (pEngine) {
        Engine* engine = *pEngine;
        FEngine::destroy(upcast(engine));
        *pEngine = nullptr;
    }
}

// -----------------------------------------------------------------------------------------------
// Resource management
// -----------------------------------------------------------------------------------------------

const Material* Engine::getDefaultMaterial() const noexcept {
    return upcast(this)->getDefaultMaterial();
}

Backend Engine::getBackend() const noexcept {
    return upcast(this)->getBackend();
}

Renderer* Engine::createRenderer() noexcept {
    return upcast(this)->createRenderer();
}

View* Engine::createView() noexcept {
    return upcast(this)->createView();
}

Scene* Engine::createScene() noexcept {
    return upcast(this)->createScene();
}

Camera* Engine::createCamera(Entity entity) noexcept {
    return upcast(this)->createCamera(entity);
}

Camera* Engine::getCameraComponent(utils::Entity entity) noexcept {
    return upcast(this)->getCameraComponent(entity);
}

void Engine::destroyCameraComponent(utils::Entity entity) noexcept {
    upcast(this)->destroyCameraComponent(entity);
}

Fence* Engine::createFence() noexcept {
    return upcast(this)->createFence(FFence::Type::SOFT);
}

SwapChain* Engine::createSwapChain(void* nativeWindow, uint64_t flags) noexcept {
    return upcast(this)->createSwapChain(nativeWindow, flags);
}

SwapChain* Engine::createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept {
    return upcast(this)->createSwapChain(width, height, flags);
}

bool Engine::destroy(const VertexBuffer* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const IndexBuffer* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const IndirectLight* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Material* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const MaterialInstance* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Renderer* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const View* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Scene* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Skybox* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const ColorGrading* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Stream* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Texture* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const RenderTarget* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Fence* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const SwapChain* p) {
    return upcast(this)->destroy(upcast(p));
}

void Engine::destroy(Entity e) {
    upcast(this)->destroy(e);
}

void Engine::flushAndWait() {
    upcast(this)->flushAndWait();
}

RenderableManager& Engine::getRenderableManager() noexcept {
    return upcast(this)->getRenderableManager();
}

LightManager& Engine::getLightManager() noexcept {
    return upcast(this)->getLightManager();
}

TransformManager& Engine::getTransformManager() noexcept {
    return upcast(this)->getTransformManager();
}

void* Engine::streamAlloc(size_t size, size_t alignment) noexcept {
    return upcast(this)->streamAlloc(size, alignment);
}

// The external-facing execute does a flush, and is meant only for single-threaded environments.
// It also discards the boolean return value, which would otherwise indicate a thread exit.
void Engine::execute() {
    ASSERT_PRECONDITION(!UTILS_HAS_THREADING, "Execute is meant for single-threaded platforms.");
    upcast(this)->flush();
    upcast(this)->execute();
}

utils::JobSystem& Engine::getJobSystem() noexcept {
    return upcast(this)->getJobSystem();
}

DebugRegistry& Engine::getDebugRegistry() noexcept {
    return upcast(this)->getDebugRegistry();
}

Camera* Engine::createCamera() noexcept {
    return createCamera(upcast(this)->getEntityManager().create());
}

void Engine::destroy(const Camera* camera) {
    Entity e = camera->getEntity();
    destroyCameraComponent(e);
    upcast(this)->getEntityManager().destroy(e);
}

} // namespace filament
