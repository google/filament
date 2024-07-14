#include "VizEngineAPIs.h" 

#ifdef WIN32
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#endif

#include <iostream>
#include <fstream>
#include <memory>

//////////////////////////////
// filament intrinsics
#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Camera.h>
#include <filament/Frustum.h>
#include <filament/Viewport.h>
#include <filament/Material.h>
#include <filament/Renderer.h>
#include <filament/SwapChain.h>
#include <filament/RenderableManager.h>
#include <filament/MaterialInstance.h>
#include <filament/TransformManager.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/Skybox.h>

#include <utils/EntityManager.h>
#include <utils/EntityInstance.h>
#include <utils/NameComponentManager.h>
#include <utils/JobSystem.h>
#include <utils/Path.h>
#include <utils/Systrace.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>
#include <gltfio/NodeManager.h>

#include <filameshio/MeshReader.h>

#include <filamentapp/Config.h>
#include <filamentapp/Cube.h>
#include <filamentapp/IBL.h>

#include "backend/platforms/VulkanPlatform.h" // requires blueVK.h

#include "../../VisualStudio/samples/generated/resources/resources.h"
#include "../../VisualStudio/samples/generated/resources/monkey.h"
#include "../../VisualStudio/samples/generated/resources/gltf_demo.h"
#include "../../VisualStudio/libs/filamentapp/generated/resources/filamentapp.h"
#include "../../VisualStudio/libs/gltfio/materials/uberarchive.h"

// FEngine
#include "../../libs/gltfio/src/FFilamentAsset.h"
#include "../../libs/gltfio/src/FNodeManager.h""
#include "../../libs/gltfio/src/extended/AssetLoaderExtended.h"
#include "../../libs/gltfio/src/FTrsTransformManager.h"
#include "../../libs/gltfio/src/GltfEnums.h"

#include "../../filament/src/details/engine.h"
#include "../../filament/src/ResourceAllocator.h"
#include "../../filament/src/components/RenderableManager.h"
//////////////////////////////

//////////////////////////////
// filament math
#include <math/mathfwd.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/norm.h>
#include <math/quat.h>
//////////////////////////////

#include "CustomComponents.h"

#if FILAMENT_DISABLE_MATOPT
#   define OPTIMIZE_MATERIALS false
#else
#   define OPTIMIZE_MATERIALS true
#endif

// name spaces
using namespace filament;
using namespace filamesh;
using namespace filament::math;
using namespace filament::backend;
using namespace filament::gltfio;
using namespace utils;

class FilamentAppVulkanPlatform : public VulkanPlatform {
public:
    FilamentAppVulkanPlatform(char const* gpuHintCstr) {
        utils::CString gpuHint{ gpuHintCstr };
        if (gpuHint.empty()) {
            return;
        }
        VulkanPlatform::Customization::GPUPreference pref;
        // Check to see if it is an integer, if so turn it into an index.
        if (std::all_of(gpuHint.begin(), gpuHint.end(), ::isdigit)) {
            char* p_end{};
            pref.index = static_cast<int8_t>(std::strtol(gpuHint.c_str(), &p_end, 10));
        }
        else {
            pref.deviceName = gpuHint;
        }
        mCustomization = {
            .gpu = pref
        };
    }

    virtual VulkanPlatform::Customization getCustomization() const noexcept override {
        return mCustomization;
    }

private:
    VulkanPlatform::Customization mCustomization;
};

#if FILAMENT_DISABLE_MATOPT
#define OPTIMIZE_MATERIALS false
#else
#define OPTIMIZE_MATERIALS true
#endif

#define CANVAS_INIT_W 16u
#define CANVAS_INIT_H 16u
#define CANVAS_INIT_DPI 96.f

#pragma region // global enumerations
#pragma endregion

namespace vzm::backlog
{
    enum class LogLevel
    {
        None,
        Default,
        Warning,
        Error,
    };
    
    void setConsoleColor(WORD color) {
#ifdef WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
#endif
    }

    void post(const std::string& input, LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Default: 
            setConsoleColor(10);
            std::cout << "[INFO] ";
            setConsoleColor(7);
            utils::slog.i << input;
            break;
        case LogLevel::Warning:
            setConsoleColor(14);
            std::cout << "[WARNING] ";
            setConsoleColor(7);
            utils::slog.w << input;
            break; 
        case LogLevel::Error:
            setConsoleColor(12);
            std::cout << "[ERROR] ";
            setConsoleColor(7);
            utils::slog.e << input;
            break;
        default: return;
        }
        std::cout << input << std::endl;
    }
}

namespace vzm
{
    struct Timer
    {
        std::chrono::high_resolution_clock::time_point timeStamp = std::chrono::high_resolution_clock::now();

        // Record a reference timestamp
        inline void Record()
        {
            timeStamp = std::chrono::high_resolution_clock::now();
        }

        // Elapsed time in seconds between the vzm::Timer creation or last recording and "timestamp2"
        inline double ElapsedSecondsSince(std::chrono::high_resolution_clock::time_point timestamp2)
        {
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timestamp2 - timeStamp);
            return time_span.count();
        }

        // Elapsed time in seconds since the vzm::Timer creation or last recording
        inline double ElapsedSeconds()
        {
            return ElapsedSecondsSince(std::chrono::high_resolution_clock::now());
        }

        // Elapsed time in milliseconds since the vzm::Timer creation or last recording
        inline double ElapsedMilliseconds()
        {
            return ElapsedSeconds() * 1000.0;
        }

        // Elapsed time in milliseconds since the vzm::Timer creation or last recording
        inline double Elapsed()
        {
            return ElapsedMilliseconds();
        }

        // Record a reference timestamp and return elapsed time in seconds since the vzm::Timer creation or last recording
        inline double RecordElapsedSeconds()
        {
            auto timestamp2 = std::chrono::high_resolution_clock::now();
            auto elapsed = ElapsedSecondsSince(timestamp2);
            timeStamp = timestamp2;
            return elapsed;
        }
    };
}

static bool gIsDisplay = true;
auto failRet = [](const std::string& err_str, const bool _warn = false)
    {
        if (gIsDisplay)
        {
            vzm::backlog::post(err_str, _warn ? vzm::backlog::LogLevel::Warning : vzm::backlog::LogLevel::Error);
        }
        return false;
    };

inline float3 transformCoord(const mat4f& m, const float3& p)
{
    float4 _p(p, 1.f);
    _p = m * _p;
    return float3(_p.x / _p.w, _p.y / _p.w, _p.z / _p.w);
}

inline float3 transformVec(const mat3f& m, const float3& v)
{
    return m * v;
}

static Config gConfig;
static Engine::Config gEngineConfig = {};
static filament::backend::VulkanPlatform* gVulkanPlatform = nullptr;
static Engine* gEngine = nullptr;
static filament::SwapChain* gDummySwapChain = nullptr;
static filament::Material* gMaterialTransparent = nullptr; // do not release
enum MaterialSource {
    JITSHADER,
    UBERSHADER,
};
gltfio::MaterialProvider* gMaterialProvider = nullptr;

static std::vector<std::string> gMProp = {
            "baseColor",              //!< float4, all shading models
            "roughness",               //!< float,  lit shading models only
            "metallic",                //!< float,  all shading models, except unlit and cloth
            "reflectance",             //!< float,  all shading models, except unlit and cloth
            "ambientOcclusion",       //!< float,  lit shading models only, except subsurface and cloth
            "clearCoat",              //!< float,  lit shading models only, except subsurface and cloth
            "clearCoatRoughness",    //!< float,  lit shading models only, except subsurface and cloth
            "clearCoatNormal",       //!< float,  lit shading models only, except subsurface and cloth
            "anisotropy",              //!< float,  lit shading models only, except subsurface and cloth
            "anisotropyDirection",    //!< float3, lit shading models only, except subsurface and cloth
            "thickness",               //!< float,  subsurface shading model only
            "subsurfacePower",        //!< float,  subsurface shading model only
            "subsurfaceColor",        //!< float3, subsurface and cloth shading models only
            "sheenColor",             //!< float3, lit shading models only, except subsurface
            "sheenRoughness",         //!< float3, lit shading models only, except subsurface and cloth
            "specularColor",          //!< float3, specular-glossiness shading model only
            "glossiness",              //!< float,  specular-glossiness shading model only
            "emissive",                //!< float4, all shading models
            "normal",                  //!< float3, all shading models only, except unlit
            "postLightingColor",     //!< float4, all shading models
            "postLightingMixFactor",//!< float, all shading models
            "clipSpaceTransform",    //!< mat4,   vertex shader only
            "absorption",              //!< float3, how much light is absorbed by the material
            "transmission",            //!< float,  how much light is refracted through the material
            "ior",                     //!< float,  material's index of refraction
            "microThickness",         //!< float, thickness of the thin layer
            "bentNormal",             //!< float3, all shading models only, except unlit
            "specularFactor",         //!< float, lit shading models only, except subsurface and cloth
            "specularColorFactor",   //!< float3, lit shading models only, except subsurface and cloth
};

using CameraManipulator = filament::camutils::Manipulator<float>;
using SceneVID = VID;
using RendererVID = VID;
using CamVID = VID;
using ActorVID = VID;
using LightVID = VID;
using GeometryVID = VID;
using MaterialVID = VID;
using MaterialInstanceVID = VID;
using MaterialVID = VID;
using AssetVID = VID;
namespace filament::gltfio {
    using namespace vzm;
    using Entity = utils::Entity;
    using FFilamentAsset = filament::gltfio::FFilamentAsset;
    using SceneMask = NodeManager::SceneMask;

    // The default glTF material.
    static constexpr cgltf_material kDefaultMat = {
        .name = (char*)"Default GLTF material",
        .has_pbr_metallic_roughness = true,
        .has_pbr_specular_glossiness = false,
        .has_clearcoat = false,
        .has_transmission = false,
        .has_volume = false,
        .has_ior = false,
        .has_specular = false,
        .has_sheen = false,
        .pbr_metallic_roughness = {
            .base_color_factor = {1.0, 1.0, 1.0, 1.0},
            .metallic_factor = 1.0,
            .roughness_factor = 1.0,
        },
    };

    class MaterialInstanceCache {
    public:
        struct Entry {
            MaterialInstance* instance;
            UvMap uvmap;
        };

        MaterialInstanceCache() {}

        MaterialInstanceCache(const cgltf_data* hierarchy) :
            mHierarchy(hierarchy),
            mMaterialInstances(hierarchy->materials_count, Entry{}),
            mMaterialInstancesWithVertexColor(hierarchy->materials_count, Entry{}) {}

        void flush(utils::FixedCapacityVector<MaterialInstance*>* dest) {
            size_t count = 0;
            for (const Entry& entry : mMaterialInstances) {
                if (entry.instance) {
                    ++count;
                }
            }
            for (const Entry& entry : mMaterialInstancesWithVertexColor) {
                if (entry.instance) {
                    ++count;
                }
            }
            if (mDefaultMaterialInstance.instance) {
                ++count;
            }
            if (mDefaultMaterialInstanceWithVertexColor.instance) {
                ++count;
            }
            assert_invariant(dest->size() == 0);
            dest->reserve(count);
            for (const Entry& entry : mMaterialInstances) {
                if (entry.instance) {
                    dest->push_back(entry.instance);
                }
            }
            for (const Entry& entry : mMaterialInstancesWithVertexColor) {
                if (entry.instance) {
                    dest->push_back(entry.instance);
                }
            }
            if (mDefaultMaterialInstance.instance) {
                dest->push_back(mDefaultMaterialInstance.instance);
            }
            if (mDefaultMaterialInstanceWithVertexColor.instance) {
                dest->push_back(mDefaultMaterialInstanceWithVertexColor.instance);
            }
        }

        Entry* getEntry(const cgltf_material** mat, bool vertexColor) {
            if (*mat) {
                EntryVector& entries = vertexColor ?
                    mMaterialInstancesWithVertexColor : mMaterialInstances;
                const cgltf_material* basePointer = mHierarchy->materials;
                return &entries[*mat - basePointer];
            }
            *mat = &kDefaultMat;
            return vertexColor ? &mDefaultMaterialInstanceWithVertexColor : &mDefaultMaterialInstance;
        }

    private:
        using EntryVector = utils::FixedCapacityVector<Entry>;
        const cgltf_data* mHierarchy = {};
        EntryVector mMaterialInstances;
        EntryVector mMaterialInstancesWithVertexColor;
        Entry mDefaultMaterialInstance = {};
        Entry mDefaultMaterialInstanceWithVertexColor = {};
    };

    struct VzAssetLoader : public AssetLoader {
        VzAssetLoader(AssetConfiguration const& config) :
            mEntityManager(config.entities ? *config.entities : EntityManager::get()),
            mRenderableManager(config.engine->getRenderableManager()),
            mNameManager(config.names),
            mTransformManager(config.engine->getTransformManager()),
            mMaterials(*config.materials),
            mEngine(*config.engine),
            mDefaultNodeName(config.defaultNodeName) {
            if (config.ext) {
                FILAMENT_CHECK_PRECONDITION(AssetConfigurationExtended::isSupported())
                    << "Extend asset loading is not supported on this platform";
                mLoaderExtended = std::make_unique<AssetLoaderExtended>(
                    *config.ext, config.engine, mMaterials);
            }
        }

        FFilamentAsset* createAsset(const uint8_t* bytes, uint32_t nbytes);
        FFilamentAsset* createInstancedAsset(const uint8_t* bytes, uint32_t numBytes,
            FilamentInstance** instances, size_t numInstances);
        FilamentInstance* createInstance(FFilamentAsset* fAsset);

        static void destroy(VzAssetLoader** loader) noexcept {
            delete* loader;
            *loader = nullptr;
        }

        void destroyAsset(const FFilamentAsset* asset) {
            delete asset;
        }

        size_t getMaterialsCount() const noexcept {
            return mMaterials.getMaterialsCount();
        }

        NameComponentManager* getNames() const noexcept {
            return mNameManager;
        }

        NodeManager& getNodeManager() noexcept {
            return mNodeManager;
        }

        const Material* const* getMaterials() const noexcept {
            return mMaterials.getMaterials();
        }

    private:
        void importSkins(FFilamentInstance* instance, const cgltf_data* srcAsset);

        // Methods used during the first traveral (creation of VertexBuffer, IndexBuffer, etc)
        FFilamentAsset* createRootAsset(const cgltf_data* srcAsset);
        void recursePrimitives(const cgltf_node* rootNode, FFilamentAsset* fAsset);
        void createPrimitives(const cgltf_node* node, const char* name, FFilamentAsset* fAsset);
        bool createPrimitive(const cgltf_primitive& inPrim, const char* name, Primitive* outPrim,
            FFilamentAsset* fAsset);

        // Methods used during subsequent traverals (creation of entities, renderables, etc)
        void createInstances(size_t numInstances, FFilamentAsset* fAsset);
        void recurseEntities(const cgltf_node* node, SceneMask scenes, Entity parent,
            FFilamentAsset* fAsset, FFilamentInstance* instance);
        void createRenderable(const cgltf_node* node, Entity entity, const char* name,
            FFilamentAsset* fAsset);
        void createLight(const cgltf_light* light, Entity entity, FFilamentAsset* fAsset);
        void createCamera(const cgltf_camera* camera, Entity entity, FFilamentAsset* fAsset);
        void addTextureBinding(MaterialInstance* materialInstance, const char* parameterName,
            const cgltf_texture* srcTexture, bool srgb);
        void createMaterialVariants(const cgltf_mesh* mesh, Entity entity, FFilamentAsset* fAsset,
            FFilamentInstance* instance);

        // Utility methods that work with MaterialProvider.
        Material* getMaterial(const cgltf_data* srcAsset, const cgltf_material* inputMat, UvMap* uvmap,
            bool vertexColor);
        MaterialInstance* createMaterialInstance(const cgltf_material* inputMat, UvMap* uvmap,
            bool vertexColor, FFilamentAsset* fAsset);
        MaterialKey getMaterialKey(const cgltf_data* srcAsset,
            const cgltf_material* inputMat, UvMap* uvmap, bool vertexColor,
            cgltf_texture_view* baseColorTexture,
            cgltf_texture_view* metallicRoughnessTexture) const;

    public:
        EntityManager& mEntityManager;
        RenderableManager& mRenderableManager;
        NameComponentManager* const mNameManager;
        TransformManager& mTransformManager;
        MaterialProvider& mMaterials;
        Engine& mEngine;
        FNodeManager mNodeManager;
        FTrsTransformManager mTrsTransformManager;

        // Transient state used only for the asset currently being loaded:
        const char* mDefaultNodeName;
        bool mError = false;
        bool mDiagnosticsEnabled = false;
        MaterialInstanceCache mMaterialInstanceCache;

        // Weak reference to the largest dummy buffer so far in the current loading phase.
        BufferObject* mDummyBufferObject = nullptr;

        std::unordered_map<const cgltf_mesh*, GeometryVID> mGeometryMap;
        std::unordered_map<const Material*, MaterialVID> mMaterialMap;
        std::unordered_map<const MaterialInstance*, MaterialInstanceVID> mMIMap;
        std::unordered_map<VID, std::string> mSceneCompMap;

    public:
        std::unique_ptr<AssetLoaderExtended> mLoaderExtended;
    };
}
namespace vzm
{
    vzm::Timer vTimer;
    std::atomic_bool profileFrameFinished = { true };

    struct GltfIO
    {
        std::unordered_map<AssetVID, gltfio::FilamentAsset*> assets;
        std::unordered_map<AssetVID, std::vector<VID>> assetComponents;
        std::unordered_map<VID, AssetVID> vzCompAssociatedAssets;   // used for ownership of filament/vz components

        //gltfio::AssetLoader* assetLoader = nullptr;
        gltfio::VzAssetLoader* assetLoader = nullptr;

        gltfio::ResourceLoader* resourceLoader = nullptr;
        gltfio::TextureProvider* stbDecoder = nullptr;
        gltfio::TextureProvider* ktxDecoder = nullptr;

        bool DestroyAsset(AssetVID vidAsset)
        {
            auto it = assets.find(vidAsset);
            if (it == assets.end())
            {
                return false;
            }
            assetLoader->destroyAsset((gltfio::FFilamentAsset*)it->second);
            assets.erase(it);
            return true;
        }

        void Destory()
        {
            if (resourceLoader) {
                resourceLoader->asyncCancelLoad();
                resourceLoader = nullptr;
            }
            if (assets.size() > 0) {
                for (auto& it : assets)
                {
                    filament::gltfio::FFilamentAsset* fasset = downcast(it.second);
                    {
                        // Destroy gltfio node components.
                        for (auto& entity : fasset->mEntities) {
                            fasset->mNodeManager->destroy(entity);
                        }
                        // Destroy gltfio trs transform components.
                        for (auto& entity : fasset->mEntities) {
                            fasset->mTrsTransformManager->destroy(entity);
                        }
                    }
                    fasset->mEntities.clear();
                    fasset->detachFilamentComponents();
                    fasset->mVertexBuffers.clear();
                    fasset->mIndexBuffers.clear();
                    //fasset->mBufferObjects.clear();
                    //fasset->mTextures.clear();
                    fasset->mMorphTargetBuffers.clear();

                    for (FFilamentInstance* instance : fasset->mInstances) {
                        instance->mMaterialInstances.clear();
                        delete instance;
                    }
                    fasset->mInstances.clear();

                    assetLoader->destroyAsset(fasset);
                }
                assets.clear();
            }

            delete resourceLoader;
            resourceLoader = nullptr;
            delete stbDecoder;
            stbDecoder = nullptr;
            delete ktxDecoder;
            ktxDecoder = nullptr;


            //FAssetLoader* temp(downcast(*loader));
            //VzAssetLoader::destroy(&temp);
            //*loader = temp;
            
            //AssetLoader::destroy(&assetLoader);
            gltfio::VzAssetLoader::destroy(&assetLoader);
            
            assetLoader = nullptr;
        }

        void Initialize()
        {
            ResourceConfiguration configuration = {};
            configuration.engine = gEngine;
            configuration.gltfPath = ""; // gltfPath.c_str();
            configuration.normalizeSkinningWeights = true;

            vzGltfIO.resourceLoader = new gltfio::ResourceLoader(configuration);
            vzGltfIO.stbDecoder = createStbProvider(gEngine);
            vzGltfIO.ktxDecoder = createKtx2Provider(gEngine);
            vzGltfIO.resourceLoader->addTextureProvider("image/png", vzGltfIO.stbDecoder);
            vzGltfIO.resourceLoader->addTextureProvider("image/jpeg", vzGltfIO.stbDecoder);
            vzGltfIO.resourceLoader->addTextureProvider("image/ktx2", vzGltfIO.ktxDecoder);

            auto& ncm = VzNameCompManager::Get();
            //vzGltfIO.assetLoader = AssetLoader::create({ gEngine, gMaterialProvider, (NameComponentManager*)&ncm });
            //vzGltfIO.assetLoader = new vzm::gltf::VzAssetLoader({ gEngine, gMaterialProvider, (NameComponentManager*)&ncm });
        }
    } vzGltfIO;

#pragma region // VzRenderPath
    struct VzCanvas
    {
    protected:
        uint32_t width_ = CANVAS_INIT_W;
        uint32_t height_ = CANVAS_INIT_H;
        float dpi_ = CANVAS_INIT_DPI;
        float scaling_ = 1; // custom DPI scaling factor (optional)
        void* nativeWindow_ = nullptr;
    };

    // note that renderPath involves 
    // 1. canvas (render targets), 2. camera, 3. scene
    class VzRenderPath : public VzCanvas
    {
    private:

        uint32_t prevWidth_ = 0;
        uint32_t prevHeight_ = 0;
        float prevDpi_ = 0;
        void* prevNativeWindow_ = nullptr;
        bool prevColorspaceConversionRequired_ = false;

        VzCamera* vzCam_ = nullptr;
        TimeStamp timeStamp_ = {};

        float targetFrameRate_ = 60.f;

        bool colorspaceConversionRequired_ = false;
        uint64_t colorSpace_ = SWAP_CHAIN_CONFIG_SRGB_COLORSPACE; // swapchain color space
        
        // note "view" involves
        // 1. camera
        // 2. scene
        filament::View* view_ = nullptr;
        filament::SwapChain* swapChain_ = nullptr;
        filament::Renderer* renderer_ = nullptr;

        void resize()
        {
            auto resizeJob = [&]()
                {
                    gEngine->destroy(swapChain_);
                    if (nativeWindow_ == nullptr)
                    {
                        swapChain_ = gEngine->createSwapChain(width_, height_);
                    }
                    else
                    {
                        swapChain_ = gEngine->createSwapChain(
                            nativeWindow_, filament::SwapChain::CONFIG_HAS_STENCIL_BUFFER);

                        // dummy calls?
                        // this code causes async error 
                        // "state->elapsed.store(int64_t(TimerQueryResult::ERROR), std::memory_order_relaxed);"
                        //renderer_->beginFrame(swapChain_);
                        //renderer_->endFrame();
                    }
                };

            //utils::JobSystem::Job* parent = js.createJob();
            //js.run(jobs::createJob(js, parent, resizeJob));
            //js.runAndWait(parent);
            resizeJob();
        }

    public:
        VzRenderPath()
        {
            assert(gEngine && "native engine is not initialized!");
            view_ = gEngine->createView();
            renderer_ = gEngine->createRenderer();
            swapChain_ = gEngine->createSwapChain(width_, height_);
        }

        ~VzRenderPath()
        {
            if (gEngine)
            {
                try {
                    if (renderer_)
                        gEngine->destroy(renderer_);
                    if (view_)
                        gEngine->destroy(view_);
                    if (swapChain_)
                        gEngine->destroy(swapChain_);
                }
                catch (const std::exception& e) {
                    std::cerr << "Error destroying renderer: " << e.what() << std::endl;
                }
            }
        }

        bool TryResizeRenderTargets()
        {
            if (gEngine == nullptr)
                return false;

            colorspaceConversionRequired_ = colorSpace_ != SWAP_CHAIN_CONFIG_SRGB_COLORSPACE;

            bool requireUpdateRenderTarget = prevWidth_ != width_ || prevHeight_ != height_ || prevDpi_ != dpi_
                || prevColorspaceConversionRequired_ != colorspaceConversionRequired_;
            if (!requireUpdateRenderTarget)
                return false;

            resize(); // how to handle rendertarget textures??

            prevWidth_ = width_;
            prevHeight_ = height_;
            prevDpi_ = dpi_;
            prevNativeWindow_ = nativeWindow_;
            prevColorspaceConversionRequired_ = colorspaceConversionRequired_;
            return true;
        }

        inline void SetFixedTimeUpdate(const float targetFPS)
        {
            targetFrameRate_ = targetFPS;
            timeStamp_ = std::chrono::high_resolution_clock::now();
        }
        inline float GetFixedTimeUpdate() const
        {
            return targetFrameRate_;
        }

        inline void GetCanvas(uint32_t* w, uint32_t* h, float* dpi, void** window)
        {
            if (w) *w = width_;
            if (h) *h = height_;
            if (dpi) *dpi = dpi_;
            if (window) *window = nativeWindow_;
        }
        inline void SetCanvas(const uint32_t w, const uint32_t h, const float dpi, void* window = nullptr)
        {
            // the resize is called during the rendering (pre-processing)
            width_ = w;
            height_ = h;
            this->dpi_ = dpi;
            nativeWindow_ = window;

            view_->setViewport(filament::Viewport(0, 0, w, h));
            timeStamp_ = std::chrono::high_resolution_clock::now();
        }
        inline filament::SwapChain* GetSwapChain()
        {
            return swapChain_;
        }

        uint64_t FRAMECOUNT = 0;
        vzm::Timer timer;
        float deltaTime = 0;
        float deltaTimeAccumulator = 0;

        inline filament::View* GetView() { return view_; }

        inline filament::Renderer* GetRenderer() { return renderer_; }
    };
#pragma endregion

#pragma region // VzEngineApp
    struct VzSceneRes
    {
    private:
        IBL* ibl_ = nullptr;
        Cube* lightmapCube_ = nullptr; // note current filament supports only one directional light's shadowmap
    public:
        VzSceneRes() { NewIBL(); };
        ~VzSceneRes() { Destory(); };
        void Destory()
        {
            if (ibl_) {
                delete ibl_;
                ibl_ = nullptr;
            }
            if (lightmapCube_) {
                delete lightmapCube_;
                lightmapCube_ = nullptr;
            }
        }
        IBL* GetIBL() { return ibl_; }
        IBL* NewIBL()
        {
            if (ibl_) {
                delete ibl_;
            }
            ibl_ = new IBL(*gEngine);
            return ibl_;
        }
        Cube* GetLightmapCube()
        {
            if (lightmapCube_)
            {
                return lightmapCube_;
            }
            lightmapCube_ = new Cube(*gEngine, gMaterialTransparent, { 0, 1, 0 }, false);
            return lightmapCube_;
        }
    };
    struct VzCameraRes
    {
    private:
        filament::Camera* camera_ = nullptr;
        Cube* cameraCube_ = nullptr;
        VzCamera::Controller camController_;
        std::unique_ptr<CameraManipulator> cameraManipulator_;
    public:
        VzCameraRes() = default;
        ~VzCameraRes()
        {
            if (cameraCube_) {
                delete cameraCube_;
                cameraCube_ = nullptr;
            }
            cameraManipulator_.reset();
        }

        inline void SetCamera(Camera* camera) { camera_ = camera; }
        inline Camera* GetCamera() { return camera_; }
        inline Cube* GetCameraCube()
        {
            if (cameraCube_)
            {
                return cameraCube_;
            }
            cameraCube_ = new Cube(*gEngine, gMaterialTransparent, { 1, 0, 0 });
            return cameraCube_;
        }
        inline void NewCameraManipulator(const VzCamera::Controller& camController)
        {
            camController_ = camController;
            cameraManipulator_.reset(CameraManipulator::Builder()
                .targetPosition(camController_.targetPosition[0], camController_.targetPosition[1], camController_.targetPosition[2])
                .upVector(camController_.upVector[0], camController_.upVector[1], camController_.upVector[2])
                .zoomSpeed(camController_.zoomSpeed)

                .orbitHomePosition(camController_.orbitHomePosition[0], camController_.orbitHomePosition[1], camController_.orbitHomePosition[2])
                .orbitSpeed(camController_.orbitSpeed[0], camController_.orbitSpeed[1])

                .fovDirection(camController_.isVerticalFov ? camutils::Fov::VERTICAL : camutils::Fov::HORIZONTAL)
                .fovDegrees(camController_.fovDegrees)
                .farPlane(camController_.farPlane)
                .mapExtent(camController_.mapExtent[0], camController_.mapExtent[1])
                .mapMinDistance(camController_.mapMinDistance)

                .flightStartPosition(camController_.flightStartPosition[0], camController_.flightStartPosition[1], camController_.flightStartPosition[2])
                .flightStartOrientation(camController_.flightStartPitch, camController_.flightStartYaw)
                .flightMaxMoveSpeed(camController_.flightMaxSpeed)
                .flightSpeedSteps(camController_.flightSpeedSteps)
                .flightPanSpeed(camController_.flightPanSpeed[0], camController_.flightPanSpeed[1])
                .flightMoveDamping(camController_.flightMoveDamping)

                .groundPlane(camController_.groundPlane[0], camController_.groundPlane[1], camController_.groundPlane[2], camController_.groundPlane[3])
                .panning(camController_.panning)
                .build((camutils::Mode)camController_.mode));
            camController_.vidCam = camera_->getEntity().getId();
        }
        inline VzCamera::Controller* GetCameraController()
        {
            return &camController_;
        }
        inline CameraManipulator* GetCameraManipulator()
        {
            return cameraManipulator_.get();
        }
        inline void UpdateCameraWithCM(float deltaTime)
        {
            if (cameraManipulator_.get() != nullptr)
            {
                cameraManipulator_->update(deltaTime);
                filament::math::float3 eye, center, up;
                cameraManipulator_->getLookAt(&eye, &center, &up);
                camera_->lookAt(eye, center, up);
            }
        }
    };
    struct VzActorRes
    {
    private:
        GeometryVID vidGeo_ = INVALID_VID;
        std::vector<MaterialInstanceVID> vidMIs_;
        std::vector<std::vector<MaterialInstanceVID>> vidMIVariants_;
    public:
        inline void SetGeometry(const GeometryVID vid) { vidGeo_ = vid; }
        inline void SetMIs(const std::vector<MaterialInstanceVID>& vidMIs)
        {
            vidMIs_ = vidMIs;
        }
        inline bool SetMI(const MaterialInstanceVID vid, const int slot)
        {
            if ((size_t)slot >= vidMIs_.size())
            {
                backlog::post("a slot cannot exceed the number of elements in the MI array", backlog::LogLevel::Error);
                return false;
            }
            vidMIs_[slot] = vid;
            return true;
        }
        inline void SetMIVariants(const std::vector<std::vector<MaterialInstanceVID>>& vidMIVariants)
        {
            vidMIVariants_ = vidMIVariants;
        }
        inline GeometryVID GetGeometryVid() { return vidGeo_; }
        inline MaterialInstanceVID GetMIVid(const int slot)
        {
            if ((size_t)slot >= vidMIs_.size())
            {
                backlog::post("a slot cannot exceed the number of elements in the MI array", backlog::LogLevel::Error);
            }
            return vidMIs_[slot];
        }
        inline std::vector<MaterialInstanceVID>& GetMIVids()
        {
            return vidMIs_;
        }
        inline std::vector<std::vector<MaterialInstanceVID>>& GetMIVariants() { return vidMIVariants_; }
    };
    struct VzLightRes
    {
    private:
    public:
        VzLightRes() {};
        ~VzLightRes() {};
    };
    struct VzGeometryRes
    {
    private:
        static std::set<VertexBuffer*> currentVBs_;
        static std::set<IndexBuffer*> currentIBs_;
        static std::set<MorphTargetBuffer*> currentMTBs_;

        std::vector<filament::gltfio::Primitive> primitives_;
        std::vector<RenderableManager::PrimitiveType> primitiveTypes_;
    public:
        bool isSystem = false;
        gltfio::FilamentAsset* assetOwner = nullptr; // has ownership
        Aabb aabb;

        void Set(const std::vector<Primitive>& primitives)
        {
            primitives_ = primitives;
            for (Primitive& prim : primitives_)
            {
                currentVBs_.insert(prim.vertices);
                currentIBs_.insert(prim.indices);
                currentMTBs_.insert(prim.morphTargetBuffer);
            }
        }
        void SetTypes(const std::vector<RenderableManager::PrimitiveType>& primitiveTypes)
        {
            primitiveTypes_ = primitiveTypes;
        }
        std::vector<filament::gltfio::Primitive>* Get() { return &primitives_; }
        std::vector<RenderableManager::PrimitiveType>* GetTypes() { return &primitiveTypes_; }

        ~VzGeometryRes()
        {
            // check the ownership
            if (assetOwner == nullptr && !isSystem)
            {
                for (auto& prim : primitives_)
                {
                    if (prim.vertices && currentVBs_.find(prim.vertices) != currentVBs_.end()) {
                        gEngine->destroy(prim.vertices);
                        currentVBs_.erase(prim.vertices);
                    }
                    if (prim.indices && currentIBs_.find(prim.indices) != currentIBs_.end()) {
                        gEngine->destroy(prim.indices);
                        currentIBs_.erase(prim.indices);
                    }
                    if (prim.morphTargetBuffer && currentMTBs_.find(prim.morphTargetBuffer) != currentMTBs_.end()) {
                        gEngine->destroy(prim.morphTargetBuffer);
                        currentMTBs_.erase(prim.morphTargetBuffer);
                    }
                }
            }
        }
    };
    std::set<filament::VertexBuffer*> VzGeometryRes::currentVBs_;
    std::set<filament::IndexBuffer*> VzGeometryRes::currentIBs_;
    std::set<filament::MorphTargetBuffer*> VzGeometryRes::currentMTBs_;

    struct VzMaterialRes
    {
        bool isSystem = false;
        gltfio::FilamentAsset* assetOwner = nullptr; // has ownership
        filament::Material* material = nullptr;
        ~VzMaterialRes()
        {
            if (assetOwner == nullptr && !isSystem)
            {
                if (material)
                    gEngine->destroy(material);
                material = nullptr;
                // check MI...
            }
        }
    };
    struct VzMIRes
    {
        bool isSystem = false;
        gltfio::FilamentAsset* assetOwner = nullptr; // has ownership
        filament::MaterialInstance* mi = nullptr;
        ~VzMIRes()
        {
            if (assetOwner == nullptr && !isSystem)
            {
                if (mi)
                    gEngine->destroy(mi);
                mi = nullptr;
                // check MI...
            }
        }
    };
    class VzEngineApp
    {
    private:
        std::unordered_map<SceneVID, filament::Scene*> scenes_;
        std::unordered_map<SceneVID, VzSceneRes> sceneResMaps_;
        // note a VzRenderPath involves a filament::view that includes
        // 1. filament::camera and 2. filament::scene
        std::unordered_map<CamVID, SceneVID> camSceneVids_;
        std::unordered_map<CamVID, VzCameraRes> camResMaps_;
        std::unordered_map<ActorVID, SceneVID> actorSceneVids_;
        std::unordered_map<ActorVID, VzActorRes> actorResMaps_; // consider when removing resources...
        std::unordered_map<LightVID, SceneVID> lightSceneVids_;
        std::unordered_map<LightVID, VzLightRes> lightResMaps_;

        std::unordered_map<RendererVID, VzRenderPath> renderPaths_;

        // Resources (ownership check!)
        std::unordered_map<GeometryVID, VzGeometryRes> geometries_;
        std::unordered_map<MaterialVID, VzMaterialRes> materials_;
        std::unordered_map<MaterialInstanceVID, VzMIRes> materialInstances_;

        std::unordered_map<VID, std::unique_ptr<VzBaseComp>> vzComponents_;

        inline bool removeScene(SceneVID vidScene)
        {
            Scene* scene = GetScene(vidScene);
            if (scene == nullptr)
            {
                return false;
            }
            auto& rcm = gEngine->getRenderableManager();
            auto& lcm = gEngine->getLightManager();
            int retired_ett_count = 0;
            scene->forEach([&](utils::Entity ett) {
                VID vid = ett.getId();
                if (rcm.hasComponent(ett))
                {
                    // note that
                    // there can be intrinsic entities in the scene
                    auto it = actorSceneVids_.find(vid);
                    if (it != actorSceneVids_.end()) {
                        it->second = 0;
                        ++retired_ett_count;
                    }
                }
                else if (lcm.hasComponent(ett))
                {
                    auto it = lightSceneVids_.find(vid);
                    if (it != lightSceneVids_.end()) {
                        it->second = 0;
                        ++retired_ett_count;
                    }
                }
                else
                {
                    // optional.
                    auto it = camSceneVids_.find(vid);
                    if (it != camSceneVids_.end()) {
                        backlog::post("cam VID : " + std::to_string(ett.getId()), backlog::LogLevel::Default);
                        it->second = 0;
                        ++retired_ett_count;
                    }
                    else
                    {
                        auto& ncm = VzNameCompManager::Get();

                        auto it = actorSceneVids_.find(vid);
                        if (it == actorSceneVids_.end())
                        {
                            backlog::post("entity VID : " + std::to_string(ett.getId()) + " (" + ncm.GetName(ett) + ") ==> not a scene component..", backlog::LogLevel::Error);
                        }
                        else
                        {
                            backlog::post("entity VID : " + std::to_string(ett.getId()) + " (" + ncm.GetName(ett) + ") is a hierarchy actor (kind of node)", backlog::LogLevel::Default);
                            it->second = 0;
                            ++retired_ett_count;
                        }
                    }
                }
                });
            gEngine->destroy(scene);    // maybe.. all views are set to nullptr scenes
            scenes_.erase(vidScene);

            for (auto& it_c : camSceneVids_)
            {
                if (it_c.second == vidScene)
                {
                    it_c.second = 0;
                    ++retired_ett_count;
                }
            }

            auto& ncm = VzNameCompManager::Get();
            utils::Entity ett = utils::Entity::import(vidScene);
            std::string name = ncm.GetName(ett);
            backlog::post("scene (" + name + ") has been removed, # associated components : " + std::to_string(retired_ett_count), 
                backlog::LogLevel::Default);

            auto it_srm = sceneResMaps_.find(vidScene);
            assert(it_srm != sceneResMaps_.end());
            //scene->remove(it_srm->second.GetLightmapCube()->getSolidRenderable());
            //scene->remove(it_srm->second.GetLightmapCube()->getWireFrameRenderable());
            sceneResMaps_.erase(it_srm); // calls destructor

            vzComponents_.erase(vidScene);
            return true;
        }

    public:
        // Runtime can create a new entity with this
        inline SceneVID CreateScene(const std::string& name)
        {
            auto& em = utils::EntityManager::get();
            utils::Entity ett = em.create();
            VID vid = ett.getId();
            scenes_[vid] = gEngine->createScene();
            sceneResMaps_[vid];

            auto it = vzComponents_.emplace(vid, std::make_unique<VzScene>());
            VzScene* v_scene = (VzScene*)it.first->second.get();
            v_scene->componentVID = vid;
            v_scene->originFrom = "CreateScene";
            v_scene->type = "VzScene";
            v_scene->timeStamp = std::chrono::high_resolution_clock::now();

            VzNameCompManager& ncm = VzNameCompManager::Get();
            ncm.CreateNameComp(ett, name);

            return vid;
        }
        inline RendererVID CreateRenderPath(const std::string& name)
        {
            // note renderpath involves a renderer
            auto& em = utils::EntityManager::get();
            utils::Entity ett = em.create();
            VID vid = ett.getId();
            VzRenderPath* renderPath = &renderPaths_[vid];

            auto it = vzComponents_.emplace(vid, std::make_unique<VzRenderer>());
            VzRenderer* v_renderer = (VzRenderer*)it.first->second.get();
            v_renderer->componentVID = vid;
            v_renderer->originFrom = "CreateRenderPath";
            v_renderer->type = "VzRenderer";
            v_renderer->timeStamp = std::chrono::high_resolution_clock::now();

            VzNameCompManager& ncm = VzNameCompManager::Get();
            ncm.CreateNameComp(ett, name);

            return vid;
        }
        inline size_t GetVidsByName(const std::string& name, std::vector<VID>& vids)
        {
            VzNameCompManager& ncm = VzNameCompManager::Get();
            std::vector<utils::Entity> etts = ncm.GetEntitiesByName(name);
            size_t num_etts = etts.size();
            if (num_etts == 0)
            {
                return 0u;
            }

            vids.clear();
            vids.reserve(num_etts);
            for (utils::Entity& ett : etts)
            {
                vids.push_back(ett.getId());
            }
            return num_etts;
        }
        inline VID GetFirstVidByName(const std::string& name)
        {
            VzNameCompManager& ncm = VzNameCompManager::Get();
            utils::Entity ett = ncm.GetFirstEntityByName(name);
            //if (ett.isNull())
            //{
            //    return INVALID_VID;
            //}
            return ett.getId();
        }
        inline std::string GetNameByVid(const VID vid)
        {
            VzNameCompManager& ncm = VzNameCompManager::Get();
            return ncm.GetName(utils::Entity::import(vid));
        }
        inline bool HasComponent(const VID vid)
        {
            VzNameCompManager& ncm = VzNameCompManager::Get();
            return ncm.hasComponent(utils::Entity::import(vid));
        }
        inline bool IsRenderable(const ActorVID vid)
        {
            bool ret = gEngine->getRenderableManager().hasComponent(utils::Entity::import(vid));
#ifdef _DEBUG
            auto it = actorSceneVids_.find(vid);
            assert(ret == (it != actorSceneVids_.end()));
#endif
            return ret;
        }
        inline bool IsSceneComponent(VID vid) // can be a node for a scene tree
        {
            return scenes_.contains(vid) || camSceneVids_.contains(vid) || lightSceneVids_.contains(vid) || actorSceneVids_.contains(vid);
        }
        inline bool IsLight(const LightVID vid)
        {
            bool ret = gEngine->getLightManager().hasComponent(utils::Entity::import(vid));
#ifdef _DEBUG
            auto it = lightSceneVids_.find(vid);
            assert(ret == (it != lightSceneVids_.end()));
#endif
            return ret;
        }
        inline Scene* GetScene(const SceneVID vid)
        {
            auto it = scenes_.find(vid);
            if (it == scenes_.end())
            {
                return nullptr;
            }
            return it->second;
        }
        inline Scene* GetFirstSceneByName(const std::string& name)
        {
            VzNameCompManager& ncm = VzNameCompManager::Get();
            std::vector<utils::Entity> etts = ncm.GetEntitiesByName(name);
            if (etts.size() == 0)
            {
                return nullptr;
            }

            for (utils::Entity& ett :etts)
            {
                SceneVID sid = ett.getId();
                auto it = scenes_.find(sid);
                if (it != scenes_.end())
                {
                    return it->second;
                }
            }
            return nullptr;
        }
        inline std::unordered_map<SceneVID, Scene*>* GetScenes()
        {
            return &scenes_;
        }

#define GET_RES_PTR(RES_MAP) auto it = RES_MAP.find(vid); if (it == RES_MAP.end()) return nullptr; return &it->second;
        inline VzSceneRes* GetSceneRes(const SceneVID vid)
        {
            GET_RES_PTR(sceneResMaps_);
        }
        inline VzRenderPath* GetRenderPath(const RendererVID vid)
        {
            GET_RES_PTR(renderPaths_);
        }
        inline VzCameraRes* GetCameraRes(const CamVID vid)
        {
            GET_RES_PTR(camResMaps_);
        }
        inline VzActorRes* GetActorRes(const ActorVID vid)
        {
            GET_RES_PTR(actorResMaps_);
        }
        inline VzLightRes* GetLightRes(const LightVID vid)
        {
            GET_RES_PTR(lightResMaps_);
        }

        inline size_t GetCameraVids(std::vector<CamVID>& camVids)
        {
            camVids.clear();
            camVids.reserve(camSceneVids_.size());
            for (auto& it : camSceneVids_)
            {
                camVids.push_back(it.first);
            }
            return camVids.size();
        }
        inline size_t GetRenderPathVids(std::vector<RendererVID>& renderPathVids)
        {
            renderPathVids.clear();
            renderPathVids.reserve(renderPaths_.size());
            for (auto& it : renderPaths_)
            {
                renderPathVids.push_back(it.first);
            }
            return renderPathVids.size();
        }
        inline VzRenderPath* GetFirstRenderPathByName(const std::string& name)
        {
            return GetRenderPath(GetFirstVidByName(name));
        }
        inline SceneVID GetSceneVidBelongTo(const VID vid)
        {
            auto itr = actorSceneVids_.find(vid);
            if (itr != actorSceneVids_.end())
            {
                return itr->second;
            }
            auto itl = lightSceneVids_.find(vid);
            if (itl != lightSceneVids_.end())
            {
                return itl->second;
            }
            auto itc = camSceneVids_.find(vid);
            if (itc != camSceneVids_.end())
            {
                return itc->second;
            }
            return INVALID_VID;
        }

        inline bool AppendSceneEntityToParent(const VID vidSrc, const VID vidDst)
        {
            assert(vidSrc != vidDst);
            auto& tcm = gEngine->getTransformManager();

            auto getSceneAndVid = [this](Scene** scene, const VID vid)
                {
                    SceneVID vid_scene = vid;
                    *scene = GetScene(vid_scene);
                    if (*scene == nullptr)
                    {
                        auto itr = actorSceneVids_.find(vid);
                        auto itl = lightSceneVids_.find(vid);
                        auto itc = camSceneVids_.find(vid);
                        if (itr == actorSceneVids_.end() 
                            && itl == lightSceneVids_.end()
                            && itc == camSceneVids_.end())
                        {
                            vid_scene = INVALID_VID;
                        }
                        else 
                        {
                            vid_scene = max(max(itl != lightSceneVids_.end()? itl->second : INVALID_VID,
                                itr != actorSceneVids_.end() ? itr->second : INVALID_VID),
                                itc != camSceneVids_.end() ? itc->second : INVALID_VID);
                            //assert(vid_scene != INVALID_VID); can be INVALID_VID
                            *scene = GetScene(vid_scene);
                        }
                    }
                    return vid_scene;
                };
            auto getDescendants = [&](auto& self, const utils::Entity ett, std::vector<utils::Entity>& decendants) -> void
                {
                    auto ins = tcm.getInstance(ett);
                    for (auto it = tcm.getChildrenBegin(ins); it != tcm.getChildrenEnd(ins); it++)
                    {
                        utils::Entity ett_child = tcm.getEntity(*it);
                        decendants.push_back(ett_child);
                        self(self, ett_child, decendants);
                    }
                };

            Scene* scene_src = nullptr;
            Scene* scene_dst = nullptr;
            SceneVID vid_scene_src = getSceneAndVid(&scene_src, vidSrc);
            SceneVID vid_scene_dst = getSceneAndVid(&scene_dst, vidDst);

            utils::Entity ett_src = utils::Entity::import(vidSrc);
            utils::Entity ett_dst = utils::Entity::import(vidDst);
            //auto& em = gEngine->getEntityManager();

            // case 1. both entities are actor
            // case 2. src is scene and dst is actor
            // case 3. src is actor and dst is scene
            // case 4. both entities are scenes
            // note that actor entity must have transform component!
            std::vector<utils::Entity> entities_moving;
            if (vidSrc != vid_scene_src && vidDst != vid_scene_dst)
            {
                // case 1. both entities are actor
                auto ins_src = tcm.getInstance(ett_src);
                auto ins_dst = tcm.getInstance(ett_dst);
                assert(ins_src.asValue() != 0 && ins_dst.asValue() != 0);

                tcm.setParent(ins_src, ins_dst);

                entities_moving.push_back(ett_src); 
                getDescendants(getDescendants, ett_src, entities_moving);
                //for (auto it = tcm.getChildrenBegin(ins_src); it != tcm.getChildrenEnd(ins_src); it++)
                //{
                //    utils::Entity ett = tcm.getEntity(*it);
                //    entities_moving.push_back(ett);
                //}
            }
            else if (vidSrc == vid_scene_src && vidDst != vid_scene_dst)
            {
                assert(scene_src != scene_dst && "scene cannot be appended to its component");

                // case 2. src is scene and dst is actor
                auto ins_dst = tcm.getInstance(ett_dst);
                assert(ins_dst.asValue() != 0 && "vidDst is invalid");
                scene_src->forEach([&](utils::Entity ett) {
                    entities_moving.push_back(ett);

                    auto ins = tcm.getInstance(ett);
                    utils::Entity ett_parent = tcm.getParent(ins);
                    if (ett_parent.isNull())
                    {
                        tcm.setParent(ins, ins_dst);
                    }
                    });
            }
            else if (vidSrc != vid_scene_src && vidDst == vid_scene_dst)
            {
                // case 3. src is actor and dst is scene
                // scene_src == scene_dst means that 
                //    vidSrc is appended to its root

                auto ins_src = tcm.getInstance(ett_src);
                assert(ins_src.asValue() != 0 && "vidSrc is invalid");

                entities_moving.push_back(ett_src);
                getDescendants(getDescendants, ett_src, entities_moving);
                //for (auto it = tcm.getChildrenBegin(ins_src); it != tcm.getChildrenEnd(ins_src); it++)
                //{
                //    utils::Entity ett = tcm.getEntity(*it);
                //    entities_moving.push_back(ett);
                //}
            }
            else 
            {
                assert(vidSrc == vid_scene_src && vidDst == vid_scene_dst);
                assert(scene_src != scene_dst);
                if (scene_src == nullptr)
                {
                    return false;
                }
                // case 4. both entities are scenes
                scene_src->forEach([&](utils::Entity ett) {
                    entities_moving.push_back(ett);
                    });

                removeScene(vid_scene_src);
                scene_src = nullptr;
            }

            // NOTE 
            // a scene can have entities with components that are not renderable
            // they are supposed to be ignored during the rendering pipeline

            for (auto& it : entities_moving)
            {
                auto itr = actorSceneVids_.find(it.getId());
                auto itl = lightSceneVids_.find(it.getId());
                auto itc = camSceneVids_.find(it.getId());
                if (itr != actorSceneVids_.end())
                    itr->second = 0;
                else if (itl != lightSceneVids_.end())
                    itl->second = 0;
                else if (itc != camSceneVids_.end())
                    itc->second = 0;
                if (scene_src)
                {
                    scene_src->remove(ett_src);
                }
            }

            if (scene_dst)
            {
                for (auto& it : entities_moving)
                {
                    // The entity is ignored if it doesn't have a Renderable or Light component.
                    scene_dst->addEntity(it);

                    auto itr = actorSceneVids_.find(it.getId());
                    auto itl = lightSceneVids_.find(it.getId());
                    auto itc = camSceneVids_.find(it.getId());
                    if (itr != actorSceneVids_.end())
                        itr->second = vid_scene_dst;
                    if (itl != lightSceneVids_.end())
                        itl->second = vid_scene_dst;
                    if (itc != camSceneVids_.end())
                        itc->second = vid_scene_dst;
                }
            }
            return true;
        }

        inline VzSceneComp* CreateSceneComponent(const SCENE_COMPONENT_TYPE compType, const std::string& name, const VID vidExist = 0)
        {
            if (compType == SCENE_COMPONENT_TYPE::SCENEBASE)
            {
                return nullptr;
            }

            VID vid = vidExist;
            auto& em = gEngine->getEntityManager();
            utils::Entity ett = utils::Entity::import(vid);
            bool is_alive = em.isAlive(ett);
            if (!is_alive)
            {
                ett = em.create();
                vid = ett.getId();
            }
            
            VzSceneComp* v_comp = nullptr;
            
            switch (compType)
            {
            case SCENE_COMPONENT_TYPE::ACTOR:
            {
                // RenderableManager::Builder... with entity registers the entity in the renderableEntities
                actorSceneVids_[vid] = 0; // first creation
                actorResMaps_[vid];

                auto it = vzComponents_.emplace(vid, std::make_unique<VzActor>());
                v_comp = (VzSceneComp*)it.first->second.get();
                v_comp->type = "VzActor";
                break;
            }
            case SCENE_COMPONENT_TYPE::LIGHT:
            {
                if (!is_alive)
                {
                    LightManager::Builder(LightManager::Type::SUN)
                        .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
                        .intensity(110000)
                        .direction({ 0.7, -1, -0.8 })
                        .sunAngularRadius(1.9f)
                        .castShadows(false)
                        .build(*gEngine, ett);
                }
                lightSceneVids_[vid] = 0; // first creation
                lightResMaps_[vid];

                auto it = vzComponents_.emplace(vid, std::make_unique<VzLight>());
                v_comp = (VzSceneComp*)it.first->second.get();
                v_comp->type = "VzLight";
                break;
            }
            case SCENE_COMPONENT_TYPE::CAMERA:
            {
                Camera* camera = nullptr;
                if (!is_alive)
                {
                    camera = gEngine->createCamera(ett);
                    camera->setExposure(16.0f, 1 / 125.0f, 100.0f); // default values used in filamentApp
                }
                else
                {
                    camera = gEngine->getCameraComponent(ett);
                }
                camSceneVids_[vid] = 0;
                VzCameraRes* cam_res = &camResMaps_[vid];
                cam_res->SetCamera(camera);

                auto it = vzComponents_.emplace(vid, std::make_unique<VzCamera>());
                v_comp = (VzSceneComp*)it.first->second.get();
                v_comp->type = "VzCamera";
                break;
            }
            default:
                assert(0);
            }
            v_comp->componentVID = vid;
            v_comp->compType = compType;
            v_comp->originFrom = "CreateSceneComponent";
            v_comp->timeStamp = std::chrono::high_resolution_clock::now();

            auto& ncm = VzNameCompManager::Get();
            auto& tcm = gEngine->getTransformManager();

            if (!ncm.getInstance(ett).isValid())
            {
                ncm.CreateNameComp(ett, name);
            }
            if (!tcm.getInstance(ett).isValid())
            {
                tcm.create(ett);
            }

            return v_comp;
        }
        inline VzActor* CreateTestActor(const std::string& modelName = "MONKEY_SUZANNE_DATA")
        {
            std::string geo_name = modelName + "_GEOMETRY";
            const std::string material_name = "_DEFAULT_STANDARD_MATERIAL";
            const std::string mi_name = modelName + "__MI";
            auto& ncm = VzNameCompManager::Get();

            MaterialInstance* mi = nullptr;
            MaterialInstanceVID vid_mi = INVALID_VID;
            for (auto& it_mi : materialInstances_)
            {
                utils::Entity ett = utils::Entity::import(it_mi.first);

                if (ncm.GetName(ett) == mi_name)
                {
                    mi = it_mi.second.mi;
                    vid_mi = it_mi.first;
                    break;
                }
            }
            if (mi == nullptr)
            {
                MaterialVID vid_m = GetFirstVidByName(material_name);
                VzMaterial* v_m = GetVzComponent<VzMaterial>(vid_m);
                assert(v_m != nullptr);
                Material* m = materials_[v_m->componentVID].material;
                mi = m->createInstance(mi_name.c_str());
                mi->setParameter("baseColor", RgbType::LINEAR, float3{ 0.8, 0.1, 0.1 });
                mi->setParameter("metallic", 1.0f);
                mi->setParameter("roughness", 0.4f);
                mi->setParameter("reflectance", 0.5f);
                VzMI* v_mi = CreateMaterialInstance(mi_name, mi);
                auto it_mi = materialInstances_.find(v_mi->componentVID);
                assert(it_mi != materialInstances_.end());
                assert(mi == it_mi->second.mi);
                vid_mi = v_mi->componentVID;
            }
            assert(vid_mi != INVALID_VID);

            MeshReader::Mesh mesh = MeshReader::loadMeshFromBuffer(gEngine, MONKEY_SUZANNE_DATA, nullptr, nullptr, mi);
            ncm.CreateNameComp(mesh.renderable, modelName);
            VID vid = mesh.renderable.getId();
            actorSceneVids_[vid] = 0;

            auto& rcm = gEngine->getRenderableManager();
            auto ins = rcm.getInstance(mesh.renderable);
            Box box = rcm.getAxisAlignedBoundingBox(ins);

            VzGeometry* geo = CreateGeometry(geo_name, {{
                .vertices = mesh.vertexBuffer,
                .indices = mesh.indexBuffer,
                .aabb = Aabb(box.getMin(), box.getMax())
                }});
            VzActorRes& actor_res = actorResMaps_[vid];
            actor_res.SetGeometry(geo->componentVID);
            actor_res.SetMIs({ vid_mi });

            auto it = vzComponents_.emplace(vid, std::make_unique<VzActor>());
            VzActor* v_actor = (VzActor*)it.first->second.get();
            v_actor->componentVID = vid;
            v_actor->originFrom = "CreateTestActor";
            v_actor->type = "VzActor";
            v_actor->compType = SCENE_COMPONENT_TYPE::ACTOR;
            v_actor->timeStamp = std::chrono::high_resolution_clock::now();
            return v_actor;
        }
        inline VzGeometry* CreateGeometry(const std::string& name, 
            const std::vector<filament::gltfio::Primitive>& primitives, 
            const filament::gltfio::FilamentAsset* assetOwner = nullptr,
            const bool isSystem = false)
        {
            auto& em = utils::EntityManager::get();
            auto& ncm = VzNameCompManager::Get();

            utils::Entity ett = em.create();
            ncm.CreateNameComp(ett, name);

            VID vid = ett.getId();
            
            VzGeometryRes& geo_res = geometries_[vid];
            geo_res.Set(primitives);
            geo_res.assetOwner = (filament::gltfio::FilamentAsset*)assetOwner;
            geo_res.isSystem = isSystem;
            for (auto& prim : primitives)
            {
                geo_res.aabb.min = min(prim.aabb.min, geo_res.aabb.min);
                geo_res.aabb.max = max(prim.aabb.max, geo_res.aabb.max);
            }

            auto it = vzComponents_.emplace(vid, std::make_unique<VzGeometry>());
            VzGeometry* v_m = (VzGeometry*)it.first->second.get();
            v_m->componentVID = vid;
            v_m->compType = RES_COMPONENT_TYPE::GEOMATRY;
            v_m->originFrom = "CreateGeometry";
            v_m->type = "VzGeometry";
            v_m->timeStamp = std::chrono::high_resolution_clock::now();

            return v_m;
        }
        inline VzMaterial* CreateMaterial(const std::string& name, 
            const Material* material = nullptr,
            const filament::gltfio::FilamentAsset* assetOwner = nullptr,
            const bool isSystem = false)
        {
            auto& em = utils::EntityManager::get();
            auto& ncm = VzNameCompManager::Get();

            if (material != nullptr)
            {
                for (auto& it : materials_)
                {
                    if (it.second.material == material)
                    {
                        backlog::post("The material has already been registered!", backlog::LogLevel::Warning);
                    }
                }
            }

            utils::Entity ett = em.create();
            ncm.CreateNameComp(ett, name);

            VID vid = ett.getId();
            VzMaterialRes& m_res = materials_[vid];
            m_res.material = (Material*)material;
            m_res.assetOwner = (filament::gltfio::FilamentAsset*)assetOwner;
            m_res.isSystem = isSystem;

            auto it = vzComponents_.emplace(vid, std::make_unique<VzMaterial>());
            VzMaterial* v_m = (VzMaterial*)it.first->second.get();
            v_m->componentVID = vid;
            v_m->originFrom = "CreateMaterial";
            v_m->type = "VzMaterial";
            v_m->compType = RES_COMPONENT_TYPE::MATERIAL;
            v_m->timeStamp = std::chrono::high_resolution_clock::now();

            return v_m;
        }
        inline VzMI* CreateMaterialInstance(const std::string& name,
            const MaterialInstance* mi = nullptr,
            const filament::gltfio::FilamentAsset* assetOwner = nullptr,
            const bool isSystem = false)
        {
            auto& em = utils::EntityManager::get();
            auto& ncm = VzNameCompManager::Get();

            if (mi != nullptr)
            {
                for (auto& it : materialInstances_)
                {
                    if (it.second.mi == mi)
                    {
                        backlog::post("The material instance has already been registered!", backlog::LogLevel::Warning);
                    }
                }
            }

            utils::Entity ett = em.create();
            ncm.CreateNameComp(ett, name);

            VID vid = ett.getId();
            VzMIRes& mi_res = materialInstances_[vid];
            mi_res.mi = (MaterialInstance*)mi;
            mi_res.assetOwner = (filament::gltfio::FilamentAsset*)assetOwner;
            mi_res.isSystem = isSystem;

            auto it = vzComponents_.emplace(vid, std::make_unique<VzMI>());
            VzMI* v_m = (VzMI*)it.first->second.get();
            v_m->componentVID = vid;
            v_m->originFrom = "CreateMaterialInstance";
            v_m->type = "VzMI";
            v_m->compType = RES_COMPONENT_TYPE::MATERIALINSTANCE;
            v_m->timeStamp = std::chrono::high_resolution_clock::now();

            return v_m;
        }

        inline void BuildRenderable(const ActorVID vid)
        {
            VzActorRes* actor_res = GetActorRes(vid);
            if (actor_res == nullptr)
            {
                backlog::post("invalid ActorVID", backlog::LogLevel::Error);
                return;
            }
            VzGeometryRes* geo_res = GetGeometryRes(actor_res->GetGeometryVid());
            if (geo_res == nullptr)
            {
                backlog::post("invalid GetGeometryVid", backlog::LogLevel::Error);
                return;
            }

            auto& rcm = gEngine->getRenderableManager();
            utils::Entity ett_actor = utils::Entity::import(vid);
            auto ins = rcm.getInstance(ett_actor);
            assert(ins.isValid());

            std::vector<Primitive>& primitives = *geo_res->Get();
            std::vector<RenderableManager::PrimitiveType>& primitive_types = *geo_res->GetTypes();
            std::vector<MaterialInstance*> mis;
            for (auto& it_mi : actor_res->GetMIVids())
            {
                mis.push_back(GetMIRes(it_mi)->mi);
            }

            RenderableManager::Builder builder(primitives.size());

            for (size_t index = 0, n = primitives.size(); index < n; ++index)
            {
                Primitive* primitive = &primitives[index];
                MaterialInstance* mi = mis.size() > index ? mis[index] : nullptr;
                RenderableManager::PrimitiveType prim_type = primitive_types.size() > index ? 
                    primitive_types[index] : RenderableManager::PrimitiveType::TRIANGLES;
                builder.material(index, mi);
                builder.geometry(index, prim_type, primitive->vertices, primitive->indices);
                if (primitive->morphTargetBuffer)
                {
                    builder.morphing(primitive->morphTargetBuffer);
                }
            }

            std::string name = VzNameCompManager::Get().GetName(ett_actor);
            Box box = Box().set(geo_res->aabb.min, geo_res->aabb.max);
            if (box.isEmpty()) {
                backlog::post("Missing bounding box in " + name, backlog::LogLevel::Warning);
                box = Box().set(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
            }

            builder
                .boundingBox(box)
                .culling(true)
                .castShadows(true)
                .receiveShadows(true)
                .build(*gEngine, ett_actor);
        }

        inline VzGeometryRes* GetGeometryRes(const GeometryVID vidGeo)
        {
            auto it = geometries_.find(vidGeo);
            if (it == geometries_.end())
            {
                return nullptr;
            }
            return &it->second;
        }
        inline VzMaterialRes* GetMaterialRes(const MaterialVID vidMaterial)
        {
            auto it = materials_.find(vidMaterial);
            if (it == materials_.end())
            {
                return nullptr;
            }
            return &it->second;
        }
        inline MaterialVID FindMaterialVID(const filament::Material* mat)
        {
            for (auto& it : materials_)
            {
                if (it.second.material == mat)
                {
                    return it.first;
                }
            }
            return INVALID_VID;
        }

        inline VzMIRes* GetMIRes(const MaterialInstanceVID vidMI)
        {
            auto it = materialInstances_.find(vidMI);
            if (it == materialInstances_.end())
            {
                return nullptr;
            }
            return &it->second;
        }
        inline MaterialInstanceVID FindMaterialInstanceVID(const filament::MaterialInstance* mi)
        {
            for (auto& it : materialInstances_)
            {
                if (it.second.mi == mi)
                {
                    return it.first;
                }
            }
            return INVALID_VID;
        }

        template <typename VZCOMP>
        inline VZCOMP* GetVzComponent(const VID vid)
        {
            auto it = vzComponents_.find(vid);
            if (it == vzComponents_.end())
            {
                return nullptr;
            }
            return (VZCOMP*)it->second.get();
        }

        inline bool RemoveComponent(const VID vid)
        {
            utils::Entity ett = utils::Entity::import(vid);

            VzBaseComp* b = GetVzComponent<VzBaseComp>(vid);
            if (b == nullptr)
            {
                return false;
            }

            auto& em = utils::EntityManager::get();
            auto& ncm = VzNameCompManager::Get();
            std::string name = ncm.GetName(ett);
            if (!removeScene(vid))
            {
                // this calls built-in destroy functions in the filament entity managers

                // destroy by engine (refer to the following)
                // void FEngine::destroy(Entity e) {
                //     mRenderableManager.destroy(e);
                //     mLightManager.destroy(e);
                //     mTransformManager.destroy(e);
                //     mCameraManager.destroy(*this, e);
                // }
#pragma region destroy by engine
                bool isRenderableResource = false; // geometry, material, or material instance
                // note filament engine handles the renderable objects (with the modified resource settings)
                auto it_m = materials_.find(vid);
                if (it_m != materials_.end())
                {
                    VzMaterialRes& m_res = it_m->second;
                    if (m_res.isSystem)
                    {
                        backlog::post("Material (" + name + ") is system-owned component, thereby preserved.", backlog::LogLevel::Warning);
                        return false;
                    }
                    else if (m_res.assetOwner)
                    {
                        auto it_asset = vzGltfIO.vzCompAssociatedAssets.find(it_m->first);
                        assert(it_asset != vzGltfIO.vzCompAssociatedAssets.end());
                        backlog::post("Material (" + name + ") is asset(" + std::to_string(it_asset->second) + ")-owned component, thereby preserved.", backlog::LogLevel::Warning);
                        return false;
                    }
                    else
                    {
                        for (auto it = materialInstances_.begin(); it != materialInstances_.end();)
                        {
                            if (it->second.mi->getMaterial() == m_res.material)
                            {
                                utils::Entity ett_mi = utils::Entity::import(it->first);
                                std::string name_mi = ncm.GetName(ett_mi);
                                if (it->second.isSystem)
                                {
                                    backlog::post("(" + name + ")-associated-MI (" + name_mi + ") is system-owned component, thereby preserved.", backlog::LogLevel::Warning);
                                }
                                else if (it->second.assetOwner != nullptr)
                                {
                                    auto it_asset = vzGltfIO.vzCompAssociatedAssets.find(it->first);
                                    assert(it_asset != vzGltfIO.vzCompAssociatedAssets.end());
                                    
                                    backlog::post("(" + name + ")-associated-MI (" + name_mi + ") is asset(" + std::to_string(it_asset->second) + ")- owned component, thereby preserved.", backlog::LogLevel::Warning);
                                }
                                else
                                {

                                    isRenderableResource = true;
                                    ncm.RemoveEntity(ett_mi); // explicitly 
                                    em.destroy(ett_mi); // double check
                                    it = materialInstances_.erase(it); // call destructor...
                                    backlog::post("(" + name + ")-associated-MI (" + name_mi + ") has been removed", backlog::LogLevel::Default);
                                }

                            }
                            else
                            {
                                ++it;
                            }
                        }
                        materials_.erase(it_m); // call destructor...
                        backlog::post("Material (" + name + ") has been removed", backlog::LogLevel::Default);
                    }
                    // caution: 
                    // before destroying the material,
                    // destroy the associated material instances
                }
                auto it_mi = materialInstances_.find(vid);
                if (it_mi != materialInstances_.end())
                {
                    VzMIRes& mi_res = it_mi->second;
                    if (mi_res.isSystem)
                    {
                        backlog::post("MI (" + name + ") is system-owned component, thereby preserved.", backlog::LogLevel::Warning);
                        return false;
                    }
                    else if (mi_res.assetOwner)
                    {
                        auto it_asset = vzGltfIO.vzCompAssociatedAssets.find(it_mi->first);
                        assert(it_asset != vzGltfIO.vzCompAssociatedAssets.end());
                        backlog::post("MI (" + name + ") is asset(" + std::to_string(it_asset->second) + ")-owned component, thereby preserved.", backlog::LogLevel::Warning);
                        return false;
                    }
                    else
                    {
                        materialInstances_.erase(it_mi); // call destructor...
                        isRenderableResource = true;
                        backlog::post("MI (" + name + ") has been removed", backlog::LogLevel::Default);
                    }
                }
                auto it_geo = geometries_.find(vid);
                if (it_geo != geometries_.end())
                {
                    VzGeometryRes& geo_res = it_geo->second;
                    if (geo_res.isSystem)
                    {
                        backlog::post("Geometry (" + name + ") is system-owned component, thereby preserved.", backlog::LogLevel::Warning);
                        return false;
                    }
                    else if (geo_res.assetOwner)
                    {
                        auto it_asset = vzGltfIO.vzCompAssociatedAssets.find(it_geo->first);
                        assert(it_asset != vzGltfIO.vzCompAssociatedAssets.end());
                        backlog::post("Geometry (" + name + ") is asset(" + std::to_string(it_asset->second) + ")-owned component, thereby preserved.", backlog::LogLevel::Warning);
                        return false;
                    }
                    else
                    {
                        geometries_.erase(it_geo); // call destructor...
                        isRenderableResource = true;
                        backlog::post("Geometry (" + name + ") has been removed", backlog::LogLevel::Default);
                    }
                }

                if (isRenderableResource)
                {
                    for (auto& it_res : actorResMaps_)
                    {
                        VzActorRes& actor_res = it_res.second;
                        if (geometries_.find(actor_res.GetGeometryVid()) == geometries_.end())
                        {
                            actor_res.SetGeometry(INVALID_VID);
                        }

                        std::vector<MaterialInstanceVID> mis = actor_res.GetMIVids();
                        for (int i = 0, n = (int)mis.size(); i < n; ++i)
                        {
                            if (materialInstances_.find(mis[i]) == materialInstances_.end())
                            {
                                actor_res.SetMI(INVALID_VID, i);
                            }
                        }
                    }
                }

                auto it_camres = camResMaps_.find(vid);
                if (it_camres != camResMaps_.end())
                {
                    VzCameraRes& cam_res = it_camres->second;
                    Cube* cam_cube = cam_res.GetCameraCube();
                    if (cam_cube)
                    {
                        SceneVID vid_scene = GetSceneVidBelongTo(vid);
                        Scene* scene = GetScene(vid_scene);
                        if (scene)
                        {
                            scene->remove(cam_cube->getSolidRenderable());
                            scene->remove(cam_cube->getWireFrameRenderable());
                        }
                    }
                    camResMaps_.erase(it_camres); // call destructor
                }

                auto it_light = lightResMaps_.find(vid);
                if (it_light != lightResMaps_.end())
                {
                    lightResMaps_.erase(it_light); // call destructor
                }
#pragma endregion 
                // the remaining etts (not engine-destory group)

                vzComponents_.erase(vid);

                actorSceneVids_.erase(vid);
                actorResMaps_.erase(vid);
                lightSceneVids_.erase(vid);
                lightResMaps_.erase(vid);
                camSceneVids_.erase(vid);
                camResMaps_.erase(vid);
                renderPaths_.erase(vid);

                for (auto& it : scenes_)
                {
                    Scene* scene = it.second;
                    scene->remove(ett);
                }
            }

            em.destroy(ett); // the associated engine components having the entity will be removed 
            gEngine->destroy(ett);

            return true;
        }

        template <typename UM> void destroyTarget(UM& umap)
        {
            std::vector<VID> vids;
            vids.reserve(umap.size());
            for (auto it = umap.begin(); it != umap.end(); it++)
            {
                vids.push_back(it->first);
            }
            for (VID vid : vids)
            {
                RemoveComponent(vid);
            }
        }

        inline void Destroy()
        {
            //std::unordered_map<SceneVID, filament::Scene*> scenes_;
            //// note a VzRenderPath involves a view that includes
            //// 1. camera and 2. scene
            //std::unordered_map<CamVID, VzRenderPath> renderPaths_;
            //std::unordered_map<ActorVID, SceneVID> actorSceneVids_;
            //std::unordered_map<LightVID, SceneVID> lightSceneVids_;
            //
            //// Resources
            //std::unordered_map<GeometryVID, filamesh::MeshReader::Mesh> geometries_;
            //std::unordered_map<MaterialVID, filament::Material*> materials_;
            //std::unordered_map<MaterialInstanceVID, filament::MaterialInstance*> materialInstances_;
            
            // iteratively calling RemoveEntity of the following keys 
            destroyTarget(camSceneVids_);
            destroyTarget(actorSceneVids_); // including actorResMaps_
            destroyTarget(lightSceneVids_); // including lightResMaps_
            destroyTarget(renderPaths_);
            destroyTarget(scenes_);
            destroyTarget(geometries_);
            destroyTarget(materialInstances_);
            destroyTarget(materials_);
        }
    };

    VzEngineApp gEngineApp;
#pragma endregion
}

#define COMP_NAME(COMP, ENTITY, FAILRET) auto& COMP = VzNameCompManager::Get(); Entity ENTITY = Entity::import(componentVID); if (ENTITY.isNull()) return FAILRET;
#define COMP_TRANSFORM(COMP, ENTITY, INS, FAILRET)  auto & COMP = gEngine->getTransformManager(); Entity ENTITY = Entity::import(componentVID); if (ENTITY.isNull()) return FAILRET; auto INS = COMP.getInstance(ENTITY);
#define COMP_RENDERPATH(RENDERPATH, FAILRET)  VzRenderPath* RENDERPATH = gEngineApp.GetRenderPath(componentVID); if (RENDERPATH == nullptr) return FAILRET;
#define COMP_LIGHT(COMP, ENTITY, INS, FAILRET)  auto & COMP = gEngine->getLightManager(); Entity ENTITY = Entity::import(componentVID); if (ENTITY.isNull()) return FAILRET; auto INS = COMP.getInstance(ENTITY);
#define COMP_ACTOR(COMP, ENTITY, INS, FAILRET)  auto & COMP = gEngine->getRenderableManager(); Entity ENTITY = Entity::import(componentVID); if (ENTITY.isNull()) return FAILRET; auto INS = COMP.getInstance(ENTITY);
#define COMP_MI(COMP, FAILRET) VzMIRes* mi_res = gEngineApp.GetMIRes(componentVID); if (mi_res == nullptr) return FAILRET; MaterialInstance* COMP = mi_res->mi; if (COMP == nullptr) return FAILRET;
#define COMP_CAMERA(COMP, ENTITY, FAILRET) Entity ENTITY = Entity::import(componentVID); Camera* COMP = gEngine->getCameraComponent(ENTITY); if (COMP == nullptr) return;

namespace vzm
{
    auto cubeToScene = [](const utils::Entity ettCubeRenderable, const VID vidCube)
        {
            SceneVID vid_scene = gEngineApp.GetSceneVidBelongTo(vidCube);
            if (vid_scene != INVALID_VID)
            {
                Scene* scene = gEngineApp.GetScene(vid_scene);
                assert(scene);
                if (!scene->hasEntity(ettCubeRenderable))
                {
                    // safely rearrangement
                    auto& scenes = *gEngineApp.GetScenes();
                    for (auto& it : scenes)
                    {
                        it.second->remove(ettCubeRenderable);
                    }
                    scene->addEntity(ettCubeRenderable);
                }
            }
        };

#pragma region // VzBaseComp
    using namespace utils;
    std::string VzBaseComp::GetName()
    {
        COMP_NAME(ncm, ett, "");
        return ncm.GetName(ett);
    }
    void VzBaseComp::SetName(const std::string& name)
    {
        COMP_NAME(ncm, ett, );
        ncm.SetName(ett, name);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
#pragma endregion

#pragma region // VzScene
    bool VzScene::LoadIBL(const std::string& path)
    {
        VzSceneRes* scene_res = gEngineApp.GetSceneRes(componentVID);
        IBL* ibl = scene_res->NewIBL();
        Path iblPath(path);
        if (!iblPath.exists()) {
            backlog::post("The specified IBL path does not exist: " + path, backlog::LogLevel::Error);
            return false;
        }
        if (!iblPath.isDirectory()) {
            if (!ibl->loadFromEquirect(iblPath)) {
                backlog::post("Could not load the specified IBL: " + path, backlog::LogLevel::Error);
                return false;
            }
        }
        else {
            if (!ibl->loadFromDirectory(iblPath)) {
                backlog::post("Could not load the specified IBL: " + path, backlog::LogLevel::Error);
                return false;
            }
        }
        ibl->getSkybox()->setLayerMask(0x7, 0x4);
        Scene* scene = gEngineApp.GetScene(componentVID);
        assert(scene);
        scene->setSkybox(ibl->getSkybox());
        scene->setIndirectLight(ibl->getIndirectLight());
        timeStamp = std::chrono::high_resolution_clock::now();
        return true;
    }
    void VzScene::SetSkyboxVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        VzSceneRes* scene_res = gEngineApp.GetSceneRes(componentVID);
        IBL* ibl = scene_res->GetIBL();
        ibl->getSkybox()->setLayerMask(layerBits, maskBits);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzScene::SetLightmapVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        VzSceneRes* scene_res = gEngineApp.GetSceneRes(componentVID);
        // create once
        Cube* light_cube = scene_res->GetLightmapCube();

        auto& rcm = gEngine->getRenderableManager();
        rcm.setLayerMask(rcm.getInstance(light_cube->getSolidRenderable()), layerBits, maskBits);
        rcm.setLayerMask(rcm.getInstance(light_cube->getWireFrameRenderable()), layerBits, maskBits);

        cubeToScene(light_cube->getSolidRenderable(), componentVID);
        cubeToScene(light_cube->getWireFrameRenderable(), componentVID);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
#pragma endregion

#pragma region // VzSceneComp
    void VzSceneComp::GetWorldPosition(float v[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& mat = tc.getWorldTransform(ins);
        *(float3*)v = mat[3].xyz;
    }
    void VzSceneComp::GetWorldForward(float v[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& mat = tc.getWorldTransform(ins);
        *(float3*)v = mat[2].xyz; // view
    }
    void VzSceneComp::GetWorldRight(float v[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& mat = tc.getWorldTransform(ins);
        *(float3*)v = mat[0].xyz;
    }
    void VzSceneComp::GetWorldUp(float v[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& mat = tc.getWorldTransform(ins);
        *(float3*)v = mat[1].xyz;
    }
    void VzSceneComp::GetWorldTransform(float mat[16], const bool rowMajor)
    {
        // note that
        // filament math stores the column major matrix
        // logically it also uses column major matrix computation
        // c.f., glm:: uses column major matrix computation but it stores a matrix according to column major convention
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& _mat = tc.getWorldTransform(ins);
        *(mat4f*)mat = _mat;
    }
    void VzSceneComp::GetLocalTransform(float mat[16], const bool rowMajor)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& _mat = tc.getTransform(ins);
        *(mat4f*)mat = _mat;
    }
    void VzSceneComp::GetWorldInvTransform(float mat[16], const bool rowMajor)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& _mat = tc.getWorldTransform(ins);
        *(mat4f*)mat = inverse(_mat);
    }
    void VzSceneComp::GetLocalInvTransform(float mat[16], const bool rowMajor)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        const math::mat4f& _mat = tc.getTransform(ins);
        *(mat4f*)mat = inverse(_mat);
    }
    void VzSceneComp::SetTransform(const float s[3], const float q[4], const float t[3], const bool additiveTransform)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        mat4f mat_s = mat4f(), mat_r = mat4f(), mat_t = mat4f(); //c.f. mat4f(no_init)
        if (s)
        {
            mat_s = math::mat4f::scaling(float3(s[0], s[1], s[2]));
        }
        if (t)
        {
            mat_t = math::mat4f::translation(float3(t[0], t[1], t[2]));
        }
        if (q)
        {
            mat_r = math::mat4f(*(math::quatf*)q);
        }
        mat4f mat = mat_t * mat_r * mat_s;
        tc.setTransform(ins, additiveTransform? mat * tc.getTransform(ins) : mat);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzSceneComp::SetMatrix(const float value[16], const bool additiveTransform, const bool rowMajor)
    {
        COMP_TRANSFORM(tc, ett, ins, );
        mat4f mat = rowMajor ? transpose(*(mat4f*)value) : *(mat4f*)value;
        tc.setTransform(ins, additiveTransform ? mat * tc.getTransform(ins) : mat);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    VID VzSceneComp::GetParentVid()
    {
        COMP_TRANSFORM(tc, ett, ins, INVALID_VID);
        Entity ett_parent = tc.getParent(ins);
        return ett_parent.getId();
    }
    VID VzSceneComp::GetSceneVid()
    {
        return gEngineApp.GetSceneVidBelongTo(componentVID);
    }
#pragma endregion 

#pragma region // VzCamera
    // Pose parameters are defined in WS (not local space)
    void VzCamera::SetWorldPose(const float pos[3], const float view[3], const float up[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );

        // up vector correction
        double3 _eye = *(float3*)pos;
        double3 _view = normalize((double3)*(float3*)view);
        double3 _up = *(float3*)up;
        double3 _right = cross(_view, _up);
        _up = normalize(cross(_right, _view));

        // note the pose info is defined in WS
        //mat4f ws2cs = mat4f::lookTo(_view, _eye, _up);
        //mat4f cs2ws = inverse(ws2cs);
        Camera* camera = gEngine->getCameraComponent(ett);
        camera->lookAt(_eye, _eye + _view, _up);
        mat4 ws2cs_d = camera->getViewMatrix();
        mat4 cs2ws_d = inverse(ws2cs_d);

        Entity ett_parent = tc.getParent(ins);
        mat4 parent2ws_d = mat4();
        while (!ett_parent.isNull())
        {
            auto ins_parent = tc.getInstance(ett_parent);
            parent2ws_d = mat4(tc.getTransform(ins_parent)) * parent2ws_d;
            ett_parent = tc.getParent(ins_parent);
        }

        mat4f local = mat4f(inverse(parent2ws_d) * cs2ws_d);
        SetMatrix((float*)&local[0][0], false, false);
    }
    void VzCamera::SetPerspectiveProjection(const float zNearP, const float zFarP, const float fovInDegree, const float aspectRatio, const bool isVertical)
    {
        COMP_CAMERA(camera, ett, );
        // aspectRatio is W / H
        camera->setProjection(fovInDegree, aspectRatio, zNearP, zFarP, 
            isVertical? Camera::Fov::VERTICAL : Camera::Fov::HORIZONTAL);
        timeStamp = std::chrono::high_resolution_clock::now();
    }

    void VzCamera::SetCameraCubeVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        VzCameraRes* cam_res = gEngineApp.GetCameraRes(componentVID);
        if (cam_res == nullptr) return;

        Cube* camera_cube = cam_res->GetCameraCube();
        auto& rcm = gEngine->getRenderableManager();
        rcm.setLayerMask(rcm.getInstance(camera_cube->getSolidRenderable()), layerBits, maskBits);
        rcm.setLayerMask(rcm.getInstance(camera_cube->getWireFrameRenderable()), layerBits, maskBits);

        cubeToScene(camera_cube->getSolidRenderable(), componentVID);
        cubeToScene(camera_cube->getWireFrameRenderable(), componentVID);
        timeStamp = std::chrono::high_resolution_clock::now();
    }

    void VzCamera::GetWorldPose(float pos[3], float view[3], float up[3])
    {
        COMP_CAMERA(camera, ett, );
        double3 p = camera->getPosition();
        double3 v = camera->getForwardVector();
        double3 u = camera->getUpVector();
        if (pos) *(float3*)pos = float3(p);
        if (view) *(float3*)view = float3(v);
        if (up) *(float3*)up = float3(u);
    }
    void VzCamera::GetPerspectiveProjection(float* zNearP, float* zFarP, float* fovInDegree, float* aspectRatio, bool isVertical)
    {
        COMP_CAMERA(camera, ett, );
        if (zNearP) *zNearP = (float)camera->getNear();
        if (zFarP) *zFarP = (float)camera->getCullingFar();
        if (fovInDegree) *fovInDegree = (float)camera->getFieldOfViewInDegrees(isVertical ? Camera::Fov::VERTICAL : Camera::Fov::HORIZONTAL);
        if (aspectRatio)
        {
            mat4 mat_proj = camera->getProjectionMatrix();
            *aspectRatio = (float)(mat_proj[1][1] / mat_proj[0][0]);
        }
    }

    VzCamera::Controller* VzCamera::GetController()
    {
        VzCameraRes* cam_res = gEngineApp.GetCameraRes(componentVID);
        if (cam_res == nullptr) return nullptr;
        CameraManipulator* cm = cam_res->GetCameraManipulator();
        Controller* cc = cam_res->GetCameraController();
        if (cm == nullptr)
        {
            cam_res->NewCameraManipulator(*cam_res->GetCameraController());
            cc = cam_res->GetCameraController();
        }
        return cc;
    }
#define GET_CM(CAMRES, CM) VzCameraRes* CAMRES = gEngineApp.GetCameraRes(vidCam); if (CAMRES == nullptr) return;  CameraManipulator* CM = CAMRES->GetCameraManipulator();
#define GET_CM_WARN(CAMRES, CM) GET_CM(CAMRES, CM) if (CM == nullptr) { backlog::post("camera manipulator is not set!", backlog::LogLevel::Warning); return; }
    void VzCamera::Controller::UpdateControllerSettings()
    {
        GET_CM(cam_res, cm);
        cam_res->NewCameraManipulator(*this);
    }
    void VzCamera::Controller::KeyDown(const Key key)
    {
        GET_CM_WARN(cam_res, cm);
        cm->keyDown((CameraManipulator::Key)key);
    }
    void VzCamera::Controller::KeyUp(const Key key)
    {
        GET_CM_WARN(cam_res, cm);
        cm->keyUp((CameraManipulator::Key)key);
    }
    void VzCamera::Controller::Scroll(const int x, const int y, const float scrollDelta)
    {
        GET_CM_WARN(cam_res, cm);
        cm->scroll(x, y, scrollDelta);
    }
    void VzCamera::Controller::GrabBegin(const int x, const int y, const bool strafe)
    {
        GET_CM_WARN(cam_res, cm);
        cm->grabBegin(x, y, strafe);
    }
    void VzCamera::Controller::GrabDrag(const int x, const int y)
    {
        GET_CM_WARN(cam_res, cm);
        cm->grabUpdate(x, y);
    }
    void VzCamera::Controller::GrabEnd()
    {
        GET_CM_WARN(cam_res, cm);
        cm->grabEnd();
    }
    void VzCamera::Controller::SetViewport(const int w, const int h)
    {
        GET_CM_WARN(cam_res, cm);
        cm->setViewport(w, h);
    }
    void VzCamera::Controller::UpdateCamera(const float deltaTime)
    {
        GET_CM_WARN(cam_res, cm);
        cam_res->UpdateCameraWithCM(deltaTime);
    }
#pragma endregion

#pragma region // VzLight
    void VzLight::SetIntensity(const float intensity)
    {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setIntensity(ins, intensity);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    float VzLight::GetIntensity() const
    {
        COMP_LIGHT(lcm, ett, ins, -1.f);
        return lcm.getIntensity(ins);
    }
#pragma endregion 

#pragma region // VzActor
    void VzActor::SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        COMP_ACTOR(rcm, ett, ins, );
        rcm.setLayerMask(ins, layerBits, maskBits);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzActor::SetMI(const VID vidMI, const int slot)
    {
        VzMIRes* mi_res = gEngineApp.GetMIRes(vidMI);
        if (mi_res == nullptr)
        {
            backlog::post("invalid material instance!", backlog::LogLevel::Error);
            return;
        }
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        if (!actor_res->SetMI(vidMI, slot))
        {
            return;
        }
        auto& rcm = gEngine->getRenderableManager();
        utils::Entity ett_actor = utils::Entity::import(componentVID);
        auto ins = rcm.getInstance(ett_actor);
        rcm.setMaterialInstanceAt(ins, slot, mi_res->mi);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzActor::SetRenderableRes(const VID vidGeo, const std::vector<VID>& vidMIs)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        actor_res->SetGeometry(vidGeo);
        actor_res->SetMIs(vidMIs);
        gEngineApp.BuildRenderable(componentVID);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    std::vector<VID> VzActor::GetMIs()
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        return actor_res->GetMIVids();
    }
    VID VzActor::GetMI(const int slot)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        return actor_res->GetMIVid(slot);
    }
    VID VzActor::GetMaterial(const int slot)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        MaterialInstanceVID vid_mi = actor_res->GetMIVid(slot);
        VzMIRes* mi_res = gEngineApp.GetMIRes(vid_mi);
        if (mi_res == nullptr)
        {
            return INVALID_VID;
        }
        MaterialInstance* mi = mi_res->mi;
        assert(mi);
        const Material* mat = mi->getMaterial();
        assert(mat != nullptr);
        return gEngineApp.FindMaterialVID(mat);
    }
    VID VzActor::GetGeometry()
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        return actor_res->GetGeometryVid();
    }
#pragma endregion

#pragma region // VzMI
    void VzMI::SetTransparencyMode(const TransparencyMode tMode)
    {
        COMP_MI(mi, );
        mi->setTransparencyMode((filament::TransparencyMode)tMode);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzMI::SetMaterialProperty(const MProp mProp, const std::vector<float>& v)
    {
        COMP_MI(mi, );
        if (mProp == MProp::BASE_COLOR)
        {
            mi->setParameter(gMProp[(uint32_t)mProp].c_str(), (filament::RgbaType)rgbType, *(filament::math::float4*)&v[0]);
        }
        timeStamp = std::chrono::high_resolution_clock::now();
    }
#pragma endregion

#pragma region // VzRenderer
    void VzRenderer::SetCanvas(const uint32_t w, const uint32_t h, const float dpi, void* window)
    {
        COMP_RENDERPATH(render_path, );
        render_path->SetCanvas(w, h, dpi, window);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzRenderer::GetCanvas(uint32_t* w, uint32_t* h, float* dpi, void** window)
    {
        COMP_RENDERPATH(render_path, );
        render_path->GetCanvas(w, h, dpi, window);
    }
    void VzRenderer::SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        COMP_RENDERPATH(render_path, );
        View* view = render_path->GetView();
        view->setVisibleLayers(layerBits, maskBits);
        timeStamp = std::chrono::high_resolution_clock::now();
    }

    VZRESULT VzRenderer::Render(const VID vidScene, const VID vidCam)
    {
        VzRenderPath* render_path = gEngineApp.GetRenderPath(componentVID);
        if (render_path == nullptr)
        {
            backlog::post("invalid render path", backlog::LogLevel::Error);
            return VZ_FAIL;
        }

        View* view = render_path->GetView();
        Scene* scene = gEngineApp.GetScene(vidScene);
        Camera* camera = gEngine->getCameraComponent(utils::Entity::import(vidCam));
        if (view == nullptr || scene == nullptr || camera == nullptr)
        {
            backlog::post("renderer has nullptr : " + std::string(view == nullptr ? "view " : "")
                + std::string(scene == nullptr ? "scene " : "") + std::string(camera == nullptr ? "camera" : "")
                , backlog::LogLevel::Error);
            return VZ_FAIL;
        }
        render_path->TryResizeRenderTargets();
        view->setScene(scene);
        view->setCamera(camera);
        //view->setVisibleLayers(0x4, 0x4);
        //SceneVID vid_scene = gEngineApp.GetSceneVidBelongTo(vidCam);
        //assert(vid_scene != INVALID_VID);

        if (!UTILS_HAS_THREADING)
        {
            gEngine->execute();
        }

        render_path->deltaTime = float(std::max(0.0, vTimer.RecordElapsedSeconds())); // timeStep

        VzCameraRes* cam_res = gEngineApp.GetCameraRes(vidCam);
        cam_res->UpdateCameraWithCM(render_path->deltaTime);
        
        // fixed time update
        {
            render_path->deltaTimeAccumulator += render_path->deltaTime;
            if (render_path->deltaTimeAccumulator > 10)
            {
                // application probably lost control, fixed update would take too long
                render_path->deltaTimeAccumulator = 0;
            }

            const float targetFrameRateInv = 1.0f / render_path->GetFixedTimeUpdate();
            while (render_path->deltaTimeAccumulator >= targetFrameRateInv)
            {
                //renderer->FixedUpdate();
                render_path->deltaTimeAccumulator -= targetFrameRateInv;
            }
        }

        // Update the cube distortion matrix used for frustum visualization.
        const Camera* lightmapCamera = view->getDirectionalShadowCamera();
        if (lightmapCamera) {
            VzSceneRes* scene_res = gEngineApp.GetSceneRes(vidScene);
            Cube* lightmapCube = scene_res->GetLightmapCube();
            lightmapCube->mapFrustum(*gEngine, lightmapCamera);
        }
        Cube* cameraCube = cam_res->GetCameraCube();
        if (cameraCube) {
            cameraCube->mapFrustum(*gEngine, camera);
        }

        if (vzGltfIO.resourceLoader)
            vzGltfIO.resourceLoader->asyncUpdateLoad();

        Renderer* renderer = render_path->GetRenderer();

        // setup
        //if (preRender) {
        //    preRender(mEngine, window->mViews[0]->getView(), mScene, renderer);
        //}

        //if (mReconfigureCameras) {
        //    window->configureCamerasForWindow();
        //    mReconfigureCameras = false;
        //}

        //if (config.splitView) {
        //    if (!window->mOrthoView->getView()->hasCamera()) {
        //        Camera const* debugDirectionalShadowCamera =
        //            window->mMainView->getView()->getDirectionalShadowCamera();
        //        if (debugDirectionalShadowCamera) {
        //            window->mOrthoView->setCamera(
        //                const_cast<Camera*>(debugDirectionalShadowCamera));
        //        }
        //    }
        //}

        filament::SwapChain* sc = render_path->GetSwapChain();
        if (renderer->beginFrame(sc)) {
            renderer->render(view);
            renderer->endFrame();
        }
        render_path->FRAMECOUNT++;

        return VZ_OK;
    }
#pragma endregion
}

namespace filament::gltfio {
    using namespace vzm::backlog;
    static const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

    static LightManager::Type getLightType(const cgltf_light_type light) {
        switch (light) {
        case cgltf_light_type_directional:
            return LightManager::Type::DIRECTIONAL;
        case cgltf_light_type_point:
            return LightManager::Type::POINT;
        case cgltf_light_type_spot:
            return LightManager::Type::FOCUSED_SPOT;
        case cgltf_light_type_max_enum:
        case cgltf_light_type_invalid:
        default: break;
        }
        assert_invariant(false && "Invalid light type");
        return LightManager::Type::DIRECTIONAL;
    }

    static const char* getNodeName(const cgltf_node* node, const char* defaultNodeName)
    {
        if (node->name) return node->name;
        if (node->mesh && node->mesh->name) return node->mesh->name;
        if (node->light && node->light->name) return node->light->name;
        if (node->camera && node->camera->name) return node->camera->name;
        return defaultNodeName;
    }

    static bool primitiveHasVertexColor(const cgltf_primitive& inPrim) {
        for (int slot = 0; slot < inPrim.attributes_count; slot++) {
            const cgltf_attribute& inputAttribute = inPrim.attributes[slot];
            if (inputAttribute.type == cgltf_attribute_type_color) {
                return true;
            }
        }
        return false;
    }

    void AddMaterialComponentsToVzEngine(const MaterialInstance* mi,
        std::vector<MaterialInstanceVID>& mi_vids,
        std::unordered_map<const MaterialInstance*, MaterialInstanceVID>& mMIMap,
        std::unordered_map<const Material*, MaterialVID>& mMaterialMap
    )
    {
        MaterialInstanceVID mi_vid = gEngineApp.CreateMaterialInstance(mi->getName(), mi)->componentVID;
        mi_vids.push_back(mi_vid);
        assert(!mMIMap.contains(mi));
        mMIMap[mi] = mi_vid;
        const Material* m = mi->getMaterial();
        if (!mMaterialMap.contains(m))
        {
            mMaterialMap[m] = gEngineApp.CreateMaterial(m->getName(), m, nullptr, true)->componentVID;
        }
    }

    // MaterialInstanceCache
    // ---------------------
    // Each glTF material definition corresponds to a single MaterialInstance, which are temporarily
    // cached when loading a FilamentInstance. If a given glTF material is referenced by multiple
    // glTF meshes, then their corresponding Filament primitives will share the same Filament
    // MaterialInstance and UvMap. The UvMap is a mapping from each texcoord slot in glTF to one of
    // Filament's 2 texcoord sets.
    //
    // Notes:
    // - The Material objects (used to create instances) are cached in MaterialProvider, not here.
    // - The cache is not responsible for destroying material instances.

    
    FFilamentAsset* VzAssetLoader::createAsset(const uint8_t* bytes, uint32_t byteCount) {
        FilamentInstance* instances;
        return createInstancedAsset(bytes, byteCount, &instances, 1);
    }

    FFilamentAsset* VzAssetLoader::createInstancedAsset(const uint8_t* bytes, uint32_t byteCount,
        FilamentInstance** instances, size_t numInstances) {
        // This method can be used to load JSON or GLB. By using a default options struct, we are asking
        // cgltf to examine the magic identifier to determine which type of file is being loaded.
        cgltf_options options{};

        if constexpr (!GLTFIO_USE_FILESYSTEM) {

            // Provide a custom free callback for each buffer that was loaded from a "file", as opposed
            // to a data:// URL.
            //
            // Since GLTFIO_USE_FILESYSTEM is false, ResourceLoader requires the app provide the file
            // content from outside, so we need to do nothing here, as opposed to the default, which is
            // to call "free".
            //
            // This callback also gets called for the root-level file_data, but since we use
            // `cgltf_parse`, the file_data field is always null.
            options.file.release = [](const cgltf_memory_options*, const cgltf_file_options*, void*) {};
        }

        // Clients can free up their source blob immediately, but cgltf has pointers into the data that
        // need to stay valid. Therefore we create a copy of the source blob and stash it inside the
        // asset.
        utils::FixedCapacityVector<uint8_t> glbdata(byteCount);
        std::copy_n(bytes, byteCount, glbdata.data());

        // The ownership of an allocated `sourceAsset` will be moved to FFilamentAsset::mSourceAsset.
        cgltf_data* sourceAsset;
        cgltf_result result = cgltf_parse(&options, glbdata.data(), byteCount, &sourceAsset);
        if (result != cgltf_result_success) {
            backlog::post("Unable to parse glTF file.", LogLevel::Error);
            return nullptr;
        }

        FFilamentAsset* fAsset = createRootAsset(sourceAsset);
        if (mError) {
            delete fAsset;
            fAsset = nullptr;
            mError = false;
            return nullptr;
        }
        glbdata.swap(fAsset->mSourceAsset->glbData);

        createInstances(numInstances, fAsset);
        if (mError) {
            delete fAsset;
            fAsset = nullptr;
            mError = false;
            return nullptr;
        }

        std::copy_n(fAsset->mInstances.data(), numInstances, instances);
        return fAsset;
    }

    FFilamentAsset* VzAssetLoader::createRootAsset(const cgltf_data* srcAsset) {
        SYSTRACE_CALL();
#if !GLTFIO_DRACO_SUPPORTED
        for (cgltf_size i = 0; i < srcAsset->extensions_required_count; i++) {
            if (!strcmp(srcAsset->extensions_required[i], "KHR_draco_mesh_compression")) {
                backlog::post("KHR_draco_mesh_compression is not supported.", LogLevel::Error);
                return nullptr;
            }
        }
#endif

        mDummyBufferObject = nullptr;
        FFilamentAsset* fAsset = new FFilamentAsset(&mEngine, mNameManager, &mEntityManager,
            &mNodeManager, &mTrsTransformManager, srcAsset, (bool)mLoaderExtended);

        // It is not an error for a glTF file to have zero scenes.
        fAsset->mScenes.clear();
        if (srcAsset->scenes == nullptr) {
            return fAsset;
        }

        // Create a single root node with an identity transform as a convenience to the client.
        VID vid_gltf_root = gEngineApp.CreateSceneComponent(SCENE_COMPONENT_TYPE::ACTOR, "gltf root")->componentVID;
        fAsset->mRoot = Entity::import(vid_gltf_root); // mEntityManager

        // Check if the asset has an extras string.
        const cgltf_asset& asset = srcAsset->asset;
        const cgltf_size extras_size = asset.extras.end_offset - asset.extras.start_offset;
        if (extras_size > 1) {
            fAsset->mAssetExtras = CString(srcAsset->json + asset.extras.start_offset, extras_size);
        }

        // Build a mapping of root nodes to scene membership sets.
        assert_invariant(srcAsset->scenes_count <= NodeManager::MAX_SCENE_COUNT);
        fAsset->mRootNodes.clear();
        const size_t sic = std::min(srcAsset->scenes_count, NodeManager::MAX_SCENE_COUNT);
        fAsset->mScenes.reserve(sic);
        for (size_t si = 0; si < sic; ++si) {
            const cgltf_scene& scene = srcAsset->scenes[si];
            fAsset->mScenes.emplace_back(scene.name);
            for (size_t ni = 0, nic = scene.nodes_count; ni < nic; ++ni) {
                fAsset->mRootNodes[scene.nodes[ni]].set(si);
            }
        }

        // Some exporters (e.g. Cinema4D) produce assets with a separate animation hierarchy and
        // modeling hierarchy, where nodes in the former have no associated scene. We need to create
        // transformable entities for "un-scened" nodes in case they have bones.
        for (size_t i = 0, n = srcAsset->nodes_count; i < n; ++i) {
            cgltf_node* node = &srcAsset->nodes[i];
            if (node->parent == nullptr && fAsset->mRootNodes.find(node) == fAsset->mRootNodes.end()) {
                fAsset->mRootNodes.insert({ node, {} });
            }
        }

        for (const auto& [node, sceneMask] : fAsset->mRootNodes) {
            recursePrimitives(node, fAsset);
        }

        // Find every unique resource URI and store a pointer to any of the cgltf-owned cstrings
        // that match the URI. These strings get freed during releaseSourceData().
        tsl::robin_set<std::string_view> resourceUris;
        auto addResourceUri = [&resourceUris](const char* uri) {
            if (uri) {
                resourceUris.insert(uri);
            }
            };
        for (cgltf_size i = 0, len = srcAsset->buffers_count; i < len; ++i) {
            addResourceUri(srcAsset->buffers[i].uri);
        }
        for (cgltf_size i = 0, len = srcAsset->images_count; i < len; ++i) {
            addResourceUri(srcAsset->images[i].uri);
        }
        fAsset->mResourceUris.reserve(resourceUris.size());
        for (std::string_view uri : resourceUris) {
            fAsset->mResourceUris.push_back(uri.data());
        }

        return fAsset;
    }


    FilamentInstance* VzAssetLoader::createInstance(FFilamentAsset* fAsset) {
        if (!fAsset->mSourceAsset) {
            post("Source data has been released; asset is frozen.", LogLevel::Error);
            return nullptr;
        }
        const cgltf_data* srcAsset = fAsset->mSourceAsset->hierarchy;
        if (srcAsset->scenes == nullptr) {
            post("There is no scene in the asset.", LogLevel::Error);
            return nullptr;
        }

        auto rootTransform = mTransformManager.getInstance(fAsset->mRoot);
        //Entity instanceRoot = mEntityManager.create();
        //mTransformManager.create(instanceRoot, rootTransform);
        ActorVID vid_ins_root = gEngineApp.CreateSceneComponent(SCENE_COMPONENT_TYPE::ACTOR, "instance root")->componentVID;
        Entity instanceRoot = Entity::import(vid_ins_root);
        mTransformManager.create(instanceRoot, rootTransform);

        mMaterialInstanceCache = MaterialInstanceCache(srcAsset);

        // Create an instance object, which is a just a lightweight wrapper around a vector of
        // entities and an animator. The creation of animator is triggered from ResourceLoader
        // because it could require external bin data.
        FFilamentInstance* instance = new FFilamentInstance(instanceRoot, fAsset);

        // Check if the asset has variants.
        instance->mVariants.reserve(srcAsset->variants_count);
        for (cgltf_size i = 0, len = srcAsset->variants_count; i < len; ++i) {
            instance->mVariants.push_back({ CString(srcAsset->variants[i].name) });
        }

        // For each scene root, recursively create all entities.
        for (const auto& pair : fAsset->mRootNodes) {
            recurseEntities(pair.first, pair.second, instanceRoot, fAsset, instance);
        }

        importSkins(instance, srcAsset);

        // Now that all entities have been created, the instance can create the animator component.
        // Note that it may need to defer actual creation until external buffers are fully loaded.
        instance->createAnimator();

        fAsset->mInstances.push_back(instance);

        // Bounding boxes are not shared because users might call recomputeBoundingBoxes() which can
        // be affected by entity transforms. However, upon instance creation we can safely copy over
        // the asset's bounding box.
        instance->mBoundingBox = fAsset->mBoundingBox;

        mMaterialInstanceCache.flush(&instance->mMaterialInstances);

        fAsset->mDependencyGraph.commitEdges();

        return instance;
    }
    
    void VzAssetLoader::recursePrimitives(const cgltf_node* node, FFilamentAsset* fAsset) {
        const char* name = getNodeName(node, mDefaultNodeName);
        name = name ? name : "node";

        if (node->mesh) {
            createPrimitives(node, name, fAsset);
            fAsset->mRenderableCount++;
        }

        for (cgltf_size i = 0, len = node->children_count; i < len; ++i) {
            recursePrimitives(node->children[i], fAsset);
        }
    }

    void VzAssetLoader::createInstances(size_t numInstances, FFilamentAsset* fAsset) {
        // Create a separate entity hierarchy for each instance. Note that MeshCache (vertex
        // buffers and index buffers) and MaterialInstanceCache (materials and textures) help avoid
        // needless duplication of resources.
        for (size_t index = 0; index < numInstances; ++index) {
            if (createInstance(fAsset) == nullptr) {
                mError = true;
                break;
            }
        }

        // Sort the entities so that the renderable ones come first. This allows us to expose
        // a "renderables only" pointer without storing a separate list.
        const auto& rm = mEngine.getRenderableManager();
        std::partition(fAsset->mEntities.begin(), fAsset->mEntities.end(), [&rm](Entity a) {
            return rm.hasComponent(a);
            });
    }

    void VzAssetLoader::recurseEntities(const cgltf_node* node, SceneMask scenes, Entity parent,
        FFilamentAsset* fAsset, FFilamentInstance* instance) {
        NodeManager& nm = mNodeManager;
        const cgltf_data* srcAsset = fAsset->mSourceAsset->hierarchy;
        const Entity entity = mEntityManager.create();
        nm.create(entity);
        const auto nodeInstance = nm.getInstance(entity);
        nm.setSceneMembership(nodeInstance, scenes);

        // Always create a transform component to reflect the original hierarchy.
        mat4f localTransform;
        if (node->has_matrix) {
            memcpy(&localTransform[0][0], &node->matrix[0], 16 * sizeof(float));
        }
        else {
            quatf* rotation = (quatf*)&node->rotation[0];
            float3* scale = (float3*)&node->scale[0];
            float3* translation = (float3*)&node->translation[0];
            mTrsTransformManager.create(entity, *translation, *rotation, *scale);
            localTransform = mTrsTransformManager.getTransform(
                mTrsTransformManager.getInstance(entity));
        }

        auto parentTransform = mTransformManager.getInstance(parent);
        mTransformManager.create(entity, parentTransform, localTransform);

        // Check if this node has an extras string.
        const cgltf_size extras_size = node->extras.end_offset - node->extras.start_offset;
        if (extras_size > 0) {
            mNodeManager.setExtras(mNodeManager.getInstance(entity),
                { srcAsset->json + node->extras.start_offset, extras_size });
        }

        // Update the asset's entity list and private node mapping.
        fAsset->mEntities.push_back(entity);
        instance->mEntities.push_back(entity);
        instance->mNodeMap[node - srcAsset->nodes] = entity;

        const char* name = getNodeName(node, mDefaultNodeName);

        if (name) {
            fAsset->mNameToEntity[name].push_back(entity);
            if (mNameManager) {
                mNameManager->addComponent(entity);
                mNameManager->setName(mNameManager->getInstance(entity), name);
            }
        }

        // If no name is provided in the glTF or AssetConfiguration, use "node" for error messages.
        name = name ? name : "node";

        // If the node has a mesh, then create a renderable component.
        if (node->mesh) {
            gEngineApp.CreateSceneComponent(SCENE_COMPONENT_TYPE::ACTOR, name, entity.getId());
            createRenderable(node, entity, name, fAsset);
            if (srcAsset->variants_count > 0) {
                createMaterialVariants(node->mesh, entity, fAsset, instance);
            }
        }
        if (node->light) {
            createLight(node->light, entity, fAsset);
        }
        if (node->camera) {
            createCamera(node->camera, entity, fAsset);
        }
        if (node->camera == nullptr && node->light == nullptr && node->mesh == nullptr)
        {
            srcAsset->
        }
        else
        {
            mSceneCompMap[entity.getId()] = name;
        }

        for (cgltf_size i = 0, len = node->children_count; i < len; ++i) {
            recurseEntities(node->children[i], scenes, entity, fAsset, instance);
        }
    }

    void VzAssetLoader::createPrimitives(const cgltf_node* node, const char* name,
        FFilamentAsset* fAsset) {
        cgltf_data* gltf = fAsset->mSourceAsset->hierarchy;
        const cgltf_mesh* mesh = node->mesh;
        assert_invariant(gltf != nullptr);
        assert_invariant(mesh != nullptr);

        // If the mesh is already loaded, obtain the list of Filament VertexBuffer / IndexBuffer objects
        // that were already generated (one for each primitive), otherwise allocate a new list of
        // pointers for the primitives.
        FixedCapacityVector<Primitive>& prims = fAsset->mMeshCache[mesh - gltf->meshes];
        if (prims.empty()) {
            prims.reserve(mesh->primitives_count);
            prims.resize(mesh->primitives_count);
        }

        Aabb aabb;

        std::vector<Primitive> v_prims;
        for (cgltf_size index = 0, n = mesh->primitives_count; index < n; ++index) {
            Primitive& outputPrim = prims[index];
            cgltf_primitive& inputPrim = mesh->primitives[index];

            if (!outputPrim.vertices) {
                if (mLoaderExtended) {
                    auto& resourceInfo = std::get<FFilamentAsset::ResourceInfoExtended>(fAsset->mResourceInfo);
                    resourceInfo.uriDataCache = mLoaderExtended->getUriDataCache();
                    AssetLoaderExtended::Input input{
                            .gltf = gltf,
                            .prim = &inputPrim,
                            .name = name,
                            .dracoCache = &fAsset->mSourceAsset->dracoCache,
                            .material = getMaterial(gltf, inputPrim.material, &outputPrim.uvmap,
                                    utility::primitiveHasVertexColor(&inputPrim)),
                    };

                    mError = !mLoaderExtended->createPrimitive(&input, &outputPrim, resourceInfo.slots);
                    if (!mError) {
                        if (outputPrim.vertices) {
                            fAsset->mVertexBuffers.push_back(outputPrim.vertices);
                        }
                        if (outputPrim.indices) {
                            fAsset->mIndexBuffers.push_back(outputPrim.indices);
                        }
                    }
                }
                else {
                    // Create a Filament VertexBuffer and IndexBuffer for this prim if we haven't
                    // already.
                    mError = !createPrimitive(inputPrim, name, &outputPrim, fAsset);
                }
                if (mError) {
                    return;
                }
            }

            // Expand the object-space bounding box.
            aabb.min = min(outputPrim.aabb.min, aabb.min);
            aabb.max = max(outputPrim.aabb.max, aabb.max);
            v_prims.push_back(outputPrim);
        }

        mGeometryMap[(cgltf_mesh*)(mesh - gltf->meshes)] = gEngineApp.CreateGeometry(name, v_prims)->componentVID;

        mat4f worldTransform;
        cgltf_node_transform_world(node, &worldTransform[0][0]);

        const Aabb transformed = aabb.transform(worldTransform);
        fAsset->mBoundingBox.min = min(fAsset->mBoundingBox.min, transformed.min);
        fAsset->mBoundingBox.max = max(fAsset->mBoundingBox.max, transformed.max);
    }

    void VzAssetLoader::createRenderable(const cgltf_node* node, Entity entity, const char* name,
        FFilamentAsset* fAsset) {
        const cgltf_data* srcAsset = fAsset->mSourceAsset->hierarchy;
        const cgltf_mesh* mesh = node->mesh;
        const cgltf_size primitiveCount = mesh->primitives_count;

        // If the mesh is already loaded, obtain the list of Filament VertexBuffer / IndexBuffer objects
        // that were already generated (one for each primitive).
        FixedCapacityVector<Primitive>& prims = fAsset->mMeshCache[mesh - srcAsset->meshes];

        std::vector<MaterialInstanceVID> mi_vids;
        mi_vids.reserve(prims.size());

        assert_invariant(prims.size() == primitiveCount);
        Primitive* outputPrim = prims.data();
        const cgltf_primitive* inputPrim = &mesh->primitives[0];

        Aabb aabb;

        // glTF spec says that all primitives must have the same number of morph targets.
        const cgltf_size numMorphTargets = inputPrim ? inputPrim->targets_count : 0;
        RenderableManager::Builder builder(primitiveCount);

        // For each prim, create a Filament VertexBuffer, IndexBuffer, and MaterialInstance.
        // The VertexBuffer and IndexBuffer objects are cached for possible re-use, but MaterialInstance
        // is not.
        size_t morphingVertexCount = 0;
        for (cgltf_size index = 0; index < primitiveCount; ++index, ++outputPrim, ++inputPrim) {
            RenderableManager::PrimitiveType primType;
            if (!getPrimitiveType(inputPrim->type, &primType)) {
                post("Unsupported primitive type in " + std::string(name), LogLevel::Warning);
            }

            if (numMorphTargets != inputPrim->targets_count) {
                post("Sister primitives must all have the same number of morph targets.", LogLevel::Warning);
                mError = true;
                continue;
            }

            // Create a material instance for this primitive or fetch one from the cache.
            UvMap uvmap{};
            bool hasVertexColor = primitiveHasVertexColor(*inputPrim);
            MaterialInstance* mi = createMaterialInstance(inputPrim->material, &uvmap, hasVertexColor,
                fAsset);
            assert_invariant(mi);
            if (!mi) {
                mError = true;
                continue;
            }

            AddMaterialComponentsToVzEngine(mi, mi_vids, mMIMap, mMaterialMap);

            fAsset->mDependencyGraph.addEdge(entity, mi);
            builder.material(index, mi);

            assert_invariant(outputPrim->vertices);

            // Expand the object-space bounding box.
            aabb.min = min(outputPrim->aabb.min, aabb.min);
            aabb.max = max(outputPrim->aabb.max, aabb.max);

            // We are not using the optional offset, minIndex, maxIndex, and count arguments when
            // calling geometry() on the builder. It appears that the glTF spec does not have
            // facilities for these parameters, which is not a huge loss since some of the buffer
            // view and accessor features already have this functionality.
            builder.geometry(index, primType, outputPrim->vertices, outputPrim->indices);

            if (numMorphTargets) {
                outputPrim->morphTargetOffset = morphingVertexCount;    // FIXME: can I do that here?
                builder.morphing(0, index, morphingVertexCount);
                morphingVertexCount += outputPrim->vertices->getVertexCount();
            }
        }

        if (numMorphTargets) {
            MorphTargetBuffer* morphTargetBuffer = MorphTargetBuffer::Builder()
                .count(numMorphTargets)
                .vertexCount(morphingVertexCount)
                .build(mEngine);

            fAsset->mMorphTargetBuffers.push_back(morphTargetBuffer);

            builder.morphing(morphTargetBuffer);

            outputPrim = prims.data();
            inputPrim = &mesh->primitives[0];
            for (cgltf_size index = 0; index < primitiveCount; ++index, ++outputPrim, ++inputPrim) {
                outputPrim->morphTargetBuffer = morphTargetBuffer;

                UTILS_UNUSED_IN_RELEASE cgltf_accessor const* previous = nullptr;
                for (int tindex = 0; tindex < numMorphTargets; ++tindex) {
                    const cgltf_morph_target& inTarget = inputPrim->targets[tindex];
                    for (cgltf_size aindex = 0; aindex < inTarget.attributes_count; ++aindex) {
                        const cgltf_attribute& attribute = inTarget.attributes[aindex];
                        const cgltf_accessor* accessor = attribute.data;
                        const cgltf_attribute_type atype = attribute.type;
                        if (atype == cgltf_attribute_type_position) {
                            // All position attributes must have the same number of components.
                            assert_invariant(!previous || previous->type == accessor->type);
                            previous = accessor;

                            assert_invariant(outputPrim->morphTargetBuffer);

                            if (std::holds_alternative<FFilamentAsset::ResourceInfo>(
                                fAsset->mResourceInfo)) {
                                using BufferSlot = FFilamentAsset::ResourceInfo::BufferSlot;
                                auto& slots = std::get<FFilamentAsset::ResourceInfo>(
                                    fAsset->mResourceInfo).mBufferSlots;
                                BufferSlot& slot = slots[outputPrim->slotIndices[tindex]];

                                assert_invariant(!slot.vertexBuffer);
                                assert_invariant(!slot.indexBuffer);

                                slot.morphTargetBuffer = outputPrim->morphTargetBuffer;
                                slot.morphTargetOffset = outputPrim->morphTargetOffset;
                                slot.morphTargetCount = outputPrim->vertices->getVertexCount();
                                slot.bufferIndex = tindex;
                            }
                            else if (std::holds_alternative<FFilamentAsset::ResourceInfoExtended>(
                                fAsset->mResourceInfo))
                            {
                                using BufferSlot = FFilamentAsset::ResourceInfoExtended::BufferSlot;
                                auto& slots = std::get<FFilamentAsset::ResourceInfoExtended>(
                                    fAsset->mResourceInfo).slots;

                                BufferSlot& slot = slots[outputPrim->slotIndices[tindex]];

                                assert_invariant(slot.slot == tindex);
                                assert_invariant(!slot.vertices);
                                assert_invariant(!slot.indices);

                                slot.target = outputPrim->morphTargetBuffer;
                                slot.offset = outputPrim->morphTargetOffset;
                                slot.count = outputPrim->vertices->getVertexCount();
                            }

                            break;
                        }
                    }
                }
            }
        }

        FixedCapacityVector<CString> morphTargetNames(numMorphTargets);
        for (cgltf_size i = 0, c = mesh->target_names_count; i < c; ++i) {
            morphTargetNames[i] = CString(mesh->target_names[i]);
        }
        auto& nm = mNodeManager;
        nm.setMorphTargetNames(nm.getInstance(entity), std::move(morphTargetNames));

        if (node->skin) {
            builder.skinning(node->skin->joints_count);
        }

        // Per the spec, glTF models must have valid mix / max annotations for position attributes.
        // If desired, clients can call "recomputeBoundingBoxes()" in FilamentInstance.
        Box box = Box().set(aabb.min, aabb.max);
        if (box.isEmpty()) {
            post("Missing bounding box in " + std::string(name), LogLevel::Warning);
            box = Box().set(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
        }


        auto it = mGeometryMap.find((cgltf_mesh*)(mesh - srcAsset->meshes));
        assert(it != mGeometryMap.end());
        GeometryVID vid_geo = it->second;
        VzGeometryRes* geo_res = gEngineApp.GetGeometryRes(vid_geo);
        assert(geo_res);
        std::vector<Primitive>& v_primitives = *geo_res->Get();
        outputPrim = prims.data();
        inputPrim = &mesh->primitives[0];
        for (cgltf_size index = 0; index < primitiveCount; ++index, ++outputPrim, ++inputPrim) {
            v_primitives[index] = *outputPrim;
        }
        assert(mi_vids.size() == primitiveCount);
        VzActorRes* actor_res = gEngineApp.GetActorRes(entity.getId());
        actor_res->SetGeometry(vid_geo);
        actor_res->SetMIs(mi_vids);

        builder
            .boundingBox(box)
            .culling(true)
            .castShadows(true)
            .receiveShadows(true)
            .build(mEngine, entity);

        // According to the spec, the mesh may or may not specify default weights, regardless of whether
        // it actually has morph targets. If it has morphing enabled then the default weights are 0. If
        // node weights are provided, they override the ones specified on the mesh.
        if (numMorphTargets > 0) {
            RenderableManager::Instance renderable = mRenderableManager.getInstance(entity);
            const auto size = std::min(MAX_MORPH_TARGETS, numMorphTargets);
            FixedCapacityVector<float> weights(size, 0.0f);
            for (cgltf_size i = 0, c = std::min(size, mesh->weights_count); i < c; ++i) {
                weights[i] = mesh->weights[i];
            }
            for (cgltf_size i = 0, c = std::min(size, node->weights_count); i < c; ++i) {
                weights[i] = node->weights[i];
            }
            mRenderableManager.setMorphWeights(renderable, weights.data(), size);
        }
    }

    void VzAssetLoader::createMaterialVariants(const cgltf_mesh* mesh, Entity entity,
        FFilamentAsset* fAsset, FFilamentInstance* instance) {
        UvMap uvmap{};

        VzActorRes* actor_res = gEngineApp.GetActorRes(entity.getId());
        std::vector<std::vector<MaterialInstanceVID>>& vid_mi_variants = actor_res->GetMIVariants();
        vid_mi_variants.clear();

        for (cgltf_size prim = 0, n = mesh->primitives_count; prim < n; ++prim) {
            const cgltf_primitive& srcPrim = mesh->primitives[prim];
            std::vector<MaterialInstanceVID> mi_vids;

            for (size_t i = 0, m = srcPrim.mappings_count; i < m; i++) {
                const size_t variantIndex = srcPrim.mappings[i].variant;
                const cgltf_material* material = srcPrim.mappings[i].material;
                bool hasVertexColor = primitiveHasVertexColor(srcPrim);
                MaterialInstance* mi =
                    createMaterialInstance(material, &uvmap, hasVertexColor, fAsset);

                AddMaterialComponentsToVzEngine(mi, mi_vids, mMIMap, mMaterialMap);
                
                assert_invariant(mi);
                if (!mi) {
                    mError = true;
                    break;
                }
                fAsset->mDependencyGraph.addEdge(entity, mi);
                instance->mVariants[variantIndex].mappings.push_back({ entity, prim, mi });
            }

            vid_mi_variants.push_back(mi_vids);
        }
    }

    bool VzAssetLoader::createPrimitive(const cgltf_primitive& inPrim, const char* name,
        Primitive* outPrim, FFilamentAsset* fAsset) {

        using BufferSlot = FFilamentAsset::ResourceInfo::BufferSlot;

        Material* material = getMaterial(fAsset->mSourceAsset->hierarchy,
            inPrim.material, &outPrim->uvmap, primitiveHasVertexColor(inPrim));
        AttributeBitset requiredAttributes = material->getRequiredAttributes();

        // TODO: populate a mapping of Texture Index => [MaterialInstance, const char*] slots.
        // By creating this mapping during the "recursePrimitives" phase, we will can allow
        // zero-instance assets to exist. This will be useful for "preloading", which is a feature
        // request from Google.

        // Create a little lambda that appends to the asset's vertex buffer slots.
        auto* const slots = &std::get<FFilamentAsset::ResourceInfo>(fAsset->mResourceInfo).mBufferSlots;
        auto addBufferSlot = [slots](FFilamentAsset::ResourceInfo::BufferSlot entry) {
            slots->push_back(entry);
            };

        // In glTF, each primitive may or may not have an index buffer.
        IndexBuffer* indices = nullptr;
        const cgltf_accessor* accessor = inPrim.indices;
        if (accessor) {
            IndexBuffer::IndexType indexType;
            if (!getIndexType(accessor->component_type, &indexType)) {
                utils::slog.e << "Unrecognized index type in " << name << utils::io::endl;
                return false;
            }

            indices = IndexBuffer::Builder()
                .indexCount(accessor->count)
                .bufferType(indexType)
                .build(mEngine);

            FFilamentAsset::ResourceInfo::BufferSlot slot = { accessor };
            slot.indexBuffer = indices;
            addBufferSlot(slot);
        }
        else if (inPrim.attributes_count > 0) {
            // If a primitive does not have an index buffer, generate a trivial one now.
            const uint32_t vertexCount = inPrim.attributes[0].data->count;

            indices = IndexBuffer::Builder()
                .indexCount(vertexCount)
                .bufferType(IndexBuffer::IndexType::UINT)
                .build(mEngine);

            const size_t indexDataSize = vertexCount * sizeof(uint32_t);
            uint32_t* indexData = (uint32_t*)malloc(indexDataSize);
            for (size_t i = 0; i < vertexCount; ++i) {
                indexData[i] = i;
            }
            IndexBuffer::BufferDescriptor bd(indexData, indexDataSize, FREE_CALLBACK);
            indices->setBuffer(mEngine, std::move(bd));
        }
        fAsset->mIndexBuffers.push_back(indices);

        VertexBuffer::Builder vbb;
        vbb.enableBufferObjects();

        bool hasUv0 = false, hasUv1 = false, hasVertexColor = false, hasNormals = false;
        uint32_t vertexCount = 0;

        const size_t firstSlot = slots->size();
        int slot = 0;

        for (cgltf_size aindex = 0; aindex < inPrim.attributes_count; aindex++) {
            const cgltf_attribute& attribute = inPrim.attributes[aindex];
            const int index = attribute.index;
            const cgltf_attribute_type atype = attribute.type;
            const cgltf_accessor* accessor = attribute.data;

            // The glTF tangent data is ignored here, but honored in ResourceLoader.
            if (atype == cgltf_attribute_type_tangent) {
                continue;
            }

            // At a minimum, surface orientation requires normals to be present in the source data.
            // Here we re-purpose the normals slot to point to the quats that get computed later.
            if (atype == cgltf_attribute_type_normal) {
                vbb.attribute(VertexAttribute::TANGENTS, slot, VertexBuffer::AttributeType::SHORT4);
                vbb.normalized(VertexAttribute::TANGENTS);
                hasNormals = true;
                addBufferSlot({ &fAsset->mGenerateTangents, atype, slot++ });
                continue;
            }

            if (atype == cgltf_attribute_type_color) {
                hasVertexColor = true;
            }

            // Translate the cgltf attribute enum into a Filament enum.
            VertexAttribute semantic;
            if (!getVertexAttrType(atype, &semantic)) {
                utils::slog.e << "Unrecognized vertex semantic in " << name << utils::io::endl;
                return false;
            }
            if (atype == cgltf_attribute_type_weights && index > 0) {
                utils::slog.e << "Too many bone weights in " << name << utils::io::endl;
                continue;
            }
            if (atype == cgltf_attribute_type_joints && index > 0) {
                utils::slog.e << "Too many joints in " << name << utils::io::endl;
                continue;
            }

            if (atype == cgltf_attribute_type_texcoord) {
                if (index >= UvMapSize) {
                    utils::slog.e << "Too many texture coordinate sets in " << name << utils::io::endl;
                    continue;
                }
                UvSet uvset = outPrim->uvmap[index];
                switch (uvset) {
                case UvSet::UV0:
                    semantic = VertexAttribute::UV0;
                    hasUv0 = true;
                    break;
                case UvSet::UV1:
                    semantic = VertexAttribute::UV1;
                    hasUv1 = true;
                    break;
                case UvSet::UNUSED:
                    // If we have a free slot, then include this unused UV set in the VertexBuffer.
                    // This allows clients to swap the glTF material with a custom material.
                    if (!hasUv0 && getNumUvSets(outPrim->uvmap) == 0) {
                        semantic = VertexAttribute::UV0;
                        hasUv0 = true;
                        break;
                    }

                    // If there are no free slots then drop this unused texture coordinate set.
                    // This should not print an error or warning because the glTF spec stipulates an
                    // order of degradation for gracefully dropping UV sets. We implement this in
                    // constrainMaterial in MaterialProvider.
                    continue;
                }
            }

            vertexCount = accessor->count;

            // The positions accessor is required to have min/max properties, use them to expand
            // the bounding box for this primitive.
            if (atype == cgltf_attribute_type_position) {
                const float* minp = &accessor->min[0];
                const float* maxp = &accessor->max[0];
                outPrim->aabb.min = min(outPrim->aabb.min, float3(minp[0], minp[1], minp[2]));
                outPrim->aabb.max = max(outPrim->aabb.max, float3(maxp[0], maxp[1], maxp[2]));
            }

            VertexBuffer::AttributeType fatype;
            VertexBuffer::AttributeType actualType;
            if (!getElementType(accessor->type, accessor->component_type, &fatype, &actualType)) {
                slog.e << "Unsupported accessor type in " << name << io::endl;
                return false;
            }
            const int stride = (fatype == actualType) ? accessor->stride : 0;

            // The cgltf library provides a stride value for all accessors, even though they do not
            // exist in the glTF file. It is computed from the type and the stride of the buffer view.
            // As a convenience, cgltf also replaces zero (default) stride with the actual stride.
            vbb.attribute(semantic, slot, fatype, 0, stride);
            vbb.normalized(semantic, accessor->normalized);
            addBufferSlot({ accessor, atype, slot++ });
        }

        // If the model is lit but does not have normals, we'll need to generate flat normals.
        if (requiredAttributes.test(VertexAttribute::TANGENTS) && !hasNormals) {
            vbb.attribute(VertexAttribute::TANGENTS, slot, VertexBuffer::AttributeType::SHORT4);
            vbb.normalized(VertexAttribute::TANGENTS);
            cgltf_attribute_type atype = cgltf_attribute_type_normal;
            addBufferSlot({ &fAsset->mGenerateNormals, atype, slot++ });
        }

        cgltf_size targetsCount = inPrim.targets_count;

        if (targetsCount > MAX_MORPH_TARGETS) {
            utils::slog.w << "WARNING: Exceeded max morph target count of "
                << MAX_MORPH_TARGETS << utils::io::endl;
            targetsCount = MAX_MORPH_TARGETS;
        }

        const Aabb baseAabb(outPrim->aabb);
        for (cgltf_size targetIndex = 0; targetIndex < targetsCount; targetIndex++) {
            const cgltf_morph_target& morphTarget = inPrim.targets[targetIndex];
            for (cgltf_size aindex = 0; aindex < morphTarget.attributes_count; aindex++) {
                const cgltf_attribute& attribute = morphTarget.attributes[aindex];
                const cgltf_accessor* accessor = attribute.data;
                const cgltf_attribute_type atype = attribute.type;

                // The glTF normal and tangent data are ignored here, but honored in ResourceLoader.
                if (atype == cgltf_attribute_type_normal || atype == cgltf_attribute_type_tangent) {
                    continue;
                }

                if (atype != cgltf_attribute_type_position) {
                    utils::slog.e << "Only positions, normals, and tangents can be morphed."
                        << utils::io::endl;
                    return false;
                }

                if (!accessor->has_min || !accessor->has_max) {
                    continue;
                }

                Aabb targetAabb(baseAabb);
                const float* minp = &accessor->min[0];
                const float* maxp = &accessor->max[0];

                // We assume that the range of morph target weight is [0, 1].
                targetAabb.min += float3(minp[0], minp[1], minp[2]);
                targetAabb.max += float3(maxp[0], maxp[1], maxp[2]);

                outPrim->aabb.min = min(outPrim->aabb.min, targetAabb.min);
                outPrim->aabb.max = max(outPrim->aabb.max, targetAabb.max);

                VertexBuffer::AttributeType fatype;
                VertexBuffer::AttributeType actualType;
                if (!getElementType(accessor->type, accessor->component_type, &fatype, &actualType)) {
                    slog.e << "Unsupported accessor type in " << name << io::endl;
                    return false;
                }
            }
        }

        if (vertexCount == 0) {
            slog.e << "Empty vertex buffer in " << name << io::endl;
            return false;
        }

        vbb.vertexCount(vertexCount);

        // We provide a single dummy buffer (filled with 0xff) for all unfulfilled vertex requirements.
        // The color data should be a sequence of normalized UBYTE4, so dummy UVs are USHORT2 to make
        // the sizes match.
        bool needsDummyData = false;

        if (mMaterials.needsDummyData(VertexAttribute::UV0) && !hasUv0) {
            needsDummyData = true;
            hasUv0 = true;
            vbb.attribute(VertexAttribute::UV0, slot, VertexBuffer::AttributeType::USHORT2);
            vbb.normalized(VertexAttribute::UV0);
        }

        if (mMaterials.needsDummyData(VertexAttribute::UV1) && !hasUv1) {
            hasUv1 = true;
            needsDummyData = true;
            vbb.attribute(VertexAttribute::UV1, slot, VertexBuffer::AttributeType::USHORT2);
            vbb.normalized(VertexAttribute::UV1);
        }

        if (mMaterials.needsDummyData(VertexAttribute::COLOR) && !hasVertexColor) {
            needsDummyData = true;
            vbb.attribute(VertexAttribute::COLOR, slot, VertexBuffer::AttributeType::UBYTE4);
            vbb.normalized(VertexAttribute::COLOR);
        }

        int numUvSets = getNumUvSets(outPrim->uvmap);
        if (!hasUv0 && numUvSets > 0) {
            needsDummyData = true;
            vbb.attribute(VertexAttribute::UV0, slot, VertexBuffer::AttributeType::USHORT2);
            vbb.normalized(VertexAttribute::UV0);
            slog.w << "Missing UV0 data in " << name << io::endl;
        }

        if (!hasUv1 && numUvSets > 1) {
            needsDummyData = true;
            vbb.attribute(VertexAttribute::UV1, slot, VertexBuffer::AttributeType::USHORT2);
            vbb.normalized(VertexAttribute::UV1);
            slog.w << "Missing UV1 data in " << name << io::endl;
        }

        vbb.bufferCount(needsDummyData ? slot + 1 : slot);

        VertexBuffer* vertices = vbb.build(mEngine);

        outPrim->indices = indices;
        outPrim->vertices = vertices;
        auto& primitives = std::get<FFilamentAsset::ResourceInfo>(fAsset->mResourceInfo).mPrimitives;
        primitives.push_back({ &inPrim, vertices });
        fAsset->mVertexBuffers.push_back(vertices);

        for (size_t i = firstSlot; i < slots->size(); ++i) {
            (*slots)[i].vertexBuffer = vertices;
        }

        if (targetsCount > 0) {
            UTILS_UNUSED_IN_RELEASE cgltf_accessor const* previous = nullptr;
            outPrim->slotIndices.resize(targetsCount);
            for (int tindex = 0; tindex < targetsCount; ++tindex) {
                const cgltf_morph_target& inTarget = inPrim.targets[tindex];
                for (cgltf_size aindex = 0; aindex < inTarget.attributes_count; ++aindex) {
                    const cgltf_attribute& attribute = inTarget.attributes[aindex];
                    const cgltf_accessor* accessor = attribute.data;
                    const cgltf_attribute_type atype = attribute.type;
                    if (atype == cgltf_attribute_type_position) {
                        // All position attributes must have the same number of components.
                        assert_invariant(!previous || previous->type == accessor->type);
                        previous = accessor;
                        BufferSlot slot = { accessor };
                        outPrim->slotIndices[tindex] = slots->size();
                        addBufferSlot(slot);
                        break;
                    }
                }
            }
        }

        if (needsDummyData) {
            const uint32_t requiredSize = sizeof(ubyte4) * vertexCount;
            if (mDummyBufferObject == nullptr || requiredSize > mDummyBufferObject->getByteCount()) {
                mDummyBufferObject = BufferObject::Builder().size(requiredSize).build(mEngine);
                fAsset->mBufferObjects.push_back(mDummyBufferObject);
                uint32_t* dummyData = (uint32_t*)malloc(requiredSize);
                memset(dummyData, 0xff, requiredSize);
                VertexBuffer::BufferDescriptor bd(dummyData, requiredSize, FREE_CALLBACK);
                mDummyBufferObject->setBuffer(mEngine, std::move(bd));
            }
            vertices->setBufferObjectAt(mEngine, slot, mDummyBufferObject);
        }

        return true;
    }

    void VzAssetLoader::createLight(const cgltf_light* light, Entity entity, FFilamentAsset* fAsset) {
        LightManager::Type type = getLightType(light->type);
        LightManager::Builder builder(type);

        builder.direction({ 0.0f, 0.0f, -1.0f });
        builder.color({ light->color[0], light->color[1], light->color[2] });

        switch (type) {
        case LightManager::Type::SUN:
        case LightManager::Type::DIRECTIONAL:
            builder.intensity(light->intensity);
            break;
        case LightManager::Type::POINT:
            builder.intensityCandela(light->intensity);
            break;
        case LightManager::Type::FOCUSED_SPOT:
        case LightManager::Type::SPOT:
            // glTF specifies half angles, so does Filament
            builder.spotLightCone(
                light->spot_inner_cone_angle,
                light->spot_outer_cone_angle);
            builder.intensityCandela(light->intensity);
            break;
        }

        if (light->range == 0.0f) {
            // Use 10.0f units as a resonable default falloff value.
            builder.falloff(10.0f);
        }
        else {
            builder.falloff(light->range);
        }

        builder.build(mEngine, entity);
        fAsset->mLightEntities.push_back(entity);
    }

    void VzAssetLoader::createCamera(const cgltf_camera* camera, Entity entity, FFilamentAsset* fAsset) {
        Camera* filamentCamera = mEngine.createCamera(entity);

        if (camera->type == cgltf_camera_type_perspective) {
            auto& projection = camera->data.perspective;

            const cgltf_float yfovDegrees = 180.0 / F_PI * projection.yfov;

            // Use an "infinite" zfar plane if the provided one is missing (set to 0.0).
            const double far = projection.zfar > 0.0 ? projection.zfar : 100000000;

            filamentCamera->setProjection(yfovDegrees, 1.0,
                projection.znear, far,
                filament::Camera::Fov::VERTICAL);

            // Use a default aspect ratio of 1.0 if the provided one is missing.
            const double aspect = projection.aspect_ratio > 0.0 ? projection.aspect_ratio : 1.0;

            // Use the scaling matrix to set the aspect ratio, so clients can easily change it.
            filamentCamera->setScaling({ 1.0 / aspect, 1.0 });
        }
        else if (camera->type == cgltf_camera_type_orthographic) {
            auto& projection = camera->data.orthographic;

            const double left = -projection.xmag * 0.5;
            const double right = projection.xmag * 0.5;
            const double bottom = -projection.ymag * 0.5;
            const double top = projection.ymag * 0.5;

            filamentCamera->setProjection(Camera::Projection::ORTHO,
                left, right, bottom, top, projection.znear, projection.zfar);
        }
        else {
            slog.e << "Invalid GLTF camera type." << io::endl;
            return;
        }

        fAsset->mCameraEntities.push_back(entity);
    }

    MaterialKey VzAssetLoader::getMaterialKey(const cgltf_data* srcAsset,
        const cgltf_material* inputMat, UvMap* uvmap, bool vertexColor,
        cgltf_texture_view* baseColorTexture, cgltf_texture_view* metallicRoughnessTexture) const {
        auto mrConfig = inputMat->pbr_metallic_roughness;
        auto sgConfig = inputMat->pbr_specular_glossiness;
        auto ccConfig = inputMat->clearcoat;
        auto trConfig = inputMat->transmission;
        auto shConfig = inputMat->sheen;
        auto vlConfig = inputMat->volume;
        auto spConfig = inputMat->specular;
        *baseColorTexture = mrConfig.base_color_texture;
        *metallicRoughnessTexture = mrConfig.metallic_roughness_texture;

        bool hasTextureTransforms =
            sgConfig.diffuse_texture.has_transform ||
            sgConfig.specular_glossiness_texture.has_transform ||
            mrConfig.base_color_texture.has_transform ||
            mrConfig.metallic_roughness_texture.has_transform ||
            inputMat->normal_texture.has_transform ||
            inputMat->occlusion_texture.has_transform ||
            inputMat->emissive_texture.has_transform ||
            ccConfig.clearcoat_texture.has_transform ||
            ccConfig.clearcoat_roughness_texture.has_transform ||
            ccConfig.clearcoat_normal_texture.has_transform ||
            shConfig.sheen_color_texture.has_transform ||
            shConfig.sheen_roughness_texture.has_transform ||
            trConfig.transmission_texture.has_transform ||
            spConfig.specular_color_texture.has_transform ||
            spConfig.specular_texture.has_transform;

        MaterialKey matkey{
            .doubleSided = !!inputMat->double_sided,
            .unlit = !!inputMat->unlit,
            .hasVertexColors = vertexColor,
            .hasBaseColorTexture = baseColorTexture->texture != nullptr,
            .hasNormalTexture = inputMat->normal_texture.texture != nullptr,
            .hasOcclusionTexture = inputMat->occlusion_texture.texture != nullptr,
            .hasEmissiveTexture = inputMat->emissive_texture.texture != nullptr,
            .enableDiagnostics = mDiagnosticsEnabled,
            .baseColorUV = (uint8_t)baseColorTexture->texcoord,
            .hasClearCoatTexture = ccConfig.clearcoat_texture.texture != nullptr,
            .clearCoatUV = (uint8_t)ccConfig.clearcoat_texture.texcoord,
            .hasClearCoatRoughnessTexture = ccConfig.clearcoat_roughness_texture.texture != nullptr,
            .clearCoatRoughnessUV = (uint8_t)ccConfig.clearcoat_roughness_texture.texcoord,
            .hasClearCoatNormalTexture = ccConfig.clearcoat_normal_texture.texture != nullptr,
            .clearCoatNormalUV = (uint8_t)ccConfig.clearcoat_normal_texture.texcoord,
            .hasClearCoat = !!inputMat->has_clearcoat,
            .hasTransmission = !!inputMat->has_transmission,
            .hasTextureTransforms = hasTextureTransforms,
            .emissiveUV = (uint8_t)inputMat->emissive_texture.texcoord,
            .aoUV = (uint8_t)inputMat->occlusion_texture.texcoord,
            .normalUV = (uint8_t)inputMat->normal_texture.texcoord,
            .hasTransmissionTexture = trConfig.transmission_texture.texture != nullptr,
            .transmissionUV = (uint8_t)trConfig.transmission_texture.texcoord,
            .hasSheenColorTexture = shConfig.sheen_color_texture.texture != nullptr,
            .sheenColorUV = (uint8_t)shConfig.sheen_color_texture.texcoord,
            .hasSheenRoughnessTexture = shConfig.sheen_roughness_texture.texture != nullptr,
            .sheenRoughnessUV = (uint8_t)shConfig.sheen_roughness_texture.texcoord,
            .hasVolumeThicknessTexture = vlConfig.thickness_texture.texture != nullptr,
            .volumeThicknessUV = (uint8_t)vlConfig.thickness_texture.texcoord,
            .hasSheen = !!inputMat->has_sheen,
            .hasIOR = !!inputMat->has_ior,
            .hasVolume = !!inputMat->has_volume,
            .hasSpecular = !!inputMat->has_specular,
            .hasSpecularTexture = spConfig.specular_texture.texture != nullptr,
            .hasSpecularColorTexture = spConfig.specular_color_texture.texture != nullptr,
            .specularTextureUV = (uint8_t)spConfig.specular_texture.texcoord,
            .specularColorTextureUV = (uint8_t)spConfig.specular_color_texture.texcoord,
        };

        if (inputMat->has_pbr_specular_glossiness) {
            matkey.useSpecularGlossiness = true;
            if (sgConfig.diffuse_texture.texture) {
                *baseColorTexture = sgConfig.diffuse_texture;
                matkey.hasBaseColorTexture = true;
                matkey.baseColorUV = (uint8_t)baseColorTexture->texcoord;
            }
            if (sgConfig.specular_glossiness_texture.texture) {
                *metallicRoughnessTexture = sgConfig.specular_glossiness_texture;
                matkey.hasSpecularGlossinessTexture = true;
                matkey.specularGlossinessUV = (uint8_t)metallicRoughnessTexture->texcoord;
            }
        }
        else {
            matkey.hasMetallicRoughnessTexture = metallicRoughnessTexture->texture != nullptr;
            matkey.metallicRoughnessUV = (uint8_t)metallicRoughnessTexture->texcoord;
        }

        switch (inputMat->alpha_mode) {
        case cgltf_alpha_mode_opaque:
            matkey.alphaMode = AlphaMode::OPAQUE;
            break;
        case cgltf_alpha_mode_mask:
            matkey.alphaMode = AlphaMode::MASK;
            break;
        case cgltf_alpha_mode_blend:
            matkey.alphaMode = AlphaMode::BLEND;
            break;
        case cgltf_alpha_mode_max_enum:
            break;
        }

        return matkey;
    }

    Material* VzAssetLoader::getMaterial(const cgltf_data* srcAsset,
        const cgltf_material* inputMat, UvMap* uvmap, bool vertexColor) {
        cgltf_texture_view baseColorTexture;
        cgltf_texture_view metallicRoughnessTexture;
        if (UTILS_UNLIKELY(inputMat == nullptr)) {
            inputMat = &kDefaultMat;
        }
        MaterialKey matkey = getMaterialKey(srcAsset, inputMat, uvmap, vertexColor,
            &baseColorTexture, &metallicRoughnessTexture);
        const char* label = inputMat->name ? inputMat->name : "material";
        Material* material = mMaterials.getMaterial(&matkey, uvmap, label);
        assert_invariant(material);
        return material;
    }

    MaterialInstance* VzAssetLoader::createMaterialInstance(const cgltf_material* inputMat, UvMap* uvmap,
        bool vertexColor, FFilamentAsset* fAsset) {
        const cgltf_data* srcAsset = fAsset->mSourceAsset->hierarchy;
        MaterialInstanceCache::Entry* const cacheEntry =
            mMaterialInstanceCache.getEntry(&inputMat, vertexColor);
        if (cacheEntry->instance) {
            *uvmap = cacheEntry->uvmap;
            return cacheEntry->instance;
        }

        cgltf_texture_view baseColorTexture;
        cgltf_texture_view metallicRoughnessTexture;
        MaterialKey matkey = getMaterialKey(srcAsset, inputMat, uvmap, vertexColor, &baseColorTexture,
            &metallicRoughnessTexture);

        // Check if this material has an extras string.
        CString extras;
        const cgltf_size extras_size = inputMat->extras.end_offset - inputMat->extras.start_offset;
        if (extras_size > 0) {
            extras = CString(srcAsset->json + inputMat->extras.start_offset, extras_size);
        }

        // This not only creates a material instance, it modifies the material key according to our
        // rendering constraints. For example, Filament only supports 2 sets of texture coordinates.
        MaterialInstance* mi = mMaterials.createMaterialInstance(&matkey, uvmap, inputMat->name,
            extras.c_str());
        if (!mi) {
            post("No material with the specified requirements exists.", LogLevel::Error);
            return nullptr;
        }

        auto mrConfig = inputMat->pbr_metallic_roughness;
        auto sgConfig = inputMat->pbr_specular_glossiness;
        auto ccConfig = inputMat->clearcoat;
        auto trConfig = inputMat->transmission;
        auto shConfig = inputMat->sheen;
        auto vlConfig = inputMat->volume;
        auto spConfig = inputMat->specular;

        // Check the material blending mode, not the cgltf blending mode, because the provider
        // might have selected an alternative blend mode (e.g. to support transmission).
        if (mi->getMaterial()->getBlendingMode() == filament::BlendingMode::MASKED) {
            mi->setMaskThreshold(inputMat->alpha_cutoff);
        }

        const float* emissive = &inputMat->emissive_factor[0];
        float3 emissiveFactor(emissive[0], emissive[1], emissive[2]);
        if (inputMat->has_emissive_strength) {
            emissiveFactor *= inputMat->emissive_strength.emissive_strength;
        }
        mi->setParameter("emissiveFactor", emissiveFactor);

        const float* c = mrConfig.base_color_factor;
        mi->setParameter("baseColorFactor", float4(c[0], c[1], c[2], c[3]));
        mi->setParameter("metallicFactor", mrConfig.metallic_factor);
        mi->setParameter("roughnessFactor", mrConfig.roughness_factor);

        if (matkey.useSpecularGlossiness) {
            const float* df = sgConfig.diffuse_factor;
            const float* sf = sgConfig.specular_factor;
            mi->setParameter("baseColorFactor", float4(df[0], df[1], df[2], df[3]));
            mi->setParameter("specularFactor", float3(sf[0], sf[1], sf[2]));
            mi->setParameter("glossinessFactor", sgConfig.glossiness_factor);
        }

        const TextureProvider::TextureFlags sRGB = TextureProvider::TextureFlags::sRGB;
        const TextureProvider::TextureFlags LINEAR = TextureProvider::TextureFlags::NONE;

        if (matkey.hasBaseColorTexture) {
            fAsset->addTextureBinding(mi, "baseColorMap", baseColorTexture.texture, sRGB);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = baseColorTexture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("baseColorUvMatrix", uvmat);
            }
        }

        if (matkey.hasMetallicRoughnessTexture) {
            // The "metallicRoughnessMap" is actually a specular-glossiness map when the extension is
            // enabled. Note that KHR_materials_pbrSpecularGlossiness specifies that diffuseTexture and
            // specularGlossinessTexture are both sRGB, whereas the core glTF spec stipulates that
            // metallicRoughness is not sRGB.
            TextureProvider::TextureFlags srgb = inputMat->has_pbr_specular_glossiness ? sRGB : LINEAR;
            fAsset->addTextureBinding(mi, "metallicRoughnessMap", metallicRoughnessTexture.texture, srgb);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = metallicRoughnessTexture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("metallicRoughnessUvMatrix", uvmat);
            }
        }

        if (matkey.hasNormalTexture) {
            fAsset->addTextureBinding(mi, "normalMap", inputMat->normal_texture.texture, LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = inputMat->normal_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("normalUvMatrix", uvmat);
            }
            mi->setParameter("normalScale", inputMat->normal_texture.scale);
        }
        else {
            mi->setParameter("normalScale", 1.0f);
        }

        if (matkey.hasOcclusionTexture) {
            fAsset->addTextureBinding(mi, "occlusionMap", inputMat->occlusion_texture.texture, LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = inputMat->occlusion_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("occlusionUvMatrix", uvmat);
            }
            mi->setParameter("aoStrength", inputMat->occlusion_texture.scale);
        }
        else {
            mi->setParameter("aoStrength", 1.0f);
        }

        if (matkey.hasEmissiveTexture) {
            fAsset->addTextureBinding(mi, "emissiveMap", inputMat->emissive_texture.texture, sRGB);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = inputMat->emissive_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("emissiveUvMatrix", uvmat);
            }
        }

        if (matkey.hasClearCoat) {
            mi->setParameter("clearCoatFactor", ccConfig.clearcoat_factor);
            mi->setParameter("clearCoatRoughnessFactor", ccConfig.clearcoat_roughness_factor);

            if (matkey.hasClearCoatTexture) {
                fAsset->addTextureBinding(mi, "clearCoatMap", ccConfig.clearcoat_texture.texture,
                    LINEAR);
                if (matkey.hasTextureTransforms) {
                    const cgltf_texture_transform& uvt = ccConfig.clearcoat_texture.transform;
                    auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                    mi->setParameter("clearCoatUvMatrix", uvmat);
                }
            }
            if (matkey.hasClearCoatRoughnessTexture) {
                fAsset->addTextureBinding(mi, "clearCoatRoughnessMap",
                    ccConfig.clearcoat_roughness_texture.texture, LINEAR);
                if (matkey.hasTextureTransforms) {
                    const cgltf_texture_transform& uvt = ccConfig.clearcoat_roughness_texture.transform;
                    auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                    mi->setParameter("clearCoatRoughnessUvMatrix", uvmat);
                }
            }
            if (matkey.hasClearCoatNormalTexture) {
                fAsset->addTextureBinding(mi, "clearCoatNormalMap",
                    ccConfig.clearcoat_normal_texture.texture, LINEAR);
                if (matkey.hasTextureTransforms) {
                    const cgltf_texture_transform& uvt = ccConfig.clearcoat_normal_texture.transform;
                    auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                    mi->setParameter("clearCoatNormalUvMatrix", uvmat);
                }
                mi->setParameter("clearCoatNormalScale", ccConfig.clearcoat_normal_texture.scale);
            }
        }

        if (matkey.hasSheen) {
            const float* s = shConfig.sheen_color_factor;
            mi->setParameter("sheenColorFactor", float3{ s[0], s[1], s[2] });
            mi->setParameter("sheenRoughnessFactor", shConfig.sheen_roughness_factor);

            if (matkey.hasSheenColorTexture) {
                fAsset->addTextureBinding(mi, "sheenColorMap", shConfig.sheen_color_texture.texture,
                    sRGB);
                if (matkey.hasTextureTransforms) {
                    const cgltf_texture_transform& uvt = shConfig.sheen_color_texture.transform;
                    auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                    mi->setParameter("sheenColorUvMatrix", uvmat);
                }
            }
            if (matkey.hasSheenRoughnessTexture) {
                bool sameTexture = shConfig.sheen_color_texture.texture == shConfig.sheen_roughness_texture.texture;
                fAsset->addTextureBinding(mi, "sheenRoughnessMap",
                    shConfig.sheen_roughness_texture.texture, sameTexture ? sRGB : LINEAR);
                if (matkey.hasTextureTransforms) {
                    const cgltf_texture_transform& uvt = shConfig.sheen_roughness_texture.transform;
                    auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                    mi->setParameter("sheenRoughnessUvMatrix", uvmat);
                }
            }
        }

        if (matkey.hasVolume) {
            mi->setParameter("volumeThicknessFactor", vlConfig.thickness_factor);

            float attenuationDistance = vlConfig.attenuation_distance;
            // TODO: We assume a color in linear sRGB, is this correct? The spec doesn't say anything
            const float* attenuationColor = vlConfig.attenuation_color;
            LinearColor absorption = Color::absorptionAtDistance(
                *reinterpret_cast<const LinearColor*>(attenuationColor), attenuationDistance);
            mi->setParameter("volumeAbsorption", RgbType::LINEAR, absorption);

            if (matkey.hasVolumeThicknessTexture) {
                fAsset->addTextureBinding(mi, "volumeThicknessMap", vlConfig.thickness_texture.texture,
                    LINEAR);
                if (matkey.hasTextureTransforms) {
                    const cgltf_texture_transform& uvt = vlConfig.thickness_texture.transform;
                    auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                    mi->setParameter("volumeThicknessUvMatrix", uvmat);
                }
            }
        }

        if (matkey.hasTransmission) {
            mi->setParameter("transmissionFactor", trConfig.transmission_factor);
            if (matkey.hasTransmissionTexture) {
                fAsset->addTextureBinding(mi, "transmissionMap", trConfig.transmission_texture.texture,
                    LINEAR);
                if (matkey.hasTextureTransforms) {
                    const cgltf_texture_transform& uvt = trConfig.transmission_texture.transform;
                    auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                    mi->setParameter("transmissionUvMatrix", uvmat);
                }
            }
        }

        // IOR can be implemented as either IOR or reflectance because of ubershaders
        if (matkey.hasIOR) {
            if (mi->getMaterial()->hasParameter("ior")) {
                mi->setParameter("ior", inputMat->ior.ior);
            }
            if (mi->getMaterial()->hasParameter("reflectance")) {
                float ior = inputMat->ior.ior;
                float f0 = (ior - 1.0f) / (ior + 1.0f);
                f0 *= f0;
                float reflectance = std::sqrt(f0 / 0.16f);
                mi->setParameter("reflectance", reflectance);
            }
        }

        if (mi->getMaterial()->hasParameter("emissiveStrength")) {
            mi->setParameter("emissiveStrength", inputMat->has_emissive_strength ?
                inputMat->emissive_strength.emissive_strength : 1.0f);
        }

        if (matkey.hasSpecular) {
            const float* s = spConfig.specular_color_factor;
            mi->setParameter("specularColorFactor", float3{ s[0], s[1], s[2] });
            mi->setParameter("specularStrength", spConfig.specular_factor);

            if (matkey.hasSpecularColorTexture) {
                fAsset->addTextureBinding(mi, "specularColorMap", spConfig.specular_color_texture.texture, sRGB);
                if (matkey.hasTextureTransforms) {
                    const cgltf_texture_transform uvt = spConfig.specular_color_texture.transform;
                    auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                    mi->setParameter("specularColorUvMatrix", uvmat);
                }
            }
            if (matkey.hasSpecularTexture) {
                bool sameTexture = spConfig.specular_color_texture.texture == spConfig.specular_texture.texture;
                fAsset->addTextureBinding(mi, "specularMap", spConfig.specular_texture.texture, sameTexture ? sRGB : LINEAR);
                if (matkey.hasTextureTransforms) {
                    const cgltf_texture_transform uvt = spConfig.specular_texture.transform;
                    auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                    mi->setParameter("specularUvMatrix", uvmat);
                }
            }
        }

        *cacheEntry = { mi, *uvmap };
        return mi;
    }

    void VzAssetLoader::importSkins(FFilamentInstance* instance, const cgltf_data* gltf) {
        instance->mSkins.reserve(gltf->skins_count);
        instance->mSkins.resize(gltf->skins_count);
        const auto& nodeMap = instance->mNodeMap;
        for (cgltf_size i = 0, len = gltf->nodes_count; i < len; ++i) {
            const cgltf_node& node = gltf->nodes[i];
            Entity entity = nodeMap[i];
            if (node.skin && entity) {
                int skinIndex = node.skin - &gltf->skins[0];
                instance->mSkins[skinIndex].targets.insert(entity);
            }
        }
        for (cgltf_size i = 0, len = gltf->skins_count; i < len; ++i) {
            FFilamentInstance::Skin& dstSkin = instance->mSkins[i];
            const cgltf_skin& srcSkin = gltf->skins[i];

            // Build a list of transformables for this skin, one for each joint.
            dstSkin.joints = FixedCapacityVector<Entity>(srcSkin.joints_count);
            for (cgltf_size i = 0, len = srcSkin.joints_count; i < len; ++i) {
                dstSkin.joints[i] = nodeMap[srcSkin.joints[i] - gltf->nodes];
            }
        }
    }
}

namespace vzm
{
    struct SafeReleaseChecker
    {
        SafeReleaseChecker() {};
        bool destroyed = false;
        ~SafeReleaseChecker()
        {
            if (!destroyed)
            {
                backlog::post("MUST CALL DeinitEngineLib before finishing the application!", backlog::LogLevel::Error);
                DeinitEngineLib();
            }
            backlog::post("Safely finished ^^", backlog::LogLevel::Default);
        };
    };
    std::unique_ptr<SafeReleaseChecker> safeReleaseChecker;
    std::vector<MaterialVID> vzmMaterials;

    VZRESULT InitEngineLib(const vzm::ParamMap<std::string>& arguments)
    {
        if (gEngine)
        {
            backlog::post("Already initialized!", backlog::LogLevel::Error);
            return VZ_WARNNING;
        }

        auto& em = utils::EntityManager::get();
        backlog::post("Entity Manager is activated (# of entities : " + std::to_string(em.getEntityCount()) + ")", 
            backlog::LogLevel::Default);

        std::string vulkanGPUHint = "0";

        gEngineConfig.stereoscopicEyeCount = gConfig.stereoscopicEyeCount;
        gEngineConfig.stereoscopicType = Engine::StereoscopicType::NONE;
        // to do : gConfig and gEngineConfig
        // using vzm::ParamMap<std::string>& argument
        //gConfig.backend = filament::Engine::Backend::VULKAN;
        gConfig.vulkanGPUHint = "0";
        gConfig.backend = filament::Engine::Backend::OPENGL;
        //gConfig.headless = true;

        gConfig.title = "hellopbr";
        //gConfig.iblDirectory = FilamentApp::getRootAssetsPath() + IBL_FOLDER;
        gConfig.vulkanGPUHint = "0";
        gConfig.backend = filament::Engine::Backend::OPENGL;

                
        gVulkanPlatform = new FilamentAppVulkanPlatform(gConfig.vulkanGPUHint.c_str());
        gEngine = Engine::Builder()
            .backend(gConfig.backend)
            //.platform(gVulkanPlatform)
            .featureLevel(filament::backend::FeatureLevel::FEATURE_LEVEL_3)
            .config(&gEngineConfig)
            .build();

        gEngine->enableAccurateTranslations();

        // this is to avoid the issue of filament safe-resource logic for Vulkan,
        // which assumes that there is at least one swapchain.
        gDummySwapChain = gEngine->createSwapChain((uint32_t)1, (uint32_t)1);

        if (safeReleaseChecker == nullptr)
        {
            safeReleaseChecker = std::make_unique<SafeReleaseChecker>();
        }
        else
        {
            safeReleaseChecker->destroyed = false;
        }

        // default resources
        {
            Material* material = Material::Builder()
                .package(FILAMENTAPP_DEPTHVISUALIZER_DATA, FILAMENTAPP_DEPTHVISUALIZER_SIZE)
                .build(*gEngine);
            vzmMaterials.push_back(gEngineApp.CreateMaterial("_DEFAULT_DEPTH_MATERIAL", material, nullptr, true)->componentVID);

            material = Material::Builder()
                .package(FILAMENTAPP_AIDEFAULTMAT_DATA, FILAMENTAPP_AIDEFAULTMAT_SIZE)
                //.package(RESOURCES_AIDEFAULTMAT_DATA, RESOURCES_AIDEFAULTMAT_SIZE)
                .build(*gEngine);
            vzmMaterials.push_back(gEngineApp.CreateMaterial("_DEFAULT_STANDARD_MATERIAL", material, nullptr, true)->componentVID);

            material = Material::Builder()
                .package(FILAMENTAPP_TRANSPARENTCOLOR_DATA, FILAMENTAPP_TRANSPARENTCOLOR_SIZE)
                .build(*gEngine);
            vzmMaterials.push_back(gEngineApp.CreateMaterial("_DEFAULT_TRANSPARENT_MATERIAL", material, nullptr, true)->componentVID);

            gMaterialTransparent = material;
        }
        
        // optional... test later
        gMaterialProvider = createJitShaderProvider(gEngine, OPTIMIZE_MATERIALS);
        // createUbershaderProvider(gEngine, UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE);

        auto& ncm = VzNameCompManager::Get();
        vzGltfIO.Initialize();
        vzGltfIO.assetLoader = new gltfio::VzAssetLoader({ gEngine, gMaterialProvider, (NameComponentManager*)&ncm });

        return VZ_OK;
    }

    VZRESULT DeinitEngineLib()
    {
        if (safeReleaseChecker.get() == nullptr)
        {
            vzm::backlog::post("MUST CALL vzm::InitEngineLib before calling vzm::DeinitEngineLib()", backlog::LogLevel::Error);
            return VZ_WARNNING;
        }

        auto& ncm = VzNameCompManager::Get();
        // system unlock
        for (auto& it : vzmMaterials)
        {
            std::string name = ncm.GetName(utils::Entity::import(it));
            vzm::backlog::post("material (" + name + ") has been system-unlocked.", backlog::LogLevel::Default);
            VzMaterialRes* m_res = gEngineApp.GetMaterialRes(it);
            assert(m_res);
            m_res->isSystem = false;
        }

        vzGltfIO.Destory();

        gEngine->destroy(gDummySwapChain);
        gDummySwapChain = nullptr;

        gEngineApp.Destroy();

        gMaterialProvider->destroyMaterials();
        delete gMaterialProvider;
        gMaterialProvider = nullptr;

        delete& ncm;

        //destroy all views belonging to renderPaths before destroy the engine 
        // note 
        // gEngine involves mJobSystem
        // when releasing gEngine, mJobSystem will be released!!
        // this calls JobSystem::requestExit() that includes JobSystem::wakeAll()
        Engine::destroy(&gEngine); // calls FEngine::shutdown()
        
        if (gVulkanPlatform) {
            delete gVulkanPlatform;
            gVulkanPlatform = nullptr;
        }

        safeReleaseChecker->destroyed = true;
        return VZ_OK;
    }

    VZRESULT ReleaseWindowHandlerTasks(void* window)
    {
        if (window == nullptr)
        {
            return VZ_OK;
        }
        if (vzGltfIO.resourceLoader)
        {
            vzGltfIO.resourceLoader->asyncCancelLoad();
        }

        std::vector<RendererVID> renderpath_vids;
        gEngineApp.GetRenderPathVids(renderpath_vids);
        for (RendererVID vid_renderpath : renderpath_vids)
        {
            VzRenderPath* render_path = gEngineApp.GetRenderPath(vid_renderpath);
            void* window_render_path = nullptr;
            render_path->GetCanvas(nullptr, nullptr, nullptr, &window_render_path);
            if (window == window_render_path)
            {
                gEngineApp.RemoveComponent(vid_renderpath);
            }
        }

        return VZ_OK;
    }

    VID GetFirstVidByName(const std::string& name)
    {
        return gEngineApp.GetFirstVidByName(name);
    }

    size_t GetVidsByName(const std::string& name, std::vector<VID>& vids)
    {
        return gEngineApp.GetVidsByName(name, vids);
    }

    bool GetNameByVid(const VID vid, std::string& name)
    {
        name = gEngineApp.GetNameByVid(vid);
        return name != "";
    }

    void RemoveComponent(const VID vid)
    {
        auto& ncm = VzNameCompManager::Get();
        std::string name = ncm.GetName(utils::Entity::import(vid));
        if (gEngineApp.RemoveComponent(vid))
        {
            backlog::post("Component (" + name + ") has been removed", backlog::LogLevel::Default);
        }
        else if (vzGltfIO.DestroyAsset(vid))
        {
            assert(0 && "TO DO: removes the associated components");
            backlog::post("Asset (" + name + ") has been removed", backlog::LogLevel::Default);
        }
        else
        {
            backlog::post("Invalid VID : " + std::to_string(vid), backlog::LogLevel::Error);
        }
    }

    VID NewScene(const std::string& sceneName, VzScene** scene)
    {
        SceneVID vid_scene = gEngineApp.CreateScene(sceneName);
        if (scene)
        {
            *scene = gEngineApp.GetVzComponent<VzScene>(vid_scene);
        }
        return vid_scene;
    }

    VID NewRenderer(const std::string& sceneName, VzRenderer** renderer)
    {
        RendererVID vid = gEngineApp.CreateRenderPath(sceneName);
        if (renderer)
        {
            *renderer = gEngineApp.GetVzComponent<VzRenderer>(vid);
        }
        return vid;
    }

    VID NewSceneComponent(const SCENE_COMPONENT_TYPE compType, const std::string& compName, const VID parentVid, VzSceneComp** sceneComp)
    {
        VzSceneComp* v_comp = nullptr;
        v_comp = gEngineApp.CreateSceneComponent(compType, compName);
        if (v_comp == nullptr)
        {
            backlog::post("NewSceneComponent >> failure to gEngineApp.CreateSceneComponent", backlog::LogLevel::Error);
            return 0;
        }

        if (parentVid != 0)
        {
            gEngineApp.AppendSceneEntityToParent(v_comp->componentVID, parentVid);
        }

        if (sceneComp)
        {
            *sceneComp = v_comp;
        }
        return v_comp->componentVID;
    }

    VID AppendSceneComponentTo(const VID vid, const VID parentVid)
    {
        if (!gEngineApp.AppendSceneEntityToParent(vid, parentVid))
        {
            return INVALID_VID;
        }
        Scene* scene = gEngineApp.GetScene(parentVid);
        if (scene)
        {
            return parentVid;
        }
        return gEngineApp.GetSceneVidBelongTo(parentVid);
    }

    VID AppendAssetTo(const VID vidAsset, const VID parentVid)
    {
        auto it = vzGltfIO.assets.find(vidAsset);
        if (it == vzGltfIO.assets.end() || !gEngineApp.IsSceneComponent(parentVid))
        {
            backlog::post("invalid vid", backlog::LogLevel::Error);
            return INVALID_VID;
        }

        FilamentAsset* asset = it->second;
        assert(vzGltfIO.assetComponents.contains(vidAsset));
        const std::vector<VID>& asset_components = vzGltfIO.assetComponents[vidAsset];

        auto& rcm = gEngine->getRenderableManager();
        auto& lcm = gEngine->getLightManager();
        auto& tcm = gEngine->getTransformManager();
        auto& ncm = VzNameCompManager::Get();

        std::map<VID, VID> aasetComp2NewComp;
        std::map<VID, VID> newComp2AssetComp;
        static std::map<std::string, SCENE_COMPONENT_TYPE> TYPEMAP = {
            {"VzActor", SCENE_COMPONENT_TYPE::ACTOR},
            {"VzLight", SCENE_COMPONENT_TYPE::LIGHT},
            {"VzCamera", SCENE_COMPONENT_TYPE::CAMERA},
        };
        for (int i = 0, n = (int)asset_components.size(); i < n; ++i)
        {
            VID vid_asset_comp = asset_components[i];
            VzBaseComp* base = gEngineApp.GetVzComponent<VzBaseComp>(vid_asset_comp);
            //utils::Entity ett_asset_comp = utils::Entity::import(vid_asset_comp);

            switch (TYPEMAP[base->type])
            {
            case SCENE_COMPONENT_TYPE::ACTOR:
            {
                VzActor* actor = (VzActor*)gEngineApp.CreateSceneComponent(SCENE_COMPONENT_TYPE::ACTOR, base->GetName(), 0);
                aasetComp2NewComp[vid_asset_comp] = actor->componentVID;
                newComp2AssetComp[actor->componentVID] = vid_asset_comp;

                VzActor* actor_asset = (VzActor*)base;
                actor->SetRenderableRes(actor_asset->GetGeometry(), actor_asset->GetMIs());
                break;
            }
            case SCENE_COMPONENT_TYPE::LIGHT:
            {
                VzLight* light = (VzLight*)gEngineApp.CreateSceneComponent(SCENE_COMPONENT_TYPE::LIGHT, base->GetName(), 0);
                aasetComp2NewComp[vid_asset_comp] = light->componentVID;
                newComp2AssetComp[light->componentVID] = vid_asset_comp;
                break;
            }
            case SCENE_COMPONENT_TYPE::CAMERA:
            {
                VzCamera* camera = (VzCamera*)gEngineApp.CreateSceneComponent(SCENE_COMPONENT_TYPE::CAMERA, base->GetName(), 0);
                aasetComp2NewComp[vid_asset_comp] = camera->componentVID;
                newComp2AssetComp[camera->componentVID] = vid_asset_comp;
                break;
            }
            default: {}
            }
        }





        for (size_t i = 0, n = asset->getRenderableEntityCount(); i < n; i++) {
            utils::Entity ett = asset->getRenderableEntities()[i];
            auto ri = rcm.getInstance(ett);
            rcm.setScreenSpaceContactShadows(ri, true);
            
            VzActor* actor = (VzActor*)gEngineApp.CreateSceneComponent(SCENE_COMPONENT_TYPE::ACTOR, ncm.GetName(ett), 0);// ett.getId());
            // TO DO : SET resources
            VID vid_actor = actor->componentVID;
            gEngineApp.AppendSceneEntityToParent(vid_actor, parentVid);
        }

        for (size_t i = 0, n = asset->getLightEntityCount(); i < n; i++) {
            utils::Entity ett = asset->getLightEntities()[i];
            auto li = lcm.getInstance(ett);
            
            VzLight* light = (VzLight*)gEngineApp.CreateSceneComponent(SCENE_COMPONENT_TYPE::LIGHT, ncm.GetName(ett), 0);// ett.getId());
            gEngineApp.AppendSceneEntityToParent(light->componentVID, parentVid);
        }

        for (size_t i = 0, n = asset->getCameraEntityCount(); i < n; i++) {
            utils::Entity ett = asset->getCameraEntities()[i];
            VzCamera* camera = (VzCamera*)gEngineApp.CreateSceneComponent(SCENE_COMPONENT_TYPE::CAMERA, ncm.GetName(ett), 0);// ett.getId());
            // TO DO : render path
            gEngineApp.AppendSceneEntityToParent(camera->componentVID, parentVid);
        }

        return gEngineApp.GetSceneVidBelongTo(parentVid);
    }

    VzBaseComp* GetVzComponent(const VID vid)
    {
        return gEngineApp.GetVzComponent<VzBaseComp>(vid);
    }

    size_t GetSceneCompoenentVids(const SCENE_COMPONENT_TYPE compType, const VID sceneVid, std::vector<VID>& vids, const bool isRenderableOnly)
    {
        Scene* scene = gEngineApp.GetScene(sceneVid);
        if (scene == nullptr)
        {
            return 0;
        }

        switch (compType)
        {
        case SCENE_COMPONENT_TYPE::CAMERA:
        {
            if (isRenderableOnly) return 0;

            std::vector<VID> cam_vids;
            size_t num_cameras = gEngineApp.GetCameraVids(cam_vids);
            for (auto& cid : cam_vids)
            {
                if (gEngineApp.GetRenderPath(cid)->GetView()->getScene() == scene)
                {
                    vids.push_back(cid);
                }
            }
            break;
        }
        case SCENE_COMPONENT_TYPE::ACTOR:
        {
            scene->forEach([&](utils::Entity ett) {
                VID vid = ett.getId();
                if (isRenderableOnly)
                {
                    if (gEngineApp.IsRenderable(vid))
                    {
                        vids.push_back(vid);
                    }
                }
                else
                {
                    VzSceneComp* v_comp = gEngineApp.GetVzComponent<VzSceneComp>(vid);
                    if (v_comp->compType == SCENE_COMPONENT_TYPE::ACTOR)
                    {
                        vids.push_back(vid);
                    }
                }
                });
            break;
        }
        case SCENE_COMPONENT_TYPE::LIGHT:
        {
            if (isRenderableOnly) return 0;

            auto& lcm = gEngine->getLightManager();
            scene->forEach([&](utils::Entity ett) {
                VID vid = ett.getId();
                if (gEngineApp.IsLight(vid))
                {
                    vids.push_back(vid);
                }
                });
            break;
        }
        default: break;
        }
        return vids.size();
    }
    
    VID LoadTestModelIntoActor(const std::string& modelName)
    {
        VzActor* actor = gEngineApp.CreateTestActor(modelName);
        return actor? actor->componentVID : INVALID_VID;
    }

    static std::ifstream::pos_type getFileSize(const char* filename) {
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        return in.tellg();
    }
    filament::gltfio::FilamentAsset* loadAsset(const utils::Path& filename) {

        filament::gltfio::FilamentAsset* asset = nullptr;

        // Peek at the file size to allow pre-allocation.
        long const contentSize = static_cast<long>(getFileSize(filename.c_str()));
        if (contentSize <= 0) {
            backlog::post("Unable to open " + std::string(filename.c_str()), backlog::LogLevel::Error);
            return nullptr;
        }

        // Consume the glTF file.
        std::ifstream in(filename.c_str(), std::ifstream::binary | std::ifstream::in);
        std::vector<uint8_t> buffer(static_cast<unsigned long>(contentSize));
        if (!in.read((char*)buffer.data(), contentSize)) {
            backlog::post("Unable to read " + std::string(filename.c_str()), backlog::LogLevel::Error);
            return nullptr;
        }

        // Parse the glTF file and create Filament entities.
        asset = vzGltfIO.assetLoader->createAsset(buffer.data(), buffer.size());
        if (!asset) {
            backlog::post("Unable to parse " + std::string(filename.c_str()), backlog::LogLevel::Error);
            return nullptr;
        }

        buffer.clear();
        buffer.shrink_to_fit();
        return asset;
    };

    VID LoadFileIntoAsset(const std::string& filename, const std::string& assetName, std::vector<VID>& gltfRootVids)
    {
        utils::Path path = filename;
        filament::gltfio::FilamentAsset* asset = nullptr;
        if (path.isEmpty()) {
            asset = vzGltfIO.assetLoader->createAsset(
                GLTF_DEMO_DAMAGEDHELMET_DATA,
                GLTF_DEMO_DAMAGEDHELMET_SIZE);
        }
        else {
            asset = loadAsset(filename);
        }
        if (asset == nullptr)
        {
            backlog::post("asset loading failed!" + filename, backlog::LogLevel::Error);
            return INVALID_VID;
        }

        filament::gltfio::FFilamentAsset* fasset = downcast(asset);
        
        size_t num_m = vzGltfIO.assetLoader->mMaterialMap.size();
        size_t num_mi = vzGltfIO.assetLoader->mMIMap.size();
        size_t num_geo = vzGltfIO.assetLoader->mGeometryMap.size();
        size_t num_scenecomp = vzGltfIO.assetLoader->mSceneCompMap.size();
        size_t num_ins = fasset->mInstances.size();
        backlog::post(std::to_string(num_m) + " system-owned material" + (num_m > 1 ? "s are" : " is") + " created", backlog::LogLevel::Default);
        backlog::post(std::to_string(num_mi) + " material instance" + (num_mi > 1 ? "s are" : " is") + " created", backlog::LogLevel::Default);
        backlog::post(std::to_string(num_geo) + (num_geo > 1 ? " geometries are" : " geometry is") + " created", backlog::LogLevel::Default);
        backlog::post(std::to_string(num_scenecomp) + " scene component" + (num_scenecomp > 1 ? "s are" : " is") + " created", backlog::LogLevel::Default);
        backlog::post(std::to_string(num_ins) + " gltf instance" + (num_ins > 1 ? "s are" : " is") + " created", backlog::LogLevel::Default);

#if !defined(__EMSCRIPTEN__)
        for (auto& it : vzGltfIO.assetLoader->mMaterialMap) {

            Material* ma = (Material*)it.first;
            // Don't attempt to precompile shaders on WebGL.
            // Chrome already suffers from slow shader compilation:
            // https://github.com/google/filament/issues/6615
            // Precompiling shaders exacerbates the problem.
            // First compile high priority variants
            ma->compile(Material::CompilerPriorityQueue::HIGH,
                UserVariantFilterBit::DIRECTIONAL_LIGHTING |
                UserVariantFilterBit::DYNAMIC_LIGHTING |
                UserVariantFilterBit::SHADOW_RECEIVER);

            // and then, everything else at low priority, except STE, which is very uncommon.
            ma->compile(Material::CompilerPriorityQueue::LOW,
                UserVariantFilterBit::FOG |
                UserVariantFilterBit::SKINNING |
                UserVariantFilterBit::SSR |
                UserVariantFilterBit::VSM);
        }
#endif

        gltfRootVids.clear();
        for (auto& instance : fasset->mInstances)
        {
            gltfRootVids.push_back(instance->mRoot.getId());
        }

        if (!vzGltfIO.resourceLoader->asyncBeginLoad(asset)) {
            vzGltfIO.assetLoader->destroyAsset((filament::gltfio::FFilamentAsset*)asset);
            backlog::post("Unable to start loading resources for " + filename, backlog::LogLevel::Error);
            return INVALID_VID;
        }

        auto& em = gEngine->getEntityManager();
        auto& rcm = gEngine->getRenderableManager();
        auto& lcm = gEngine->getLightManager();
        auto& tcm = gEngine->getTransformManager();
        auto& ncm = VzNameCompManager::Get();

        utils::Entity ett_asset = em.create();
        AssetVID vid_asset = ett_asset.getId();
        vzGltfIO.assets[vid_asset] = asset;
        std::vector<VID>& resComp = vzGltfIO.assetComponents[vid_asset];
        ncm.CreateNameComp(ett_asset, assetName);

        asset->releaseSourceData();

        // Enable stencil writes on all material instances.
        filament::gltfio::FilamentInstance* asset_ins = asset->getInstance();
        const size_t mi_count = asset_ins->getMaterialInstanceCount();
        MaterialInstance* const* const mis = asset_ins->getMaterialInstances();
        for (int mi = 0; mi < mi_count; mi++) {
            mis[mi]->setStencilWrite(true);
            mis[mi]->setStencilOpDepthStencilPass(MaterialInstance::StencilOperation::INCR);
        } 

        return ett_asset.getId();
    }

    float GetAsyncLoadProgress()
    {
        if (vzGltfIO.resourceLoader == nullptr)
        {
            backlog::post("resource loader is not activated!", backlog::LogLevel::Error);
            return -1.f;
        }
        return vzGltfIO.resourceLoader->asyncGetLoadProgress();
    }

    void ReloadShader()
    {
        //wi::renderer::ReloadShaders();
    }

    VID DisplayEngineProfiling(const int w, const int h, const bool displayProfile, const bool displayEngineStates)
    {
        //static bool isFirstCall = true;
        //static VID sceneVid = gEngineApp.CreateSceneEntity("__VZM_ENGINE_INTERNAL__");
        //VzmScene* sceneInternalState = gEngineApp.GetScene(sceneVid);
        //static Entity canvasEtt = sceneInternalState->Entity_CreateCamera("INFO_CANVAS", w, h);
        //static VzmRenderer* sysInfoRenderer = gEngineApp.CreateRenderer(canvasEtt);
        //
        //if (isFirstCall)
        //{
        //    sysInfoRenderer->init(w, h, CANVAS_INIT_DPI);
        //
        //    sysInfoRenderer->infoDisplay.active = true;
        //    sysInfoRenderer->infoDisplay.watermark = true;
        //    //sysInfoRenderer->infoDisplay.fpsinfo = true;
        //    //sysInfoRenderer->infoDisplay.resolution = true;
        //    //sysInfoRenderer->infoDisplay.colorspace = true;
        //    sysInfoRenderer->infoDisplay.device_name = true;
        //    sysInfoRenderer->infoDisplay.vram_usage = true;
        //    sysInfoRenderer->infoDisplay.heap_allocation_counter = true;
        //
        //    sysInfoRenderer->DisplayProfile = true;
        //    wi::profiler::SetEnabled(true);
        //
        //    {
        //        const float fadeSeconds = 0.f;
        //        wi::Color fadeColor = wi::Color(0, 0, 0, 255);
        //        // Fade manager will activate on fadeout
        //        sysInfoRenderer->fadeManager.Clear();
        //        sysInfoRenderer->fadeManager.Start(fadeSeconds, fadeColor, []() {
        //            sysInfoRenderer->Start();
        //            });
        //
        //        sysInfoRenderer->fadeManager.Update(0); // If user calls ActivatePath without fadeout, it will be instant
        //    }
        //    isFirstCall = false;
        //}
        //
        //sysInfoRenderer->camEntity = canvasEtt;
        //sysInfoRenderer->width = w;
        //sysInfoRenderer->height = h;
        //sysInfoRenderer->UpdateVmCamera();
        //
        //sysInfoRenderer->setSceneUpdateEnabled(false);
        //sysInfoRenderer->scene->camera = *sysInfoRenderer->camera;
        //
        //wi::font::UpdateAtlas(sysInfoRenderer->GetDPIScaling());
        //
        //if (!wi::initializer::IsInitializeFinished())
        //{
        //    // Until engine is not loaded, present initialization screen...
        //    //sysInfoRenderer->WaitRender();
        //    return VZ_JOB_WAIT;
        //}
        //
        //if (profileFrameFinished)
        //{
        //    profileFrameFinished = false;
        //    wi::profiler::BeginFrame();
        //}
        //sysInfoRenderer->RenderFinalize(); // set profileFrameFinished to true inside
        //
        //return (VID)canvasEtt;
        return 0;
    }

    void* GetGraphicsSharedRenderTarget(const int vidCam, const void* graphicsDev2, const void* srv_desc_heap2, const int descriptor_index, uint32_t* w, uint32_t* h)
    {
        return nullptr;
    }
}
