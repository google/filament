//#pragma warning(disable:4819)
#include "VizEngineAPIs.h" 

#include <iostream>

#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Camera.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/MaterialInstance.h>
#include <filament/TransformManager.h>
#include <filament/Scene.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/EntityInstance.h>
#include <utils/NameComponentManager.h>
#include <utils/Log.h>

#include <filameshio/MeshReader.h>

//#include <filamentapp/FilamentApp.h>
//
//#include "generated/resources/resources.h"
//#include "generated/resources/monkey.h"

#include <filamentapp/Config.h>

#include "CustomComponents.h"
#include "backend/platforms/VulkanPlatform.h" // requires blueVK.h

// name spaces
using namespace filament;
using namespace filamesh;
using namespace filament::math;
using namespace filament::backend;

static std::unordered_map<std::string, vzm::COMPONENT_TYPE> vzcomptypes = {
    {typeid(vzm::VzBaseComp).name(), vzm::COMPONENT_TYPE::BASE},
    {typeid(vzm::VzSceneComp).name(), vzm::COMPONENT_TYPE::SCENE},
    {typeid(vzm::VzCamera).name(), vzm::COMPONENT_TYPE::CAMERA},
    {typeid(vzm::VzLight).name(), vzm::COMPONENT_TYPE::LIGHT},
    {typeid(vzm::VzActor).name(), vzm::COMPONENT_TYPE::ACTOR},
};


enum class FCompType
{
    INVALID,
    NameComponent,
    TransformComponent,
    LightComponent,
    CameraComponent,
    RenderableComponent,
};

static std::unordered_map<std::string, FCompType> comptypes = {
    {typeid(vzm::VzNameCompManager::Instance).name(), FCompType::NameComponent},
    {typeid(filament::TransformManager::Instance).name(), FCompType::TransformComponent},
    {typeid(filament::RenderableManager::Instance).name(), FCompType::RenderableComponent},
    {typeid(filament::LightManager::Instance).name(), FCompType::LightComponent},
    {typeid(filament::Camera).name(), FCompType::CameraComponent},
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

#define MathSet(Type, Name, Src, Num) Type Name; memcpy(&Name, Src, sizeof(Type)); 
inline float3 transformCoord(const mat4f& m, const float3& p)
{
    float4 _p(p, 1.f);
    _p = m * _p;
    return float3(_p.x / _p.w, _p.y / _p.w, _p.z / _p.w);
}

//inline float3 transformVec(const mat3f& m, const float3& v)
//{
//    return m * v;
//}

static Config gConfig;
static Engine::Config gEngineConfig = {};
static filament::backend::VulkanPlatform* gVulkanPlatform = nullptr;
static Engine* gEngine = nullptr;
static filament::SwapChain* gDummySwapChain = nullptr;

namespace vzm
{

    std::atomic_bool profileFrameFinished = { true };

    void TransformPoint(const float posSrc[3], const float mat[16], float posDst[3])
    {
        MathSet(float3, p, posSrc);
        MathSet(mat4f, m, mat);
        float3 _p = transformCoord(m, p);
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
        uint32_t width = 0;
        uint32_t height = 0;
        float dpi = 96;
        float scaling = 1; // custom DPI scaling factor (optional)

        // Create a canvas from physical measurements
        inline void Init(uint32_t width, uint32_t height, float dpi = 96)
        {
            this->width = width;
            this->height = height;
            this->dpi = dpi;
        }
        // Copy canvas from other canvas
        inline void Init(const VzCanvas& other)
        {
            *this = other;
        }
    };

    class VzRenderPath : VzCanvas
    {
    private:

        uint32_t prevWidth_ = 0;
        uint32_t prevHeight_ = 0;
        float prevDpi_ = 96;
        bool prevColorspaceConversionRequired_ = false;

        VzCamera* vzCam_ = nullptr;
        TimeStamp timeStampUpdate_;


        bool colorspaceConversionRequired_ = false;
        uint64_t colorSpace_ = SWAP_CHAIN_CONFIG_SRGB_COLORSPACE; // swapchain color space

        // note "view" involves
        // 1. camera
        // 2. scene
        filament::View* view_ = nullptr;
        filament::SwapChain* swapChain_ = nullptr;

        void resize()
        {
            // refer to configureCamerasForWindow();
        }

        void tryResizeRenderTargets()
        {
            if (gEngine == nullptr)
                return;

            colorspaceConversionRequired_ = colorSpace_ != SWAP_CHAIN_CONFIG_SRGB_COLORSPACE;

            bool requireUpdateRenderTarget = prevWidth_ != width || prevHeight_ != height || prevDpi_ != dpi
                || prevColorspaceConversionRequired_ != colorspaceConversionRequired_;
            if (!requireUpdateRenderTarget)
                return;

            Init(width, height, dpi);

            resize(); // how to handle rendertarget textures??

            prevWidth_ = width;
            prevHeight_ = height;
            prevDpi_ = dpi;
            prevColorspaceConversionRequired_ = colorspaceConversionRequired_;
        }

    public:
        VzRenderPath()
        {
            view_ = gEngine->createView();
        }

        ~VzRenderPath()
        {
            gEngine->destroy(view_);
        }

        uint64_t FRAMECOUNT = 0;

        float deltaTime = 0;
        float deltaTimeAccumulator = 0;
        float targetFrameRate = 60;
        bool frameSkip = true;
        vzm::Timer timer;

        int fpsAvgCounter = 0;
        float time;
        float timePrevious;


        // display all-time engine information text
        float deltatimes[20] = {};

        bool UpdateVzCamera(const VzCamera* _vzCam = nullptr)
        {            
            filament::Camera* camera = &view_->getCamera();

            if (_vzCam != nullptr)
            {
                vzCam_ = (VzCamera*)_vzCam;
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

        VzCamera* GetVzCamera()
        {
            return vzCam_;
        }

        filament::View* GetView() { return view_; }
    };

    class VzEngineApp
    {
    private:
        using SceneVID = VID;
        using CamVID = VID;
        using RenderableVID = VID;
        // archive
        // note a VzRenderPath involves a view that includes
        // 1. camera and 2. scene
        std::unordered_map<CamVID, VzRenderPath> renderPaths_;
        std::unordered_map<SceneVID, filament::Scene*> scenes_;

        std::unordered_multimap<SceneVID, CamVID> sceneCameraVids_;
        std::unordered_map<RenderableVID, SceneVID> renderableSceneVids_; // lights and renderable entities

        std::unordered_map<VID, std::unique_ptr<VzBaseComp>> vzComponents_;

    public:
        // Runtime can create a new entity with this
        inline SceneVID CreateSceneEntity(const std::string& name)
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
        inline VzRenderPath* CreateRendePath(const CamVID camVid, const SceneVID sceneVid)
        {
            auto it = renderPaths_.find(camVid);
            assert(it == renderPaths_.end());

            auto sit = scenes_.find(sceneVid);
            assert(sit != scenes_.end());
            Scene* scene = sit->second;

            // I organize the renderPath with filament::View that involves
            // 1. Camera (involving the camera component and the transform component)
            // 2. Scene
            VzRenderPath* renderPath = &renderPaths_[camVid];
            filament::View* view = renderPath->GetView();
            view->setScene(scene);

            utils::EntityManager& em = utils::EntityManager::get();
            utils::Entity camEntity = em.create();

            // 1. Instance const i = manager.addComponent(entity);
            // 2. tcm.create(entity); where FTransformManager& tcm = engine.getTransformManager();
            filament::Camera* camera = gEngine->createCamera(camEntity);
            camera->setExposure(16.0f, 1 / 125.0f, 100.0f); // default values used in filamentApp

            view->setCamera(camera); // attached to "view.scene"

            return renderPath;
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
            utils::Entity ett;
            ett.import(vid);
            return ncm.GetName(ett);
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
        inline VzRenderPath* GetFirstRenderPathByName(const std::string& name)
        {
            return GetRenderPath(GetFirstVidByName(name));
        }

        inline void AppendSceneEntityToParent(const VID vidSrc, const VID vidDst)
        {
            assert(vidSrc != vidDst);
            auto getSceneAndVid = [this](Scene** scene, const VID vid)
                {
                    SceneVID vidScene = vid;
                    *scene = GetScene(vidScene);
                    if (scene == nullptr)
                    {
                        auto it = renderableSceneVids_.find(vidScene);
                        if (it == renderableSceneVids_.end())
                        {
                            vidScene = INVALID_VID;
                        }
                        else
                        {
                            vidScene = it->second;
                            *scene = GetScene(vidScene);
                        }
                    }
                    return vidScene;
                };

            Scene* sceneSrc = nullptr;
            Scene* sceneDst = nullptr;
            SceneVID vidSceneSrc = getSceneAndVid(&sceneSrc, vidSrc);
            SceneVID vidSceneDst = getSceneAndVid(&sceneDst, vidDst);

            utils::Entity ettSrc, ettDst;
            ettSrc.import(vidSrc);
            ettDst.import(vidDst);
            //auto& em = gEngine->getEntityManager();
            auto& tcm = gEngine->getTransformManager();

            // case 1. both entities are renderable
            // case 2. src is scene and dst is renderable
            // case 3. src is renderable and dst is scene
            // case 4. both entities are scenes
            // note that renderable entity must have transform component!
            std::vector<utils::Entity> entitiesMoving;
            auto moveToRenderable = [&tcm, this](Scene* sceneSrc, Scene* sceneDst,
                std::vector<utils::Entity>& entitiesMoving,
                utils::Entity ettSrc, utils::Entity ettDst)
                {
                    auto insDst = tcm.getInstance(ettDst);
                    assert(insDst.asValue() != 0);
                    sceneSrc->forEach([&](utils::Entity ett) {
                        entitiesMoving.push_back(ett);

                        auto ins = tcm.getInstance(ett);
                        utils::Entity ettParent = tcm.getParent(ins);
                        if (ettParent.isNull())
                        {
                            tcm.setParent(ins, insDst);
                        }
                        });

                    if (sceneSrc && sceneSrc != sceneDst)
                    {
                        sceneSrc->remove(ettSrc);
                        renderableSceneVids_[ettSrc.getId()] = 0;
                    }
                };
            if (vidSrc != vidSceneSrc && vidDst != vidSceneDst)
            {
                // case 1. both entities are renderable
                auto insSrc = tcm.getInstance(ettSrc);
                auto insDst = tcm.getInstance(ettDst);
                assert(insSrc.asValue() != 0 && insDst.asValue() != 0);

                tcm.setParent(insSrc, insDst);
                if (sceneSrc && sceneSrc != sceneDst)
                {
                    sceneSrc->remove(ettSrc);
                    renderableSceneVids_[ettSrc.getId()] = 0;

                    entitiesMoving.push_back(ettSrc);

                    for (auto it = tcm.getChildrenBegin(insSrc); it != tcm.getChildrenEnd(insSrc); it++)
                    {
                        utils::Entity ett = tcm.getEntity(*it);
                        sceneSrc->remove(ett);
                        renderableSceneVids_[ett.getId()] = 0;

                        entitiesMoving.push_back(ett);
                    }
                }
            }
            else if (vidSrc == vidSceneSrc && vidDst != vidSceneDst)
            {
                // case 2. src is scene and dst is renderable
                moveToRenderable(sceneSrc, sceneDst, entitiesMoving, ettSrc, ettDst);
            }
            else if (vidSrc != vidSceneSrc && vidDst == vidSceneDst)
            {
                // case 3. src is renderable and dst is scene
                moveToRenderable(sceneDst, sceneSrc, entitiesMoving, ettDst, ettSrc);
            }
            else 
            {
                assert(vidSrc == vidSceneSrc && vidDst == vidSceneDst);
                assert(sceneSrc != sceneDst);
                // case 4. both entities are scenes
                sceneSrc->forEach([&](utils::Entity ett) {
                    entitiesMoving.push_back(ett);
                    });

                for (auto& it : entitiesMoving)
                {
                    sceneSrc->remove(it);
                    sceneDst->addEntity(it);
                    renderableSceneVids_[it.getId()] = vidSceneDst;
                }
                return;
            }

            if (sceneDst && sceneSrc != sceneDst)
            {
                for (auto& it : entitiesMoving)
                {
                    sceneDst->addEntity(it);
                    renderableSceneVids_[it.getId()] = vidSceneDst;
                }
            }
        }

        template <typename VZCOMP>
        inline VZCOMP* CreateVzComp(const VID vid)
        {
            auto it = vzComponents_.find(vid);
            assert(it == vzComponents_.end());

            std::string typeName = typeid(VZCOMP).name();
            COMPONENT_TYPE compType = vzcomptypes[typeName];
            if (compType == COMPONENT_TYPE::UNDEFINED)
            {
                return nullptr;
            }

            utils::Entity ett;
            ett.import(vid);

            auto& ncm = VzNameCompManager::Get();
            auto ins = ncm.getInstance(ett);
            if (ins.asValue() == 0)
            {
                return nullptr;
            }

            VZCOMP vzComp;
            vzComp.componentVID = vid;
            vzComp.compType = compType;
            vzComponents_.insert(std::make_pair(vid, std::make_unique<VZCOMP>(vzComp)));
            return (VZCOMP*)vzComponents_[vid].get();
        }

        template <typename VZCOMP>
        inline VZCOMP* GetVzComp(const VID vid)
        {
            auto it = vzComponents_.find(vid);
            if (it == vzComponents_.end())
            {
                return nullptr;
            }
            return (VZCOMP*)it->second.get();
        }
        template <typename COMP>
        inline COMP* GetEngineCompInstance(const VID vid)
        {
            std::string typeName = typeid(COMP).name();
            FCompType compType = comptypes[typeName];
            COMP* comp = nullptr;

            utils::Entity ett;
            ett.import(vid);

            switch (compType)
            {
            case FCompType::NameComponent:
            {
                auto& ncm = VzNameCompManager::Get();
                comp = ncm->getInstance(ett);
                break;
            }
            case FCompType::CameraComponent:
            {
                comp = gEngine->getCameraComponent(ett);
                break;
            }
            case FCompType::TransformComponent:
            {
                auto& tcm = gEngine->getTransformManager();
                comp = tcm->getInstance(ett);
                break;
            }
            case FCompType::LightComponent:
            {
                auto& lcm = gEngine->getLightManager();
                comp = lcm->getInstance(ett);
                break;
            }
            case FCompType::RenderableComponent:
            {
                auto& rcm = gEngine->getRenderableManager();
                comp = rcm->getInstance(ett);
                break;
            }
            default: assert(0 && "Not allowed GetEngineCompInstance");  return nullptr;
            }
            return comp;
        }

        inline void RemoveEntity(const VID vid)
        {
            utils::Entity ett;
            ett.import(vid);
            
            auto& ncm = VzNameCompManager::Get();
            ncm.RemoveEntity(ett);

            Scene* scene = GetScene(vid);
            if (scene)
            {
                // vid is SCeneVID
                gEngine->destroy(scene);

                // check if the views reset the scene.
                auto range = sceneCameraVids_.equal_range(vid);
                for (auto it = range.first; it != range.second; ++it) {
                    auto renderPath = GetRenderPath(it->second);
                    assert(renderPath);
                    renderPath->GetView()->setScene(nullptr);
                }

                scenes_.erase(vid);
                sceneCameraVids_.erase(vid);
            }
            else
            {
                auto& em = utils::EntityManager::get();
                
                if (em.isAlive(ett))
                {
                    em.destroy(ett);

                    // check the following //
                    vzComponents_.erase(vid);

                    if (renderPaths_.erase(vid) > 0)
                    {
                        // vid is CamVID
                        for (auto it = sceneCameraVids_.begin(); it != sceneCameraVids_.end(); )
                        {
                            if (it->second == vid)
                            {
                                it = sceneCameraVids_.erase(it);
                            }
                            else
                            {
                                it++;
                            }
                        }
                    }
                    //sceneCameraVids_.erase(
                    //    std::remove_if(sceneCameraVids_.begin(), sceneCameraVids_.end(),
                    //        [vid](const auto& pair) { return pair.second == vid; }),
                    //    sceneCameraVids_.end()
                    //);
                }
            }
        }
    };
#pragma endregion

    VzEngineApp gEngineApp;
}

#define COMP_GET(COMP, PARAM, RET) COMP* PARAM = gEngineApp.GetEngineComp<COMP>(componentVID); if (!PARAM) return RET;

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

    VZRESULT InitEngineLib(const vzm::ParamMap<std::string>& argument)
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

        // to do : gConfig and gEngineConfig
        // using vzm::ParamMap<std::string>& argument
        gConfig.backend = filament::Engine::Backend::VULKAN;
        gConfig.vulkanGPUHint = "0";
        gConfig.headless = true;
                
        gVulkanPlatform = new FilamentAppVulkanPlatform(gConfig.vulkanGPUHint.c_str());
        gEngine = Engine::Builder()
            .backend(gConfig.backend)
            .platform(gVulkanPlatform)
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

        VzNameCompManager& ncm = VzNameCompManager::Get();
        delete &ncm;

        //gEngine->destroy(gRenderer);
        gEngine->destroy(gDummySwapChain);
        gDummySwapChain = nullptr;
        Engine::destroy(&gEngine); // calls FEngine::shutdown()
        // note 
        // gEngine involves mJobSystem
        // when releasing gEngine, mJobSystem will be released!!
        // this calls JobSystem::requestExit() that including JobSystem::wakeAll()
        gEngine = nullptr;

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
            return INVALID_VID;
        }

        return gEngineApp.CreateSceneEntity(sceneName);
    }

    bool moveToParent(const utils::Entity ett, const utils::Entity ettParent, Scene* scene)
    {
        assert(ett != ettParent);

        auto& tcm = gEngine->getTransformManager();
        auto ins = tcm.getInstance(ett);
        auto insParent = tcm.getInstance(ettParent);
        if (ins.asValue() == 0 || insParent.asValue() == 0)
        {
            backlog::post("moveToParent >> invald entities", backlog::LogLevel::Error);
            return;
        }

        if (!ettParent.isNull())
        {

            // TO DO

            for (auto& entry : scene->componentLibrary.entries)
            {
                if (entry.second.component_manager->Contains(ettParent))
                {
                    scene->hierarchy.Create(ett).parentID = ettParent;
                    return true;
                }
            }
        }
        return false;
    }

    VID NewSceneComponent(const COMPONENT_TYPE compType, const VID sceneVid, const std::string& compName, const VID parentVid, VmBaseComponent** baseComp)
    {
        VzmScene* scene = gEngineApp.GetScene(sceneVid);
        if (scene == nullptr)
        {
            return INVALID_ENTITY;
        }
        Entity ett = INVALID_ENTITY;
        switch (compType)
        {
        case COMPONENT_TYPE::ACTOR:
        {
            ett = scene->Entity_CreateObject(compName);
            VmActor* vmActor = gEngineApp.CreateVmComp<VmActor>(ett);
            if (baseComp) *baseComp = vmActor;
            break;
        }
        case COMPONENT_TYPE::CAMERA:
        {
            ett = scene->Entity_CreateCamera(compName, CANVAS_INIT_W, CANVAS_INIT_H);
            VmCamera* vmCam = gEngineApp.CreateVmComp<VmCamera>(ett);

            CameraComponent* camComponent = scene->cameras.GetComponent(ett);
            VzmRenderer* renderer = gEngineApp.CreateRenderer(ett);
            renderer->scene = scene;
            assert(renderer->camera == camComponent);

            vmCam->compType = compType;
            vmCam->componentVID = ett;
            vmCam->renderer = (void*)renderer;
            renderer->UpdateVmCamera(vmCam);
            renderer->init(CANVAS_INIT_W, CANVAS_INIT_H, CANVAS_INIT_DPI);
            renderer->Load(); // Calls renderer->Start()
            if (baseComp) *baseComp = renderer->GetVmCamera();
            break;
        }
        case COMPONENT_TYPE::LIGHT:
        {
            ett = scene->Entity_CreateLight(compName);// , XMFLOAT3(0, 3, 0), XMFLOAT3(1, 1, 1), 2, 60);
            VmLight* vmLight = gEngineApp.CreateVmComp<VmLight>(ett);
            if (baseComp) *baseComp = vmLight;
            break;
        }
        case COMPONENT_TYPE::EMITTER:
        {
            ett = scene->Entity_CreateEmitter(compName);
            VmEmitter* vmEmitter = gEngineApp.CreateVmComp<VmEmitter>(ett);
            if (baseComp) *baseComp = vmEmitter;
            break;
        }
        case COMPONENT_TYPE::MATERIAL:
        {
            ett = scene->Entity_CreateMaterial(compName);
            VmMaterial* vmMat = gEngineApp.CreateVmComp<VmMaterial>(ett);
            if (baseComp) *baseComp = vmMat;
            break;
        }
        case COMPONENT_TYPE::COLLIDER:
        {
            ett = CreateEntity();
            scene->names.Create(ett) = compName;
            scene->colliders.Create(ett);
            scene->transforms.Create(ett);
            VmCollider* vmCollider = gEngineApp.CreateVmComp<VmCollider>(ett);
            if (baseComp) *baseComp = vmCollider;
            break;
        }
        case COMPONENT_TYPE::WEATHER:
        {
            ett = CreateEntity();
            scene->names.Create(ett) = compName;
            scene->weathers.Create(ett);
            VmWeather* vmWeather = gEngineApp.CreateVmComp<VmWeather>(ett);
            if (baseComp) *baseComp = vmWeather;
            break;
        }
        case COMPONENT_TYPE::ANIMATION:
        default:
            return INVALID_ENTITY;
        }
        moveToParent(ett, parentVid, scene);
        return ett;
    }

    VID AppendComponentTo(const VID vid, const VID parentVid)
    {
        HierarchyComponent* hierarchy = gEngineApp.GetEngineComp<HierarchyComponent>(vid);
        if (hierarchy)
        {
            if (hierarchy->parentID == parentVid)
            {
                auto scenes = gEngineApp.GetScenes();
                for (auto it = scenes->begin(); it != scenes->end(); it++)
                {
                    VzmScene* scene = &it->second;
                    if (scene->hierarchy.GetComponent(vid))
                    {
                        return scene->sceneVid;
                    }
                }
                assert("There must be a scene containing the hierarchy entity");
            }
        }

        auto scenes = gEngineApp.GetScenes();
        bool ret = false;
        for (auto it = scenes->begin(); it != scenes->end(); it++)
        {
            VzmScene* scene = &it->second;
            if (moveToParent(vid, parentVid, scene))
            {
                return scene->sceneVid;
            }
        }
        return INVALID_VID;
    }

    VmBaseComponent* GetComponent(const COMPONENT_TYPE compType, const VID vid)
    {
        switch (compType)
        {
        case COMPONENT_TYPE::BASE: return gEngineApp.GetVmComp<VmBaseComponent>(vid);
        case COMPONENT_TYPE::CAMERA: return gEngineApp.GetVmComp<VmCamera>(vid);
        case COMPONENT_TYPE::ACTOR: return gEngineApp.GetVmComp<VmActor>(vid);
        case COMPONENT_TYPE::LIGHT: return gEngineApp.GetVmComp<VmLight>(vid);
        case COMPONENT_TYPE::EMITTER: return gEngineApp.GetVmComp<VmEmitter>(vid);
        case COMPONENT_TYPE::WEATHER: return gEngineApp.GetVmComp<VmWeather>(vid);
        case COMPONENT_TYPE::ANIMATION: return gEngineApp.GetVmComp<VmAnimation>(vid);
        default: break;
        }
        return nullptr;
    }

    uint32_t GetSceneCompoenentVids(const COMPONENT_TYPE compType, const VID sceneVid, std::vector<VID>& vids)
    {
        Scene* scene = gEngineApp.GetScene(sceneVid);
        if (scene == nullptr)
        {
            return 0u;
        }
        switch (compType)
        {
        case COMPONENT_TYPE::CAMERA: vids = scene->cameras.GetEntityArray(); break;
        case COMPONENT_TYPE::ACTOR: vids = scene->objects.GetEntityArray(); break;
        case COMPONENT_TYPE::LIGHT: vids = scene->lights.GetEntityArray(); break;
        case COMPONENT_TYPE::EMITTER: vids = scene->emitters.GetEntityArray(); break;
        case COMPONENT_TYPE::WEATHER: vids = scene->weathers.GetEntityArray(); break;
        case COMPONENT_TYPE::ANIMATION: vids = scene->animations.GetEntityArray(); break;
        default: break;
        }
        return (uint32_t)vids.size();
    }

    VmWeather* GetSceneActivatedWeather(const VID sceneVid)
    {
        VzmScene* scene = gEngineApp.GetScene(sceneVid);
        if (scene == nullptr)
        {
            return nullptr;
        }
        return &scene->vmWeather;
    }

    void LoadFileIntoNewSceneAsync(const std::string& file, const std::string& rootName, const std::string& sceneName, const std::function<void(VID sceneVid, VID rootVid)>& callback)
    {
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
    }

    VID LoadFileIntoNewScene(const std::string& file, const std::string& rootName, const std::string& sceneName, VID* rootVid)
    {
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
    }

    VZRESULT MergeScenes(const VID srcSceneVid, const VID dstSceneVid)
    {
        Scene* srcScene = gEngineApp.GetScene(srcSceneVid);
        Scene* dstScene = gEngineApp.GetScene(dstSceneVid);
        if (srcScene == nullptr || dstScene == nullptr)
        {
            wi::backlog::post("Invalid Scene", wi::backlog::LogLevel::Error);
            return VZ_FAIL;
        }

        // base
        wi::vector<Entity> transformEntities = srcScene->transforms.GetEntityArray();

        // camera wirh renderer
        wi::vector<Entity> camEntities = srcScene->cameras.GetEntityArray();
        // actors
        wi::vector<Entity> aniEntities = srcScene->animations.GetEntityArray();
        wi::vector<Entity> objEntities = srcScene->objects.GetEntityArray();
        wi::vector<Entity> lightEntities = srcScene->lights.GetEntityArray();
        wi::vector<Entity> emitterEntities = srcScene->emitters.GetEntityArray();

        // resources
        wi::vector<Entity> meshEntities = srcScene->meshes.GetEntityArray();
        wi::vector<Entity> materialEntities = srcScene->materials.GetEntityArray();

        dstScene->Merge(*srcScene);

        for (Entity& ett : camEntities)
        {
            VmCamera* vmCam = gEngineApp.CreateVmComp<VmCamera>(ett);
            CameraComponent* camComponent = dstScene->cameras.GetComponent(ett);
            VzmRenderer* renderer = gEngineApp.CreateRenderer(ett);
            renderer->scene = dstScene;
            assert(renderer->camera == camComponent);

            vmCam->compType = COMPONENT_TYPE::CAMERA;
            vmCam->componentVID = ett;
            vmCam->renderer = (void*)renderer;
            renderer->UpdateVmCamera(vmCam);
            renderer->init(CANVAS_INIT_W, CANVAS_INIT_H, CANVAS_INIT_DPI);
            renderer->Load(); // Calls renderer->Start()
        }

        // actors
        {
            for (Entity& ett : aniEntities)
            {
                gEngineApp.CreateVmComp<VmAnimation>(ett);
            }
            for (Entity& ett : objEntities)
            {
                gEngineApp.CreateVmComp<VmActor>(ett);
            }
            for (Entity& ett : lightEntities)
            {
                gEngineApp.CreateVmComp<VmLight>(ett);
            }
            for (Entity& ett : emitterEntities)
            {
                gEngineApp.CreateVmComp<VmEmitter>(ett);
            }
        }

        // must be posterior to actors
        for (Entity& ett : transformEntities)
        {
            if (!gEngineApp.GetVmComp<VmBaseComponent>(ett))
            {
                gEngineApp.CreateVmComp<VmBaseComponent>(ett);
            }
        }

        // resources
        {
            for (Entity& ett : meshEntities)
            {
                gEngineApp.CreateVmComp<VmMesh>(ett);
            }
            for (Entity& ett : materialEntities)
            {
                gEngineApp.CreateVmComp<VmMaterial>(ett);
            }
        }
        //static Scene scene_resPool;
        //scene_resPool.meshes.Merge(dstScene->meshes);
        //scene_resPool.Update(0);
        //int gg = 0;
        return VZ_OK;
    }

    VZRESULT Render(const VID camVid, const bool updateScene)
    {
        VzmRenderer* renderer = gEngineApp.GetRenderer(camVid);
        if (renderer == nullptr)
        {
            return VZ_FAIL;
        }

        wi::font::UpdateAtlas(renderer->GetDPIScaling());

        renderer->UpdateVmCamera();

        // DOJO TO DO : CHECK updateScene across cameras belonging to a scene and force to use a oldest one...
        renderer->setSceneUpdateEnabled(updateScene || renderer->FRAMECOUNT == 0);
        if (!updateScene)
        {
            renderer->scene->camera = *renderer->camera;
        }

        if (!wi::initializer::IsInitializeFinished())
        {
            // Until engine is not loaded, present initialization screen...
            renderer->WaitRender();
            return VZ_JOB_WAIT;
        }

        if (profileFrameFinished)
        {
            profileFrameFinished = false;
            wi::profiler::BeginFrame();
        }

        VzmScene* scene = (VzmScene*)renderer->scene;

        {
            // for frame info.
            renderer->deltaTime = float(std::max(0.0, renderer->timer.record_elapsed_seconds()));
            const float target_deltaTime = 1.0f / renderer->targetFrameRate;
            if (renderer->framerate_lock && renderer->deltaTime < target_deltaTime)
            {
                wi::helper::QuickSleep((target_deltaTime - renderer->deltaTime) * 1000);
                renderer->deltaTime += float(std::max(0.0, renderer->timer.record_elapsed_seconds()));
            }

            scene->deltaTime = float(std::max(0.0, scene->timer.record_elapsed_seconds()));
        }


        //wi::input::Update(nullptr, *renderer);
        // Wake up the events that need to be executed on the main thread, in thread safe manner:
        wi::eventhandler::FireEvent(wi::eventhandler::EVENT_THREAD_SAFE_POINT, 0);
        renderer->fadeManager.Update(renderer->deltaTime);

        renderer->PreUpdate(); // current to previous

        // Fixed time update:
        {
            auto range = wi::profiler::BeginRangeCPU("Fixed Update");
            if (renderer->frameskip)
            {
                renderer->deltaTimeAccumulator += renderer->deltaTime;
                if (renderer->deltaTimeAccumulator > 10)
                {
                    // application probably lost control, fixed update would take too long
                    renderer->deltaTimeAccumulator = 0;
                }

                const float targetFrameRateInv = 1.0f / renderer->targetFrameRate;
                while (renderer->deltaTimeAccumulator >= targetFrameRateInv)
                {
                    renderer->FixedUpdate();
                    renderer->deltaTimeAccumulator -= targetFrameRateInv;
                }
            }
            else
            {
                renderer->FixedUpdate();
            }
            wi::profiler::EndRange(range); // Fixed Update
        }
        {
            // use scene->deltaTime
            auto range = wi::profiler::BeginRangeCPU("Update");
            wi::backlog::Update(*renderer, scene->deltaTime);
            renderer->Update(scene->deltaTime);
            renderer->PostUpdate();
            wi::profiler::EndRange(range); // Update

            // we ill use the separate framecount for each renderer (not global device)
            //
            renderer->FRAMECOUNT++;
            renderer->frameCB.frame_count = (uint)renderer->FRAMECOUNT;
            //renderer->frameCB.delta_time = renderer->deltaTime;
            // note here frameCB's time is computed based on the scene timeline
            //renderer->frameCB.time_previous = renderer->frameCB.time;
            //renderer->frameCB.time = scene->deltaTimeAccumulator;
        }

        {
            auto range = wi::profiler::BeginRangeCPU("Render");
            scene->dt = renderer->deltaTime;
            renderer->Render();
            wi::profiler::EndRange(range); // Render
        }
        renderer->RenderFinalize();

        return VZ_OK;
    }

    void ReloadShader()
    {
        wi::renderer::ReloadShaders();
    }

    VID DisplayEngineProfiling(const int w, const int h, const bool displayProfile, const bool displayEngineStates)
    {
        static bool isFirstCall = true;
        static VID sceneVid = gEngineApp.CreateSceneEntity("__VZM_ENGINE_INTERNAL__");
        VzmScene* sceneInternalState = gEngineApp.GetScene(sceneVid);
        static Entity canvasEtt = sceneInternalState->Entity_CreateCamera("INFO_CANVAS", w, h);
        static VzmRenderer* sysInfoRenderer = gEngineApp.CreateRenderer(canvasEtt);

        if (isFirstCall)
        {
            sysInfoRenderer->init(w, h, CANVAS_INIT_DPI);

            sysInfoRenderer->infoDisplay.active = true;
            sysInfoRenderer->infoDisplay.watermark = true;
            //sysInfoRenderer->infoDisplay.fpsinfo = true;
            //sysInfoRenderer->infoDisplay.resolution = true;
            //sysInfoRenderer->infoDisplay.colorspace = true;
            sysInfoRenderer->infoDisplay.device_name = true;
            sysInfoRenderer->infoDisplay.vram_usage = true;
            sysInfoRenderer->infoDisplay.heap_allocation_counter = true;

            sysInfoRenderer->DisplayProfile = true;
            wi::profiler::SetEnabled(true);

            {
                const float fadeSeconds = 0.f;
                wi::Color fadeColor = wi::Color(0, 0, 0, 255);
                // Fade manager will activate on fadeout
                sysInfoRenderer->fadeManager.Clear();
                sysInfoRenderer->fadeManager.Start(fadeSeconds, fadeColor, []() {
                    sysInfoRenderer->Start();
                    });

                sysInfoRenderer->fadeManager.Update(0); // If user calls ActivatePath without fadeout, it will be instant
            }
            isFirstCall = false;
        }

        sysInfoRenderer->camEntity = canvasEtt;
        sysInfoRenderer->width = w;
        sysInfoRenderer->height = h;
        sysInfoRenderer->UpdateVmCamera();

        sysInfoRenderer->setSceneUpdateEnabled(false);
        sysInfoRenderer->scene->camera = *sysInfoRenderer->camera;

        wi::font::UpdateAtlas(sysInfoRenderer->GetDPIScaling());

        if (!wi::initializer::IsInitializeFinished())
        {
            // Until engine is not loaded, present initialization screen...
            //sysInfoRenderer->WaitRender();
            return VZ_JOB_WAIT;
        }

        if (profileFrameFinished)
        {
            profileFrameFinished = false;
            wi::profiler::BeginFrame();
        }
        sysInfoRenderer->RenderFinalize(); // set profileFrameFinished to true inside

        return (VID)canvasEtt;
    }

    void* GetGraphicsSharedRenderTarget(const int camVid, const void* graphicsDev2, const void* srv_desc_heap2, const int descriptor_index, uint32_t* w, uint32_t* h)
    {
        VzmRenderer* renderer = gEngineApp.GetRenderer(camVid);
        if (renderer == nullptr)
        {
            return nullptr;
        }

        if (w) *w = renderer->width;
        if (h) *h = renderer->height;

        wi::graphics::GraphicsDevice* graphicsDevice = wi::graphics::GetDevice();
        if (graphicsDevice == nullptr) return nullptr;
        //return graphicsDevice->OpenSharedResource(graphicsDev2, const_cast<wi::graphics::Texture*>(&renderer->GetRenderResult()));
        //return graphicsDevice->OpenSharedResource(graphicsDev2, &renderer->rtPostprocess);
        return graphicsDevice->OpenSharedResource(graphicsDev2, srv_desc_heap2, descriptor_index, const_cast<wi::graphics::Texture*>(&renderer->renderResult));
    }

}
