#pragma once
#include "VzComponents.h"

namespace vzm
{
    // This must be called before using engine APIs
    //  - paired with DeinitEngineLib()
    __dojostatic VZRESULT InitEngineLib(const vzm::ParamMap<std::string>& arguments = vzm::ParamMap<std::string>());
    __dojostatic VZRESULT DeinitEngineLib();
    __dojostatic VZRESULT ReleaseWindowHandlerTasks(void* window);
    // Get Entity ID 
    //  - return zero in case of failure 
    __dojostatic VID GetFirstVidByName(const std::string& name);
    // Get Entity IDs whose name is the input name (VID is allowed for redundant name)
    //  - return # of entities
    __dojostatic size_t GetVidsByName(const std::string& name, std::vector<VID>& vids);
    // Get Entity's name if possible
    //  - return name string if entity's name exists, if not, return "" 
    __dojostatic bool GetNameByVid(const VID vid, std::string& name);
    // Remove an entity (scene, scene components, renderer, asset) 
    __dojostatic void RemoveComponent(const VID vid);
    // Create new scene and return scene (NOT a scene item) ID, a scene 
    //  - return zero in case of failure (the name is already registered or overflow VID)
    __dojostatic VzScene* NewScene(const std::string& sceneName);
    __dojostatic VzRenderer* NewRenderer(const std::string& sceneName);
    // Create new scene component (SCENE_COMPONENT_TYPE::CAMERA, ACTOR, LIGHT) NOT SCENE_COMPONENT_TYPE::SCENEBASE
    //  - Must belong to a scene
    //  - parentVid cannot be a scene (renderable or 0)
    //  - return zero in case of failure (invalid sceneID, the name is already registered, or overflow VID)
    __dojostatic VzSceneComp* NewSceneComponent(const SCENE_COMPONENT_TYPE compType, const std::string& compName, const VID parentVid = 0u);
    __dojostatic VzResource* NewResComponent(const RES_COMPONENT_TYPE compType, const std::string& compName);
    // Get Component and return its pointer registered in renderer
    //  - return nullptr in case of failure
    __dojostatic VzBaseComp* GetVzComponent(const VID vid);
    __dojostatic VzBaseComp* GetFirstVzComponentByName(const std::string& name);
    __dojostatic size_t GetVzComponentsByName(const std::string& name, std::vector<VzBaseComp*>& components);
    __dojostatic size_t GetVzComponentsByType(const std::string& type, std::vector<VzBaseComp*>& components);
    // Append Component to the parent component
    //  - return sceneId containing the parent component 
    __dojostatic VID AppendSceneCompVidTo(const VID vid, const VID parentVid);
    __dojostatic VzScene* AppendSceneCompTo(const VzBaseComp* comp, const VzBaseComp* parentComp);
    // Get Component IDs in a scene
    //  - return # of scene Components 
    __dojostatic size_t GetSceneCompoenentVids(const SCENE_COMPONENT_TYPE compType, const VID sceneVid, std::vector<VID>& vids, const bool isRenderableOnly = false);	// Get CameraParams and return its pointer registered in renderer
    // Load scene components into a new scene and return the actor ID
    //  - return zero in case of failure
    __dojostatic VzActor* LoadTestModelIntoActor(const std::string& modelName);
    // Load gltf components into a new scene and return the asset ID
    //  - the lifespan of resComponents follows that of the associated asset (vidAsset) and cannot be deleted by the client
    //  - return zero in case of failure
    __dojostatic VzAsset* LoadFileIntoAsset(const std::string& filename, const std::string& assetName);
    __dojostatic float GetAsyncLoadProgress();
    // Get a graphics render target view 
    //  - Must belong to the internal scene
    __dojostatic void* GetGraphicsSharedRenderTarget();
    // Reload shaders
    __dojostatic void ReloadShader();

    // Display Engine's states and profiling information
    //  - return canvas VID (use this as a camVid)
    __dojostatic VID DisplayEngineProfiling(const int w, const int h, const bool displayProfile = true, const bool displayEngineStates = true);
    
    __dojostatic void ExportAssetToGlb(const VzAsset* asset, const std::string& filename);
}
