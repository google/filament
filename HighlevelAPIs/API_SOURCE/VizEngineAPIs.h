#pragma once
#include "VzComponents.h"

namespace vzm
{
    // This must be called before using engine APIs
    //  - paired with DeinitEngineLib()
    extern "C" API_EXPORT VZRESULT InitEngineLib(const vzm::ParamMap<std::string>& arguments = vzm::ParamMap<std::string>());
    extern "C" API_EXPORT VZRESULT DeinitEngineLib();
    extern "C" API_EXPORT VZRESULT ReleaseWindowHandlerTasks(void* window);
    // Get Entity ID 
    //  - return zero in case of failure 
    extern "C" API_EXPORT VID GetFirstVidByName(const std::string& name);
    // Get Entity IDs whose name is the input name (VID is allowed for redundant name)
    //  - return # of entities
    extern "C" API_EXPORT size_t GetVidsByName(const std::string& name, std::vector<VID>& vids);
    // Get Entity's name if possible
    //  - return name string if entity's name exists, if not, return "" 
    extern "C" API_EXPORT bool GetNameByVid(const VID vid, std::string& name);
    // Remove an entity (scene, scene components, renderer, asset) 
    extern "C" API_EXPORT void RemoveComponent(const VID vid);
    // Create new scene and return scene (NOT a scene item) ID, a scene 
    //  - return zero in case of failure (the name is already registered or overflow VID)
    extern "C" API_EXPORT VzScene* NewScene(const std::string& sceneName);
    extern "C" API_EXPORT VzRenderer* NewRenderer(const std::string& sceneName);
    // Create new scene component (SCENE_COMPONENT_TYPE::CAMERA, ACTOR, LIGHT) NOT SCENE_COMPONENT_TYPE::SCENEBASE
    //  - Must belong to a scene
    //  - parentVid cannot be a scene (renderable or 0)
    //  - return zero in case of failure (invalid sceneID, the name is already registered, or overflow VID)
    extern "C" API_EXPORT VzSceneComp* NewSceneComponent(const SCENE_COMPONENT_TYPE compType, const std::string& compName, const VID parentVid = 0u);
    extern "C" API_EXPORT VzResource* NewResComponent(const RES_COMPONENT_TYPE compType, const std::string& compName);
    // Get Component and return its pointer registered in renderer
    //  - return nullptr in case of failure
    extern "C" API_EXPORT VzBaseComp* GetVzComponent(const VID vid);
    extern "C" API_EXPORT VzBaseComp* GetFirstVzComponentByName(const std::string& name);
    extern "C" API_EXPORT size_t GetVzComponentsByName(const std::string& name, std::vector<VzBaseComp*>& components);
    extern "C" API_EXPORT size_t GetVzComponentsByType(const std::string& type, std::vector<VzBaseComp*>& components);
    // Append Component to the parent component
    //  - return sceneId containing the parent component 
    extern "C" API_EXPORT VID AppendSceneCompVidTo(const VID vid, const VID parentVid);
    extern "C" API_EXPORT VzScene* AppendSceneCompTo(const VzBaseComp* comp, const VzBaseComp* parentComp);
    // Get Component IDs in a scene
    //  - return # of scene Components 
    extern "C" API_EXPORT size_t GetSceneCompoenentVids(const SCENE_COMPONENT_TYPE compType, const VID sceneVid, std::vector<VID>& vids, const bool isRenderableOnly = false);	// Get CameraParams and return its pointer registered in renderer
    // Load a system actor and return the actor
    //  - return zero in case of failure
    extern "C" API_EXPORT VzActor* LoadTestModelIntoActor(const std::string& modelName);
    // Load a mesh file (obj and stl) into actors and return the first actor
    //  - return zero in case of failure
    extern "C" API_EXPORT VzActor* LoadModelFileIntoActors(const std::string& filename, std::vector<VzActor*>& actors);
    // Load gltf components into a new scene and return the asset ID
    //  - the lifespan of resComponents follows that of the associated asset (vidAsset) and cannot be deleted by the client
    //  - return zero in case of failure
    extern "C" API_EXPORT VzAsset* LoadFileIntoAsset(const std::string& filename, const std::string& assetName);
    extern "C" API_EXPORT float GetAsyncLoadProgress();
    // Get a graphics render target view 
    //  - Must belong to the internal scene
    extern "C" API_EXPORT void* GetGraphicsSharedRenderTarget();
    // Reload shaders
    extern "C" API_EXPORT void ReloadShader();

    // Display Engine's states and profiling information
    //  - return canvas VID (use this as a camVid)
    extern "C" API_EXPORT VID DisplayEngineProfiling(const int w, const int h, const bool displayProfile = true, const bool displayEngineStates = true);
    
    extern "C" API_EXPORT void ExportAssetToGlb(const VzAsset* asset, const std::string& filename);
}
