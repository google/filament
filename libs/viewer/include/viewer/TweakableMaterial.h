#pragma once

#include <viewer/TweakableProperty.h>

#include <math/mat4.h>
#include <math/vec3.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#endif

#define JSON_NOEXCEPTION
#include <viewer/json.hpp>
#undef JSON_NOEXCEPTION

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <vector>
#include <string>
#include <iostream>

// The following template specializations have to be defined under the nlohmann namespace
namespace nlohmann {
    template<>
    struct adl_serializer<filament::math::float4> {
        static void to_json(json& j, const filament::math::float4& p) {
            j = json{ {"x", p.x}, {"y", p.y}, {"z", p.z}, {"w", p.w} };
        }

        static void from_json(const json& j, filament::math::float4& p) {
            j.at("x").get_to(p.x);
            j.at("y").get_to(p.y);
            j.at("z").get_to(p.z);
            j.at("w").get_to(p.w);
        }
    };

    template<>
    struct adl_serializer<filament::math::float3> {
        static void to_json(json& j, const filament::math::float3& p) {
            j = json{ {"x", p.x}, {"y", p.y}, {"z", p.z} };
        }

        static void from_json(const json& j, filament::math::float3& p) {
            j.at("x").get_to(p.x);
            j.at("y").get_to(p.y);
            j.at("z").get_to(p.z);
        }
    };

    template<>
    struct adl_serializer<filament::math::float2> {
        static void to_json(json& j, const filament::math::float2& p) {
            j = json{ {"x", p.x}, {"y", p.y} };
        }

        static void from_json(const json& j, filament::math::float2& p) {
            j.at("x").get_to(p.x);
            j.at("y").get_to(p.y);
        }
    };
}

using nlohmann::json;

struct TweakableMaterial {
public:
    TweakableMaterial();

    struct RequestedTexture {
        std::string filename{};
        bool isSrgb{};
        bool isAlpha{};
        bool doRequestReload{};
    };

    json toJson();
    void fromJson(const json& source);
    void drawUI();

    const TweakableMaterial::RequestedTexture nextRequestedTexture();

    TweakablePropertyTextured<filament::math::float4> mBaseColor{ {0.0f, 0.0f, 0.0f, 1.0f} };

    TweakablePropertyTextured<float, false> mNormal{};
    TweakablePropertyTextured<float, false> mOcclusionIntensity{ 1.0f };
    TweakablePropertyTextured<float, false> mOcclusion{1.0f};
    TweakableProperty<float> mRoughnessScale{};
    TweakablePropertyTextured<float> mRoughness;
    TweakablePropertyTextured<float> mMetallic{};

    TweakableProperty<float> mClearCoat{};
    TweakableProperty<float> mClearCoatNormalIntensity{1.0f};
    TweakablePropertyTextured<float, false> mClearCoatNormal{};
    TweakablePropertyTextured<float> mClearCoatRoughness{};

    std::vector< RequestedTexture > mRequestedTextures{};

    float mBaseTextureScale = 1.0f; // applied to baseColor texture
    float mNormalTextureScale = 1.0f; // applied to normal.xy, roughness, and metallic maps
    float mClearCoatTextureScale = 1.0f; // applied to clearcloat normal.xy, roughness, and value textures
    float mRefractiveTextureScale = 1.0f; // applied to ior, transmission, {micro}Thickness textures
    TweakableProperty<float> mSpecularIntensity{}; // scales Filament's reflectance attribute - should be equivalent to specular in Blender
    TweakableProperty<float> mNormalIntensity{}; // scales the normal vectors along the XY axes

    TweakableProperty<float> mAnisotropy{}; // for metals
    TweakableProperty<filament::math::float3, false, false> mAnisotropyDirection{}; // for metals; not color
    
    TweakableProperty<filament::math::float3> mSubsurfaceColor{}; // for cloth and subsurface
    TweakablePropertyDerivable<filament::math::float3> mSheenColor{}; // for cloth
    TweakablePropertyTextured<float> mSheenRoughness{}; // for cloth

    TweakableProperty<float> mSubsurfacePower{1.0f}; // for subsurface

    TweakablePropertyDerivable<filament::math::float3> mAbsorption{}; // for refractive
    TweakablePropertyTextured<float> mTransmission{}; // for refractive
    TweakableProperty<float> mMaxThickness{}; // for refractive; this scales the values read from a thickness property/texture
    TweakablePropertyTextured<float> mThickness{}; // for refractive and subsurface
    TweakableProperty<float> mIorScale{}; // for refractive
    TweakableProperty<float> mIor{}; // for refractive

    bool mUseWard{};
    bool mDoRelease{}; // this notifies the material integrator tool that this material needs to be checked into the codebase

    enum MaterialType { Opaque, TransparentSolid, TransparentThin, Cloth, Subsurface };
    MaterialType mShaderType{};

    void resetWithType(MaterialType newType);

private:
    template< typename T, bool MayContainFile = false, bool IsColor = true, bool IsDerivable = false, typename = IsValidTweakableType<T> >
    void enqueueTextureRequest(TweakableProperty<T, MayContainFile, IsColor>& item, bool isSrgb = false, bool isAlpa = false) {
        enqueueTextureRequest(item.filename, item.doRequestReload, isSrgb, isAlpa);
        item.doRequestReload = false;
    }

    void enqueueTextureRequest(const std::string& filename, bool doRequestReload, bool isSrgb = false, bool isAlpa = false);

    template< typename T, bool MayContainFile = false, bool IsColor = true, bool IsDerivable = false, typename = IsValidTweakableType<T> >
    void writeTexturedToJson(json& result, const std::string& prefix, const TweakableProperty<T, MayContainFile, IsColor>& item) {
        result[prefix] = item.value;
        result[prefix + "IsFile"] = item.isFile;
        result[prefix + "Texture"] = (item.isFile) ? item.filename.asString() : "";
    }

    template< typename T, bool MayContainFile = false, bool IsColor = true, bool IsDerivable = false, typename = IsValidTweakableType<T> >
    void readTexturedFromJson(const json& source, const std::string& prefix, TweakableProperty<T, MayContainFile, IsColor>& item, bool isSrgb = false, bool isAlpha = false, T defaultValue = {}) {
        item.value = source.value(prefix, defaultValue);
        if (source.find(prefix) == source.end()) {
            std::cout << "Unable to read textured property '" << prefix << "', reverting to default value without texture." << std::endl;
            item.value = defaultValue;
            item.isFile = false;
            item.filename.clear();
        } else {
            item.isFile = source.value(prefix + "IsFile", false);
            std::string filename = source.value(prefix + "Texture", "");
            item.filename = filename;
            item.doRequestReload = true;
            if (item.isFile) {
                enqueueTextureRequest(item.filename.asString(), item.doRequestReload, isSrgb, isAlpha);
            }
        }
    }

    template< typename T, bool IsColor = false, bool IsDerivable = false, typename = IsValidTweakableType<T> >
    void readValueFromJson(const json& source, const std::string& prefix, TweakableProperty<T, false, IsColor, IsDerivable>& item, const T defaultValue = T()) {
        item.value = source.value(prefix, defaultValue);
        if (IsDerivable) {
            item.useDerivedQuantity = source.value(prefix + "UseDerivable", false);
        }

        if (source.find(prefix) == source.end()) {
            std::cout << "Material file did not have attribute '" << prefix << "'. Using default (" << defaultValue << ") instead." << std::endl;
        }
    }

    // If you pass in the plain value without the encapsulating TweakableProperty
    template< typename T, bool IsColor = false, typename = IsValidTweakableType<T> >
    void readValueFromJson(const json& source, const std::string& prefix, T& item, const T defaultValue = T()) {
        item = source.value(prefix, defaultValue);

        if (source.find(prefix) == source.end()) {
            std::cout << "Material file did not have attribute '" << prefix << "'. Using default (" << defaultValue << ") instead." << std::endl;
        }
    }

    // And this is only here for the non-directly tweakable/transferred types, such as bools
    // TODO: clean up this mess!
    void readValueFromJson(const json& source, const std::string& prefix, bool& item, const bool defaultValue ) {
        item = source.value(prefix, defaultValue);

        if (source.find(prefix) == source.end()) {
            std::cout << "Material file did not have attribute '" << prefix << "'. Using default (" << defaultValue << ") instead." << std::endl;
        }
    }
};
