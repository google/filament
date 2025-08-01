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

//! \file

#ifndef TNT_FILAMAT_MATERIAL_PACKAGE_BUILDER_H
#define TNT_FILAMAT_MATERIAL_PACKAGE_BUILDER_H

#include <filament/MaterialEnums.h>

#include <filamat/IncludeCallback.h>
#include <filamat/Package.h>

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <mutex>
#include <utils/BitmaskEnum.h>
#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/CString.h>

#include <math/vec3.h>

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <variant>
#include <optional>

#include <stddef.h>
#include <stdint.h>

namespace utils {
class JobSystem;
}

namespace filament {
class BufferInterfaceBlock;
}

namespace filamat {

struct MaterialInfo;
struct Variant;
class ChunkContainer;

class UTILS_PUBLIC MaterialBuilderBase {
public:
    /**
     * High-level hint that works in concert with TargetApi to determine the shader models (used to
     * generate GLSL) and final output representations (spirv and/or text).
     * When generating the GLSL this is used to differentiate OpenGL from OpenGLES, it is also
     * used to make some performance adjustments.
     */
    enum class Platform {
        DESKTOP,
        MOBILE,
        ALL
    };

    /**
     * TargetApi defines which language after transpilation will be used, it is used to
     * account for some differences between these languages when generating the GLSL.
     */
    enum class TargetApi : uint8_t {
        OPENGL      = 0x01u,
        VULKAN      = 0x02u,
        METAL       = 0x04u,
        WEBGPU        = 0x08u,
#ifdef FILAMENT_SUPPORTS_WEBGPU
        ALL         = OPENGL | VULKAN | METAL | WEBGPU
#else
        ALL         = OPENGL | VULKAN | METAL
#endif
    };

    /*
     * Generally we generate GLSL that will be converted to SPIRV, optimized and then
     * transpiled to the backend's language such as MSL, ESSL300, GLSL410 or SPIRV, in this
     * case the generated GLSL uses ESSL310 or GLSL450 and has Vulkan semantics and
     * TargetLanguage::SPIRV must be used.
     *
     * However, in some cases (e.g. when no optimization is asked) we generate the *final* GLSL
     * directly, this GLSL must be ESSL300 or GLSL410 and cannot use any Vulkan syntax, for this
     * situation we use TargetLanguage::GLSL. In this case TargetApi is guaranteed to be OPENGL.
     *
     * Note that TargetLanguage::GLSL is not the common case, as it is generally not used in
     * release builds.
     *
     * Also note that glslang performs semantics analysis on whichever GLSL ends up being generated.
     */
    enum class TargetLanguage {
        GLSL,           // GLSL with OpenGL 4.1 / OpenGL ES 3.0 semantics
        SPIRV           // GLSL with Vulkan semantics
    };

    enum class Optimization {
        NONE,
        PREPROCESSOR,
        SIZE,
        PERFORMANCE
    };

    /**
     * Initialize MaterialBuilder.
     *
     * init must be called first before building any materials.
     */
    static void init();

    /**
     * Release internal MaterialBuilder resources.
     *
     * Call shutdown when finished building materials to release all internal resources. After
     * calling shutdown, another call to MaterialBuilder::init must precede another material build.
     */
    static void shutdown();

protected:
    // Looks at platform and target API, then decides on shader models and output formats.
    void prepare(bool vulkanSemantics, filament::backend::FeatureLevel featureLevel);

    using ShaderModel = filament::backend::ShaderModel;
    Platform mPlatform = Platform::DESKTOP;
    TargetApi mTargetApi = (TargetApi) 0;
    Optimization mOptimization = Optimization::PERFORMANCE;
    bool mPrintShaders = false;
    bool mSaveRawVariants = false;
    bool mGenerateDebugInfo = false;
    bool mIncludeEssl1 = true;
    utils::bitset32 mShaderModels;
    struct CodeGenParams {
        ShaderModel shaderModel;
        TargetApi targetApi;
        TargetLanguage targetLanguage;
        filament::backend::FeatureLevel featureLevel;
    };
    std::vector<CodeGenParams> mCodeGenPermutations;

    // Keeps track of how many times MaterialBuilder::init() has been called without a call to
    // MaterialBuilder::shutdown(). Internally, glslang does something similar. We keep track for
    // ourselves, so we can inform the user if MaterialBuilder::init() hasn't been called before
    // attempting to build a material.
    static std::atomic<int> materialBuilderClients;
};

// Utility function that looks at an Engine backend to determine TargetApi
inline constexpr MaterialBuilderBase::TargetApi targetApiFromBackend(
        filament::backend::Backend backend) noexcept {
    using filament::backend::Backend;
    using TargetApi = MaterialBuilderBase::TargetApi;
    switch (backend) {
        case Backend::DEFAULT: return TargetApi::ALL;
        case Backend::OPENGL:  return TargetApi::OPENGL;
        case Backend::VULKAN:  return TargetApi::VULKAN;
        case Backend::METAL:   return TargetApi::METAL;
        case Backend::WEBGPU:    return TargetApi::WEBGPU;
        case Backend::NOOP:    return TargetApi::OPENGL;
    }
}

/**
 * MaterialBuilder builds Filament materials from shader code.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * #include <filamat/MaterialBuilder.h>
 * using namespace filamat;
 *
 * // Must be called before any materials can be built.
 * MaterialBuilder::init();

 * MaterialBuilder builder;
 * builder
 *     .name("My material")
 *     .material("void material (inout MaterialInputs material) {"
 *               "  prepareMaterial(material);"
 *               "  material.baseColor.rgb = float3(1.0, 0.0, 0.0);"
 *               "}")
 *     .shading(MaterialBuilder::Shading::LIT)
 *     .targetApi(MaterialBuilder::TargetApi::ALL)
 *     .platform(MaterialBuilder::Platform::ALL);

 * Package package = builder.build();
 * if (package.isValid()) {
 *     // success!
 * }

 * // Call when finished building all materials to release internal
 * // MaterialBuilder resources.
 * MaterialBuilder::shutdown();
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @see filament::Material
 */
class UTILS_PUBLIC MaterialBuilder : public MaterialBuilderBase {
public:
    MaterialBuilder();
    ~MaterialBuilder();

    MaterialBuilder(const MaterialBuilder& rhs) = delete;
    MaterialBuilder& operator=(const MaterialBuilder& rhs) = delete;

    MaterialBuilder(MaterialBuilder&& rhs) noexcept = default;
    MaterialBuilder& operator=(MaterialBuilder&& rhs) noexcept = default;

    static constexpr size_t MATERIAL_VARIABLES_COUNT = 5;
    enum class Variable : uint8_t {
        CUSTOM0,
        CUSTOM1,
        CUSTOM2,
        CUSTOM3,
        CUSTOM4, // CUSTOM4 is only available if the vertex attribute `color` is not required.
        // when adding more variables, make sure to update MATERIAL_VARIABLES_COUNT
    };

    using MaterialDomain = filament::MaterialDomain;
    using RefractionMode = filament::RefractionMode;
    using RefractionType = filament::RefractionType;
    using ReflectionMode = filament::ReflectionMode;
    using VertexAttribute = filament::VertexAttribute;

    using ShaderQuality = filament::ShaderQuality;
    using BlendingMode = filament::BlendingMode;
    using BlendFunction = filament::backend::BlendFunction;
    using Shading = filament::Shading;
    using Interpolation = filament::Interpolation;
    using VertexDomain = filament::VertexDomain;
    using TransparencyMode = filament::TransparencyMode;
    using SpecularAmbientOcclusion = filament::SpecularAmbientOcclusion;

    using AttributeType = filament::backend::UniformType;
    using UniformType = filament::backend::UniformType;
    using ConstantType = filament::backend::ConstantType;
    using SamplerType = filament::backend::SamplerType;
    using SubpassType = filament::backend::SubpassType;
    using SamplerFormat = filament::backend::SamplerFormat;
    using ParameterPrecision = filament::backend::Precision;
    using Precision = filament::backend::Precision;
    using CullingMode = filament::backend::CullingMode;
    using FeatureLevel = filament::backend::FeatureLevel;
    using StereoscopicType = filament::backend::StereoscopicType;
    using ShaderStage = filament::backend::ShaderStage;
    using ShaderStageFlags = filament::backend::ShaderStageFlags;

    enum class VariableQualifier : uint8_t {
        OUT
    };

    enum class OutputTarget : uint8_t {
        COLOR,
        DEPTH
    };

    enum class OutputType : uint8_t {
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4
    };

    struct PreprocessorDefine {
        std::string name;
        std::string value;

        PreprocessorDefine(std::string  name, std::string  value) :
                name(std::move(name)), value(std::move(value)) {}
    };
    using PreprocessorDefineList = std::vector<PreprocessorDefine>;


    MaterialBuilder& noSamplerValidation(bool enabled) noexcept;

    //! Enable generation of ESSL 1.0 code in FL0 materials.
    MaterialBuilder& includeEssl1(bool enabled) noexcept;

    //! Set the name of this material.
    MaterialBuilder& name(const char* name) noexcept;

    //! Set the file name of this material file. Used in error reporting.
    MaterialBuilder& fileName(const char* name) noexcept;

    //! Set the shading model.
    MaterialBuilder& shading(Shading shading) noexcept;

    //! Set the interpolation mode.
    MaterialBuilder& interpolation(Interpolation interpolation) noexcept;

    //! Add a parameter (i.e., a uniform) to this material.
    MaterialBuilder& parameter(const char* name, UniformType type,
            ParameterPrecision precision = ParameterPrecision::DEFAULT) noexcept;

    //! Add a parameter array to this material.
    MaterialBuilder& parameter(const char* name, size_t size, UniformType type,
            ParameterPrecision precision = ParameterPrecision::DEFAULT);

    //! Add a constant parameter to this material.
    template<typename T>
    using is_supported_constant_parameter_t = typename std::enable_if<
            std::is_same<int32_t, T>::value ||
            std::is_same<float, T>::value ||
            std::is_same<bool, T>::value>::type;
    template<typename T, typename = is_supported_constant_parameter_t<T>>
    MaterialBuilder& constant(const char *name, ConstantType type, T defaultValue = 0);

    /**
     * Add a sampler parameter to this material.
     *
     * When SamplerType::SAMPLER_EXTERNAL is specified, format and precision are ignored.
     */
    MaterialBuilder& parameter(const char* name, SamplerType samplerType,
            SamplerFormat format = SamplerFormat::FLOAT,
            ParameterPrecision precision = ParameterPrecision::DEFAULT,
            bool filterable = true, /* defaulting to filterable because format is default to float */
            bool multisample = false, const char* transformName = "",
            std::optional<ShaderStageFlags> stages = {});

    MaterialBuilder& buffer(filament::BufferInterfaceBlock bib);

    //! Custom variables (all float4).
    MaterialBuilder& variable(Variable v, const char* name) noexcept;

    MaterialBuilder& variable(Variable v, const char* name,
            ParameterPrecision precision) noexcept;

    /**
     * Require a specified attribute.
     *
     * position is always required and normal depends on the shading model.
     */
    MaterialBuilder& require(VertexAttribute attribute) noexcept;

    //! Specify the domain that this material will operate in.
    MaterialBuilder& materialDomain(MaterialBuilder::MaterialDomain materialDomain) noexcept;

    /**
     * Set the code content of this material.
     *
     * Surface Domain
     * --------------
     *
     * Materials in the SURFACE domain must declare a function:
     * ~~~~~
     * void material(inout MaterialInputs material) {
     *     prepareMaterial(material);
     *     material.baseColor.rgb = float3(1.0, 0.0, 0.0);
     * }
     * ~~~~~
     * this function *must* call `prepareMaterial(material)` before it returns.
     *
     * Post-process Domain
     * -------------------
     *
     * Materials in the POST_PROCESS domain must declare a function:
     * ~~~~~
     * void postProcess(inout PostProcessInputs postProcess) {
     *     postProcess.color = float4(1.0);
     * }
     * ~~~~~
     *
     * @param code The source code of the material.
     * @param line The line number offset of the material, where 0 is the first line. Used for error
     *             reporting
     */
    MaterialBuilder& material(const char* code, size_t line = 0) noexcept;

    /**
     * Set the callback used for resolving include directives.
     * The default is no callback, which disallows all includes.
     */
    MaterialBuilder& includeCallback(IncludeCallback callback) noexcept;

    /**
     * Set the vertex code content of this material.
     *
     * Surface Domain
     * --------------
     *
     * Materials in the SURFACE domain must declare a function:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * void materialVertex(inout MaterialVertexInputs material) {
     *
     * }
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     * Post-process Domain
     * -------------------
     *
     * Materials in the POST_PROCESS domain must declare a function:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * void postProcessVertex(inout PostProcessVertexInputs postProcess) {
     *
     * }
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

     * @param code The source code of the material.
     * @param line The line number offset of the material, where 0 is the first line. Used for error
     *             reporting
     */
    MaterialBuilder& materialVertex(const char* code, size_t line = 0) noexcept;


    MaterialBuilder& quality(ShaderQuality quality) noexcept;

    MaterialBuilder& featureLevel(FeatureLevel featureLevel) noexcept;

    /**
     * Set the blending mode for this material. When set to MASKED, alpha to coverage is turned on.
     * You can override this behavior using alphaToCoverage(false).
     */
    MaterialBuilder& blending(BlendingMode blending) noexcept;

    /**
     * Set the blend function  for this material. blending must be et to CUSTOM.
     */
    MaterialBuilder& customBlendFunctions(
            BlendFunction srcRGB,
            BlendFunction srcA,
            BlendFunction dstRGB,
            BlendFunction dstA) noexcept;

    /**
     * Set the blending mode of the post-lighting color for this material.
     * Only OPAQUE, TRANSPARENT and ADD are supported, the default is TRANSPARENT.
     * This setting requires the material properties "postLightingColor" and
     * "postLightingMixFactor" to be set.
     */
    MaterialBuilder& postLightingBlending(BlendingMode blending) noexcept;

    //! Set the vertex domain for this material.
    MaterialBuilder& vertexDomain(VertexDomain domain) noexcept;

    /**
     * How triangles are culled by default (doesn't affect points or lines, BACK by default).
     * Material instances can override this.
     */
    MaterialBuilder& culling(CullingMode culling) noexcept;

    //! Enable / disable color-buffer write (enabled by default, material instances can override).
    MaterialBuilder& colorWrite(bool enable) noexcept;

    //! Enable / disable depth-buffer write (enabled by default for opaque, disabled for others, material instances can override).
    MaterialBuilder& depthWrite(bool enable) noexcept;

    //! Enable / disable depth based culling (enabled by default, material instances can override).
    MaterialBuilder& depthCulling(bool enable) noexcept;

    //! Enable / disable instanced primitives (disabled by default).
    MaterialBuilder& instanced(bool enable) noexcept;

    /**
     * Double-sided materials don't cull faces, equivalent to culling(CullingMode::NONE).
     * doubleSided() overrides culling() if called.
     * When called with "false", this enables the capability for a run-time toggle.
     */
    MaterialBuilder& doubleSided(bool doubleSided) noexcept;

    /**
     * Any fragment with an alpha below this threshold is clipped (MASKED blending mode only).
     * The mask threshold can also be controlled by using the float material parameter called
     * `_maskThreshold`, or by calling
     * @ref filament::MaterialInstance::setMaskThreshold "MaterialInstance::setMaskThreshold".
     */
    MaterialBuilder& maskThreshold(float threshold) noexcept;

    /**
     * Enables or disables alpha-to-coverage. When enabled, the coverage of a fragment is based
     * on its alpha value. This parameter is only useful when MSAA is in use. Alpha to coverage
     * is enabled automatically when the blend mode is set to MASKED; this behavior can be
     * overridden by calling alphaToCoverage(false).
     */
    MaterialBuilder& alphaToCoverage(bool enable) noexcept;

    //! The material output is multiplied by the shadowing factor (UNLIT model only).
    MaterialBuilder& shadowMultiplier(bool shadowMultiplier) noexcept;

    //! This material casts transparent shadows. The blending mode must be TRANSPARENT or FADE.
    MaterialBuilder& transparentShadow(bool transparentShadow) noexcept;

    /**
     * Reduces specular aliasing for materials that have low roughness. Turning this feature on also
     * helps preserve the shapes of specular highlights as an object moves away from the camera.
     * When turned on, two float material parameters are added to control the effect:
     * `_specularAAScreenSpaceVariance` and `_specularAAThreshold`. You can also use
     * @ref filament::MaterialInstance::setSpecularAntiAliasingVariance
     * "MaterialInstance::setSpecularAntiAliasingVariance" and
     * @ref filament::MaterialInstance::setSpecularAntiAliasingThreshold
     * "setSpecularAntiAliasingThreshold"
     *
     * Disabled by default.
     */
    MaterialBuilder& specularAntiAliasing(bool specularAntiAliasing) noexcept;

    /**
     * Sets the screen-space variance of the filter kernel used when applying specular
     * anti-aliasing. The default value is set to 0.15. The specified value should be between 0 and
     * 1 and will be clamped if necessary.
     */
    MaterialBuilder& specularAntiAliasingVariance(float screenSpaceVariance) noexcept;

    /**
     * Sets the clamping threshold used to suppress estimation errors when applying specular
     * anti-aliasing. The default value is set to 0.2. The specified value should be between 0 and 1
     * and will be clamped if necessary.
     */
    MaterialBuilder& specularAntiAliasingThreshold(float threshold) noexcept;

    /**
     * Enables or disables the index of refraction (IoR) change caused by the clear coat layer when
     * present. When the IoR changes, the base color is darkened. Disabling this feature preserves
     * the base color as initially specified.
     *
     * Enabled by default.
     */
    MaterialBuilder& clearCoatIorChange(bool clearCoatIorChange) noexcept;

    //! Enable / disable flipping of the Y coordinate of UV attributes, enabled by default.
    MaterialBuilder& flipUV(bool flipUV) noexcept;

    //! Enable / disable multi-bounce ambient occlusion, disabled by default on mobile.
    MaterialBuilder& multiBounceAmbientOcclusion(bool multiBounceAO) noexcept;

    //! Set the specular ambient occlusion technique. Disabled by default on mobile.
    MaterialBuilder& specularAmbientOcclusion(SpecularAmbientOcclusion specularAO) noexcept;

    //! Specify the refraction
    MaterialBuilder& refractionMode(RefractionMode refraction) noexcept;

    //! Specify the refraction type
    MaterialBuilder& refractionType(RefractionType refractionType) noexcept;

    //! Specifies how reflections should be rendered (default is DEFAULT).
    MaterialBuilder& reflectionMode(ReflectionMode mode) noexcept;

    //! Specifies how transparent objects should be rendered (default is DEFAULT).
    MaterialBuilder& transparencyMode(TransparencyMode mode) noexcept;

    //! Specify the stereoscopic type (default is INSTANCED)
    MaterialBuilder& stereoscopicType(StereoscopicType stereoscopicType) noexcept;

    //! Specify the number of eyes for stereoscopic rendering
    MaterialBuilder& stereoscopicEyeCount(uint8_t eyeCount) noexcept;

    /**
     * Enable / disable custom surface shading. Custom surface shading requires the LIT
     * shading model. In addition, the following function must be defined in the fragment
     * block:
     *
     * ~~~~~
     * vec3 surfaceShading(const MaterialInputs materialInputs,
     *         const ShadingData shadingData, const LightData lightData) {
     *
     *     return vec3(1.0); // Compute surface shading with custom BRDF, etc.
     * }
     * ~~~~~
     *
     * This function is invoked once per light. Please refer to the materials documentation
     * for more information about the different parameters.
     *
     * @param customSurfaceShading Enables or disables custom surface shading
     */
    MaterialBuilder& customSurfaceShading(bool customSurfaceShading) noexcept;

    /**
     * Specifies desktop vs mobile; works in concert with TargetApi to determine the shader models
     * (used to generate code) and final output representations (spirv and/or text).
     */
    MaterialBuilder& platform(Platform platform) noexcept;

    /**
     * Specifies OpenGL, Vulkan, or Metal.
     * This can be called repeatedly to build for multiple APIs.
     * Works in concert with Platform to determine the shader models (used to generate code) and
     * final output representations (spirv and/or text).
     * If linking against filamat_lite, only `OPENGL` is allowed.
     */
    MaterialBuilder& targetApi(TargetApi targetApi) noexcept;

    /**
     * Specifies the level of optimization to apply to the shaders (default is PERFORMANCE).
     * If linking against filamat_lite, this _must_ be called with Optimization::NONE.
     */
    MaterialBuilder& optimization(Optimization optimization) noexcept;

    // TODO: this is present here for matc's "--print" flag, but ideally does not belong inside MaterialBuilder.
    //! If true, will output the generated GLSL shader code to stdout.
    MaterialBuilder& printShaders(bool printShaders) noexcept;

    /**
     * If true, this will write the raw generated GLSL for each variant to a text file in the
     * current directory. The file will be named after the material name and the variant name. Its
     * extension will be derived from the shader stage. For example, mymaterial_0x0e.frag,
     * mymaterial_0x18.vert, etc.
     */
    MaterialBuilder& saveRawVariants(bool saveRawVariants) noexcept;

    //! If true, will include debugging information in generated SPIRV.
    MaterialBuilder& generateDebugInfo(bool generateDebugInfo) noexcept;

    //! Specifies a list of variants that should be filtered out during code generation.
    MaterialBuilder& variantFilter(filament::UserVariantFilterMask variantFilter) noexcept;

    //! Adds a new preprocessor macro definition to the shader code. Can be called repeatedly.
    MaterialBuilder& shaderDefine(const char* name, const char* value) noexcept;

    //! Add a new fragment shader output variable. Only valid for materials in the POST_PROCESS domain.
    MaterialBuilder& output(VariableQualifier qualifier, OutputTarget target, Precision precision,
            OutputType type, const char* name, int location = -1);

    MaterialBuilder& enableFramebufferFetch() noexcept;

    MaterialBuilder& vertexDomainDeviceJittered(bool enabled) noexcept;

    /**
     * Legacy morphing uses the data in the VertexAttribute slots (\c MORPH_POSITION_0, etc) and is
     * limited to 4 morph targets. See filament::RenderableManager::Builder::morphing().
     */
    MaterialBuilder& useLegacyMorphing() noexcept;

    //! specify compute kernel group size
    MaterialBuilder& groupSize(filament::math::uint3 groupSize) noexcept;

    /**
     * Force Filament to use its default variant for depth passes. Useful if a material provides a
     * custom vertex shader which can be skipped during depth-only passes.
     */
    MaterialBuilder& useDefaultDepthVariant() noexcept;

    /**
     * Build the material. If you are using the Filament engine with this library, you should use
     * the job system provided by Engine.
     */
    Package build(utils::JobSystem& jobSystem);

public:
    // The methods and types below are for internal use
    /// @cond never

    /**
     * Add a subpass parameter to this material.
     */
    MaterialBuilder& subpass(SubpassType subpassType,
            SamplerFormat format, ParameterPrecision precision, const char* name);
    MaterialBuilder& subpass(SubpassType subpassType,
            SamplerFormat format, const char* name);
    MaterialBuilder& subpass(SubpassType subpassType,
            ParameterPrecision precision, const char* name);
    MaterialBuilder& subpass(SubpassType subpassType, const char* name);

    struct Parameter {
        Parameter() noexcept: parameterType(INVALID) {}

        // Sampler
        Parameter(const char* paramName, SamplerType t, SamplerFormat f, ParameterPrecision p,
                bool filterable, bool ms, const char* tn, std::optional<ShaderStageFlags> s)
            : name(paramName),
              size(1),
              precision(p),
              samplerType(t),
              format(f),
              filterable(filterable),
              multisample(ms),
              transformName(tn),
              stages(s),
              parameterType(SAMPLER) {}

        // Uniform
        Parameter(const char* paramName, UniformType t, size_t typeSize, ParameterPrecision p)
            : name(paramName),
              size(typeSize),
              uniformType(t),
              precision(p),
              format{ 0 },
              filterable(false),
              multisample(false),
              parameterType(UNIFORM) {}

        // Subpass
        Parameter(const char* paramName, SubpassType t, SamplerFormat f, ParameterPrecision p)
            : name(paramName),
              size(1),
              precision(p),
              subpassType(t),
              format(f),
              filterable(false),
              multisample(false),
              parameterType(SUBPASS) {}

        utils::CString name;
        size_t size;
        UniformType uniformType;
        ParameterPrecision precision;
        SamplerType samplerType;
        SubpassType subpassType;
        SamplerFormat format;
        bool filterable;
        bool multisample;
        utils::CString transformName;
        std::optional<ShaderStageFlags> stages;
        enum {
            INVALID,
            UNIFORM,
            SAMPLER,
            SUBPASS
        } parameterType;

        bool isSampler() const { return parameterType == SAMPLER; }
        bool isUniform() const { return parameterType == UNIFORM; }
        bool isSubpass() const { return parameterType == SUBPASS; }
    };

    struct Output {
        Output() noexcept = default;
        Output(const char* outputName, VariableQualifier qualifier, OutputTarget target,
                Precision precision, OutputType type, int location) noexcept
                : name(outputName), qualifier(qualifier), target(target), precision(precision),
                  type(type), location(location) { }

        utils::CString name;
        VariableQualifier qualifier;
        OutputTarget target;
        Precision precision;
        OutputType type;
        int location;
    };

    struct Constant {
        utils::CString name;
        ConstantType type;
        union {
            int32_t i;
            float f;
            bool b;
        } defaultValue;
    };

    struct PushConstant {
        utils::CString name;
        ConstantType type;
        ShaderStage stage;
    };

    struct CustomVariable {
        utils::CString name;
        Precision precision = Precision::DEFAULT;
        bool hasPrecision = false;
    };

    static constexpr size_t MATERIAL_PROPERTIES_COUNT = filament::MATERIAL_PROPERTIES_COUNT;
    using Property = filament::Property;

    using PropertyList = bool[MATERIAL_PROPERTIES_COUNT];
    using VariableList = CustomVariable[MATERIAL_VARIABLES_COUNT];
    using OutputList = std::vector<Output>;

    static constexpr size_t MAX_COLOR_OUTPUT = filament::backend::MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT;
    static constexpr size_t MAX_DEPTH_OUTPUT = 1;
    static_assert(MAX_COLOR_OUTPUT == 8,
            "When updating MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT, manually update post_process_inputs.fs"
            " and post_process_main.fs");

    // Preview the first shader generated by the given CodeGenParams.
    // This is used to run Static Code Analysis before generating a package.
    std::string peek(filament::backend::ShaderStage type,
            const CodeGenParams& params, const PropertyList& properties) noexcept;

    // Returns true if any of the parameter samplers matches the specified type.
    bool hasSamplerType(SamplerType samplerType) const noexcept;

    static constexpr size_t MAX_PARAMETERS_COUNT = 48;
    static constexpr size_t MAX_SUBPASS_COUNT = 1;
    static constexpr size_t MAX_BUFFERS_COUNT = 4;
    using ParameterList = Parameter[MAX_PARAMETERS_COUNT];
    using SubpassList = Parameter[MAX_SUBPASS_COUNT];
    using BufferList = std::vector<std::unique_ptr<filament::BufferInterfaceBlock>>;
    using ConstantList = std::vector<Constant>;
    using PushConstantList = std::vector<PushConstant>;

    // returns the number of parameters declared in this material
    uint8_t getParameterCount() const noexcept { return mParameterCount; }

    // returns a list of at least getParameterCount() parameters
    const ParameterList& getParameters() const noexcept { return mParameters; }

    // returns the number of parameters declared in this material
    uint8_t getSubpassCount() const noexcept { return mSubpassCount; }

    // returns a list of at least getParameterCount() parameters
    const SubpassList& getSubPasses() const noexcept { return mSubpasses; }

    filament::UserVariantFilterMask getVariantFilter() const { return mVariantFilter; }

    FeatureLevel getFeatureLevel() const noexcept { return mFeatureLevel; }
    /// @endcond

    struct Attribute {
        std::string_view name;
        AttributeType type;
        MaterialBuilder::VertexAttribute location;
        std::string getAttributeName() const noexcept {
            return "mesh_" + std::string{ name };
        }
        std::string getDefineName() const noexcept {
            std::string uppercase{ name };
            transform(uppercase.cbegin(), uppercase.cend(), uppercase.begin(), ::toupper);
            return "HAS_ATTRIBUTE_" + uppercase;
        }
    };

    using AttributeDatabase = std::array<Attribute, filament::backend::MAX_VERTEX_ATTRIBUTE_COUNT>;

    static inline AttributeDatabase const& getAttributeDatabase() noexcept {
        return sAttributeDatabase;
    }

private:
    static const AttributeDatabase sAttributeDatabase;

    void prepareToBuild(MaterialInfo& info) noexcept;

    // Initialize internal push constants that will both be written to the shaders and material
    // chunks (like user-defined spec constants).
    void initPushConstants() noexcept;

    // Return true if the shader is syntactically and semantically valid.
    // This method finds all the properties defined in the fragment and
    // vertex shaders of the material.
    bool findAllProperties(CodeGenParams const& semanticCodeGenParams) noexcept;

    // Multiple calls to findProperties accumulate the property sets across fragment
    // and vertex shaders in mProperties.
    bool findProperties(filament::backend::ShaderStage type,
            MaterialBuilder::PropertyList const& allProperties,
            CodeGenParams const& semanticCodeGenParams) noexcept;

    bool runSemanticAnalysis(MaterialInfo* inOutInfo,
            CodeGenParams const& semanticCodeGenParams) noexcept;

    bool checkLiteRequirements() noexcept;

    bool checkMaterialLevelFeatures(MaterialInfo const& info) const noexcept;

    void writeCommonChunks(ChunkContainer& container, MaterialInfo& info) const noexcept;
    void writeSurfaceChunks(ChunkContainer& container) const noexcept;

    bool generateShaders(
            utils::JobSystem& jobSystem,
            const std::vector<filamat::Variant>& variants, ChunkContainer& container,
            const MaterialInfo& info) const noexcept;

    bool hasCustomVaryings() const noexcept;
    bool needsStandardDepthProgram() const noexcept;

    bool isLit() const noexcept { return mShading != filament::Shading::UNLIT; }

    utils::CString mMaterialName;
    utils::CString mFileName;

    class ShaderCode {
    public:
        void setLineOffset(size_t offset) noexcept { mLineOffset = offset; }
        void setUnresolved(const utils::CString& code) noexcept {
            mIncludesResolved = false;
            mCode = code;
        }

        // Resolve all the #include directives, returns true if successful.
        bool resolveIncludes(IncludeCallback callback, const utils::CString& fileName) noexcept;

        const utils::CString& getResolved() const noexcept {
            assert(mIncludesResolved);
            return mCode;
        }

        size_t getLineOffset() const noexcept { return mLineOffset; }

    private:
        utils::CString mCode;
        size_t mLineOffset = 0;
        bool mIncludesResolved = false;
    };

    ShaderCode mMaterialFragmentCode;
    ShaderCode mMaterialVertexCode;

    IncludeCallback mIncludeCallback = nullptr;

    PropertyList mProperties;
    ParameterList mParameters;
    ConstantList mConstants;
    PushConstantList mPushConstants;
    SubpassList mSubpasses;
    VariableList mVariables;
    OutputList mOutputs;
    BufferList mBuffers;

    ShaderQuality mShaderQuality = ShaderQuality::DEFAULT;
    FeatureLevel mFeatureLevel = FeatureLevel::FEATURE_LEVEL_1;
    BlendingMode mBlendingMode = BlendingMode::OPAQUE;
    BlendingMode mPostLightingBlendingMode = BlendingMode::TRANSPARENT;
    std::array<BlendFunction, 4> mCustomBlendFunctions = {};
    CullingMode mCullingMode = CullingMode::BACK;
    Shading mShading = Shading::LIT;
    MaterialDomain mMaterialDomain = MaterialDomain::SURFACE;
    RefractionMode mRefractionMode = RefractionMode::NONE;
    RefractionType mRefractionType = RefractionType::SOLID;
    ReflectionMode mReflectionMode = ReflectionMode::DEFAULT;
    Interpolation mInterpolation = Interpolation::SMOOTH;
    VertexDomain mVertexDomain = VertexDomain::OBJECT;
    TransparencyMode mTransparencyMode = TransparencyMode::DEFAULT;
    StereoscopicType mStereoscopicType = StereoscopicType::INSTANCED;
    uint8_t mStereoscopicEyeCount = 2;

    filament::AttributeBitset mRequiredAttributes;

    float mMaskThreshold = 0.4f;
    float mSpecularAntiAliasingVariance = 0.15f;
    float mSpecularAntiAliasingThreshold = 0.2f;

    filament::math::uint3 mGroupSize = { 1, 1, 1 };

    bool mShadowMultiplier = false;
    bool mTransparentShadow = false;

    uint8_t mParameterCount = 0;
    uint8_t mSubpassCount = 0;

    bool mDoubleSided = false;
    bool mDoubleSidedCapability = false;
    bool mColorWrite = true;
    bool mDepthTest = true;
    bool mInstanced = false;
    bool mDepthWrite = true;
    bool mDepthWriteSet = false;
    bool mAlphaToCoverage = false;
    bool mAlphaToCoverageSet = false;

    bool mSpecularAntiAliasing = false;
    bool mClearCoatIorChange = true;

    bool mFlipUV = true;

    bool mMultiBounceAO = false;
    bool mMultiBounceAOSet = false;

    SpecularAmbientOcclusion mSpecularAO = SpecularAmbientOcclusion::NONE;
    bool mSpecularAOSet = false;

    bool mCustomSurfaceShading = false;

    bool mEnableFramebufferFetch = false;

    bool mVertexDomainDeviceJittered = false;

    bool mUseLegacyMorphing = false;

    PreprocessorDefineList mDefines;

    filament::UserVariantFilterMask mVariantFilter = {};

    bool mNoSamplerValidation = false;

    bool mUseDefaultDepthVariant = false;
};

} // namespace filamat

template<> struct utils::EnableBitMaskOperators<filamat::MaterialBuilder::TargetApi>
        : public std::true_type {};

#endif
