#include "VzScene.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
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

        cubeToScene(light_cube->getSolidRenderable().getId(), componentVID);
        cubeToScene(light_cube->getWireFrameRenderable().getId(), componentVID);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
}
