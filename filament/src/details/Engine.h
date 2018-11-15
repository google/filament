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

#ifndef TNT_FILAMENT_DETAILS_ENGINE_H
#define TNT_FILAMENT_DETAILS_ENGINE_H

#include "upcast.h"
#include "PostProcessManager.h"
#include "RenderTargetPool.h"

#include "components/CameraManager.h"
#include "components/LightManager.h"
#include "components/TransformManager.h"
#include "components/RenderableManager.h"

#include "details/Allocators.h"
#include "details/Camera.h"
#include "details/DebugRegistry.h"
#include "details/ResourceList.h"
#include "details/Skybox.h"

#include "driver/CommandStream.h"
#include "driver/CommandBufferQueue.h"
#include "driver/DriverApi.h"

#include <filament/Engine.h>
#include <filament/VertexBuffer.h>
#include <filament/IndirectLight.h>
#include <filament/Material.h>
#include <filament/Texture.h>
#include <filament/Skybox.h>
#include <filament/Stream.h>

#include <filaflat/MaterialParser.h>
#include <filaflat/ShaderBuilder.h>

#include <utils/compiler.h>
#include <utils/Allocator.h>
#include <utils/JobSystem.h>
#include <utils/CountDownLatch.h>

#include <math/mat4.h>
#include <math/quat.h>

#include <chrono>
#include <memory>
#include <unordered_map>

namespace filament {

class Renderer;
class Driver;
class Program;

namespace details {

class FFence;
class FMaterialInstance;
class FRenderer;
class FScene;
class FSwapChain;
class FView;

class DFG;

/*
 * Concrete implementation of the Engine interface. This keeps track of all hardware resources
 * for a given context.
 */
class FEngine : public Engine {
public:

    inline void* operator new(std::size_t count) noexcept {
        return utils::aligned_alloc(count * sizeof(FEngine), alignof(FEngine));
    }

    inline void operator delete(void* p) noexcept {
        utils::aligned_free(p);
    }

    using DriverApi = driver::DriverApi;
    using clock = std::chrono::steady_clock;
    using Epoch = clock::time_point;
    using duration = clock::duration;

    // TODO: these should come from a configuration object
    static constexpr float  CONFIG_Z_LIGHT_NEAR            = 5;
    static constexpr float  CONFIG_Z_LIGHT_FAR             = 100;
    static constexpr size_t CONFIG_FROXEL_SLICE_COUNT      = 16;
    static constexpr bool   CONFIG_IBL_USE_IRRADIANCE_MAP  = false;

    static constexpr size_t CONFIG_PER_RENDER_PASS_ARENA_SIZE   = details::CONFIG_PER_RENDER_PASS_ARENA_SIZE;
    static constexpr size_t CONFIG_PER_FRAME_COMMANDS_SIZE      = details::CONFIG_PER_FRAME_COMMANDS_SIZE;
    static constexpr size_t CONFIG_MIN_COMMAND_BUFFERS_SIZE     = details::CONFIG_MIN_COMMAND_BUFFERS_SIZE;
    static constexpr size_t CONFIG_COMMAND_BUFFERS_SIZE         = details::CONFIG_COMMAND_BUFFERS_SIZE;

public:
    static FEngine* create(Backend backend = Backend::DEFAULT,
            Platform* platform = nullptr, void* sharedGLContext = nullptr);

    ~FEngine() noexcept;

    Driver& getDriver() const noexcept { return *mDriver; }
    DriverApi& getDriverApi() noexcept { return mCommandStream; }
    DFG* getDFG() const noexcept { return mDFG.get(); }


    // Uniforms...
    const UniformInterfaceBlock& getPerViewUib() const noexcept { return mPerViewUib; }
    const UniformInterfaceBlock& getPerPostProcessUib() const noexcept { return mPostProcessUib; }

    // Samplers...
    const SamplerInterfaceBlock& getPerViewSib() const noexcept { return mPerViewSib; }
    const SamplerInterfaceBlock& getPostProcessSib() const noexcept { return mPostProcessSib; }

    // the per-frame Area is used by all Renderer, so they must run in sequence and
    // have freed all allocated memory when done. If this needs to change in the future,
    // we'll simply have to use separate Areas (for instance).
    LinearAllocatorArena& getPerRenderPassAllocator() noexcept { return mPerRenderPassAllocator; }

    // Material IDs...
    uint32_t getMaterialId() const noexcept { return mMaterialId++; }

    const FMaterial* getDefaultMaterial() const noexcept { return mDefaultMaterial; }
    const FMaterial* getSkyboxMaterial(bool rgbm) const noexcept;
    const FIndirectLight* getDefaultIndirectLight() const noexcept { return mDefaultIbl; }

    Handle <HwProgram> getPostProcessProgramSlow(PostProcessStage stage) const noexcept;
    Handle<HwProgram> getPostProcessProgram(PostProcessStage stage) const noexcept {
        Handle<HwProgram> program = mPostProcessPrograms[uint8_t(stage)];
        if (UTILS_UNLIKELY(!program)) {
            return getPostProcessProgramSlow(stage);
        }
        return program;
    }

    Handle<HwRenderPrimitive> getFullScreenRenderPrimitive() const noexcept {
        return mFullScreenTriangleRph;
    }

    FVertexBuffer* getFullScreenVertexBuffer() const noexcept {
        return mFullScreenTriangleVb;
    }

    FIndexBuffer* getFullScreenIndexBuffer() const noexcept {
        return mFullScreenTriangleIb;
    }

    PostProcessManager const& getPostProcessManager() const noexcept {
        return mPostProcessManager;
    }

    PostProcessManager& getPostProcessManager() noexcept {
        return mPostProcessManager;
    }

    RenderTargetPool const& getRenderTargetPool() const noexcept {
        return mRenderTargetPool;
    }

    RenderTargetPool& getRenderTargetPool() noexcept {
        return mRenderTargetPool;
    }

    FRenderableManager& getRenderableManager() noexcept {
        return mRenderableManager;
    }

    FLightManager& getLightManager() noexcept {
        return mLightManager;
    }

    FCameraManager& getCameraManager() noexcept {
        return mCameraManager;
    }

    FTransformManager& getTransformManager() noexcept {
        return mTransformManager;
    }

    utils::EntityManager& getEntityManager() noexcept {
        return mEntityManager;
    }

    HeapAllocatorArena& getHeapAllocator() noexcept {
        return mHeapAllocator;
    }

    Backend getBackend() const noexcept {
        return mBackend;
    }

    duration getTime() const noexcept {
        return clock::now() - getEpoch();
    }

    void* streamAlloc(size_t size, size_t alignment) noexcept;

    utils::JobSystem& getJobSystem() noexcept { return mJobSystem; }

    Epoch getEpoch() const { return mEpoch; }

    void shutdown();

    template <typename T>
    T* create(ResourceList<T>& list, typename T::Builder const& builder) noexcept;

    FVertexBuffer* createVertexBuffer(const VertexBuffer::Builder& builder) noexcept;
    FIndexBuffer* createIndexBuffer(const IndexBuffer::Builder& builder) noexcept;
    FIndirectLight* createIndirectLight(const IndirectLight::Builder& builder) noexcept;
    FMaterial* createMaterial(const Material::Builder& builder) noexcept;
    FTexture* createTexture(const Texture::Builder& builder) noexcept;
    FSkybox* createSkybox(const Skybox::Builder& builder) noexcept;
    FStream* createStream(const Stream::Builder& builder) noexcept;

    void createRenderable(const RenderableManager::Builder& builder, utils::Entity entity);
    void createLight(const LightManager::Builder& builder, utils::Entity entity);

    FRenderer* createRenderer() noexcept;
    FMaterialInstance* createMaterialInstance(const FMaterial* material) noexcept;

    FScene* createScene() noexcept;
    FView* createView() noexcept;
    FCamera* createCamera(utils::Entity entity) noexcept;
    FFence* createFence(Fence::Type type = Fence::Type::SOFT) noexcept;
    FSwapChain* createSwapChain(void* nativeWindow, uint64_t flags) noexcept;

    void destroy(const FVertexBuffer* p);
    void destroy(const FFence* p);
    void destroy(const FIndexBuffer* p);
    void destroy(const FIndirectLight* p);
    void destroy(const FMaterial* p);
    void destroy(const FMaterialInstance* p);
    void destroy(const FRenderer* p);
    void destroy(const FScene* p);
    void destroy(const FSkybox* p);
    void destroy(const FStream* p);
    void destroy(const FTexture* p);
    void destroy(const FSwapChain* p);
    void destroy(const FView* p);
    void destroy(utils::Entity e);

    // flush the current buffer
    void flush();

    void prepare();
    void gc();

    filaflat::ShaderBuilder& getVertexShaderBuilder() const noexcept {
        return mVertexShaderBuilder;
    }

    filaflat::ShaderBuilder& getFragmentShaderBuilder() const noexcept {
        return mFragmentShaderBuilder;
    }

    FDebugRegistry& getDebugRegistry() noexcept {
        return mDebugRegistry;
    }

    bool execute();

private:
    FEngine(Backend backend, Platform* platform, void* sharedGLContext);
    void init();

    int loop();
    void flushCommandBuffer(CommandBufferQueue& commandBufferQueue);

    template<typename T, typename L>
    void terminateAndDestroy(const T* p, ResourceList<T, L>& list);

    template<typename T, typename L>
    void cleanupResourceList(ResourceList<T, L>& list);

    Handle<HwProgram> createPostProcessProgram(filaflat::MaterialParser& parser,
            driver::ShaderModel model, PostProcessStage stage) const noexcept;

    Driver* mDriver = nullptr;

    Backend mBackend;
    Platform* mPlatform = nullptr;
    void* mSharedGLContext = nullptr;
    bool mTerminated = false;
    Handle<HwRenderPrimitive> mFullScreenTriangleRph;
    FVertexBuffer* mFullScreenTriangleVb = nullptr;
    FIndexBuffer* mFullScreenTriangleIb = nullptr;

    PostProcessManager mPostProcessManager;
    RenderTargetPool mRenderTargetPool;

    utils::EntityManager& mEntityManager;
    FRenderableManager mRenderableManager;
    FTransformManager mTransformManager;
    FLightManager mLightManager;
    FCameraManager mCameraManager;

    ResourceList<FRenderer> mRenderers{ "Renderer" };
    ResourceList<FView> mViews{ "View" };
    ResourceList<FScene> mScenes{ "Scene" };
    ResourceList<FFence, utils::LockingPolicy::SpinLock> mFences{"Fence"};
    ResourceList<FSwapChain> mSwapChains{ "SwapChain" };
    ResourceList<FStream> mStreams{ "Stream" };
    ResourceList<FIndexBuffer> mIndexBuffers{ "IndexBuffer" };
    ResourceList<FVertexBuffer> mVertexBuffers{ "VertexBuffer" };
    ResourceList<FIndirectLight> mIndirectLights{ "IndirectLight" };
    ResourceList<FMaterial> mMaterials{ "Material" };
    ResourceList<FTexture> mTextures{ "Texture" };
    ResourceList<FSkybox> mSkyboxes{ "Skybox" };

    mutable uint32_t mMaterialId = 0;

    // FMaterialInstance are handled directly by FMaterial
    std::unordered_map<const FMaterial*, ResourceList<FMaterialInstance>> mMaterialInstances;

    std::unique_ptr<DFG> mDFG;

    // Per-view Uniform interface block
    UniformInterfaceBlock mPerViewUib;

    // Per-view Sampler interface block
    SamplerInterfaceBlock mPerViewSib;

    // post-process interface blocks
    UniformInterfaceBlock mPostProcessUib;
    SamplerInterfaceBlock mPostProcessSib;

    std::thread mDriverThread;
    CommandBufferQueue mCommandBufferQueue;
    DriverApi mCommandStream;

    LinearAllocatorArena mPerRenderPassAllocator;
    HeapAllocatorArena mHeapAllocator;

    utils::JobSystem mJobSystem;

    Epoch mEpoch;

    mutable FMaterial const* mDefaultMaterial = nullptr;
    mutable FMaterial const* mSkyboxMaterials[2] = { nullptr, nullptr };

    mutable FTexture* mDefaultIblTexture = nullptr;
    mutable FIndirectLight* mDefaultIbl = nullptr;

    mutable Handle<HwProgram> mPostProcessPrograms[POST_PROCESS_STAGES_COUNT];
    mutable std::unique_ptr<filaflat::MaterialParser> mPostProcessParser;

    mutable utils::CountDownLatch mDriverBarrier;

    mutable filaflat::ShaderBuilder mVertexShaderBuilder;
    mutable filaflat::ShaderBuilder mFragmentShaderBuilder;
    FDebugRegistry mDebugRegistry;

public:
    // these are the debug properties used by FDebug. They're accessed directly by modules who need them.
    struct {
        struct {
            bool far_uses_shadowcasters = true;
            bool focus_shadowcasters = true;
            bool lispsm = true;
            float dzn = -1.0f;
            float dzf =  1.0f;
        } shadowmap;
    } debug;
};

FILAMENT_UPCAST(Engine)

class UTILS_PUBLIC EnginePerformanceTest {
public:
    //! \privatesection
    using PFN = void(*)(void *);
    virtual void activateBigBang() noexcept;
    void activateOmegaThirteen() noexcept;
    PFN getDestroyUniverseApi();
    virtual ~EnginePerformanceTest() noexcept;
};

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_ENGINE_H
