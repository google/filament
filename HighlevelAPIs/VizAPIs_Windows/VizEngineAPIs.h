#pragma once
#include "VizComponentAPIs.h"

#define SCPP(P) (vzm::VzSceneComp**)&P

// user32.lib;gdi32.lib;opengl32.lib;backend.lib;basis_transcoder.lib;bluegl.lib;bluevk.lib;camutils.lib;civetweb.lib;dracodec.lib;filabridge.lib;filaflat.lib;filamat.lib;filament-iblprefilter.lib;filament.lib;filameshio.lib;geometry.lib;gltfio.lib;gltfio_core.lib;ibl-lite.lib;ibl.lib;image.lib;ktxreader.lib;matdbg.lib;meshoptimizer.lib;mikktspace.lib;shaders.lib;smol-v.lib;stb.lib;uberarchive.lib;uberzlib.lib;utils.lib;viewer.lib;vkshaders.lib;zstd.lib;../../VisualStudio/samples/Debug/suzanne-resources.lib;../../VisualStudio/samples/Debug/sample-resources.lib;%(AdditionalDependencies)

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
    __dojostatic VID NewScene(const std::string& sceneName, VzScene** scene = nullptr);
    __dojostatic VID NewRenderer(const std::string& sceneName, VzRenderer** renderer = nullptr);
    // Create new scene component (SCENE_COMPONENT_TYPE::CAMERA, ACTOR, LIGHT) NOT SCENE_COMPONENT_TYPE::SCENEBASE
    //  - Must belong to a scene
    //  - parentVid cannot be a scene (renderable or 0)
    //  - return zero in case of failure (invalid sceneID, the name is already registered, or overflow VID)
    __dojostatic VID NewSceneComponent(const SCENE_COMPONENT_TYPE compType, const std::string& compName, const VID parentVid = 0u, VzSceneComp** sceneComp = nullptr);
    // Append Component to the parent component
    //  - return sceneId containing the parent component 
    __dojostatic VID AppendSceneComponentTo(const VID vid, const VID parentVid);
    // Reveal asset's Components by appending the parent component
    //  - return sceneId containing the parent component 
    __dojostatic VID AppendAssetTo(const VID vidAsset, const VID parentVid, std::vector<VID>& resComponents);
    // Get Component and return its pointer registered in renderer
    //  - return nullptr in case of failure
    __dojostatic VzBaseComp* GetVzComponent(const VID vid);
    // Get Component IDs in a scene
    //  - return # of scene Components 
    __dojostatic size_t GetSceneCompoenentVids(const SCENE_COMPONENT_TYPE compType, const VID sceneVid, std::vector<VID>& vids, const bool isRenderableOnly = false);	// Get CameraParams and return its pointer registered in renderer
    // Load scene components into a new scene and return the actor ID
    //  - return zero in case of failure
    __dojostatic VID LoadTestModelIntoActor(const std::string& modelName);
    // Load gltf components into a new scene and return the asset ID
    //  - the lifespan of resComponents follows that of the associated asset (vidAsset) and cannot be deleted by the client
    //  - return zero in case of failure
    __dojostatic VID LoadFileIntoAsset(const std::string& filename, const std::string& assetName, std::vector<VID>* resComponents = nullptr);
    __dojostatic float GetAsyncLoadProgress();
    // Get a graphics render target view 
    //  - Must belong to the internal scene
    __dojostatic void* GetGraphicsSharedRenderTarget(const int vidCam, const void* device2, const void* srv_desc_heap2, const int descriptor_index, uint32_t* w = nullptr, uint32_t* h = nullptr);
    // Reload shaders
    __dojostatic void ReloadShader();

    // Display Engine's states and profiling information
    //  - return canvas VID (use this as a camVid)
    __dojostatic VID DisplayEngineProfiling(const int w, const int h, const bool displayProfile = true, const bool displayEngineStates = true);
    
}
