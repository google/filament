#include "VizEngineAPIs.h" 

#ifdef WIN32
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#endif

#include <iostream>
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

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

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

namespace vzm
{
    vzm::Timer vTimer;
    std::atomic_bool profileFrameFinished = { true };

    struct GltfIO
    {
        std::unordered_map<AssetVID, gltfio::FilamentAsset*> assets;
        std::unordered_map<AssetVID, std::vector<VID>> assetComponents;
        std::unordered_map<VID, AssetVID> vzCompAssociatedAssets;   // used for ownership of filament/vz components

        gltfio::AssetLoader* assetLoader = nullptr;

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
            assetLoader->destroyAsset(it->second);
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
                    assetLoader->destroyAsset(it.second);
                }
                assets.clear();
            }

            delete resourceLoader;
            resourceLoader = nullptr;
            delete stbDecoder;
            stbDecoder = nullptr;
            delete ktxDecoder;
            ktxDecoder = nullptr;

            AssetLoader::destroy(&assetLoader);
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
            vzGltfIO.assetLoader = AssetLoader::create({ gEngine, gMaterialProvider, (NameComponentManager*)&ncm });
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
    public:
        inline void SetGeometry(const GeometryVID vid) { vidGeo_ = vid; }
        inline void SetMIs(std::vector<MaterialInstanceVID> vidMIs)
        {
            vidMIs_ = vidMIs;
        }
        inline void SetMI(const MaterialInstanceVID vid, const int slot)
        {
            if ((size_t)slot >= vidMIs_.size())
            {
                backlog::post("a slot cannot exceed the number of elements in the MI array", backlog::LogLevel::Error);
            }
            vidMIs_[slot] = vid;
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
        inline std::vector<MaterialInstanceVID> GetAllMIVids()
        {
            return vidMIs_;
        }
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
                    }
                    else
                    {
                        backlog::post("entity VID : " + std::to_string(ett.getId()), backlog::LogLevel::Warning);
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

            Scene* scene_src = nullptr;
            Scene* scene_dst = nullptr;
            SceneVID vid_scene_src = getSceneAndVid(&scene_src, vidSrc);
            SceneVID vid_scene_dst = getSceneAndVid(&scene_dst, vidDst);

            utils::Entity ett_src = utils::Entity::import(vidSrc);
            utils::Entity ett_dst = utils::Entity::import(vidDst);
            //auto& em = gEngine->getEntityManager();
            auto& tcm = gEngine->getTransformManager();

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
                for (auto it = tcm.getChildrenBegin(ins_src); it != tcm.getChildrenEnd(ins_src); it++)
                {
                    utils::Entity ett = tcm.getEntity(*it);
                    entities_moving.push_back(ett);
                }
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
                for (auto it = tcm.getChildrenBegin(ins_src); it != tcm.getChildrenEnd(ins_src); it++)
                {
                    utils::Entity ett = tcm.getEntity(*it);
                    entities_moving.push_back(ett);
                }
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
            for (auto it_mi : materialInstances_)
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
                for (auto it : materials_)
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
                for (auto it : materialInstances_)
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

        inline void SetActorResources(const ActorVID vidActor, const GeometryVID vidGeo, const MaterialInstanceVID vidMI)
        {
            auto it = actorResMaps_.find(vidActor);
            if (it == actorResMaps_.end())
            {
                backlog::post("invalid actor VID", backlog::LogLevel::Error);
                return;
            }

            // to do... complex scenario...
            assert(0 && "TO DO");

            auto it_geo = geometries_.find(vidGeo);
            auto it_mi = materialInstances_.find(vidMI);
            // to do.... test with gltf asset...
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

                        std::vector<MaterialInstanceVID> mis = actor_res.GetAllMIVids();
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
                    for (auto it : scenes)
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
    //void VzActor::SetMaterialInstanceVid(VID vidMI)
    //{
    //    // to do
    //}
    //void VzActor::SetMaterialVid(VID vidMaterial)
    //{
    //    // to do
    //}
    //void VzActor::SetGeometryVid(VID vidGeometry)
    //{
    //    // to do
    //}
    void VzActor::SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        COMP_ACTOR(rcm, ett, ins, );
        rcm.setLayerMask(ins, layerBits, maskBits);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzActor::SetMaterialInstance(const VID vidMI, const int slot)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        actor_res->SetMI(vidMI, slot);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzActor::SetMaterialInstances(const std::vector<VID>& mis)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        actor_res->SetMIs(mis);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzActor::SetGeometry(const VID vidGeometry)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        actor_res->SetGeometry(vidGeometry);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    VID VzActor::GetMaterialInstance(const int slot)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        return actor_res->GetMIVid(slot);
    }
    VID VzActor::GetMaterial(const int slot)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(componentVID);
        MaterialInstanceVID vid_mi = actor_res->GetMIVid(slot);
        MaterialInstance* mi = gEngineApp.GetMIRes(vid_mi)->mi;
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
                std::cout << "MUST CALL DeinitEngineLib before finishing the application!" << std::endl;
                DeinitEngineLib();
            }
            std::cout << "Safely finished ^^" << std::endl;
        };
    };
    std::unique_ptr<SafeReleaseChecker> safeReleaseChecker;
    std::vector<MaterialVID> systemMaterials;

    VZRESULT InitEngineLib(const vzm::ParamMap<std::string>& arguments)
    {
        //std::string gg = arguments.GetParam("GG hello~~1", std::string(""));
        //float gg1 = arguments.GetParam("GG hello~~2", 0.f);
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
            systemMaterials.push_back(gEngineApp.CreateMaterial("_DEFAULT_DEPTH_MATERIAL", material, nullptr, true)->componentVID);
            
            material = Material::Builder() 
                .package(FILAMENTAPP_AIDEFAULTMAT_DATA, FILAMENTAPP_AIDEFAULTMAT_SIZE)
                //.package(RESOURCES_AIDEFAULTMAT_DATA, RESOURCES_AIDEFAULTMAT_SIZE)
                .build(*gEngine);
            systemMaterials.push_back(gEngineApp.CreateMaterial("_DEFAULT_STANDARD_MATERIAL", material, nullptr, true)->componentVID);

            material = Material::Builder()
                .package(FILAMENTAPP_TRANSPARENTCOLOR_DATA, FILAMENTAPP_TRANSPARENTCOLOR_SIZE)
                .build(*gEngine);
            systemMaterials.push_back(gEngineApp.CreateMaterial("_DEFAULT_TRANSPARENT_MATERIAL", material, nullptr, true)->componentVID);

            gMaterialTransparent = material;
        }
        
        // optional... test later
        gMaterialProvider = createJitShaderProvider(gEngine, OPTIMIZE_MATERIALS);
        // createUbershaderProvider(gEngine, UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE);
        
        vzGltfIO.Initialize();

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
        for (auto& it : systemMaterials)
        {
            std::string name = ncm.GetName(utils::Entity::import(it));
            vzm::backlog::post("material (" + name + ") has been system-unlocked.", backlog::LogLevel::Default);
            VzMaterialRes* m_res = gEngineApp.GetMaterialRes(it);
            assert(m_res);
            m_res->isSystem = false;
        }

        vzGltfIO.Destory();

        gMaterialProvider->destroyMaterials();
        delete gMaterialProvider;
        gMaterialProvider = nullptr;

        gEngine->destroy(gDummySwapChain);
        gDummySwapChain = nullptr;

        gEngineApp.Destroy();

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

        auto& rcm = gEngine->getRenderableManager();
        auto& lcm = gEngine->getLightManager();
        auto& tcm = gEngine->getTransformManager();
        auto& ncm = VzNameCompManager::Get();

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
            for (auto cid : cam_vids)
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

    VID LoadFileIntoAsset(const std::string& filename, const std::string& assetName, std::vector<VID>* resComponents)
    {
        utils::Path path = filename;
        gltfio::FilamentAsset* asset = nullptr;
        if (path.isEmpty()) {
            asset = vzGltfIO.assetLoader->createAsset(
                GLTF_DEMO_DAMAGEDHELMET_DATA,
                GLTF_DEMO_DAMAGEDHELMET_SIZE);

            //auto createSceneActors = [&ncm, &rcm, &tcm](auto& self, const utils::Entity ettParent) -> void
            //    {
            //        assert(tcm.hasComponent(ettParent));
            //        if (!ncm.hasComponent(ettParent))
            //        {
            //            ncm.CreateNameComp(ettParent, "Node");
            //        }
            //        VID vid_parent = ettParent.getId();
            //        VzSceneComp* v_comp = gEngineApp.GetVzComponent<VzSceneComp>(vid_parent);
            //        if (v_comp == nullptr)
            //        {
            //            gEngineApp.CreateSceneComponent(SCENE_COMPONENT_TYPE::ACTOR, ncm.GetName(ettParent), vid_parent);
            //        }
            //
            //        auto ins = tcm.getInstance(ettParent);
            //        
            //        for (auto it = tcm.getChildrenBegin(ins); it != tcm.getChildrenEnd(ins); it++)
            //        {
            //            self(self, tcm.getEntity(*it));
            //        }
            //    };
        }
        else {
            //loadAsset(filename);
        }

        if (!vzGltfIO.resourceLoader->asyncBeginLoad(asset)) {

            vzGltfIO.assetLoader->destroyAsset(asset);
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
        gltfio::FilamentInstance* fm_instance = asset->getInstance();
        const size_t matInstanceCount = fm_instance->getMaterialInstanceCount();
        MaterialInstance* const* const instances = fm_instance->getMaterialInstances();
        for (int mi = 0; mi < matInstanceCount; mi++) {
            instances[mi]->setStencilWrite(true);
            instances[mi]->setStencilOpDepthStencilPass(MaterialInstance::StencilOperation::INCR);
        } 

        std::unordered_map<ActorVID, GeometryVID> actorGeoMap;
        for (auto& it : downcast(asset)->renderablePritmitives)
        {
            GeometryVID vid = gEngineApp.CreateGeometry(assetName + ":Primitive #" + std::to_string(actorGeoMap.size()), it.second, asset)->componentVID;
            actorGeoMap[it.first] = vid;
            resComp.push_back(vid);
        }
        std::unordered_map<Material*, MaterialVID> materials;
        std::unordered_map<MaterialInstance*, MaterialInstanceVID> mis;
        std::unordered_map<ActorVID, std::vector<MaterialInstanceVID>> actorMIsMap;
        for (auto& it : downcast(asset)->renderableMIs)
        {
            std::vector<MaterialInstanceVID> actor_mis;
            for (auto& it_mi : it.second)
            {
                MaterialInstance* mi = it_mi;
                Material* m = (Material*)mi->getMaterial();
                if (!materials.contains(m))
                {
                    MaterialVID vid = gEngineApp.CreateMaterial(assetName + ":Material #" + std::to_string(materials.size()), m, asset)->componentVID;
                    materials[m] = vid;
                    resComp.push_back(vid);
                }
                if (!mis.contains(mi))
                {
                    MaterialInstanceVID vid = gEngineApp.CreateMaterialInstance(assetName + ":MI #" + std::to_string(mis.size()), mi, asset)->componentVID;
                    mis[mi] = vid;
                    resComp.push_back(vid);

                    actor_mis.push_back(vid);
                }
                else
                {
                    actor_mis.push_back(mis[mi]);
                }
            }
            actorMIsMap[it.first] = actor_mis;
        }

        for (auto it : resComp)
        {
            assert(!vzGltfIO.vzCompAssociatedAssets.contains(it));
            vzGltfIO.vzCompAssociatedAssets[it] = vid_asset;
        }

        if (resComponents) *resComponents = resComp;

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
