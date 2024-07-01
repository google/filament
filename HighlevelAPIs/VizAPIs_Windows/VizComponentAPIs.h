#pragma once
#define _ITERATOR_DEBUG_LEVEL 0 // this is for using STL containers as our standard parameters

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

    enum class SCENE_COMPONENT_TYPE // every component involves a transform
    {
        SCENEBASE = 0,
        CAMERA,
        LIGHT,
        ACTOR, // kind of renderable component
    };

    enum class RES_COMPONENT_TYPE
    {
        GEOMATRY = 0,
        MATERIAL,
        MATERIALINSTANCE,
    };

    __dojostruct VzBaseComp
    {
        VID componentVID = INVALID_VID;
        TimeStamp timeStamp = {}; // will be automatically set 
        ParamMap<std::string> attributes;

        std::string GetName();
        void SetName(const std::string& name);
    };
    __dojostruct VzSceneComp : VzBaseComp
    {
        SCENE_COMPONENT_TYPE compType = SCENE_COMPONENT_TYPE::SCENEBASE;

        void GetWorldPosition(float v[3]);
        void GetWorldForward(float v[3]);
        void GetWorldRight(float v[3]);
        void GetWorldUp(float v[3]);
        void GetWorldTransform(float mat[16], const bool rowMajor = false);
        void GetWorldInvTransform(float mat[16], const bool rowMajor = false);

        void GetLocalTransform(float mat[16], const bool rowMajor = false);
        void GetLocalInvTransform(float mat[16], const bool rowMajor = false);

        // local transforms
        void SetTransform(const float s[3] = nullptr, const float q[4] = nullptr, const float t[3] = nullptr, const bool additiveTransform = false);
        void SetMatrix(const float value[16], const bool additiveTransform = false, const bool rowMajor = false);

        VID GetParentVid();
    };

    class VzRenderPath;
    __dojostruct VzRenderer
    {
        VzRenderPath* renderPath = nullptr;

        // setters and getters of rendering options
    };
    __dojostruct VzCamera : VzSceneComp, VzRenderer
    {
        void SetCanvas(const uint32_t w, const uint32_t h, const float dpi, void* window = nullptr);
        void GetCanvas(uint32_t* w, uint32_t* h, float* dpi, void** window = nullptr);

        // Pose parameters are defined in WS (not local space)
        void SetWorldPose(const float pos[3], const float view[3], const float up[3]);
        void SetPerspectiveProjection(const float zNearP, const float zFarP, const float fovY, const float aspectRatio);
        void GetWorldPose(float pos[3], float view[3], float up[3]);
        void GetPerspectiveProjection(float* zNearP, float* zFarP, float* fovY, float* aspectRatio);
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


    __dojostruct VzResource : VzBaseComp
    {
        RES_COMPONENT_TYPE compType = RES_COMPONENT_TYPE::GEOMATRY;
    };
    __dojostruct VzGeometry : VzResource
    {
        // 
    };
    __dojostruct VzMaterial : VzResource
    {
        // 
    };
    __dojostruct VzMaterialInstance : VzMaterial
    {
        // 
    };
}
