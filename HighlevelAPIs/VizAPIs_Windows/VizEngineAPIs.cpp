//#pragma warning(disable:4819)
#include "VizEngineAPIs.h" 
#include "VizComponentAPIs.h"

#include <iostream>
#include <memory>

#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Camera.h>
#include <filament/Viewport.h>
#include <filament/Material.h>
#include <filament/Renderer.h>
#include <filament/SwapChain.h>
#include <filament/RenderableManager.h>
#include <filament/MaterialInstance.h>
#include <filament/TransformManager.h>
#include <filament/Scene.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/EntityInstance.h>
#include <utils/NameComponentManager.h>
#include <utils/JobSystem.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <filameshio/MeshReader.h>
#include <filamentapp/Config.h>
#include "../../VisualStudio/samples/generated/resources/resources.h"
#include "../../VisualStudio/samples/generated/resources/monkey.h"

#include <math/mathfwd.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/norm.h>
#include <math/quat.h>

#include "CustomComponents.h"
#include "backend/platforms/VulkanPlatform.h" // requires blueVK.h

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

enum class FCompType
{
    INVALID,
    NameComponent,
    TransformComponent,
    LightComponent,
    CameraComponent,
    RenderableComponent,
};

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

    void post(const std::string& input, LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Default: utils::slog.i << input;
        case LogLevel::Warning: utils::slog.w << input;
        case LogLevel::Error: utils::slog.e << input;
        default: return;
        }
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

namespace vzm
{
    vzm::Timer vTimer;
    std::atomic_bool profileFrameFinished = { true };

    void TransformPoint(const float posSrc[3], const float mat[16], float posDst[3])
    {
        float3 _p = transformCoord(*(mat4f*)mat, *(float3*)posSrc);
        memcpy(posDst, &_p, sizeof(float3));
    }
    void TransformVector(const float vecSrc[3], const float mat[16], float vecDst[3])
    {
        //MathSet(float3, v, vecSrc);
        //MathSet(mat4f, m, mat);
        //mat3f _m(m);
        //float3 _v = transformVec(_m, v);
        //memcpy(vecDst, &_v, sizeof(float3));
    }
    void ComputeBoxTransformMatrix(const float cubeScale[3], const float posCenter[3],
        const float yAxis[3], const float zAxis[3], float mat[16], float matInv[16])
    {
        //XMVECTOR vec_scale = XMLoadFloat3((XMFLOAT3*)cubeScale);
        //XMVECTOR pos_eye = XMLoadFloat3((XMFLOAT3*)posCenter);
        //XMVECTOR vec_up = XMLoadFloat3((XMFLOAT3*)yAxis);
        //XMVECTOR vec_view = -XMLoadFloat3((XMFLOAT3*)zAxis);
        //XMMATRIX ws2cs = VZMatrixLookTo(pos_eye, vec_view, vec_up);
        //vec_scale = XMVectorReciprocal(vec_scale);
        //XMMATRIX scale = XMMatrixScaling(XMVectorGetX(vec_scale), XMVectorGetY(vec_scale), XMVectorGetZ(vec_scale));
        ////glm::fmat4x4 translate = glm::translate(glm::fvec3(0.5f));
        //
        //XMMATRIX ws2cs_unit = ws2cs * scale; // row major
        //*(XMMATRIX*)mat = rowMajor ? ws2cs_unit : XMMatrixTranspose(ws2cs_unit); // note that our renderer uses row-major
        //*(XMMATRIX*)matInv = XMMatrixInverse(NULL, ws2cs_unit);
    }
}

namespace vzm
{
#pragma region // VZM DATA STRUCTURES
    struct VzCanvas
    {
    protected:
        uint32_t width_ = CANVAS_INIT_W;
        uint32_t height_ = CANVAS_INIT_H;
        float dpi_ = CANVAS_INIT_DPI;
        float scaling = 1; // custom DPI scaling factor (optional)
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
        TimeStamp timeStampUpdate_ = {};

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
            bool resize = width_ != prevWidth_ || height_ != prevHeight_ || dpi_ != prevDpi_;
            if (nativeWindow_ == prevNativeWindow_ && !resize)
            {
                return;
            }

            // filament will handle this resizing process internally !?
            // ??? // 만약 그렇다면, ... window 에 따라 offscreen 이냐 아니냐로 재생성 처리.
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
                renderer_->beginFrame(swapChain_);
                //renderer_->render(view_);
                renderer_->endFrame();
            }
        }

        void tryResizeRenderTargets()
        {
            if (gEngine == nullptr)
                return;

            colorspaceConversionRequired_ = colorSpace_ != SWAP_CHAIN_CONFIG_SRGB_COLORSPACE;

            bool requireUpdateRenderTarget = prevWidth_ != width_ || prevHeight_ != height_ || prevDpi_ != dpi_
                || prevColorspaceConversionRequired_ != colorspaceConversionRequired_;
            if (!requireUpdateRenderTarget)
                return;

            resize(); // how to handle rendertarget textures??

            prevWidth_ = width_;
            prevHeight_ = height_;
            prevDpi_ = dpi_;
            prevColorspaceConversionRequired_ = colorspaceConversionRequired_;
        }

    public:
        VzRenderPath()
        {
            view_ = gEngine->createView();
            renderer_ = gEngine->createRenderer();
            swapChain_ = gEngine->createSwapChain(width_, height_);
        }

        ~VzRenderPath()
        {
            if (gEngine)
            {
                if (view_)
                    gEngine->destroy(view_);
                if (renderer_)
                    gEngine->destroy(renderer_);
                if (swapChain_)
                    gEngine->destroy(swapChain_);
            }
        }

        inline void SetFixedTimeUpdate(const float targetFPS)
        {
            targetFrameRate_ = targetFPS;
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
        }
        inline filament::SwapChain* GetSwapChain()
        {
            return swapChain_;
        }

        uint64_t FRAMECOUNT = 0;
        vzm::Timer timer;
        float deltaTime = 0;
        float deltaTimeAccumulator = 0;

        bool UpdateVzCamera(const VzCamera* _vzCam = nullptr)
        {            
            filament::Camera* camera = &view_->getCamera();

            if (_vzCam != nullptr)
            {
                vzCam_ = (VzCamera*)_vzCam;
                VID id = camera->getEntity().getId();
                assert(vzCam_->componentVID == camera->getEntity().getId());
            }
            if (vzCam_)
            {
                std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timeStampUpdate_ - vzCam_->timeStamp);
                if (time_span.count() > 0)
                {
                    return true;
                }
            }
            tryResizeRenderTargets();

            timeStampUpdate_ = std::chrono::high_resolution_clock::now();

            return true;
        }

        inline VzCamera* GetVzCamera()
        {
            return vzCam_;
        }

        inline filament::View* GetView() { return view_; }

        inline filament::Renderer* GetRenderer() { return renderer_; }
    };

    class VzEngineApp
    {
    private:
        using SceneVID = VID;
        using CamVID = VID;
        using RenderableVID = VID;
        using LightVID = VID;
        using GeometryVID = VID;
        using MaterialVID = VID;
        using MaterialInstanceVID = VID;

        std::unordered_map<SceneVID, filament::Scene*> scenes_;
        // note a VzRenderPath involves a view that includes
        // 1. camera and 2. scene
        std::unordered_map<CamVID, VzRenderPath> renderPaths_;
        std::unordered_map<RenderableVID, SceneVID> renderableSceneVids_;
        std::unordered_map<LightVID, SceneVID> lightSceneVids_;

        // Resources
        std::unordered_map<GeometryVID, filamesh::MeshReader::Mesh> geometries_;
        std::unordered_map<MaterialVID, filament::Material*> materials_;
        std::unordered_map<MaterialInstanceVID, filament::MaterialInstance*> materialInstances_;

        std::unordered_map<VID, std::unique_ptr<VzBaseComp>> vzComponents_;

        inline bool removeScene(SceneVID sceneVid)
        {
            Scene* scene = GetScene(sceneVid);
            if (scene == nullptr)
            {
                return false;
            }
            auto& rcm = gEngine->getRenderableManager();
            auto& lcm = gEngine->getLightManager();
            scene->forEach([&](utils::Entity ett) {
                if (rcm.hasComponent(ett))
                {
                    renderableSceneVids_[ett.getId()] = 0;
                }
                else if (lcm.hasComponent(ett))
                {
                    lightSceneVids_[ett.getId()] = 0;
                }
                else
                {
                    backlog::post("entity : " + std::to_string(ett.getId()), backlog::LogLevel::Warning);
                }
                });
            gEngine->destroy(scene);
            scenes_.erase(sceneVid);
            for (auto& it : renderPaths_)
            {
                VzRenderPath* render_path = &it.second;
                render_path->GetView()->setScene(nullptr);
            }
            return true;
        }

        inline VzRenderPath* createRendePath(const CamVID camVid = 0)
        {
            utils::EntityManager& em = utils::EntityManager::get();
            utils::Entity camEtt = utils::Entity::import(camVid);
            CamVID cam_vid = camVid;
            bool is_alive_cam = em.isAlive(camEtt);
            filament::Camera* camera = nullptr;
            if (!is_alive_cam)
            {
                camEtt = em.create();
                cam_vid = camEtt.getId();
            }
            else
            {
                // if the entity has the camera, then re-use it
                // elsewhere create new camera
                camera = gEngine->getCameraComponent(camEtt);
            }
            if (camera == nullptr)
            {
                camera = gEngine->createCamera(camEtt);
            }

            // I organize the renderPath with filament::View that involves
            // 1. Camera (involving the camera component and the transform component)
            // 2. Scene
            VzRenderPath* renderPath = &renderPaths_[camVid];
            filament::View* view = renderPath->GetView();
            view->setCamera(camera); // attached to "view.scene"

            // 1. Instance const i = manager.addComponent(entity);
            // 2. tcm.create(entity); where FTransformManager& tcm = engine.getTransformManager();
            camera->setExposure(16.0f, 1 / 125.0f, 100.0f); // default values used in filamentApp

            return renderPath;
        }
    public:
        // Runtime can create a new entity with this
        inline SceneVID CreateScene(const std::string& name)
        {
            if (GetFirstSceneByName(name))
            {
                vzm::backlog::post("(" + name + ") is already registered as a scene!", backlog::LogLevel::Error);
                return INVALID_VID;
            }

            auto& em = utils::EntityManager::get();
            utils::Entity ett = em.create();
            VID vid = ett.getId();
            scenes_[vid] = gEngine->createScene();

            VzNameCompManager& ncm = VzNameCompManager::Get();
            ncm.CreateNameComp(ett, name);

            return vid;
        }

        inline void SetSceneToRenderPath(const CamVID camVid, const SceneVID sceneVid)
        {
            Scene* scene = GetScene(sceneVid);
            if (scene == nullptr)
            {
                backlog::post("SetSceneToRenderPath >> no Scene", backlog::LogLevel::Error);
                return;
            }
            VzRenderPath* renderPath = GetRenderPath(camVid);
            if (renderPath == nullptr)
            {
                backlog::post("SetSceneToRenderPath >> no RenderPath", backlog::LogLevel::Error);
                return;
            }
            renderPath->GetView()->setScene(scene);
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
        inline bool IsRenderable(const RenderableVID vid)
        {
            auto it = renderableSceneVids_.find(vid);
            bool ret = it != renderableSceneVids_.end();
#ifdef _DEBUG
            assert(ret == gEngine->getRenderableManager().hasComponent(utils::Entity::import(vid)));
#endif
            return ret;
        }
        inline bool IsLight(const LightVID vid)
        {
            auto it = lightSceneVids_.find(vid);
            bool ret = it != lightSceneVids_.end();
#ifdef _DEBUG
            assert(ret == gEngine->getRenderableManager().hasComponent(utils::Entity::import(vid)));
#endif
            return ret;
        }
        inline Scene* GetScene(const SceneVID sid)
        {
            auto it = scenes_.find(sid);
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
        inline VzRenderPath* GetRenderPath(const CamVID camVid)
        {
            auto it = renderPaths_.find(camVid);
            if (it == renderPaths_.end())
            {
                return nullptr;
            }
            return &it->second;
        }
        inline size_t GetCameraVids(std::vector<CamVID>& camVids)
        {
            camVids.clear();
            camVids.reserve(renderPaths_.size());
            for (auto& it : renderPaths_)
            {
                camVids.push_back(it.first);
            }
            return camVids.size();
        }
        inline VzRenderPath* GetFirstRenderPathByName(const std::string& name)
        {
            return GetRenderPath(GetFirstVidByName(name));
        }
        inline SceneVID GetSceneVidBelongTo(const VID vid)
        {
            auto itr = renderableSceneVids_.find(vid);
            if (itr != renderableSceneVids_.end())
            {
                return itr->second;
            }
            auto itl = lightSceneVids_.find(vid);
            if (itl != lightSceneVids_.end())
            {
                return itl->second;
            }
            return INVALID_VID;
        }

        inline void AppendSceneEntityToParent(const VID vidSrc, const VID vidDst)
        {
            assert(vidSrc != vidDst);
            auto getSceneAndVid = [this](Scene** scene, const VID vid)
                {
                    SceneVID vid_scene = vid;
                    *scene = GetScene(vid_scene);
                    if (*scene == nullptr)
                    {
                        auto itr = renderableSceneVids_.find(vid_scene);
                        auto itl = lightSceneVids_.find(vid_scene);
                        if (itr == renderableSceneVids_.end() && itl == renderableSceneVids_.end())
                        {
                            vid_scene = INVALID_VID;
                        }
                        else 
                        {
                            vid_scene = itr == renderableSceneVids_.end() ? itl->second : itr->second;
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

            // case 1. both entities are renderable
            // case 2. src is scene and dst is renderable
            // case 3. src is renderable and dst is scene
            // case 4. both entities are scenes
            // note that renderable entity must have transform component!
            std::vector<utils::Entity> entities_moving;
            if (vidSrc != vid_scene_src && vidDst != vid_scene_dst)
            {
                // case 1. both entities are renderable
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

                // case 2. src is scene and dst is renderable
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
                // case 3. src is renderable and dst is scene
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
                // case 4. both entities are scenes
                scene_src->forEach([&](utils::Entity ett) {
                    entities_moving.push_back(ett);
                    });

                removeScene(vid_scene_src);
            }

            for (auto& it : entities_moving)
            {
                auto itr = renderableSceneVids_.find(it.getId());
                auto itl = lightSceneVids_.find(it.getId());
                if (itr != renderableSceneVids_.end())
                    itr->second = 0;
                else if (itl != lightSceneVids_.end())
                    itl->second = 0;
                if (scene_src)
                {
                    scene_src->remove(ett_src);
                }
            }

            if (scene_dst)
            {
                for (auto& it : entities_moving)
                {
                    scene_dst->addEntity(it);

                    auto itr = renderableSceneVids_.find(it.getId());
                    auto itl = lightSceneVids_.find(it.getId());
                    if (itr != renderableSceneVids_.end())
                        itr->second = vid_scene_dst;
                    else if (itl != lightSceneVids_.end())
                        itl->second = vid_scene_dst;
                }

                auto it_render_path = renderPaths_.find(vidSrc);
                if (it_render_path != renderPaths_.end())
                {
                    VzRenderPath* render_path = &it_render_path->second;
                    render_path->GetView()->setScene(scene_dst);
                }
            }
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
                if (!is_alive)
                {
                    RenderableManager::Builder(0)
                        .build(*gEngine, ett);
                }
                renderableSceneVids_[vid] = 0;

                auto it = vzComponents_.emplace(vid, std::make_unique<VzActor>());
                v_comp = (VzSceneComp*)it.first->second.get();
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
                lightSceneVids_[vid] = 0;

                auto it = vzComponents_.emplace(vid, std::make_unique<VzLight>());
                v_comp = (VzSceneComp*)it.first->second.get();
                break;
            }
            case SCENE_COMPONENT_TYPE::CAMERA:
            {
                auto it = vzComponents_.emplace(vid, std::make_unique<VzCamera>());
                v_comp = (VzSceneComp*)it.first->second.get();
                break;
            }
            default:
                assert(0);
            }
            v_comp->componentVID = vid;
            v_comp->compType = compType;
            v_comp->timeStamp = std::chrono::high_resolution_clock::now();

            if (compType == SCENE_COMPONENT_TYPE::CAMERA)
            {
                vzm::VzCamera* v_cam = (vzm::VzCamera*)v_comp;
                vzm::VzRenderPath* render_path = createRendePath(vid);
                v_cam->renderPath = render_path;
                render_path->SetCanvas(CANVAS_INIT_W, CANVAS_INIT_H, CANVAS_INIT_DPI);
                render_path->UpdateVzCamera(v_cam);
            }

            auto& ncm = VzNameCompManager::Get();
            auto& tcm = gEngine->getTransformManager();
            ncm.CreateNameComp(ett, name);
            tcm.create(ett);

            return v_comp;
        }
        
        inline VzActor* CreateTestActor(const std::string& modelName = "MONKEY_SUZANNE_DATA")
        {
            std::string geo_name = modelName + "_GEOMETRY";
            std::string material_name = modelName + "_MATERIAL";
            std::string mi_name = modelName + "_MATERIAL_INSTANCE";
            auto& ncm = VzNameCompManager::Get();

            MaterialInstance* mi = nullptr;
            for (auto it_mi : materialInstances_)
            {
                utils::Entity ett = utils::Entity::import(it_mi.first);

                if (ncm.GetName(ett) == mi_name)
                {
                    mi = it_mi.second;
                    break;
                }
            }
            if (mi == nullptr)
            {
                VzMaterial* v_m = CreateMaterial(material_name);
                VzMaterialInstance* v_mi = CreateMaterialInstance(mi_name, v_m->componentVID);
                auto itmi = materialInstances_.find(v_mi->componentVID);
                assert(itmi != materialInstances_.end());
                mi = itmi->second;
            }

            MeshReader::Mesh mesh = MeshReader::loadMeshFromBuffer(gEngine, MONKEY_SUZANNE_DATA, nullptr, nullptr, mi);
            ncm.CreateNameComp(mesh.renderable, modelName);
            VID vid = mesh.renderable.getId();
            renderableSceneVids_[vid] = 0;

            CreateGeometry(geo_name, &mesh);

            auto it = vzComponents_.emplace(vid, std::make_unique<VzActor>());
            VzActor* v_actor = (VzActor*)it.first->second.get();
            v_actor->componentVID = vid;
            v_actor->compType = SCENE_COMPONENT_TYPE::ACTOR;
            v_actor->timeStamp = std::chrono::high_resolution_clock::now();
            return v_actor;
        }

        inline VzGeometry* CreateGeometry(const std::string& name, const MeshReader::Mesh* mesh = nullptr)
        {
            auto& em = utils::EntityManager::get();
            auto& ncm = VzNameCompManager::Get();

            utils::Entity ett = em.create();
            ncm.CreateNameComp(ett, name);

            MeshReader::Mesh geo;
            if (mesh != nullptr)
            {
                geo = *mesh;
            }

            VID vid = ett.getId();
            geometries_[vid] = geo;

            auto it = vzComponents_.emplace(vid, std::make_unique<VzGeometry>());
            VzGeometry* v_m = (VzGeometry*)it.first->second.get();
            v_m->componentVID = vid;
            v_m->compType = RES_COMPONENT_TYPE::GEOMATRY;
            v_m->timeStamp = std::chrono::high_resolution_clock::now();

            return v_m;
        }

        inline VzMaterial* CreateMaterial(const std::string& name, const Material* material = nullptr)
        {
            auto& em = utils::EntityManager::get();
            auto& ncm = VzNameCompManager::Get();

            utils::Entity ett = em.create();
            ncm.CreateNameComp(ett, name);

            Material* m = (Material*)material;
            if (m == nullptr)
            {
                m = Material::Builder()
                    .package(RESOURCES_AIDEFAULTMAT_DATA, RESOURCES_AIDEFAULTMAT_SIZE)
                    .build(*gEngine);
            }

            VID vid = ett.getId();
            materials_[vid] = m;

            auto it = vzComponents_.emplace(vid, std::make_unique<VzMaterial>());
            VzMaterial* v_m = (VzMaterial*)it.first->second.get();
            v_m->componentVID = vid;
            v_m->compType = RES_COMPONENT_TYPE::MATERIAL;
            v_m->timeStamp = std::chrono::high_resolution_clock::now();

            return v_m;
        }

        inline VzMaterialInstance* CreateMaterialInstance(const std::string& name, const MaterialVID mVid, const MaterialInstance* materialInstance = nullptr)
        {
            auto itm = materials_.find(mVid);
            if (itm == materials_.end())
            {
                backlog::post("CreateMaterialInstance >> mVid is invalid", backlog::LogLevel::Error);
                return nullptr;
            }
            Material* m = itm->second;

            auto& em = utils::EntityManager::get();
            auto& ncm = VzNameCompManager::Get();

            utils::Entity ett = em.create();
            ncm.CreateNameComp(ett, name);

            MaterialInstance* mi = (MaterialInstance*)materialInstance;
            if (mi == nullptr)
            {
                mi = m->createInstance();
                mi->setParameter("baseColor", RgbType::LINEAR, float3{ 0.8 });
                mi->setParameter("metallic", 1.0f);
                mi->setParameter("roughness", 0.4f);
                mi->setParameter("reflectance", 0.5f);
            }

            VID vid = ett.getId();
            materialInstances_[vid] = mi;

            auto it = vzComponents_.emplace(vid, std::make_unique<VzMaterialInstance>());
            VzMaterialInstance* v_m = (VzMaterialInstance*)it.first->second.get();
            v_m->componentVID = vid;
            v_m->compType = RES_COMPONENT_TYPE::MATERIALINSTANCE;
            v_m->timeStamp = std::chrono::high_resolution_clock::now();

            return v_m;
        }

        inline void SetActorMaterialInstance(const RenderableVID actorVid, const MaterialInstanceVID miVid)
        {

        }

        template <typename VZCOMP>
        inline VZCOMP* GetSceneComponent(const VID vid)
        {
            auto it = vzComponents_.find(vid);
            if (it == vzComponents_.end())
            {
                return nullptr;
            }
            return (VZCOMP*)it->second.get();
        }

        // SceneVID, CamVID, RenderableVID (light and actor), 
        inline void RemoveEntity(const VID vid)
        {
            utils::Entity ett = utils::Entity::import(vid);
            
            auto& ncm = VzNameCompManager::Get();
            ncm.RemoveEntity(ett);

            if (!removeScene(vid))
            {
                auto& em = utils::EntityManager::get();
                // this calls built-in destroy functions in the filament entity managers

                // destroy by engine (refer to the following)
                // void FEngine::destroy(Entity e) {
                //     mRenderableManager.destroy(e);
                //     mLightManager.destroy(e);
                //     mTransformManager.destroy(e);
                //     mCameraManager.destroy(*this, e);
                // }
#pragma region destroy by engine
                gEngine->destroy(ett);
                auto it_m = materials_.find(vid);
                if (it_m != materials_.end())
                {
                    for (auto it = materialInstances_.begin(); it != materialInstances_.end();) 
                    {
                        if (it->second->getMaterial() == it_m->second) 
                        {
                            gEngine->destroy(it->second);
                            it = materialInstances_.erase(it);
                        }
                        else 
                        {
                            ++it;
                        }
                    }
                    // caution: 
                    // before destroying the material,
                    // destroy the associated material instances
                    gEngine->destroy(it_m->second);
                
                    materials_.erase(vid);
                }
                auto it_mi = materialInstances_.find(vid);
                if (it_mi != materialInstances_.end())
                {
                    gEngine->destroy(it_mi->second);
                    materialInstances_.erase(vid);
                }
                auto it_geo = geometries_.find(vid);
                if (it_geo != geometries_.end())
                {
                    gEngine->destroy(it_geo->second.vertexBuffer);
                    gEngine->destroy(it_geo->second.indexBuffer);
                    geometries_.erase((it_geo));
                }
                auto it_render_path = renderPaths_.find(vid);
                if (it_render_path != renderPaths_.end())
                {
                    VzRenderPath& render_path = it_render_path->second;
                    //View* view = render_path.GetView();
                    //gEngine->destroy(view);
                    //Camera* cam = &view->getCamera(); // this is an filament entity
                    //filament::SwapChain* sc = render_path.GetSwapChain();
                    //gEngine->destroy(sc);
                    renderPaths_.erase(it_render_path); // call destructor
                }
#pragma endregion 
                // the remaining etts (not engine-destory group)
                em.destroy(ett);

                vzComponents_.erase(vid);
                renderableSceneVids_.erase(vid);
                lightSceneVids_.erase(vid);
                renderPaths_.erase(vid);

                for (auto& it : scenes_)
                {
                    Scene* scene = it.second;
                    scene->remove(ett);
                }
            }
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
                RemoveEntity(vid);
            }
        }

        inline void Destroy()
        {
            //std::unordered_map<SceneVID, filament::Scene*> scenes_;
            //// note a VzRenderPath involves a view that includes
            //// 1. camera and 2. scene
            //std::unordered_map<CamVID, VzRenderPath> renderPaths_;
            //std::unordered_map<RenderableVID, SceneVID> renderableSceneVids_;
            //std::unordered_map<LightVID, SceneVID> lightSceneVids_;
            //
            //// Resources
            //std::unordered_map<GeometryVID, filamesh::MeshReader::Mesh> geometries_;
            //std::unordered_map<MaterialVID, filament::Material*> materials_;
            //std::unordered_map<MaterialInstanceVID, filament::MaterialInstance*> materialInstances_;
            
            //destroyTarget(scenes_);
            //destroyTarget(renderPaths_);
            //destroyTarget(renderableSceneVids_);
            //destroyTarget(lightSceneVids_);
            //destroyTarget(geometries_);
            //destroyTarget(materials_);
            //destroyTarget(materialInstances_);
        }
    };
#pragma endregion

    VzEngineApp gEngineApp;
}

#define COMP_NAME(COMP, ENTITY, FAILRET) auto& COMP = VzNameCompManager::Get(); Entity ENTITY = Entity::import(componentVID); if (ENTITY.isNull()) return FAILRET;
#define COMP_TRANSFORM(COMP, ENTITY, INS, FAILRET)  auto & COMP = gEngine->getTransformManager(); Entity ENTITY = Entity::import(componentVID); if (ENTITY.isNull()) return FAILRET; auto INS = COMP.getInstance(ENTITY);
#define COMP_RENDERPATH(RENDERPATH, FAILRET)  VzRenderPath* RENDERPATH = gEngineApp.GetRenderPath(componentVID); if (RENDERPATH == nullptr) return FAILRET;
namespace vzm
{
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
#pragma endregion 

#pragma region // VzCamera
    void VzCamera::SetCanvas(const uint32_t w, const uint32_t h, const float dpi, void* window)
    {
        COMP_RENDERPATH(render_path, );
        renderPath = render_path;
        render_path->SetCanvas(w, h, dpi, window);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzCamera::GetCanvas(uint32_t* w, uint32_t* h, float* dpi, void** window)
    {
        COMP_RENDERPATH(render_path, );
        renderPath = render_path;
        render_path->GetCanvas(w, h, dpi, window);
    }

    // Pose parameters are defined in WS (not local space)
    void VzCamera::SetWorldPose(const float pos[3], const float view[3], const float up[3])
    {
        COMP_TRANSFORM(tc, ett, ins, );
        COMP_RENDERPATH(render_path, );
        renderPath = render_path;

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
        COMP_RENDERPATH(render_path, );
        renderPath = render_path;
        View* view = render_path->GetView();
        Camera* camera = &view->getCamera();
#if _DEBUG
        Entity ett = Entity::import(componentVID);
        assert(camera == gEngine->getCameraComponent(ett) && "camera pointer is mismatching!!");
#endif
        // aspectRatio is W / H
        camera->setProjection(fovInDegree, aspectRatio, zNearP, zFarP, 
            isVertical? Camera::Fov::VERTICAL : Camera::Fov::HORIZONTAL);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    void VzCamera::GetWorldPose(float pos[3], float view[3], float up[3])
    {
        COMP_RENDERPATH(render_path, );
        renderPath = render_path;
        Camera* camera = &render_path->GetView()->getCamera();
#if _DEBUG
        Entity ett = Entity::import(componentVID);
        assert(camera == gEngine->getCameraComponent(ett) && "camera pointer is mismatching!!");
#endif

        double3 p = camera->getPosition();
        double3 v = camera->getForwardVector();
        double3 u = camera->getUpVector();
        if (pos) *(float3*)pos = float3(p);
        if (view) *(float3*)view = float3(v);
        if (up) *(float3*)up = float3(u);
    }
    void VzCamera::GetPerspectiveProjection(float* zNearP, float* zFarP, float* fovInDegree, float* aspectRatio, bool isVertical)
    {
        COMP_RENDERPATH(render_path, );
        renderPath = render_path;
        Camera* camera = &render_path->GetView()->getCamera();
#if _DEBUG
        Entity ett = Entity::import(componentVID);
        assert(camera == gEngine->getCameraComponent(ett) && "camera pointer is mismatching!!");
#endif
        if (zNearP) *zNearP = (float)camera->getNear();
        if (zFarP) *zFarP = (float)camera->getCullingFar();
        if (fovInDegree) *fovInDegree = (float)camera->getFieldOfViewInDegrees(isVertical ? Camera::Fov::VERTICAL : Camera::Fov::HORIZONTAL);
        if (aspectRatio)
        {
            const filament::Viewport& vp = render_path->GetView()->getViewport();
            *aspectRatio = vp.width / vp.height;
        }
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
    struct GltfIO
    {
        gltfio::ResourceLoader* resourceLoader = nullptr;
        gltfio::TextureProvider* stbDecoder = nullptr;
        gltfio::TextureProvider* ktxDecoder = nullptr;
    };
    std::unique_ptr<SafeReleaseChecker> safeReleaseChecker;

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

        return VZ_OK;
    }

    VZRESULT DeinitEngineLib()
    {
        if (safeReleaseChecker.get() == nullptr)
        {
            vzm::backlog::post("MUST CALL vzm::InitEngineLib before calling vzm::DeinitEngineLib()", backlog::LogLevel::Error);
            return VZ_WARNNING;
        }
        //utils::JobSystem& job_system = gEngine->getJobSystem();
        //job_system.runAndWait(job_system.createJob(nullptr, [&]() {
        //    // 이 작업은 다른 모든 작업이 완료된 후에 실행됩니다.
        //    }));
        //while (job_system.getThreadCount() > 0) {
        //    std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //}

        gEngine->destroy(gDummySwapChain);
        gDummySwapChain = nullptr;

        gEngineApp.Destroy();

        VzNameCompManager& ncm = VzNameCompManager::Get();
        delete& ncm;
        //destroy all views belonging to renderPaths before destroy the engine 
        //To do???
        Engine::destroy(&gEngine); // calls FEngine::shutdown()
        // note 
        // gEngine involves mJobSystem
        // when releasing gEngine, mJobSystem will be released!!
        // this calls JobSystem::requestExit() that includes JobSystem::wakeAll()
        while (gEngine) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (gVulkanPlatform) {
            delete gVulkanPlatform;
            gVulkanPlatform = nullptr;
        }

        safeReleaseChecker->destroyed = true;
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
        gEngineApp.RemoveEntity(vid);
    }

    VID NewScene(const std::string& sceneName)
    {
        Scene* scene = gEngineApp.GetFirstSceneByName(sceneName);
        if (scene != nullptr)
        {
            backlog::post("scene name must be unique!", backlog::LogLevel::Error);
            return INVALID_VID;
        }

        return gEngineApp.CreateScene(sceneName);
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
        gEngineApp.AppendSceneEntityToParent(vid, parentVid);
        Scene* scene = gEngineApp.GetScene(parentVid);
        if (scene)
        {
            return parentVid;
        }
        return gEngineApp.GetSceneVidBelongTo(parentVid);
    }
    
    VzSceneComp* GetSceneComponent(const VID vid)
    {
        return gEngineApp.GetSceneComponent<VzSceneComp>(vid);;
    }

    size_t GetSceneCompoenentVids(const SCENE_COMPONENT_TYPE compType, const VID sceneVid, std::vector<VID>& vids)
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
                if (gEngineApp.IsRenderable(vid))
                {
                    vids.push_back(vid);
                }
                });
            break;
        }
        case SCENE_COMPONENT_TYPE::LIGHT:
        {
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
    
    void LoadFileIntoNewSceneAsync(const std::string& file, const std::string& rootName, const std::string& sceneName, const std::function<void(VID sceneVid, VID rootVid)>& callback)
    {
        /*
        struct loadingJob
        {
            wi::Timer timer;
            wi::jobsystem::context ctx;
            // input param
            std::string rootName;
            std::string sceneName;
            std::string file;
            std::function<void(VID sceneVid, VID rootVid)> callback;

            bool isFinished = false;
        };

        static uint32_t jobIndex = 0;
        static std::map<uint32_t, loadingJob> loadingJobStore;
        bool isBusy = false;
        for (auto& it : loadingJobStore)
        {
            if (!it.second.isFinished)
            {
                isBusy = true;
                break;
            }
        }
        if (!isBusy)
        {
            loadingJobStore.clear();
            jobIndex = 0;
        }

        loadingJob& jobInfo = loadingJobStore[jobIndex++];
        jobInfo.file = file;
        jobInfo.rootName = rootName;
        jobInfo.sceneName = sceneName;
        jobInfo.callback = callback;

        wi::backlog::post("");
        wi::jobsystem::Execute(jobInfo.ctx, [&](wi::jobsystem::JobArgs args) {
            VID rootVid = INVALID_VID;
            VID sceneVid = LoadFileIntoNewScene(jobInfo.file, jobInfo.rootName, jobInfo.sceneName, &rootVid);
            if (jobInfo.callback != nullptr)
            {
                jobInfo.callback(sceneVid, rootVid);
            }
            });
        std::thread([&jobInfo] {
            wi::jobsystem::Wait(jobInfo.ctx);
            wi::backlog::post("\n[vzm::LoadMeshModelAsync] GLTF Loading (" + std::to_string((int)std::round(jobInfo.timer.elapsed())) + " ms)");
            }).detach();
            /**/
    }

    VID LoadTestModel(const std::string& modelName)
    {
        VzActor* actor = gEngineApp.CreateTestActor(modelName);
        return actor? actor->componentVID : INVALID_VID;
    }

    VID LoadFileIntoNewScene(const std::string& file, const std::string& rootName, const std::string& sceneName, VID* rootVid)
    {

        /*
        VID sid = gEngineApp.CreateSceneEntity(sceneName);
        VzmScene* scene = gEngineApp.GetScene(sid);
        if (scene == nullptr)
        {
            return INVALID_ENTITY;
        }

        Entity rootEntity = INVALID_ENTITY;
        // loading.. with file

        std::string extension = wi::helper::toUpper(wi::helper::GetExtensionFromFileName(file));
        FileType type = FileType::INVALID;
        auto it = filetypes.find(extension);
        if (it != filetypes.end())
        {
            type = it->second;
        }
        if (type == FileType::INVALID)
            return INVALID_ENTITY;

        if (type == FileType::OBJ) // wavefront-obj
        {
            rootEntity = ImportModel_OBJ(file, *scene);	// reassign transform components
        }
        else if (type == FileType::GLTF || type == FileType::GLB || type == FileType::VRM) // gltf, vrm
        {
            rootEntity = ImportModel_GLTF(file, *scene);
        }
        scene->names.GetComponent(rootEntity)->name = rootName;

        if (rootVid) *rootVid = rootEntity;

        return sid;
        /**/
        return 0;
    }

    VZRESULT Render(const VID camVid)
    {
        VzRenderPath* render_path = gEngineApp.GetRenderPath(camVid);
        render_path->UpdateVzCamera();
        if (render_path == nullptr)
        {
            return VZ_FAIL;
        }

        View* view = render_path->GetView();
        Scene* scene = view->getScene();
        Camera* camera = &view->getCamera();
        if (view == nullptr || scene == nullptr)
        {
            return VZ_FAIL;
        }

        if (!UTILS_HAS_THREADING) 
        {
            gEngine->execute();
        }

        render_path->deltaTime = float(std::max(0.0, vTimer.RecordElapsedSeconds())); // timeStep

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
        //const Camera* lightmapCamera = view->getDirectionalShadowCamera();
        //if (lightmapCamera) {
        //    lightmapCube->mapFrustum(*gEngine, lightmapCamera);
        //}
        //cameraCube->mapFrustum(*gEngine, camera);

        // Delay rendering for roughly one monitor refresh interval
        // TODO: Use SDL_GL_SetSwapInterval for proper vsync
        //SDL_DisplayMode Mode;
        //int refreshIntervalMS = (SDL_GetDesktopDisplayMode(
        //    SDL_GetWindowDisplayIndex(window->mWindow), &Mode) == 0 &&
        //    Mode.refresh_rate != 0) ? round(1000.0 / Mode.refresh_rate) : 16;
        //SDL_Delay(refreshIntervalMS);

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

    void* GetGraphicsSharedRenderTarget(const int camVid, const void* graphicsDev2, const void* srv_desc_heap2, const int descriptor_index, uint32_t* w, uint32_t* h)
    {
        return nullptr;
    }
}
