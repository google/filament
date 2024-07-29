#include "VzScene.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
    std::vector<VID> VzScene::GetSceneCompChildren()
    {
        std::vector<VID> children_vids;
        gEngineApp.GetSceneCompChildren(GetVID(), children_vids);
        return children_vids;
    }
    bool VzScene::LoadIBL(const std::string& path)
    {
        VzSceneRes* scene_res = gEngineApp.GetSceneRes(GetVID());
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
        Scene* scene = gEngineApp.GetScene(GetVID());
        assert(scene);
        scene->setSkybox(ibl->getSkybox());
        scene->setIndirectLight(ibl->getIndirectLight());
        UpdateTimeStamp();
        return true;
    }
    float VzScene::GetIBLIntensity()
    {
        VzSceneRes* scene_res = gEngineApp.GetSceneRes(GetVID());
        IBL* ibl = scene_res->GetIBL();
        return ibl->getIndirectLight()->getIntensity();
    }
    float VzScene::GetIBLRotation()
    {
        VzSceneRes* scene_res = gEngineApp.GetSceneRes(GetVID());
        IBL* ibl = scene_res->GetIBL();
        return iblRotation_;
    }
    void VzScene::SetIBLIntensity(float intensity)
    {
        VzSceneRes* scene_res = gEngineApp.GetSceneRes(GetVID());
        IBL* ibl = scene_res->GetIBL();
        ibl->getIndirectLight()->setIntensity(intensity);
        UpdateTimeStamp();
    }
    void VzScene::SetIBLRotation(float rotation)
    {
        VzSceneRes* scene_res = gEngineApp.GetSceneRes(GetVID());
        IBL* ibl = scene_res->GetIBL();
        ibl->getIndirectLight()->setRotation(math::mat3f::rotation(rotation, math::float3{0, 1, 0}));
        iblRotation_ = rotation;
        UpdateTimeStamp();
    }
    void VzScene::SetSkyboxVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        VzSceneRes* scene_res = gEngineApp.GetSceneRes(GetVID());
        IBL* ibl = scene_res->GetIBL();
        ibl->getSkybox()->setLayerMask(layerBits, maskBits);
        UpdateTimeStamp();
    }
    void VzScene::SetLightmapVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        VzSceneRes* scene_res = gEngineApp.GetSceneRes(GetVID());
        // create once
        Cube* light_cube = scene_res->GetLightmapCube();

        auto& rcm = gEngine->getRenderableManager();
        rcm.setLayerMask(rcm.getInstance(light_cube->getSolidRenderable()), layerBits, maskBits);
        rcm.setLayerMask(rcm.getInstance(light_cube->getWireFrameRenderable()), layerBits, maskBits);

        cubeToScene(light_cube->getSolidRenderable().getId(), GetVID());
        cubeToScene(light_cube->getWireFrameRenderable().getId(), GetVID());
        UpdateTimeStamp();
    }
}
