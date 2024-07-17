#ifndef VZENGINEAPP_H
#define VZENGINEAPP_H
#include "VzComponents.h"

#include "filament/VertexBuffer.h"
#include "filament/IndexBuffer.h"
#include "filament/MorphTargetBuffer.h"

#include "camutils/Manipulator.h"
#include "filament/Box.h"

#include "gltfio/MaterialProvider.h"
#include "gltfio/FilamentAsset.h"
#include "gltfio/ResourceLoader.h"

#include <array>

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

namespace filament::gltfio {
    struct VzAssetLoader;
}

class IBL;
class Cube;
// component contents
namespace vzm
{
    enum class PrimitiveType : uint8_t {
        // don't change the enums values (made to match GL)
        POINTS = 0,    //!< points
        LINES = 1,    //!< lines
        LINE_STRIP = 3,    //!< line strip
        TRIANGLES = 4,    //!< triangles
        TRIANGLE_STRIP = 5     //!< triangle strip
    };

    using namespace filament;
    using namespace filament::gltfio;

    using CameraManipulator = filament::camutils::Manipulator<float>;

    struct GltfIO;

    struct VzPrimitive {
        VertexBuffer* vertices = nullptr;
        IndexBuffer* indices = nullptr;
        Aabb aabb; // object-space bounding box
        UvMap uvmap; // mapping from each glTF UV set to either UV0 or UV1 (8 bytes)
        MorphTargetBuffer* morphTargetBuffer = nullptr;
        uint32_t morphTargetOffset;
        std::vector<int> slotIndices;

        PrimitiveType ptype = PrimitiveType::TRIANGLES;
    };

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
        CameraManipulator* cameraManipulator_;
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

        std::vector<VzPrimitive> primitives_;
    public:
        bool isSystem = false;
        gltfio::FilamentAsset* assetOwner = nullptr; // has ownership
        filament::Aabb aabb;
        void Set(const std::vector<VzPrimitive>& primitives);
        std::vector<VzPrimitive>* Get();

        ~VzGeometryRes();
    };
    struct VzMaterialRes
    {
        bool isSystem = false;
        gltfio::FilamentAsset* assetOwner = nullptr; // has ownership
        Material* material = nullptr;
        ~VzMaterialRes();
    };
    struct VzMIRes
    {
        bool isSystem = false;
        gltfio::FilamentAsset* assetOwner = nullptr; // has ownership
        MaterialInstance* mi = nullptr;
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
    using namespace filament;

    class VzRenderPath;

    class VzEngineApp
    {
    private:
        std::unordered_map<SceneVID, Scene*> scenes_;
        std::unordered_map<SceneVID, std::unique_ptr<VzSceneRes>> sceneResMap_;
        // note a VzRenderPath involves a filament::view that includes
        // 1. filament::camera and 2. filament::scene
        std::unordered_map<CamVID, SceneVID> camSceneMap_;
        std::unordered_map<CamVID, std::unique_ptr<VzCameraRes>> camResMap_;
        std::unordered_map<ActorVID, SceneVID> actorSceneMap_;
        std::unordered_map<ActorVID, std::unique_ptr<VzActorRes>> actorResMap_; // consider when removing resources...
        std::unordered_map<LightVID, SceneVID> lightSceneMap_;
        std::unordered_map<LightVID, std::unique_ptr<VzLightRes>> lightResMap_;

        std::unordered_map<RendererVID, std::unique_ptr<VzRenderPath>> renderPathMap_;

        // Resources (ownership check!)
        std::unordered_map<GeometryVID, std::unique_ptr<VzGeometryRes>> geometryResMap_;
        std::unordered_map<MaterialVID, std::unique_ptr<VzMaterialRes>> materialResMap_;
        std::unordered_map<MInstanceVID, std::unique_ptr<VzMIRes>> miResMap_;

        // GLTF Asset
        std::unordered_map<AssetVID, std::unique_ptr<VzAssetRes>> assetResMap_;
        std::unordered_map<SkeletonVID, std::unique_ptr<VzSkeletonRes>> skeletonResMap_;

        std::unordered_map<VID, std::unique_ptr<VzBaseComp>> vzCompMap_;

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
        std::unordered_map<AssetVID, std::unique_ptr<VzAssetRes>>* GetAssetResMap() {
            return &assetResMap_;
        }
        VzSkeletonRes* GetSkeletonRes(const SkeletonVID vid);
        AssetVID GetAssetOwner(VID vid);

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
            const std::vector<VzPrimitive>& primitives,
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
            auto it = vzCompMap_.find(vid);
            if (it == vzCompMap_.end())
            {
                return nullptr;
            }
            return (VZCOMP*)it->second.get();
        }

        gltfio::VzAssetLoader* GetGltfAssetLoader();
        gltfio::ResourceLoader* GetGltfResourceLoader();

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
