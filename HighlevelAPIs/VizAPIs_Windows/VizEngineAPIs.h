#pragma once
#include "VizComponentAPIs.h"

#define CMPP(P) (vzm::VmBaseComponent**)&P

namespace vzm
{
    // This must be called before using engine APIs
    //  - paired with DeinitEngineLib()
    __dojostatic VZRESULT InitEngineLib(const vzm::ParamMap<std::string>& argument = vzm::ParamMap<std::string>());
    __dojostatic VZRESULT DeinitEngineLib();
    // Get Entity ID 
    //  - return zero in case of failure 
    __dojostatic VID GetFirstVidByName(const std::string& name);
    // Get Entity IDs whose name is the input name (VID is allowed for redundant name)
    //  - return # of entities
    __dojostatic size_t GetVidsByName(const std::string& name, std::vector<VID>& vids);
    // Get Entity's name if possible
    //  - return name string if entity's name exists, if not, return "" 
    __dojostatic bool GetNameByVid(const VID vid, std::string& name);
    // Remove an entity (scene, scene components, renderer) 
    __dojostatic void RemoveComponent(const VID vid);
    // Create new scene and return scene (NOT a scene item) ID, a scene 
    //  - return zero in case of failure (the name is already registered or overflow VID)
    __dojostatic VID NewScene(const std::string& sceneName);
    // Create new scene component (SCENE_COMPONENT_TYPE::CAMERA, ACTOR, LIGHT) NOT SCENE_COMPONENT_TYPE::SCENEBASE
    //  - Must belong to a scene
    //  - parentVid cannot be a scene (renderable or 0)
    //  - return zero in case of failure (invalid sceneID, the name is already registered, or overflow VID)
    __dojostatic VID NewSceneComponent(const SCENE_COMPONENT_TYPE compType, const VID sceneVid, const std::string& compName, const VID parentVid = 0u, VzSceneComp** sceneComp = nullptr);
    // Append Component to the parent component
    //  - return sceneId containing the parent component 
    __dojostatic VID AppendSceneComponentTo(const VID vid, const VID parentVid);
    // Get Component and return its pointer registered in renderer
    //  - return nullptr in case of failure
    __dojostatic VzSceneComp* GetSceneComponent(const SCENE_COMPONENT_TYPE compType, const VID vid);
    // Get Component IDs in a scene
    //  - return # of scene Components 
    __dojostatic size_t GetSceneCompoenentVids(const SCENE_COMPONENT_TYPE compType, const VID sceneVid, std::vector<VID>& vids);	// Get CameraParams and return its pointer registered in renderer
    /*
    // Load scene components into a new scene and return the scene ID
    //  - Must belong to the internal scene
    //  - return zero in case of failure
    __dojostatic VID LoadFileIntoNewScene(const std::string& file, const std::string& rootName, const std::string& sceneName = "", VID* rootVid = nullptr);
    // Async version of LoadFileIntoNewScene
    __dojostatic void LoadFileIntoNewSceneAsync(const std::string& file, const std::string& rootName, const std::string& sceneName = "", const std::function<void(VID sceneVid, VID rootVid)>& callback = nullptr);
    // Merge src scene to dest scene 
    //  - This is not THREAD-SAFE 
    __dojostatic VZRESULT MergeScenes(const VID srcSceneVid, const VID dstSceneVid);
    // Render a scene on camera (camVid)
    //  - Must belong to the internal scene
    //  - if updateScene is true, uses the camera for camera-dependent scene updates
    //  - strongly recommend a single camera-dependent update per a scene 
    __dojostatic VZRESULT Render(const VID camVid, const bool updateScene = true);
    // Get a graphics render target view 
    //  - Must belong to the internal scene
    __dojostatic void* GetGraphicsSharedRenderTarget(const int camVid, const void* device2, const void* srv_desc_heap2, const int descriptor_index, uint32_t* w = nullptr, uint32_t* h = nullptr);
    // Reload shaders
    __dojostatic void ReloadShader();

    // Display Engine's states and profiling information
    //  - return canvas VID (use this as a camVid)
    __dojostatic VID DisplayEngineProfiling(const int w, const int h, const bool displayProfile = true, const bool displayEngineStates = true);
    /**/
}
