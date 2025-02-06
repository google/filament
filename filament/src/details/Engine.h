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

#include "downcast.h"

#include "Allocators.h"
#include "DFG.h"
#include "PostProcessManager.h"
#include "ResourceList.h"
#include "HwDescriptorSetLayoutFactory.h"
#include "HwVertexBufferInfoFactory.h"

#include "components/CameraManager.h"
#include "components/LightManager.h"
#include "components/TransformManager.h"
#include "components/RenderableManager.h"

#include "ds/DescriptorSetLayout.h"

#include "details/BufferObject.h"
#include "details/Camera.h"
#include "details/ColorGrading.h"
#include "details/DebugRegistry.h"
#include "details/Fence.h"
#include "details/IndexBuffer.h"
#include "details/InstanceBuffer.h"
#include "details/RenderTarget.h"
#include "details/SkinningBuffer.h"
#include "details/MorphTargetBuffer.h"
#include "details/Skybox.h"

#include "private/backend/CommandBufferQueue.h"
#include "private/backend/CommandStream.h"
#include "private/backend/DriverApi.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/BufferInterfaceBlock.h>

#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/Material.h>
#include <filament/Skybox.h>
#include <filament/Stream.h>
#include <filament/Texture.h>
#include <filament/VertexBuffer.h>

#include <backend/DriverEnums.h>

#include <utils/Allocator.h>
#include <utils/compiler.h>
#include <utils/CountDownLatch.h>
#include <utils/FixedCapacityVector.h>
#include <utils/JobSystem.h>
#include <utils/Slice.h>

#include <array>
#include <chrono>
#include <memory>
#include <new>
#include <optional>
#include <string_view>
#include <random>
#include <thread>
#include <type_traits>
#include <unordered_map>

#if FILAMENT_ENABLE_MATDBG
#include <matdbg/DebugServer.h>
#else
namespace filament::matdbg {
class DebugServer;
using MaterialKey = uint32_t;
} // namespace filament::matdbg
#endif

#if FILAMENT_ENABLE_FGVIEWER
#include <fgviewer/DebugServer.h>
#else
namespace filament::fgviewer {
    class DebugServer;
} // namespace filament::fgviewer
#endif

namespace filament {

class Renderer;
class MaterialParser;
class ResourceAllocatorDisposer;

namespace backend {
class Driver;
class Program;
} // namespace driver

class FFence;
class FMaterialInstance;
class FRenderer;
class FScene;
class FSwapChain;
class FView;

class ResourceAllocator;

/*
 * Concrete implementation of the Engine interface. This keeps track of all hardware resources
 * for a given context.
 */
class FEngine : public Engine {
public:
    void* operator new(std::size_t const size) noexcept {
        return utils::aligned_alloc(size, alignof(FEngine));
    }

    void operator delete(void* p) noexcept {
        utils::aligned_free(p);
    }

    using DriverApi = backend::DriverApi;
    using clock = std::chrono::steady_clock;
    using Epoch = clock::time_point;
    using duration = clock::duration;

public:
    static Engine* create(Builder const& builder);

#if UTILS_HAS_THREADING
    static void create(Builder const& builder, utils::Invocable<void(void* token)>&& callback);
    static FEngine* getEngine(void* token);
#endif

    static void destroy(FEngine* engine);

    ~FEngine() noexcept;

    backend::ShaderModel getShaderModel() const noexcept { return getDriver().getShaderModel(); }

    DriverApi& getDriverApi() noexcept {
        return *std::launder(reinterpret_cast<DriverApi*>(&mDriverApiStorage));
    }

    DFG const& getDFG() const noexcept { return mDFG; }

    // the per-frame Area is used by all Renderer, so they must run in sequence and
    // have freed all allocated memory when done. If this needs to change in the future,
    // we'll simply have to use separate Areas (for instance).
    LinearAllocatorArena& getPerRenderPassArena() noexcept { return mPerRenderPassArena; }

    // Material IDs...
    uint32_t getMaterialId() const noexcept { return mMaterialId++; }

    const FMaterial* getDefaultMaterial() const noexcept { return mDefaultMaterial; }
    const FMaterial* getSkyboxMaterial() const noexcept;
    const FIndirectLight* getDefaultIndirectLight() const noexcept { return mDefaultIbl; }
    const FTexture* getDummyCubemap() const noexcept { return mDefaultIblTexture; }
    const FColorGrading* getDefaultColorGrading() const noexcept { return mDefaultColorGrading; }
    FMorphTargetBuffer* getDummyMorphTargetBuffer() const { return mDummyMorphTargetBuffer; }

    backend::Handle<backend::HwRenderPrimitive> getFullScreenRenderPrimitive() const noexcept {
        return mFullScreenTriangleRph;
    }

    FVertexBuffer* getFullScreenVertexBuffer() const noexcept {
        return mFullScreenTriangleVb;
    }

    FIndexBuffer* getFullScreenIndexBuffer() const noexcept {
        return mFullScreenTriangleIb;
    }

    math::mat4f getUvFromClipMatrix() const noexcept {
        return mUvFromClipMatrix;
    }

    FeatureLevel getSupportedFeatureLevel() const noexcept;

    FeatureLevel setActiveFeatureLevel(FeatureLevel featureLevel);

    FeatureLevel getActiveFeatureLevel() const noexcept {
        return mActiveFeatureLevel;
    }

    size_t getMaxAutomaticInstances() const noexcept {
        return CONFIG_MAX_INSTANCES;
    }

    bool isStereoSupported() const noexcept {
        return getDriver().isStereoSupported();
    }

    static size_t getMaxStereoscopicEyes() noexcept {
        return CONFIG_MAX_STEREOSCOPIC_EYES;
    }

    PostProcessManager const& getPostProcessManager() const noexcept {
        return mPostProcessManager;
    }

    PostProcessManager& getPostProcessManager() noexcept {
        return mPostProcessManager;
    }

    FRenderableManager& getRenderableManager() noexcept {
        return mRenderableManager;
    }

    FRenderableManager const& getRenderableManager() const noexcept {
        return mRenderableManager;
    }

    FLightManager& getLightManager() noexcept {
        return mLightManager;
    }

    FLightManager const& getLightManager() const noexcept {
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

    Platform* getPlatform() const noexcept {
        return mPlatform;
    }

    size_t getMaxShadowMapCount() const noexcept;

    // Return a vector of shader languages, in order of preference.
    utils::FixedCapacityVector<backend::ShaderLanguage> getShaderLanguage() const noexcept {
        switch (mBackend) {
            default:
                return { getDriver().getShaderLanguage() };
            case Backend::METAL:
                const auto& lang = mConfig.preferredShaderLanguage;
                if (lang == Config::ShaderLanguage::MSL) {
                    return { backend::ShaderLanguage::MSL, backend::ShaderLanguage::METAL_LIBRARY};
                }
                return { backend::ShaderLanguage::METAL_LIBRARY, backend::ShaderLanguage::MSL };
        }
    }

    ResourceAllocatorDisposer& getResourceAllocatorDisposer() noexcept {
        assert_invariant(mResourceAllocatorDisposer);
        return *mResourceAllocatorDisposer;
    }

    std::shared_ptr<ResourceAllocatorDisposer> const& getSharedResourceAllocatorDisposer() noexcept {
        return mResourceAllocatorDisposer;
    }

    void* streamAlloc(size_t size, size_t alignment) noexcept;

    Epoch getEngineEpoch() const { return mEngineEpoch; }
    duration getEngineTime() const noexcept {
        return clock::now() - getEngineEpoch();
    }

    backend::Handle<backend::HwRenderTarget> getDefaultRenderTarget() const noexcept {
        return mDefaultRenderTarget;
    }

    template <typename T, typename ... ARGS>
    T* create(ResourceList<T>& list, typename T::Builder const& builder, ARGS&& ... args) noexcept;

    FBufferObject* createBufferObject(const BufferObject::Builder& builder) noexcept;
    FVertexBuffer* createVertexBuffer(const VertexBuffer::Builder& builder) noexcept;
    FIndexBuffer* createIndexBuffer(const IndexBuffer::Builder& builder) noexcept;
    FSkinningBuffer* createSkinningBuffer(const SkinningBuffer::Builder& builder) noexcept;
    FMorphTargetBuffer* createMorphTargetBuffer(const MorphTargetBuffer::Builder& builder) noexcept;
    FInstanceBuffer* createInstanceBuffer(const InstanceBuffer::Builder& builder) noexcept;
    FIndirectLight* createIndirectLight(const IndirectLight::Builder& builder) noexcept;
    FMaterial* createMaterial(const Material::Builder& builder, std::unique_ptr<MaterialParser> materialParser) noexcept;
    FTexture* createTexture(const Texture::Builder& builder) noexcept;
    FSkybox* createSkybox(const Skybox::Builder& builder) noexcept;
    FColorGrading* createColorGrading(const ColorGrading::Builder& builder) noexcept;
    FStream* createStream(const Stream::Builder& builder) noexcept;
    FRenderTarget* createRenderTarget(const RenderTarget::Builder& builder) noexcept;

    void createRenderable(const RenderableManager::Builder& builder, utils::Entity entity);
    void createLight(const LightManager::Builder& builder, utils::Entity entity);

    FRenderer* createRenderer() noexcept;

    FMaterialInstance* createMaterialInstance(const FMaterial* material,
            const FMaterialInstance* other, const char* name) noexcept;

    FMaterialInstance* createMaterialInstance(const FMaterial* material,
                                              const char* name) noexcept;

    FScene* createScene() noexcept;
    FView* createView() noexcept;
    FFence* createFence() noexcept;
    FSwapChain* createSwapChain(void* nativeWindow, uint64_t flags) noexcept;
    FSwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept;

    FCamera* createCamera(utils::Entity entity) noexcept;
    FCamera* getCameraComponent(utils::Entity entity) noexcept;
    void destroyCameraComponent(utils::Entity entity) noexcept;


    bool destroy(const FBufferObject* p);
    bool destroy(const FVertexBuffer* p);
    bool destroy(const FFence* p);
    bool destroy(const FIndexBuffer* p);
    bool destroy(const FSkinningBuffer* p);
    bool destroy(const FMorphTargetBuffer* p);
    bool destroy(const FIndirectLight* p);
    bool destroy(const FMaterial* p);
    bool destroy(const FMaterialInstance* p);
    bool destroy(const FRenderer* p);
    bool destroy(const FScene* p);
    bool destroy(const FSkybox* p);
    bool destroy(const FColorGrading* p);
    bool destroy(const FStream* p);
    bool destroy(const FTexture* p);
    bool destroy(const FRenderTarget* p);
    bool destroy(const FSwapChain* p);
    bool destroy(const FView* p);
    bool destroy(const FInstanceBuffer* p);

    bool isValid(const FBufferObject* p) const;
    bool isValid(const FVertexBuffer* p) const;
    bool isValid(const FFence* p) const;
    bool isValid(const FIndexBuffer* p) const;
    bool isValid(const FSkinningBuffer* p) const;
    bool isValid(const FMorphTargetBuffer* p) const;
    bool isValid(const FIndirectLight* p) const;
    bool isValid(const FMaterial* p) const;
    bool isValid(const FMaterial* m, const FMaterialInstance* p) const;
    bool isValidExpensive(const FMaterialInstance* p) const;
    bool isValid(const FRenderer* p) const;
    bool isValid(const FScene* p) const;
    bool isValid(const FSkybox* p) const;
    bool isValid(const FColorGrading* p) const;
    bool isValid(const FSwapChain* p) const;
    bool isValid(const FStream* p) const;
    bool isValid(const FTexture* p) const;
    bool isValid(const FRenderTarget* p) const;
    bool isValid(const FView* p) const;
    bool isValid(const FInstanceBuffer* p) const;

    size_t getBufferObjectCount() const noexcept;
    size_t getViewCount() const noexcept;
    size_t getSceneCount() const noexcept;
    size_t getSwapChainCount() const noexcept;
    size_t getStreamCount() const noexcept;
    size_t getIndexBufferCount() const noexcept;
    size_t getSkinningBufferCount() const noexcept;
    size_t getMorphTargetBufferCount() const noexcept;
    size_t getInstanceBufferCount() const noexcept;
    size_t getVertexBufferCount() const noexcept;
    size_t getIndirectLightCount() const noexcept;
    size_t getMaterialCount() const noexcept;
    size_t getTextureCount() const noexcept;
    size_t getSkyboxeCount() const noexcept;
    size_t getColorGradingCount() const noexcept;
    size_t getRenderTargetCount() const noexcept;

    void destroy(utils::Entity e);

    bool isPaused() const noexcept;
    void setPaused(bool paused);

    void flushAndWait();
    bool flushAndWait(uint64_t timeout);

    // flush the current buffer
    void flush();

    // flush the current buffer based on some heuristics
    void flushIfNeeded() {
        auto counter = mFlushCounter + 1;
        if (UTILS_LIKELY(counter < 128)) {
            mFlushCounter = counter;
        } else {
            mFlushCounter = 0;
            flush();
        }
    }

    /**
     * Processes the platform's event queue when called from the platform's event-handling thread.
     * Returns false when called from any other thread.
     */
    bool pumpPlatformEvents() {
        return mPlatform->pumpEvents();
    }

    void prepare();
    void gc();

    using ShaderContent = utils::FixedCapacityVector<uint8_t>;

    ShaderContent& getVertexShaderContent() const noexcept {
        return mVertexShaderContent;
    }

    ShaderContent& getFragmentShaderContent() const noexcept {
        return mFragmentShaderContent;
    }

    FDebugRegistry& getDebugRegistry() noexcept {
        return mDebugRegistry;
    }

    bool execute();

    utils::JobSystem& getJobSystem() const noexcept {
        // JobSystem is thread-safe, and it's always okay to return a non-const one,
        // it's conceptually the same as if we were holding a non-const reference, as opposed
        // to by-value class attribute.
        return const_cast<utils::JobSystem&>(mJobSystem);
    }

    std::default_random_engine& getRandomEngine() {
        return mRandomEngine;
    }

    void pumpMessageQueues() const {
        getDriver().purge();
    }

    void unprotected() noexcept;

    void setAutomaticInstancingEnabled(bool const enable) noexcept {
        // instancing is not allowed at feature level 0
        if (hasFeatureLevel(FeatureLevel::FEATURE_LEVEL_1)) {
            mAutomaticInstancingEnabled = enable;
        }
    }

    bool isAutomaticInstancingEnabled() const noexcept {
        return mAutomaticInstancingEnabled;
    }

    HwVertexBufferInfoFactory& getVertexBufferInfoFactory() noexcept {
        return mHwVertexBufferInfoFactory;
    }

    HwDescriptorSetLayoutFactory& getDescriptorSetLayoutFactory() noexcept {
        return mHwDescriptorSetLayoutFactory;
    }

    DescriptorSetLayout const& getPerViewDescriptorSetLayoutDepthVariant() const noexcept {
        return mPerViewDescriptorSetLayoutDepthVariant;
    }

    DescriptorSetLayout const& getPerViewDescriptorSetLayoutSsrVariant() const noexcept {
        return mPerViewDescriptorSetLayoutSsrVariant;
    }

    DescriptorSetLayout const& getPerRenderableDescriptorSetLayout() const noexcept {
        return mPerRenderableDescriptorSetLayout;
    }

    backend::Handle<backend::HwTexture> getOneTexture() const { return mDummyOneTexture; }
    backend::Handle<backend::HwTexture> getZeroTexture() const { return mDummyZeroTexture; }
    backend::Handle<backend::HwTexture> getOneTextureArray() const { return mDummyOneTextureArray; }
    backend::Handle<backend::HwTexture> getZeroTextureArray() const { return mDummyZeroTextureArray; }

    static constexpr size_t MiB = 1024u * 1024u;
    size_t getMinCommandBufferSize() const noexcept { return mConfig.minCommandBufferSizeMB * MiB; }
    size_t getCommandBufferSize() const noexcept { return mConfig.commandBufferSizeMB * MiB; }
    size_t getPerFrameCommandsSize() const noexcept { return mConfig.perFrameCommandsSizeMB * MiB; }
    size_t getPerRenderPassArenaSize() const noexcept { return mConfig.perRenderPassArenaSizeMB * MiB; }
    size_t getRequestedDriverHandleArenaSize() const noexcept { return mConfig.driverHandleArenaSizeMB * MiB; }
    Config const& getConfig() const noexcept { return mConfig; }

    bool hasFeatureLevel(backend::FeatureLevel const neededFeatureLevel) const noexcept {
        return getActiveFeatureLevel() >= neededFeatureLevel;
    }

    auto const& getMaterialInstanceResourceList() const noexcept {
        return mMaterialInstances;
    }

#if defined(__EMSCRIPTEN__)
    void resetBackendState() noexcept;
#endif

    backend::Driver& getDriver() const noexcept { return *mDriver; }

private:
    explicit FEngine(Builder const& builder);
    void init();
    void shutdown();

    int loop();
    void flushCommandBuffer(backend::CommandBufferQueue& commandBufferQueue);

    template<typename T>
    bool isValid(const T* ptr, ResourceList<T> const& list) const;

    template<typename T>
    bool terminateAndDestroy(const T* p, ResourceList<T>& list);

    template<typename T, typename Lock>
    bool terminateAndDestroyLocked(Lock& lock, const T* p, ResourceList<T>& list);

    template<typename T>
    void cleanupResourceList(ResourceList<T>&& list);

    template<typename T, typename Lock>
    void cleanupResourceListLocked(Lock& lock, ResourceList<T>&& list);

    backend::Driver* mDriver = nullptr;
    backend::Handle<backend::HwRenderTarget> mDefaultRenderTarget;

    Backend mBackend;
    FeatureLevel mActiveFeatureLevel = FeatureLevel::FEATURE_LEVEL_1;
    Platform* mPlatform = nullptr;
    bool mOwnPlatform = false;
    bool mAutomaticInstancingEnabled = false;
    void* mSharedGLContext = nullptr;
    backend::Handle<backend::HwRenderPrimitive> mFullScreenTriangleRph;
    FVertexBuffer* mFullScreenTriangleVb = nullptr;
    FIndexBuffer* mFullScreenTriangleIb = nullptr;
    math::mat4f mUvFromClipMatrix;

    PostProcessManager mPostProcessManager;

    utils::EntityManager& mEntityManager;
    FRenderableManager mRenderableManager;
    FTransformManager mTransformManager;
    FLightManager mLightManager;
    FCameraManager mCameraManager;
    std::shared_ptr<ResourceAllocatorDisposer> mResourceAllocatorDisposer;
    HwVertexBufferInfoFactory mHwVertexBufferInfoFactory;
    HwDescriptorSetLayoutFactory mHwDescriptorSetLayoutFactory;
    DescriptorSetLayout mPerViewDescriptorSetLayoutDepthVariant;
    DescriptorSetLayout mPerViewDescriptorSetLayoutSsrVariant;
    DescriptorSetLayout mPerRenderableDescriptorSetLayout;

    ResourceList<FBufferObject> mBufferObjects{ "BufferObject" };
    ResourceList<FRenderer> mRenderers{ "Renderer" };
    ResourceList<FView> mViews{ "View" };
    ResourceList<FScene> mScenes{ "Scene" };
    ResourceList<FSwapChain> mSwapChains{ "SwapChain" };
    ResourceList<FStream> mStreams{ "Stream" };
    ResourceList<FIndexBuffer> mIndexBuffers{ "IndexBuffer" };
    ResourceList<FSkinningBuffer> mSkinningBuffers{ "SkinningBuffer" };
    ResourceList<FMorphTargetBuffer> mMorphTargetBuffers{ "MorphTargetBuffer" };
    ResourceList<FInstanceBuffer> mInstanceBuffers{ "InstanceBuffer" };
    ResourceList<FVertexBuffer> mVertexBuffers{ "VertexBuffer" };
    ResourceList<FIndirectLight> mIndirectLights{ "IndirectLight" };
    ResourceList<FMaterial> mMaterials{ "Material" };
    ResourceList<FTexture> mTextures{ "Texture" };
    ResourceList<FSkybox> mSkyboxes{ "Skybox" };
    ResourceList<FColorGrading> mColorGradings{ "ColorGrading" };
    ResourceList<FRenderTarget> mRenderTargets{ "RenderTarget" };

    // the fence list is accessed from multiple threads
    utils::Mutex mFenceListLock;
    ResourceList<FFence> mFences{"Fence"};

    mutable uint32_t mMaterialId = 0;

    // FMaterialInstance are handled directly by FMaterial
    std::unordered_map<const FMaterial*, ResourceList<FMaterialInstance>> mMaterialInstances;

    DFG mDFG;

    std::thread mDriverThread;
    backend::CommandBufferQueue mCommandBufferQueue;
    std::aligned_storage<sizeof(DriverApi), alignof(DriverApi)>::type mDriverApiStorage;
    static_assert( sizeof(mDriverApiStorage) >= sizeof(DriverApi) );

    uint32_t mFlushCounter = 0;

    RootArenaScope::Arena mPerRenderPassArena;
    HeapAllocatorArena mHeapAllocator;

    utils::JobSystem mJobSystem;
    static uint32_t getJobSystemThreadPoolSize(Config const& config) noexcept;

    std::default_random_engine mRandomEngine;

    Epoch mEngineEpoch;

    mutable FMaterial const* mDefaultMaterial = nullptr;
    mutable FMaterial const* mSkyboxMaterial = nullptr;
    mutable FSwapChain* mUnprotectedDummySwapchain = nullptr;

    mutable FTexture* mDefaultIblTexture = nullptr;
    mutable FIndirectLight* mDefaultIbl = nullptr;

    mutable FColorGrading* mDefaultColorGrading = nullptr;
    FMorphTargetBuffer* mDummyMorphTargetBuffer = nullptr;

    mutable utils::CountDownLatch mDriverBarrier;

    mutable ShaderContent mVertexShaderContent;
    mutable ShaderContent mFragmentShaderContent;
    FDebugRegistry mDebugRegistry;

    backend::Handle<backend::HwTexture> mDummyOneTexture;
    backend::Handle<backend::HwTexture> mDummyOneTextureArray;
    backend::Handle<backend::HwTexture> mDummyZeroTextureArray;
    backend::Handle<backend::HwTexture> mDummyZeroTexture;

    std::thread::id mMainThreadId{};

    bool mInitialized = false;

    // Creation parameters
    Config mConfig;

public:
    // These are the debug properties used by FDebug.
    // They're accessed directly by modules who need them.
    struct {
        struct {
            bool debug_directional_shadowmap = false;
            bool display_shadow_texture = false;
            bool far_uses_shadowcasters = true;
            bool focus_shadowcasters = true;
            bool visualize_cascades = false;
            bool disable_light_frustum_align = false;
            bool depth_clamp = true;
            float dzn = -1.0f;
            float dzf =  1.0f;
            float display_shadow_texture_scale = 0.25f;
            int display_shadow_texture_layer = 0;
            int display_shadow_texture_level = 0;
            int display_shadow_texture_channel = 0;
            int display_shadow_texture_layer_count = 0;
            int display_shadow_texture_level_count = 0;
            float display_shadow_texture_power = 1;
        } shadowmap;
        struct {
            bool camera_at_origin = true;
            struct {
                float kp = 0.0f;
                float ki = 0.0f;
                float kd = 0.0f;
            } pid;
        } view;
        struct {
            // When set to true, the backend will attempt to capture the next frame and write the
            // capture to file. At the moment, only supported by the Metal backend.
            bool doFrameCapture = false;
            bool disable_buffer_padding = false;
            bool disable_subpasses = false;
        } renderer;
        struct {
            bool debug_froxel_visualization = false;
        } lighting;
        struct {
            bool combine_multiview_images = false;
        } stereo;
        matdbg::DebugServer* server = nullptr;
        fgviewer::DebugServer* fgviewerServer = nullptr;
    } debug;

    struct {
        struct {
            struct {
                bool use_shadow_atlas = false;
            } shadows;
            struct {
                // TODO: default the following two flags to true.
                bool assert_material_instance_in_use = false;
                bool assert_destroy_material_before_material_instance = false;
            } debug;
        } engine;
        struct {
            struct {
                bool assert_native_window_is_valid = false;
            } opengl;
            bool disable_parallel_shader_compile = false;
            bool disable_handle_use_after_free_check = false;
        } backend;
    } features;

    std::array<FeatureFlag, sizeof(features)> const mFeatures{{
            { "backend.disable_parallel_shader_compile",
              "Disable parallel shader compilation in GL and Metal backends.",
              &features.backend.disable_parallel_shader_compile, true },
            { "backend.disable_handle_use_after_free_check",
              "Disable Handle<> use-after-free checks.",
              &features.backend.disable_handle_use_after_free_check, true },
            { "backend.opengl.assert_native_window_is_valid",
              "Asserts that the ANativeWindow is valid when rendering starts.",
              &features.backend.opengl.assert_native_window_is_valid, true },
            { "engine.shadows.use_shadow_atlas",
              "Uses an array of atlases to store shadow maps.",
              &features.engine.shadows.use_shadow_atlas, false },
            { "features.engine.debug.assert_material_instance_in_use",
              "Assert when a MaterialInstance is destroyed while it is in use by RenderableManager.",
              &features.engine.debug.assert_material_instance_in_use, false },
            { "features.engine.debug.assert_destroy_material_before_material_instance",
              "Assert when a Material is destroyed but its instances are still alive.",
              &features.engine.debug.assert_destroy_material_before_material_instance, false },
    }};

    utils::Slice<const FeatureFlag> getFeatureFlags() const noexcept {
        return { mFeatures.data(), mFeatures.size() };
    }

    bool setFeatureFlag(char const* name, bool value) noexcept;
    std::optional<bool> getFeatureFlag(char const* name) const noexcept;
    bool* getFeatureFlagPtr(std::string_view name, bool allowConstant = false) const noexcept;
};

FILAMENT_DOWNCAST(Engine)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_ENGINE_H
