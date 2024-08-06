#ifndef VIZCOMPONENTAPIS
#define VIZCOMPONENTAPIS

#pragma warning (disable : 4251)
#pragma warning (disable : 4819)
// FEngine Warnings
#pragma warning (disable : 4146)
#pragma warning (disable : 4068)
#pragma warning (disable : 4267)
#pragma warning (disable : 4244)
#pragma warning (disable : 4067)

#define _ITERATOR_DEBUG_LEVEL 0 // this is for using STL containers as our standard parameters

#ifdef WIN32
#define API_EXPORT __declspec(dllexport)
#else
#define API_EXPORT __attribute__((visibility("default")))
#endif
#define __dojostatic extern "C" API_EXPORT
#define __dojoclass class API_EXPORT
#define __dojostruct struct API_EXPORT

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

namespace vzm
{
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

    enum class SCENE_COMPONENT_TYPE // every component involves a transform and a name
    {
        // camera, light, actor component can have renderable resources technically
        SCENEBASE = 0,  // empty (only transform and name)
        CAMERA,
        LIGHT,
        ACTOR,
    };

    enum class RES_COMPONENT_TYPE
    {
        RESOURCE = 0,
        GEOMATRY,
        MATERIAL,
        MATERIALINSTANCE,
        TEXTURE,
    };

    __dojostruct VzBaseComp
    {
    private:
        VID componentVID_ = INVALID_VID;
        TimeStamp timeStamp_ = {}; // will be automatically set 
        std::string originFrom_;
        std::string type_;
    public:
        // User data
        ParamMap<std::string> attributes;
        VzBaseComp(const VID vid, const std::string& originFrom, const std::string& typeName)
            : componentVID_(vid), originFrom_(originFrom), type_(typeName)
        {
            UpdateTimeStamp();
        }
        VID GetVID() const { return componentVID_; }
        std::string GetType() { return type_; };
        TimeStamp GetTimeStamp() { return timeStamp_; };
        void UpdateTimeStamp() 
        {
            timeStamp_ = std::chrono::high_resolution_clock::now();
        }
        std::string GetName();
        void SetName(const std::string& name);
    };
    __dojostruct VzSceneComp : VzBaseComp
    {
    public:
        enum class EULER_ORDER { XYZ,YXZ, ZXY, ZYX, YZX, XZY };
    private:
        SCENE_COMPONENT_TYPE scenecompType_ = SCENE_COMPONENT_TYPE::SCENEBASE;
        float position_[3] = {0.0f, 0.0f, 0.0f};
        float rotation_[3] = {0.0f, 0.0f, 0.0f};
        EULER_ORDER order_ = EULER_ORDER::XYZ;
        float quaternion_[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        float scale_[3] = {1.0f, 1.0f, 1.0f};
        bool matrixAutoUpdate_ = false;

        void setQuaternionFromEuler();
        void setEulerFromQuaternion();
    public:
        VzSceneComp(const VID vid, const std::string& originFrom, const std::string& typeName, const SCENE_COMPONENT_TYPE scenecompType)
            : VzBaseComp(vid, originFrom, typeName), scenecompType_(scenecompType) {}
        SCENE_COMPONENT_TYPE GetSceneCompType() { return scenecompType_; };

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

        VID GetParent();
        std::vector<VID> GetChildren();
        VID GetScene();

        void GetPosition(float position[3]) const;
        void GetRotation(float rotation[3], EULER_ORDER* order = nullptr) const;
        void GetQuaternion(float quaternion[4]) const;
        void GetScale(float scale[3]) const;

        void SetPosition(const float position[3]);
        void SetRotation(const float rotation[3], const EULER_ORDER order = EULER_ORDER::XYZ);
        void SetQuaternion(const float quaternion[4]);
        void SetScale(const float scale[3]);
         
        bool IsMatrixAutoUpdate() const;
        void SetMatrixAutoUpdate(const bool matrixAutoUpdate);

        void UpdateMatrix();
    };
    __dojostruct VzResource : VzBaseComp
    {
    private:
        RES_COMPONENT_TYPE resType_ = RES_COMPONENT_TYPE::RESOURCE;
    public:
        VzResource(const VID vid, const std::string& originFrom, const std::string& typeName, const RES_COMPONENT_TYPE resType)
            : VzBaseComp(vid, originFrom, typeName), resType_(resType) {}
        RES_COMPONENT_TYPE GetResType() { return resType_; }
    };
}

// enumerations
namespace vzm
{
    // update this when a new version of filament wouldn't work with older materials
    static constexpr size_t MATERIAL_VERSION = 53;

    enum class UniformType : uint8_t {
        BOOL,
        BOOL2,
        BOOL3,
        BOOL4,
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        INT,
        INT2,
        INT3,
        INT4,
        UINT,
        UINT2,
        UINT3,
        UINT4,
        MAT3,   //!< a 3x3 float matrix
        MAT4,   //!< a 4x4 float matrix
        STRUCT
    };
    enum class SamplerType : uint8_t {
        SAMPLER_2D,             //!< 2D texture
        SAMPLER_2D_ARRAY,       //!< 2D array texture
        SAMPLER_CUBEMAP,        //!< Cube map texture
        SAMPLER_EXTERNAL,       //!< External texture
        SAMPLER_3D,             //!< 3D texture
        SAMPLER_CUBEMAP_ARRAY,  //!< Cube map array texture (feature level 2)
    };
    enum class SubpassType : uint8_t {
        SUBPASS_INPUT
    };

    enum class Precision : uint8_t {
        LOW,
        MEDIUM,
        HIGH,
        DEFAULT
    };

    //! types of RGB colors
    enum class RgbType : uint8_t {
        sRGB,   //!< the color is defined in Rec.709-sRGB-D65 (sRGB) space
        LINEAR, //!< the color is defined in Rec.709-Linear-D65 ("linear sRGB") space
    };
    //! types of RGBA colors
    enum class RgbaType : uint8_t {
        /**
         * the color is defined in Rec.709-sRGB-D65 (sRGB) space and the RGB values
         * have not been pre-multiplied by the alpha (for instance, a 50%
         * transparent red is <1,0,0,0.5>)
         */
        sRGB,
        /**
         * the color is defined in Rec.709-Linear-D65 ("linear sRGB") space and the
         * RGB values have not been pre-multiplied by the alpha (for instance, a 50%
         * transparent red is <1,0,0,0.5>)
         */
        LINEAR,
        /**
         * the color is defined in Rec.709-sRGB-D65 (sRGB) space and the RGB values
         * have been pre-multiplied by the alpha (for instance, a 50%
         * transparent red is <0.5,0,0,0.5>)
         */
        PREMULTIPLIED_sRGB,
        /**
         * the color is defined in Rec.709-Linear-D65 ("linear sRGB") space and the
         * RGB values have been pre-multiplied by the alpha (for instance, a 50%
         * transparent red is <0.5,0,0,0.5>)
         */
        PREMULTIPLIED_LINEAR
    };

    //! Sampler Wrap mode
    enum class SamplerWrapMode : uint8_t {
        CLAMP_TO_EDGE,      //!< clamp-to-edge. The edge of the texture extends to infinity.
        REPEAT,             //!< repeat. The texture infinitely repeats in the wrap direction.
        MIRRORED_REPEAT,    //!< mirrored-repeat. The texture infinitely repeats and mirrors in the wrap direction.
    };

    //! Sampler minification filter
    enum class SamplerMinFilter : uint8_t {
        // don't change the enums values
        NEAREST = 0,                //!< No filtering. Nearest neighbor is used.
        LINEAR = 1,                 //!< Box filtering. Weighted average of 4 neighbors is used.
        NEAREST_MIPMAP_NEAREST = 2, //!< Mip-mapping is activated. But no filtering occurs.
        LINEAR_MIPMAP_NEAREST = 3,  //!< Box filtering within a mip-map level.
        NEAREST_MIPMAP_LINEAR = 4,  //!< Mip-map levels are interpolated, but no other filtering occurs.
        LINEAR_MIPMAP_LINEAR = 5    //!< Both interpolated Mip-mapping and linear filtering are used.
    };

    //! Sampler magnification filter
    enum class SamplerMagFilter : uint8_t {
        // don't change the enums values
        NEAREST = 0,                //!< No filtering. Nearest neighbor is used.
        LINEAR = 1,                 //!< Box filtering. Weighted average of 4 neighbors is used.
    };
}
#endif
