/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMAT_MATERIAL_PACKAGE_BUILDER_H
#define TNT_FILAMAT_MATERIAL_PACKAGE_BUILDER_H

#include <cstddef>
#include <cstdint>

#include <atomic>
#include <string>
#include <vector>

#include <backend/DriverEnums.h>
#include <filament/MaterialEnums.h>

#include <filamat/Package.h>

#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/CString.h>

namespace filamat {

struct MaterialInfo;
class ChunkContainer;
struct Variant;

class UTILS_PUBLIC MaterialBuilderBase {
public:
    // High-level hint that works in concert with TargetApi to determine the shader models
    // (used to generate GLSL) and final output representations (spirv and/or text).
    enum class Platform {
        DESKTOP,
        MOBILE,
        ALL
    };

    enum class TargetApi : uint8_t {
        OPENGL      = 0x01u,
        VULKAN      = 0x02u,
        METAL       = 0x04u,
        ALL         = OPENGL | VULKAN | METAL
    };

    enum class TargetLanguage {
        GLSL,
        SPIRV
    };

    enum class Optimization {
        NONE,
        PREPROCESSOR,
        SIZE,
        PERFORMANCE
    };

    // Must be called first before building any materials.
    static void init();

    // Call when finished building materials to release all internal resources. After calling
    // shutdown, another call to MaterialBuilder::init must precede another material build.
    static void shutdown();

protected:
    // Looks at platform and target API, then decides on shader models and output formats.
    void prepare();

    using ShaderModel = filament::backend::ShaderModel;
    Platform mPlatform = Platform::DESKTOP;
    TargetApi mTargetApi = (TargetApi) 0;
    Optimization mOptimization = Optimization::PERFORMANCE;
    bool mPrintShaders = false;
    utils::bitset32 mShaderModels;
    struct CodeGenParams {
        int shaderModel;
        TargetApi targetApi;
        TargetLanguage targetLanguage;
    };
    std::vector<CodeGenParams> mCodeGenPermutations;
    uint8_t mVariantFilter = 0;

    // Keeps track of how many times MaterialBuilder::init() has been called without a call to
    // MaterialBuilder::shutdown(). Internally, glslang does something similar. We keep track for
    // ourselves so we can inform the user if MaterialBuilder::init() hasn't been called before
    // attempting to build a material.
    static std::atomic<int> materialBuilderClients;
};

inline constexpr MaterialBuilderBase::TargetApi operator|(MaterialBuilderBase::TargetApi lhs,
        MaterialBuilderBase::TargetApi rhs) noexcept {
    return MaterialBuilderBase::TargetApi(uint8_t(lhs) | uint8_t(rhs));
}

inline constexpr MaterialBuilderBase::TargetApi operator|=(MaterialBuilderBase::TargetApi& lhs,
        MaterialBuilderBase::TargetApi rhs) noexcept {
    return lhs = (lhs | rhs);
}

inline constexpr bool operator&(MaterialBuilderBase::TargetApi lhs,
        MaterialBuilderBase::TargetApi rhs) noexcept {
    return bool(uint8_t(lhs) & uint8_t(rhs));
}

class UTILS_PUBLIC MaterialBuilder : public MaterialBuilderBase {
public:
    MaterialBuilder();

    static constexpr size_t MATERIAL_VARIABLES_COUNT = 4;
    enum class Variable : uint8_t {
        CUSTOM0,
        CUSTOM1,
        CUSTOM2,
        CUSTOM3
        // when adding more variables, make sure to update MATERIAL_VARIABLES_COUNT
    };

    static constexpr size_t MATERIAL_PROPERTIES_COUNT = 19;
    enum class Property : uint8_t {
        BASE_COLOR,              // float4, all shading models
        ROUGHNESS,               // float,  lit shading models only
        METALLIC,                // float,  all shading models, except unlit and cloth
        REFLECTANCE,             // float,  all shading models, except unlit and cloth
        AMBIENT_OCCLUSION,       // float,  lit shading models only, except subsurface and cloth
        CLEAR_COAT,              // float,  lit shading models only, except subsurface and cloth
        CLEAR_COAT_ROUGHNESS,    // float,  lit shading models only, except subsurface and cloth
        CLEAR_COAT_NORMAL,       // float,  lit shading models only, except subsurface and cloth
        ANISOTROPY,              // float,  lit shading models only, except subsurface and cloth
        ANISOTROPY_DIRECTION,    // float3, lit shading models only, except subsurface and cloth
        THICKNESS,               // float,  subsurface shading model only
        SUBSURFACE_POWER,        // float,  subsurface shading model only
        SUBSURFACE_COLOR,        // float3, subsurface and cloth shading models only
        SHEEN_COLOR,             // float3, cloth shading model only
        SPECULAR_COLOR,          // float3, specular-glossiness shading model only
        GLOSSINESS,              // float,  specular-glossiness shading model only
        EMISSIVE,                // float4, all shading models
        NORMAL,                  // float3, all shading models only, except unlit
        POST_LIGHTING_COLOR,     // float4, all shading models
        // when adding new Properties, make sure to update MATERIAL_PROPERTIES_COUNT
    };

    using BlendingMode = filament::BlendingMode;
    using Shading = filament::Shading;
    using Interpolation = filament::Interpolation;
    using VertexDomain = filament::VertexDomain;
    using TransparencyMode = filament::TransparencyMode;

    using UniformType = filament::backend::UniformType;
    using SamplerType = filament::backend::SamplerType;
    using SamplerFormat = filament::backend::SamplerFormat;
    using SamplerPrecision = filament::backend::Precision;
    using CullingMode = filament::backend::CullingMode;

    // set name of this material
    MaterialBuilder& name(const char* name) noexcept;

    // set the shading model
    using MaterialDomain = filament::MaterialDomain;
    MaterialBuilder& shading(Shading shading) noexcept;

    // set the interpolation mode
    MaterialBuilder& interpolation(Interpolation interpolation) noexcept;

    // add a parameter (i.e.: a uniform) to this material
    MaterialBuilder& parameter(UniformType type, const char* name) noexcept;

    // add a parameter array to this material
    MaterialBuilder& parameter(UniformType type, size_t size, const char* name) noexcept;

    // add a sampler parameter to this material
    // When SamplerType::SAMPLER_EXTERNAL is specifed, format and precision are ignored
    MaterialBuilder& parameter(SamplerType samplerType, SamplerFormat format,
            SamplerPrecision precision, const char* name) noexcept;
    MaterialBuilder& parameter(SamplerType samplerType, SamplerFormat format,
            const char* name) noexcept;
    MaterialBuilder& parameter(SamplerType samplerType, SamplerPrecision precision,
            const char* name) noexcept;
    MaterialBuilder& parameter(SamplerType samplerType, const char* name) noexcept;

    // custom variables (all float4)
    MaterialBuilder& variable(Variable v, const char* name) noexcept;

    // require a specified attribute, position is always required and normal
    // depends on the shading model
    MaterialBuilder& require(filament::VertexAttribute attribute) noexcept;

    // specify the domain that this material will operate in
    MaterialBuilder& materialDomain(MaterialDomain materialDomain) noexcept;

    // set the code content of this material
    // for materials in the SURFACE domain:
    //     must declare a function "void material(inout MaterialInputs material)"
    //     this function *must* call "prepareMaterial(material)" before it returns
    // for materials in the POST_PROCESS domain:
    //     must declare a function "void postProcess(inout PostProcessInputs postProcess)"
    MaterialBuilder& material(const char* code, size_t line = 0) noexcept;

    // set the vertex code content of this material
    // for materials in the SURFACE domain:
    //     must declare a function "void materialVertex(inout MaterialVertexInputs material)"
    // for materials in the POST_PROCESS domain:
    //     must declare a function "void postProcessVertex(inout PostProcessVertexInputs postProcess)"
    MaterialBuilder& materialVertex(const char* code, size_t line = 0) noexcept;

    // set blending mode for this material
    MaterialBuilder& blending(BlendingMode blending) noexcept;

    // set blending mode of the post lighting color for this material
    // only OPAQUE, TRANSPARENT and ADD are supported, the default is TRANSPARENT
    // this setting requires the material property "postLightingColor" to be set
    MaterialBuilder& postLightingBlending(BlendingMode blending) noexcept;

    // set vertex domain for this material
    MaterialBuilder& vertexDomain(VertexDomain domain) noexcept;

    // how triangles are culled (doesn't affect points or lines, back-face culling by default)
    MaterialBuilder& culling(CullingMode culling) noexcept;

    // enable/disable color-buffer write (enabled by default)
    MaterialBuilder& colorWrite(bool enable) noexcept;

    // enable/disable depth-buffer write (enabled by default for opaque, disabled for others)
    MaterialBuilder& depthWrite(bool enable) noexcept;

    // enable/disable depth based culling (enabled by default)
    MaterialBuilder& depthCulling(bool enable) noexcept;

    // double-sided materials don't cull faces, equivalent to culling(CullingMode::NONE)
    // doubleSided() overrides culling() if called
    // when called with "false", this enables the capability for a run-time toggle
    MaterialBuilder& doubleSided(bool doubleSided) noexcept;

    // any fragment with an alpha below this threshold is clipped (MASKED blending mode only)
    // the mask threshold can also be controlled by using the float material parameter
    // called "_maskTrehshold", or by calling MaterialInstance::setMaskTreshold
    MaterialBuilder& maskThreshold(float threshold) noexcept;

    // the material output is multiplied by the shadowing factor (UNLIT model only)
    MaterialBuilder& shadowMultiplier(bool shadowMultiplier) noexcept;

    // reduces specular aliasing for materials that have low roughness. Turning this feature
    // on also helps preserve the shapes of specular highlights as an object moves away from
    // the camera. When turned on, two float material parameters are added to control the effect:
    // "_specularAAScreenSpaceVariance" and "_specularAAThreshold". You can also use
    // MaterialInstance::setSpecularAntiAliasingVariance and setSpecularAntiAliasingThreshold
    // disabled by default
    MaterialBuilder& specularAntiAliasing(bool specularAntiAliasing) noexcept;

    // sets the screen space variance of the filter kernel used when applying specular
    // anti-aliasing. The default value is set to 0.15. The specified value should be between
    // 0 and 1 and will be clamped if necessary.
    MaterialBuilder& specularAntiAliasingVariance(float screenSpaceVariance) noexcept;

    // sets the clamping threshold used to suppress estimation errors when applying specular
    // anti-aliasing. The default value is set to 0.2. The specified value should be between 0
    // and 1 and will be clamped if necessary.
    MaterialBuilder& specularAntiAliasingThreshold(float threshold) noexcept;

    // enables or disables the index of refraction (IoR) change caused by the clear coat layer when
    // present. When the IoR changes, the base color is darkened. Disabling this feature preserves
    // the base color as initially specified
    // enabled by default
    MaterialBuilder& clearCoatIorChange(bool clearCoatIorChange) noexcept;

    // enable/disable flipping of the Y coordinate of UV attributes, enabled by default
    MaterialBuilder& flipUV(bool flipUV) noexcept;

    // enable/disable multi-bounce ambient occlusion, disabled by default on mobile
    MaterialBuilder& multiBounceAmbientOcclusion(bool multiBounceAO) noexcept;

    // enable/disable specular ambient occlusion, disabled by default on mobile
    MaterialBuilder& specularAmbientOcclusion(bool specularAO) noexcept;

    // specifies how transparent objects should be rendered (default is DEFAULT)
    MaterialBuilder& transparencyMode(TransparencyMode mode) noexcept;

    // specifies desktop vs mobile; works in concert with TargetApi to determine the shader models
    // (used to generate code) and final output representations (spirv and/or text).
    MaterialBuilder& platform(Platform platform) noexcept;

    // specifies opengl, vulkan, or metal
    // This can be called repeatedly to build for multiple APIs.
    // Works in concert with Platform to determine the shader models (used to generate code) and
    // final output representations (spirv and/or text).
    // If linking against filamat_lite, only "opengl" is allowed.
    MaterialBuilder& targetApi(TargetApi targetApi) noexcept;

    // specifies the level of optimization to apply to the shaders (default is PERFORMANCE)
    // if linking against filamat_lite, this _must_ be called with Optimization::NONE.
    MaterialBuilder& optimization(Optimization optimization) noexcept;

    // if true, will output the generated GLSL shader code to stdout
    // TODO: this is present here for matc's "--print" flag, but ideally does not belong inside
    // MaterialBuilder
    MaterialBuilder& printShaders(bool printShaders) noexcept;

    // specifies a list of variants that should be filtered out during code generation.
    MaterialBuilder& variantFilter(uint8_t variantFilter) noexcept;

    // build the material
    Package build() noexcept;

public:
    // The methods and types below are for internal use
    struct Parameter {
        Parameter() noexcept = default;
        Parameter(const char* paramName, SamplerType t, SamplerFormat f, SamplerPrecision p)
                : name(paramName), size(1), samplerType(t), samplerFormat(f), samplerPrecision(p),
                isSampler(true) { }
        Parameter(const char* paramName, UniformType t, size_t typeSize)
                : name(paramName), size(typeSize), uniformType(t), isSampler(false) { }
        utils::CString name;
        size_t size;
        union {
            UniformType uniformType;
            struct {
                SamplerType samplerType;
                SamplerFormat samplerFormat;
                SamplerPrecision samplerPrecision;
            };
        };
        bool isSampler;
    };

    using PropertyList = bool[MATERIAL_PROPERTIES_COUNT];
    using VariableList = utils::CString[MATERIAL_VARIABLES_COUNT];

    // Preview the first shader generated by the given CodeGenParams.
    // This is used to run Static Code Analysis before generating a package.
    const std::string peek(filament::backend::ShaderType type,
            const CodeGenParams& params, const PropertyList& properties) noexcept;

    // Returns true if any of the parameter samplers is of type samplerExternal
    bool hasExternalSampler() const noexcept;

    static constexpr size_t MAX_PARAMETERS_COUNT = 32;
    using ParameterList = Parameter[MAX_PARAMETERS_COUNT];

    // returns the number of parameters declared in this material
    uint8_t getParameterCount() const noexcept { return mParameterCount; }

    // returns a list of at least getParameterCount() parameters
    const ParameterList& getParameters() const noexcept { return mParameters; }

    uint8_t getVariantFilter() const { return mVariantFilter; }

private:
    void prepareToBuild(MaterialInfo& info) noexcept;

    // Return true if:
    // The shader is syntactically and semantically valid
    bool findProperties() noexcept;
    bool runSemanticAnalysis() noexcept;

    bool checkLiteRequirements() noexcept;

    void writeCommonChunks(ChunkContainer& container, MaterialInfo& info) const noexcept;
    void writeSurfaceChunks(ChunkContainer& container) const noexcept;

    bool generateShaders(const std::vector<Variant>& variants, ChunkContainer& container,
            const MaterialInfo& info) const noexcept;

    bool isLit() const noexcept { return mShading != filament::Shading::UNLIT; }

    utils::CString mMaterialName;

    utils::CString mMaterialCode;
    utils::CString mMaterialVertexCode;
    size_t mMaterialLineOffset = 0;
    size_t mMaterialVertexLineOffset = 0;

    PropertyList mProperties;
    ParameterList mParameters;
    VariableList mVariables;

    BlendingMode mBlendingMode = BlendingMode::OPAQUE;
    BlendingMode mPostLightingBlendingMode = BlendingMode::TRANSPARENT;
    CullingMode mCullingMode = CullingMode::BACK;
    Shading mShading = Shading::LIT;
    MaterialDomain mMaterialDomain = MaterialDomain::SURFACE;
    Interpolation mInterpolation = Interpolation::SMOOTH;
    VertexDomain mVertexDomain = VertexDomain::OBJECT;
    TransparencyMode mTransparencyMode = TransparencyMode::DEFAULT;

    filament::AttributeBitset mRequiredAttributes;

    float mMaskThreshold = 0.4f;
    float mSpecularAntiAliasingVariance = 0.15f;
    float mSpecularAntiAliasingThreshold = 0.2f;

    bool mShadowMultiplier = false;

    uint8_t mParameterCount = 0;

    bool mDoubleSided = false;
    bool mDoubleSidedCapability = false;
    bool mColorWrite = true;
    bool mDepthTest = true;
    bool mDepthWrite = true;
    bool mDepthWriteSet = false;

    bool mSpecularAntiAliasing = false;
    bool mClearCoatIorChange = true;

    bool mFlipUV = true;

    bool mMultiBounceAO = false;
    bool mMultiBounceAOSet = false;
    bool mSpecularAO = false;
    bool mSpecularAOSet = false;
};

} // namespace filamat
#endif
