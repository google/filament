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

#include "private/backend/Program.h"

#include <private/filament/SibGenerator.h>

#include <filament/Exposure.h>
#include <filament/MaterialEnums.h>

#include <MaterialParser.h>

#include <filaflat/ShaderBuilder.h>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>

#include <math/fast.h>
#include <math/scalar.h>

#include <functional>

#include <stdio.h>

#include "generated/resources/materials.h"

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;
using namespace filaflat;

namespace details {

// The global list of engines
static std::unordered_map<Engine const*, std::unique_ptr<FEngine>> sEngines;
static std::mutex sEnginesLock;

FEngine* FEngine::create(Backend backend, Platform* platform, void* sharedGLContext) {
    FEngine* instance = new FEngine(backend, platform, sharedGLContext);

    slog.i << "FEngine (" << sizeof(void*) * 8 << " bits) created at " << instance << " "
            << "(threading is " << (UTILS_HAS_THREADING ? "enabled)" : "disabled)") << io::endl;

    // initialize all fields that need an instance of FEngine
    // (this cannot be done safely in the ctor)

    // Normally we launch a thread and create the context and Driver from there (see FEngine::loop).
    // In the single-threaded case, we do so in the here and now.
    if (!UTILS_HAS_THREADING) {
        // we don't own the external context at that point, set it to null
        instance->mPlatform = nullptr;
        if (platform == nullptr) {
            platform = DefaultPlatform::create(&instance->mBackend);
            instance->mPlatform = platform;
            instance->mOwnPlatform = true;
        }
        instance->mDriver = platform->createDriver(sharedGLContext);
        instance->init();
        instance->execute();
        return instance;
    }

    // start the driver thread
    instance->mDriverThread = std::thread(&FEngine::loop, instance);

    // wait for the driver to be ready
    instance->mDriverBarrier.await();

    if (UTILS_UNLIKELY(!instance->mDriver)) {
        // something went horribly wrong during driver initialization
        instance->mDriverThread.join();
        return nullptr;
    }

    // now we can initialize the largest part of the engine
    instance->init();

    return instance;
}


// these must be static because only a pointer is copied to the render stream
static const half4 sFullScreenTriangleVertices[3] = {
        { -1.0_h, -1.0_h, 1.0_h, 1.0_h },
        {  3.0_h, -1.0_h, 1.0_h, 1.0_h },
        { -1.0_h,  3.0_h, 1.0_h, 1.0_h }
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
    SYSTRACE_ENABLE();

    // we're assuming we're on the main thread here.
    // (it may not be the case)
    mJobSystem.adopt();
}

/*
 * init() is called just after the driver thread is initialized. Driver commands are therefore
 * possible.
 */

void FEngine::init() {
    // this must be first.
    mCommandStream = CommandStream(*mDriver, mCommandBufferQueue.getCircularBuffer());
    DriverApi& driverApi = getDriverApi();

    // Parse all post process shaders now, but create them lazily
    mPostProcessParser = std::make_unique<MaterialParser>(mBackend,
            MATERIALS_POSTPROCESS_DATA, MATERIALS_POSTPROCESS_SIZE);

    UTILS_UNUSED_IN_RELEASE bool ppMaterialOk =
            mPostProcessParser->parse() && mPostProcessParser->isPostProcessMaterial();
    assert(ppMaterialOk);

    uint32_t version;
    mPostProcessParser->getPostProcessVersion(&version);
    ASSERT_PRECONDITION(version == MATERIAL_VERSION, "Post-process material version mismatch. "
            "Expected %d but received %d.", MATERIAL_VERSION, version);

    mFullScreenTriangleVb = upcast(VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::HALF4, 0)
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
            .rgbm(true)
            .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
            .build(*this));
    static uint32_t pixel = 0;
    Texture::PixelBufferDescriptor buffer(
            &pixel, 4, // 4 bytes in 1 RGBM pixel
            Texture::Format::RGBM, Texture::Type::UBYTE);
    Texture::FaceOffsets offsets = {};
    mDefaultIblTexture->setImage(*this, 0, std::move(buffer), offsets);

    // 3 bands = 9 float3
    const float sh[9 * 3] = { 0.0f };
    mDefaultIbl = upcast(IndirectLight::Builder()
            .reflections(mDefaultIblTexture)
            .irradiance(3, reinterpret_cast<const float3*>(sh))
            .intensity(1.0f)
            .build(*this));

    // Always initialize the default material, most materials' depth shaders fallback on it.
    mDefaultMaterial = upcast(
            FMaterial::DefaultMaterialBuilder()
                    .package(MATERIALS_DEFAULTMATERIAL_DATA, MATERIALS_DEFAULTMATERIAL_SIZE)
                    .build(*const_cast<FEngine*>(this)));

    mPostProcessManager.init();
    mLightManager.init(*this);
    mDFG.reset(new DFG(*this));
}

FEngine::~FEngine() noexcept {
    ASSERT_DESTRUCTOR(mTerminated, "Engine destroyed but not terminated!");
    delete mDriver;
    if (mOwnPlatform) {
        DefaultPlatform::destroy((DefaultPlatform**)&mPlatform);
    }
}

void FEngine::shutdown() {
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
    mDFG->terminate();                      // free-up the DFG
    mRenderableManager.terminate();         // free-up all renderables
    mLightManager.terminate();              // free-up all lights
    mCameraManager.terminate();             // free-up all cameras

    driver.destroyRenderPrimitive(mFullScreenTriangleRph);
    destroy(mFullScreenTriangleIb);
    destroy(mFullScreenTriangleVb);

    destroy(mDefaultIblTexture);
    destroy(mDefaultIbl);

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

    // this must be done after Skyboxes and before materials
    for (FMaterial const* material : mSkyboxMaterials) {
        destroy(material);
    }

    cleanupResourceList(mIndexBuffers);
    cleanupResourceList(mVertexBuffers);
    cleanupResourceList(mTextures);
    cleanupResourceList(mMaterials);
    for (auto& item : mMaterialInstances) {
        cleanupResourceList(item.second);
    }
    cleanupResourceList(mFences);

    for (const auto& mPostProcessProgram : mPostProcessPrograms) {
        driver.destroyProgram(mPostProcessProgram);
    }

    // There might be commands added by the terminate() calls
    flushCommandBuffer(mCommandBufferQueue);
    if (!UTILS_HAS_THREADING) {
        execute();
    }

    /*
     * terminate the rendering engine
     */

    mCommandBufferQueue.requestExit();
    if (UTILS_HAS_THREADING) {
        mDriverThread.join();
    }

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

// -----------------------------------------------------------------------------------------------
// Render thread / command queue
// -----------------------------------------------------------------------------------------------

int FEngine::loop() {
    // we don't own the external context at that point, set it to null
    Platform* platform = mPlatform;
    mPlatform = nullptr;

    if (platform == nullptr) {
        platform = DefaultPlatform::create(&mBackend);
        mPlatform = platform;
        mOwnPlatform = true;
        slog.d << "FEngine resolved backend: ";
        switch (mBackend) {
            case backend::Backend::NOOP:
                slog.d << "Noop";
                break;

            case backend::Backend::OPENGL:
                slog.d << "OpenGL";
                break;

            case backend::Backend::VULKAN:
                slog.d << "Vulkan";
                break;

            case backend::Backend::METAL:
                slog.d << "Metal";
                break;

            default:
                slog.d << "Unknown";
                break;
        }
        slog.d << io::endl;
    }
    mDriver = platform->createDriver(mSharedGLContext);
    mDriverBarrier.latch();
    if (UTILS_UNLIKELY(!mDriver)) {
        // if we get here, it's because the driver couldn't be initialized and the problem has
        // been logged.
        return 0;
    }

    JobSystem::setThreadName("FEngine::loop");
    JobSystem::setThreadPriority(JobSystem::Priority::DISPLAY);

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

const FMaterial* FEngine::getSkyboxMaterial(bool rgbm) const noexcept {
    size_t index = rgbm ? 0 : 1;
    FMaterial const* material = mSkyboxMaterials[index];
    if (UTILS_UNLIKELY(material == nullptr)) {
        material = FSkybox::createMaterial(*const_cast<FEngine*>(this), rgbm);
        mSkyboxMaterials[index] = material;
    }
    return material;
}


backend::Handle<backend::HwProgram> FEngine::createPostProcessProgram(MaterialParser& parser,
        ShaderModel shaderModel, PostProcessStage stage) const noexcept {
    ShaderBuilder& vShaderBuilder = getVertexShaderBuilder();
    ShaderBuilder& fShaderBuilder = getFragmentShaderBuilder();
    parser.getShader(vShaderBuilder, shaderModel, (uint8_t)stage, ShaderType::VERTEX);
    parser.getShader(fShaderBuilder, shaderModel, (uint8_t)stage, ShaderType::FRAGMENT);

    // For the post-process program, we don't care about per-material sampler bindings but we still
    // need to populate a SamplerBindingMap and pass a weak reference to Program. Binding maps are
    // normally owned by Material, but in this case we'll simply own a copy right here in static
    // storage.
    static const SamplerBindingMap* pBindings = [] {
        static SamplerBindingMap bindings;
        bindings.populate();
        return &bindings;
    }();

    Program pb;
    pb      .diagnostics(CString("Post Process"))
            .withVertexShader(vShaderBuilder.data(), vShaderBuilder.size())
            .withFragmentShader(fShaderBuilder.data(), fShaderBuilder.size())
            .setUniformBlock(BindingPoints::PER_VIEW, PerViewUib::getUib().getName())
            .setUniformBlock(BindingPoints::POST_PROCESS, PostProcessingUib::getUib().getName());

    auto addSamplerGroup = [&pb]
            (uint8_t bindingPoint, SamplerInterfaceBlock const& sib, SamplerBindingMap const& map) {
        const size_t samplerCount = sib.getSize();
        if (samplerCount) {
            std::vector<Program::Sampler> samplers(samplerCount);
            auto const& list = sib.getSamplerInfoList();
            for (size_t i = 0, c = samplerCount; i < c; ++i) {
                CString uniformName(
                        SamplerInterfaceBlock::getUniformName(sib.getName().c_str(),
                                list[i].name.c_str()));
                uint8_t binding;
                map.getSamplerBinding(bindingPoint, (uint8_t)i, &binding);
                samplers[i] = { std::move(uniformName), binding };
            }
            pb.setSamplerGroup(bindingPoint, samplers.data(), samplers.size());
        }
    };

    addSamplerGroup(BindingPoints::POST_PROCESS, SibGenerator::getPostProcessSib(), *pBindings);

    auto program = const_cast<DriverApi&>(mCommandStream).createProgram(std::move(pb));
    assert(program);
    return program;
}

backend::Handle<backend::HwProgram> FEngine::getPostProcessProgramSlow(PostProcessStage stage) const noexcept {
    backend::Handle<backend::HwProgram>* const postProcessPrograms = mPostProcessPrograms;
    if (!postProcessPrograms[(uint8_t)stage]) {
        ShaderModel shaderModel = getDriver().getShaderModel();
        postProcessPrograms[(uint8_t)stage] = createPostProcessProgram(*mPostProcessParser, shaderModel, stage);
    }
    return postProcessPrograms[(uint8_t)stage];
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

FStream* FEngine::createStream(const Stream::Builder& builder) noexcept {
    return create(mStreams, builder);
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

FMaterialInstance* FEngine::createMaterialInstance(const FMaterial* material) noexcept {
    FMaterialInstance* p = mHeapAllocator.make<FMaterialInstance>(*this, material);
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

FFence* FEngine::createFence(Fence::Type type) noexcept {
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
void FEngine::terminateAndDestroy(const T* ptr, ResourceList<T, L>& list) {
    if (ptr != nullptr) {
        if (list.remove(ptr)) {
            const_cast<T*>(ptr)->terminate(*this);
            mHeapAllocator.destroy(const_cast<T*>(ptr));
        } else {
            // object not found, do nothing and log an error on DEBUG builds.
#ifndef NDEBUG
            slog.d << "object "
                   << CallStack::typeName<T>().c_str()
                   << " at " << ptr << " doesn't exist!"
                   << io::endl;
#endif
        }
    }
}

// -----------------------------------------------------------------------------------------------

void FEngine::destroy(const FVertexBuffer* p) {
    terminateAndDestroy(p, mVertexBuffers);
}

void FEngine::destroy(const FIndexBuffer* p) {
    terminateAndDestroy(p, mIndexBuffers);
}

inline void FEngine::destroy(const FRenderer* p) {
    terminateAndDestroy(p, mRenderers);
}

inline void FEngine::destroy(const FScene* p) {
    terminateAndDestroy(p, mScenes);
}

inline void FEngine::destroy(const FSkybox* p) {
    terminateAndDestroy(p, mSkyboxes);
}

UTILS_NOINLINE
void FEngine::destroy(const FTexture* p) {
    terminateAndDestroy(p, mTextures);
}

inline void FEngine::destroy(const FView* p) {
    terminateAndDestroy(p, mViews);
}

inline void FEngine::destroy(const FIndirectLight* p) {
    terminateAndDestroy(p, mIndirectLights);
}

UTILS_NOINLINE
void FEngine::destroy(const FFence* p) {
    terminateAndDestroy(p, mFences);
}

void FEngine::destroy(const FSwapChain* p) {
    terminateAndDestroy(p, mSwapChains);
}

void FEngine::destroy(const FStream* p) {
    terminateAndDestroy(p, mStreams);
}


void FEngine::destroy(const FMaterial* ptr) {
    if (ptr != nullptr) {
        auto pos = mMaterialInstances.find(ptr);
        if (pos != mMaterialInstances.cend()) {
            // ensure we've destroyed all instances before destroying the material
            if (!ASSERT_PRECONDITION_NON_FATAL(pos->second.empty(),
                    "destroying material \"%s\" but %u instances still alive",
                    ptr->getName().c_str(), (*pos).second.size())) {
                return;
            }
        }
        terminateAndDestroy(ptr, mMaterials);
    }
}

void FEngine::destroy(const FMaterialInstance* ptr) {
    if (ptr != nullptr) {
        auto pos = mMaterialInstances.find(ptr->getMaterial());
        assert(pos != mMaterialInstances.cend());
        if (pos != mMaterialInstances.cend()) {
            terminateAndDestroy(ptr, pos->second);
        }
    }
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

} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

Engine* Engine::create(Backend backend, Platform* platform, void* sharedGLContext) {
    std::unique_ptr<FEngine> engine(FEngine::create(backend, platform, sharedGLContext));
    if (UTILS_UNLIKELY(!engine)) {
        // something went wrong during the driver or engine initialization
        return nullptr;
    }

    std::lock_guard<std::mutex> guard(sEnginesLock);
    Engine* handle = engine.get();
    sEngines[handle] = std::move(engine);
    return handle;
}

void Engine::destroy(Engine** engine) {
    if (engine) {
        std::unique_ptr<FEngine> filamentEngine;

        std::unique_lock<std::mutex> guard(sEnginesLock);
        auto const& pos = sEngines.find(*engine);
        if (pos != sEngines.end()) {
            std::swap(filamentEngine, pos->second);
            sEngines.erase(pos);
        }
        guard.unlock();

        // Make sure to call into shutdown() without the lock held
        if (filamentEngine) {
            filamentEngine->shutdown();
            // clear the user's handle
            *engine = nullptr;
        }
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

Fence* Engine::createFence(Fence::Type type) noexcept {
    return upcast(this)->createFence(type);
}

SwapChain* Engine::createSwapChain(void* nativeWindow, uint64_t flags) noexcept {
    return upcast(this)->createSwapChain(nativeWindow, flags);
}

void Engine::destroy(const VertexBuffer* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const IndexBuffer* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const IndirectLight* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const Material* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const MaterialInstance* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const Renderer* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const View* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const Scene* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const Skybox* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const Stream* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const Texture* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const Fence* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(const SwapChain* p) {
    upcast(this)->destroy(upcast(p));
}

void Engine::destroy(Entity e) {
    upcast(this)->destroy(e);
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

DebugRegistry& Engine::getDebugRegistry() noexcept {
    return upcast(this)->getDebugRegistry();
}


} // namespace filament
