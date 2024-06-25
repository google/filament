#pragma once
#define __dojostatic extern "C" __declspec(dllexport)
#define __dojoclass class __declspec(dllexport)
#define __dojostruct struct __declspec(dllexport)

#define __FP (float*)&
#define VZRESULT int
#define VZ_OK 0
#define VZ_FAIL 1
#define VZ_JOB_WAIT 2
#define VZ_WARNNING 3

// std
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <any>
#include <list>
#include <memory>
#include <algorithm>
#include <chrono>
#include <functional>


using VID = uint32_t;
inline constexpr VID INVALID_VID = 0;
using TimeStamp = std::chrono::high_resolution_clock::time_point;

constexpr float VZ_PI = 3.141592654f;
constexpr float VZ_2PI = 6.283185307f;
constexpr float VZ_1DIVPI = 0.318309886f;
constexpr float VZ_1DIV2PI = 0.159154943f;
constexpr float VZ_PIDIV2 = 1.570796327f;
constexpr float VZ_PIDIV4 = 0.785398163f;

using uint = uint32_t;

// DOJO TO DO (or suggestion)
// 1. separate mesh and material (mesh to geometry... and material is an option for objectcomponent)
// 2. set up a resource pool (or a scene for this?! for the geometry anbd material, animations,....)

namespace vzm
{
    __dojostatic inline void TransformPoint(const float posSrc[3], const float mat[16], float posDst[3]);
    __dojostatic inline void TransformVector(const float vecSrc[3], const float mat[16], float vecDst[3]);
    __dojostatic inline void ComputeBoxTransformMatrix(const float cubeScale[3], const float posCenter[3],
        const float yAxis[3], const float zAxis[3], float mat[16], float matInv[16]);

    template <typename ID> struct ParamMap {
    private:
        std::string __PM_VERSION = "LIBI_1.4";
        std::unordered_map<ID, std::any> __params;
    public:
        bool FindParam(const ID& param_name) {
            auto it = __params.find(param_name);
            return !(it == __params.end());
        }
        template <typename SRCV> bool GetParamCheck(const ID& key, SRCV& param) {
            auto it = __params.find(key);
            if (it == __params.end()) return false;
            param = std::any_cast<SRCV&>(it->second);
            return true;
        }
        template <typename SRCV> SRCV GetParam(const ID& key, const SRCV& init_v) const {
            auto it = __params.find(key);
            if (it == __params.end()) return init_v;
            return std::any_cast<const SRCV&>(it->second);
        }
        template <typename SRCV> SRCV* GetParamPtr(const ID& key) {
            auto it = __params.find(key);
            if (it == __params.end()) return NULL;
            return (SRCV*)&std::any_cast<SRCV&>(it->second);
        }
        template <typename SRCV, typename DSTV> bool GetParamCastingCheck(const ID& key, DSTV& param) {
            auto it = __params.find(key);
            if (it == __params.end()) return false;
            param = (DSTV)std::any_cast<SRCV&>(it->second);
            return true;
        }
        template <typename SRCV, typename DSTV> DSTV GetParamCasting(const ID& key, const DSTV& init_v) {
            auto it = __params.find(key);
            if (it == __params.end()) return init_v;
            return (DSTV)std::any_cast<SRCV&>(it->second);
        }
        void SetParam(const ID& key, const std::any& param) {
            __params[key] = param;
        }
        void RemoveParam(const ID& key) {
            auto it = __params.find(key);
            if (it != __params.end()) {
                __params.erase(it);
            }
        }
        void RemoveAll() {
            __params.clear();
        }
        size_t Size() {
            return __params.size();
        }
        std::string GetPMapVersion() {
            return __PM_VERSION;
        }

        typedef std::unordered_map<ID, std::any> MapType;
        typename typedef MapType::iterator iterator;
        typename typedef MapType::const_iterator const_iterator;
        typename typedef MapType::reference reference;
        iterator begin() { return __params.begin(); }
        const_iterator begin() const { return __params.begin(); }
        iterator end() { return __params.end(); }
        const_iterator end() const { return __params.end(); }
    };

    enum class COMPONENT_TYPE
    {
        UNDEFINED = 0,
        BASE,
        SCENE,
        CAMERA,
        LIGHT,
        ACTOR,
    };

    __dojostruct VzBaseComp
    {
        VID componentVID = INVALID_VID;
        COMPONENT_TYPE compType = COMPONENT_TYPE::UNDEFINED;
        TimeStamp timeStamp = {}; // will be automatically set 
        ParamMap<std::string> attributes;
    };
    __dojostruct VzSceneComp : VzBaseComp
    {
        void GetWorldPosition(float v[3]);
        void GetWorldForward(float v[3]);
        void GetWorldRight(float v[3]);
        void GetWorldUp(float v[3]);

        void GetLocalTransform(float mat[16], const bool rowMajor = false);
        void GetWorldTransform(float mat[16], const bool rowMajor = false);
        void GetLocalInvTransform(float mat[16], const bool rowMajor = false);
        void GetWorldInvTransform(float mat[16], const bool rowMajor = false);

        void SetTranslate(const float value[3]);
        void SetScale(const float value[3]);
        void SetQuaternion(const float value[4]);
        void SetMatrix(const float value[16], const bool additiveTransform = false, const bool rowMajor = false);

        VID GetParentVid();
    };
    __dojostruct VzRenderer
    {
        void* renderer;
        bool IsActivated = false;
        // Renderer Configurations
        enum class Tonemap
        {
            Reinhard,
            ACES
        };
        enum class AO
        {
            AO_DISABLED,	// no ambient occlusion
            AO_SSAO,		// simple brute force screen space ambient occlusion
            AO_HBAO,		// horizon based screen space ambient occlusion
            AO_MSAO,		// multi scale screen space ambient occlusion
            AO_RTAO,		// ray traced ambient occlusion
            // Don't alter order! (bound to lua manually)
        };
        enum class FSR2_Preset
        {
            // Guidelines: https://github.com/GPUOpen-Effects/FidelityFX-FSR2#scaling-modes
            Quality,
            Balanced,
            Performance,
            Ultra_Performance,
        };
        enum class SURFEL_DEBUG
        {
            SURFEL_DEBUG_NONE,
            SURFEL_DEBUG_NORMAL,
            SURFEL_DEBUG_COLOR,
            SURFEL_DEBUG_POINT,
            SURFEL_DEBUG_RANDOM,
            SURFEL_DEBUG_HEATMAP,
            SURFEL_DEBUG_INCONSISTENCY,

            SURFEL_DEBUG_FORCE_UINT = 0xFFFFFFFF,
        };

        // renderpath-bind
        float GetExposure();
        float GetBrightness();
        float GetContrast();
        float GetSaturation();
        float GetBloomThreshold();
        float GetMotionBlurStrength();
        float GetDepthOfFieldStrength();
        float GetSharpenFilterAmount();
        float GetOutlineThreshold();
        float GetOutlineThickness();
        void GetOutlineColor(const float color[4]);
        float GetAORange();
        uint32_t GetAOSampleCount();
        float GetAOPower();
        float GetChromaticAberrationAmount();
        uint32_t GetScreenSpaceShadowSampleCount();
        float GetScreenSpaceShadowRange();
        float GetEyeAdaptionKey();
        float GetEyeAdaptionRate();
        float GetFSRSharpness();
        float GetFSR2Sharpness();
        float GetLightShaftsStrength();
        float GetRaytracedDiffuseRange();
        float GetRaytracedReflectionsRange();
        float GetReflectionRoughnessCutoff();
        float GetSSGIDepthRejection();
        Tonemap GetTonemap();

        bool GetAOEnabled();
        AO GetAO();
        bool GetSSREnabled();
        bool GetSSGIEnabled();
        bool GetRaytracedDiffuseEnabled();
        bool GetRaytracedReflectionEnabled();
        bool GetShadowsEnabled();
        bool GetReflectionsEnabled();
        bool GetFXAAEnabled();
        bool GetBloomEnabled();
        bool GetColorGradingEnabled();
        bool GetVolumeLightsEnabled();
        bool GetLightShaftsEnabled();
        bool GetLensFlareEnabled();
        bool GetMotionBlurEnabled();
        bool GetDepthOfFieldEnabled();
        bool GetEyeAdaptionEnabled();
        bool GetSharpenFilterEnabled();
        bool GetOutlineEnabled();
        bool GetChromaticAberrationEnabled();
        bool GetDitherEnabled();
        bool GetOcclusionCullingEnabled();
        bool GetSceneUpdateEnabled();
        bool GetFSREnabled();
        bool GetFSR2Enabled();

        uint32_t GetMSAASampleCount();

        void SetExposure(const float value);
        void SetBrightness(const float value);
        void SetContrast(const float value);
        void SetSaturation(const float value);
        void SetBloomThreshold(const float value);
        void SetMotionBlurStrength(const float value);
        void SetDepthOfFieldStrength(const float value);
        void SetSharpenFilterAmount(const float value);
        void SetOutlineThreshold(const float value);
        void SetOutlineThickness(const float value);
        void SetOutlineColor(const float value[4]);
        void SetAORange(const float value);
        void SetAOSampleCount(uint32_t value);
        void SetAOPower(const float value);
        void SetChromaticAberrationAmount(const float value);
        void SetScreenSpaceShadowSampleCount(uint32_t value);
        void SetScreenSpaceShadowRange(const float value);
        void SetEyeAdaptionKey(const float value);
        void SetEyeAdaptionRate(const float value);
        void SetFSRSharpness(const float value);
        void SetFSR2Sharpness(const float value);
        void SetLightShaftsStrength(const float value);
        void SetRaytracedDiffuseRange(const float value);
        void SetRaytracedReflectionsRange(const float value);
        void SetReflectionRoughnessCutoff(const float value);
        void SetSSGIDepthRejection(const float value);
        void SetTonemap(const Tonemap value);

        void SetAO(const AO value);
        void SetSSREnabled(bool value);
        void SetSSGIEnabled(const bool value);
        void SetRaytracedReflectionsEnabled(const bool value);
        void SetRaytracedDiffuseEnabled(const bool value);
        void SetMotionBlurEnabled(const bool value);
        void SetDepthOfFieldEnabled(const bool value);
        void SetEyeAdaptionEnabled(const bool value);
        void SetReflectionsEnabled(const bool value);
        void SetBloomEnabled(const bool value);
        void SetVolumeLightsEnabled(const bool value);
        void SetLightShaftsEnabled(const bool value);
        void SetOutlineEnabled(const bool value);
        void SetShadowsEnabled(const bool value);
        void SetFXAAEnabled(const bool value);
        void SetColorGradingEnabled(const bool value);
        void SetLensFlareEnabled(const bool value);
        void SetSharpenFilterEnabled(const bool value);
        void SetChromaticAberrationEnabled(const bool value);
        void SetDitherEnabled(const bool value);
        void SetOcclusionCullingEnabled(const bool value);
        void SetSceneUpdateEnabled(const bool value);
        void SetFSREnabled(const bool value);
        void SetFSR2Enabled(const bool value);
        void SetFSR2Preset(const FSR2_Preset preset); // this will modify resolution scaling and sampler lod bias

        void SetMSAASampleCount(const uint32_t value);

        // rendercore-bind (basically global options)
        void SetShadowProps2D(int max_resolution);
        void SetShadowPropsCube(int max_resolution);
        void SetTransparentShadowsEnabled(bool value);
        float GetTransparentShadowsEnabled();
        void SetWireRender(bool value);
        bool IsWireRender();
        void SetToDrawDebugBoneLines(bool param);
        bool GetToDrawDebugBoneLines();
        void SetToDrawDebugPartitionTree(bool param);
        bool GetToDrawDebugPartitionTree();
        bool GetToDrawDebugEnvProbes();
        void SetToDrawDebugEnvProbes(bool value);
        void SetToDrawDebugEmitters(bool param);
        bool GetToDrawDebugEmitters();
        void SetToDrawDebugForceFields(bool param);
        bool GetToDrawDebugForceFields();
        void SetToDrawDebugCameras(bool param);
        bool GetToDrawDebugCameras();
        void SetToDrawDebugColliders(bool param);
        bool GetToDrawDebugColliders();
        bool GetToDrawGridHelper();
        void SetToDrawGridHelper(bool value);
        bool GetToDrawVoxelHelper();
        void SetToDrawVoxelHelper(bool value, int clipmap_level);
        void SetDebugLightCulling(bool enabled);
        bool GetDebugLightCulling();
        void SetAdvancedLightCulling(bool enabled);
        bool GetAdvancedLightCulling();
        void SetVariableRateShadingClassification(bool enabled);
        bool GetVariableRateShadingClassification();
        void SetVariableRateShadingClassificationDebug(bool enabled);
        bool GetVariableRateShadingClassificationDebug();
        void SetTemporalAAEnabled(bool enabled);
        bool GetTemporalAAEnabled();
        void SetTemporalAADebugEnabled(bool enabled);
        bool GetTemporalAADebugEnabled();
        void SetFreezeCullingCameraEnabled(bool enabled);
        bool GetFreezeCullingCameraEnabled();
        void SetVXGIEnabled(bool enabled);
        bool GetVXGIEnabled();
        void SetVXGIReflectionsEnabled(bool enabled);
        bool GetVXGIReflectionsEnabled();
        void SetGameSpeed(float value);
        float GetGameSpeed();
        void SetRaytraceBounceCount(uint32_t bounces);
        uint32_t GetRaytraceBounceCount();
        void SetRaytraceDebugBVHVisualizerEnabled(bool value);
        bool GetRaytraceDebugBVHVisualizerEnabled();
        void SetRaytracedShadowsEnabled(bool value);
        bool GetRaytracedShadowsEnabled();
        void SetTessellationEnabled(bool value);
        bool GetTessellationEnabled();
        void SetDisableAlbedoMaps(bool value);
        bool IsDisableAlbedoMaps();
        void SetForceDiffuseLighting(bool value);
        bool IsForceDiffuseLighting();
        void SetScreenSpaceShadowsEnabled(bool value);
        bool GetScreenSpaceShadowsEnabled();
        void SetSurfelGIEnabled(bool value);
        bool GetSurfelGIEnabled();
        void SetSurfelGIDebugEnabled(SURFEL_DEBUG value);
        SURFEL_DEBUG GetSurfelGIDebugEnabled();
        void SetDDGIEnabled(bool value);
        bool GetDDGIEnabled();
        void SetDDGIDebugEnabled(bool value);
        bool GetDDGIDebugEnabled();
        void SetDDGIRayCount(uint32_t value);
        uint32_t GetDDGIRayCount();
        void SetDDGIBlendSpeed(float value);
        float GetDDGIBlendSpeed();
        void SetGIBoost(float value);
        float GetGIBoost();
    };
    __dojostruct VzCamera : VzSceneComp, VzRenderer
    {
        // Pose parameters are defined in WS (not local space)
        void SetPose(const float pos[3], const float view[3], const float up[3]);
        void SetPerspectiveProjection(const float zNearP, const float zFarP, const float fovY, const float aspectRatio);
        void SetCanvasSize(const float w, const float h, const float dpi);
        void GetPose(float pos[3], float view[3], float up[3]);
        void GetPerspectiveProjection(float* zNearP, float* zFarP, float* fovY, float* aspectRatio);
        void GetCanvasSize(float* w, float* h, float* dpi);
    };
    __dojostruct VzLight : VzSceneComp
    {
        void SetColor(const float value[3]);
        void SetIntensity(const float value);
        void SetRange(const float value);
        void SetConeOuterRange(const float value);
        void SetConeInnerRange(const float value);
        void SetRadius(const float value);
        void SetLength(const float value);

        void SetCastShadow(const bool value);
        void SetVolumetricsEnabled(const bool value);
        void SetVisualizerEnabled(const bool value);
        void SetStatic(const bool value);
        void SetVolumetricCloudsEnabled(const bool value);

        bool IsCastingShadow();
        bool IsVolumetricsEnabled();
        bool IsVisualizerEnabled();
        bool IsStatic();
        bool IsVolumetricCloudsEnabled();

        float GetRange();

        enum LightType
        {
            DIRECTIONAL = 0,
            POINT,
            SPOT,
            LIGHTTYPE_COUNT,
            ENUM_FORCE_UINT32 = 0xFFFFFFFF,
        };
        void SetType(const LightType val);
        LightType GetType();
    };
    __dojostruct VzActor : VzSceneComp
    {
        // 
    };
}
