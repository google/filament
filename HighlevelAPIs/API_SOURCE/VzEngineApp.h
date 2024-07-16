#ifndef VZENGINEAPP_H
#define VZENGINEAPP_H
#include "VzComponents.h"

using SceneVID = VID;
using RendererVID = VID;
using CamVID = VID;
using ActorVID = VID;
using LightVID = VID;
using GeometryVID = VID;
using MaterialVID = VID;
using MInstanceVID = VID;
using MaterialVID = VID;
using AssetVID = VID;
using SkeletonVID = VID;
using BoneVID = VID;


namespace vzm::backlog
{
    enum class LogLevel
    {
        None,
        Default,
        Warning,
        Error,
    };

    void post(const std::string& input, LogLevel level);
}

// internal helpers
namespace vzm
{
    void getDescendants(const VID vid, std::vector<VID>& decendants);
    void cubeToScene(const VID vidCubeRenderable, const VID vidCube);
}

class IBL;
class Cube;
class CameraManipulator;
namespace filament {
    class Camera;
    namespace gltfio {
        class VzAssetLoader;
    }
}

// component contents
namespace vzm
{
    struct GltfIO;

    struct VzSceneRes
    {
    private:
        IBL* ibl_ = nullptr;
        Cube* lightmapCube_ = nullptr; // note current filament supports only one directional light's shadowmap
    public:
        VzSceneRes();
        ~VzSceneRes();
        void Destory();
        IBL* GetIBL();
        IBL* NewIBL();
        Cube* GetLightmapCube();
    };
    struct VzCameraRes
    {
    private:
        filament::Camera* camera_ = nullptr;
        Cube* cameraCube_ = nullptr;
        VzCamera::Controller camController_ = VzCamera::Controller(0);
        std::unique_ptr<CameraManipulator> cameraManipulator_;
    public:
        VzCameraRes() = default;
        ~VzCameraRes();

        uint64_t FRAMECOUNT = 0;
        TimeStamp timer;
        float deltaTime = 0;
        float deltaTimeAccumulator = 0;

        void SetCamera(Camera* camera);
        Camera* GetCamera();
        Cube* GetCameraCube();
        void NewCameraManipulator(const VzCamera::Controller& camController);
        VzCamera::Controller* GetCameraController();
        CameraManipulator* GetCameraManipulator();
        void UpdateCameraWithCM(float deltaTime);
    };
    struct VzActorRes
    {
    private:
        GeometryVID vidGeo_ = INVALID_VID;
        std::vector<MInstanceVID> vidMIs_;
        std::vector<std::vector<MInstanceVID>> vidMIVariants_;
    public:
        void SetGeometry(const GeometryVID vid);
        void SetMIs(const std::vector<MInstanceVID>& vidMIs);
        bool SetMI(const MInstanceVID vid, const int slot);
        void SetMIVariants(const std::vector<std::vector<MInstanceVID>>& vidMIVariants);
        GeometryVID GetGeometryVid();
        MInstanceVID GetMIVid(const int slot);
        std::vector<MInstanceVID>& GetMIVids();
        std::vector<std::vector<MInstanceVID>>& GetMIVariants();
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

        void Set(const std::vector<Primitive>& primitives);
        void SetTypes(const std::vector<RenderableManager::PrimitiveType>& primitiveTypes);
        std::vector<filament::gltfio::Primitive>* Get();
        std::vector<RenderableManager::PrimitiveType>* GetTypes();

        ~VzGeometryRes();
    };
    struct VzMaterialRes
    {
        bool isSystem = false;
        gltfio::FilamentAsset* assetOwner = nullptr; // has ownership
        filament::Material* material = nullptr;
        ~VzMaterialRes();
    };
    struct VzMIRes
    {
        bool isSystem = false;
        gltfio::FilamentAsset* assetOwner = nullptr; // has ownership
        filament::MaterialInstance* mi = nullptr;
        ~VzMIRes();
    };

    struct VzAssetRes
    {
        gltfio::FilamentAsset* asset = nullptr;
        std::vector<VID> rootVIDs;
        std::set<VID> assetOwnershipComponents;
        std::vector<SkeletonVID> skeletons;

        VzAsset::Animator animator = VzAsset::Animator(0);
    };
    struct VzSkeletonRes
    {
        std::unordered_map<BoneVID, std::string> bones;
    };
}

namespace vzm
{
    class VzEngineApp
    {
    private:
        std::unordered_map<SceneVID, Scene*> scenes_;
        std::unordered_map<SceneVID, std::unique_ptr<VzSceneRes>> sceneResMaps_;
        // note a VzRenderPath involves a filament::view that includes
        // 1. filament::camera and 2. filament::scene
        std::unordered_map<CamVID, SceneVID> camSceneVids_;
        std::unordered_map<CamVID, std::unique_ptr<VzCameraRes>> camResMaps_;
        std::unordered_map<ActorVID, SceneVID> actorSceneVids_;
        std::unordered_map<ActorVID, std::unique_ptr<VzActorRes>> actorResMaps_; // consider when removing resources...
        std::unordered_map<LightVID, SceneVID> lightSceneVids_;
        std::unordered_map<LightVID, std::unique_ptr<VzLightRes>> lightResMaps_;

        std::unordered_map<RendererVID, std::unique_ptr<VzRenderPath>> renderPaths_;

        // Resources (ownership check!)
        std::unordered_map<GeometryVID, std::unique_ptr<VzGeometryRes>> geometries_;
        std::unordered_map<MaterialVID, std::unique_ptr<VzMaterialRes>> materials_;
        std::unordered_map<MInstanceVID, std::unique_ptr<VzMIRes>> materialInstances_;

        // GLTF Asset
        std::unordered_map<AssetVID, std::unique_ptr<VzAssetRes>> assetResMaps_;
        std::unordered_map<SkeletonVID, std::unique_ptr<VzSkeletonRes>> skeletonResMaps_;

        std::unordered_map<VID, std::unique_ptr<VzBaseComp>> vzComponents_;

        bool removeScene(SceneVID vidScene);

    public:
        // Runtime can create a new entity with this
        SceneVID CreateScene(const std::string& name);
        RendererVID CreateRenderPath(const std::string& name);
        AssetVID CreateAsset(const std::string& name);
        SkeletonVID CreateSkeleton(const std::string& name, const SkeletonVID vidExist = 0);
        size_t GetVidsByName(const std::string& name, std::vector<VID>& vids);
        VID GetFirstVidByName(const std::string& name);
        std::string GetNameByVid(const VID vid);
        bool HasComponent(const VID vid);
        bool IsRenderable(const ActorVID vid);
        bool IsSceneComponent(VID vid);
        bool IsLight(const LightVID vid);
        Scene* GetScene(const SceneVID vid);
        Scene* GetFirstSceneByName(const std::string& name);
        std::unordered_map<SceneVID, Scene*>* GetScenes();

        VzSceneRes* GetSceneRes(const SceneVID vid);
        VzRenderPath* GetRenderPath(const RendererVID vid);
        VzCameraRes* GetCameraRes(const CamVID vid);
        VzActorRes* GetActorRes(const ActorVID vid);
        VzLightRes* GetLightRes(const LightVID vid);
        VzAssetRes* GetAssetRes(const AssetVID vid);
        VzSkeletonRes* GetSkeletonRes(const SkeletonVID vid);
        AssetVID GetAssetOwner(VID vid)
        {
            for (auto& it : assetResMaps)
            {
                VzAssetRes& asset_res = *it.second.get();
                if (asset_res.assetOwnershipComponents.contains(vid))
                {
                    return it.first;
                }
            }
            return INVALID_VID;
        }

        size_t GetCameraVids(std::vector<CamVID>& camVids);
        size_t GetActorVids(std::vector<ActorVID>& actorVids);
        size_t GetLightVids(std::vector<LightVID>& lightVids);
        size_t GetRenderPathVids(std::vector<RendererVID>& renderPathVids);
        VzRenderPath* GetFirstRenderPathByName(const std::string& name);
        SceneVID GetSceneVidBelongTo(const VID vid);

        bool AppendSceneEntityToParent(const VID vidSrc, const VID vidDst);

        VzSceneComp* CreateSceneComponent(const SCENE_COMPONENT_TYPE compType,
            const std::string& name, const VID vidExist = 0);
        VzActor* CreateTestActor(const std::string& modelName = "MONKEY_SUZANNE_DATA");
        VzGeometry* CreateGeometry(const std::string& name,
            const std::vector<filament::gltfio::Primitive>& primitives,
            const filament::gltfio::FilamentAsset* assetOwner = nullptr,
            const bool isSystem = false);
        VzMaterial* CreateMaterial(const std::string& name,
            const Material* material = nullptr,
            const filament::gltfio::FilamentAsset* assetOwner = nullptr,
            const bool isSystem = false);
        VzMI* CreateMaterialInstance(const std::string& name,
            const MaterialInstance* mi = nullptr,
            const filament::gltfio::FilamentAsset* assetOwner = nullptr,
            const bool isSystem = false);

        void BuildRenderable(const ActorVID vid);

        VzGeometryRes* GetGeometryRes(const GeometryVID vidGeo);
        VzMaterialRes* GetMaterialRes(const MaterialVID vidMaterial);
        MaterialVID FindMaterialVID(const filament::Material* mat);

        VzMIRes* GetMIRes(const MInstanceVID vidMI);
        MInstanceVID FindMaterialInstanceVID(const filament::MaterialInstance* mi);

        template <typename VZCOMP>
        VZCOMP* GetVzComponent(const VID vid)
        {
            auto it = vzComponents_.find(vid);
            if (it == vzComponents_.end())
            {
                return nullptr;
            }
            return (VZCOMP*)it->second.get();
        }

        VzAssetLoader* GetGltfAssetLoader();
        ResourceLoader* GetGltfResourceLoader();

        bool RemoveComponent(const VID vid, const bool forceToRemove = false);

        void CancelAyncLoad();
        void Initialize();
        void Destroy();
    };
}

#define COMP_NAME(COMP, ENTITY, FAILRET) auto& COMP = VzNameCompManager::Get(); Entity ENTITY = Entity::import(componentVID); if (ENTITY.isNull()) return FAILRET;
#define COMP_TRANSFORM(COMP, ENTITY, INS, FAILRET)  auto & COMP = gEngine->getTransformManager(); Entity ENTITY = Entity::import(componentVID); if (ENTITY.isNull()) return FAILRET; auto INS = COMP.getInstance(ENTITY);
#define COMP_RENDERPATH(RENDERPATH, FAILRET)  VzRenderPath* RENDERPATH = gEngineApp.GetRenderPath(componentVID); if (RENDERPATH == nullptr) return FAILRET;
#define COMP_LIGHT(COMP, ENTITY, INS, FAILRET)  auto & COMP = gEngine->getLightManager(); Entity ENTITY = Entity::import(componentVID); if (ENTITY.isNull()) return FAILRET; auto INS = COMP.getInstance(ENTITY);
#define COMP_ACTOR(COMP, ENTITY, INS, FAILRET)  auto & COMP = gEngine->getRenderableManager(); Entity ENTITY = Entity::import(componentVID); if (ENTITY.isNull()) return FAILRET; auto INS = COMP.getInstance(ENTITY);
#define COMP_MI(COMP, FAILRET) VzMIRes* mi_res = gEngineApp.GetMIRes(componentVID); if (mi_res == nullptr) return FAILRET; MaterialInstance* COMP = mi_res->mi; if (COMP == nullptr) return FAILRET;
#define COMP_CAMERA(COMP, ENTITY, FAILRET) Entity ENTITY = Entity::import(componentVID); Camera* COMP = gEngine->getCameraComponent(ENTITY); if (COMP == nullptr) return;
#define COMP_ASSET(COMP, FAILRET)  VzAssetRes* COMP = gEngineApp.GetAssetRes(componentVID); assert(COMP->asset->getAssetInstanceCount() == 1); // later... for multi-instance cases

using namespace vzm;
#endif
