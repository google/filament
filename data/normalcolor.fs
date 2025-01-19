/////////////////////////////// Source file 0/////////////////////////////
#version 410 core

/////////////////////////////// Source file 1/////////////////////////////

#extension GL_ARB_shading_language_packing : enable

#extension GL_GOOGLE_cpp_style_line_directive : enable

/////////////////////////////// Source file 2/////////////////////////////
#define SPIRV_CROSS_CONSTANT_ID_0 3
#define SPIRV_CROSS_CONSTANT_ID_1 1
#define SPIRV_CROSS_CONSTANT_ID_4 2048
#define SPIRV_CROSS_CONSTANT_ID_6 false
#define SPIRV_CROSS_CONSTANT_ID_7 false
#define SPIRV_CROSS_CONSTANT_ID_2 false
#define SPIRV_CROSS_CONSTANT_ID_5 false
#define SPIRV_CROSS_CONSTANT_ID_8 2
#define SPIRV_CROSS_CONSTANT_ID_9 3
#define SPIRV_CROSS_CONSTANT_ID_10 1


/////////////////////////////// Source file 3/////////////////////////////

#define TARGET_GL_ENVIRONMENT
#define FILAMENT_OPENGL_SEMANTICS
#define FILAMENT_HAS_FEATURE_TEXTURE_GATHER
#define FILAMENT_HAS_FEATURE_INSTANCING
#define FILAMENT_EFFECTIVE_VERSION __VERSION__
#define FILAMENT_STEREO_INSTANCED
#define VARYING in
#define SHADING_MODEL_UNLIT
#define FILAMENT_QUALITY_LOW    0
#define FILAMENT_QUALITY_NORMAL 1
#define FILAMENT_QUALITY_HIGH   2
#define FILAMENT_QUALITY FILAMENT_QUALITY_HIGH

precision highp float;
precision highp int;

#ifndef SPIRV_CROSS_CONSTANT_ID_0
#define SPIRV_CROSS_CONSTANT_ID_0 1
#endif
const int BACKEND_FEATURE_LEVEL = SPIRV_CROSS_CONSTANT_ID_0;

#ifndef SPIRV_CROSS_CONSTANT_ID_1
#define SPIRV_CROSS_CONSTANT_ID_1 64
#endif
const int CONFIG_MAX_INSTANCES = SPIRV_CROSS_CONSTANT_ID_1;

#ifndef SPIRV_CROSS_CONSTANT_ID_4
#define SPIRV_CROSS_CONSTANT_ID_4 1024
#endif
const int CONFIG_FROXEL_BUFFER_HEIGHT = SPIRV_CROSS_CONSTANT_ID_4;

#ifndef SPIRV_CROSS_CONSTANT_ID_6
#define SPIRV_CROSS_CONSTANT_ID_6 false
#endif
const bool CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP = SPIRV_CROSS_CONSTANT_ID_6;

#ifndef SPIRV_CROSS_CONSTANT_ID_7
#define SPIRV_CROSS_CONSTANT_ID_7 false
#endif
const bool CONFIG_DEBUG_FROXEL_VISUALIZATION = SPIRV_CROSS_CONSTANT_ID_7;

#ifndef SPIRV_CROSS_CONSTANT_ID_2
#define SPIRV_CROSS_CONSTANT_ID_2 false
#endif
const bool CONFIG_STATIC_TEXTURE_TARGET_WORKAROUND = SPIRV_CROSS_CONSTANT_ID_2;

#ifndef SPIRV_CROSS_CONSTANT_ID_5
#define SPIRV_CROSS_CONSTANT_ID_5 false
#endif
const bool CONFIG_POWER_VR_SHADER_WORKAROUNDS = SPIRV_CROSS_CONSTANT_ID_5;

#ifndef SPIRV_CROSS_CONSTANT_ID_8
#define SPIRV_CROSS_CONSTANT_ID_8 2
#endif
const int CONFIG_STEREO_EYE_COUNT = SPIRV_CROSS_CONSTANT_ID_8;

#ifndef SPIRV_CROSS_CONSTANT_ID_9
#define SPIRV_CROSS_CONSTANT_ID_9 3
#endif
const int CONFIG_SH_BANDS_COUNT = SPIRV_CROSS_CONSTANT_ID_9;

#ifndef SPIRV_CROSS_CONSTANT_ID_10
#define SPIRV_CROSS_CONSTANT_ID_10 1
#endif
const int CONFIG_SHADOW_SAMPLING_METHOD = SPIRV_CROSS_CONSTANT_ID_10;

#define CONFIG_MAX_STEREOSCOPIC_EYES 4

#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                      
#endif
#if defined(FILAMENT_VULKAN_SEMANTICS)
#define LAYOUT_LOCATION(x) layout(location = x)
#else
#define LAYOUT_LOCATION(x)
#endif

#define bool2    bvec2
#define bool3    bvec3
#define bool4    bvec4

#define int2     ivec2
#define int3     ivec3
#define int4     ivec4

#define uint2    uvec2
#define uint3    uvec3
#define uint4    uvec4

#define float2   vec2
#define float3   vec3
#define float4   vec4

#define float3x3 mat3
#define float4x4 mat4

// To workaround an adreno crash (#5294), we need ensure that a method with
// parameter 'const mat4' does not call another method also with a 'const mat4'
// parameter (i.e. mulMat4x4Float3). So we remove the const modifier for
// materials compiled for vulkan+mobile.
#if defined(TARGET_VULKAN_ENVIRONMENT) && defined(TARGET_MOBILE)
   #define highp_mat4 highp mat4
#else
   #define highp_mat4 const highp mat4
#endif
#define gl_MaxVaryingVectors 8
#define texture2D texture
#define texture2DProj textureProj
#define texture3D texture
#define texture3DProj textureProj
#define textureCube texture


#define MATERIAL_FEATURE_LEVEL 1

#define BLEND_MODE_OPAQUE
#define POST_LIGHTING_BLEND_MODE_TRANSPARENT

#define CLEAR_COAT_IOR_CHANGE
#define SPECULAR_AMBIENT_OCCLUSION 1
#define MULTI_BOUNCE_AMBIENT_OCCLUSION 1
#define MATERIAL_HAS_BASE_COLOR
#define SHADING_INTERPOLATION 

#define HAS_ATTRIBUTE_POSITION
#define HAS_ATTRIBUTE_COLOR
#define HAS_ATTRIBUTE_CUSTOM0

#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                
#endif
//------------------------------------------------------------------------------
// Varyings
//------------------------------------------------------------------------------

LAYOUT_LOCATION(4) VARYING highp vec4 vertex_worldPosition;

#if defined(HAS_ATTRIBUTE_TANGENTS)
LAYOUT_LOCATION(5) SHADING_INTERPOLATION VARYING mediump vec3 vertex_worldNormal;
#if defined(MATERIAL_NEEDS_TBN)
LAYOUT_LOCATION(6) SHADING_INTERPOLATION VARYING mediump vec4 vertex_worldTangent;
#endif
#endif

LAYOUT_LOCATION(7) VARYING highp vec4 vertex_position;

#if defined(FILAMENT_HAS_FEATURE_INSTANCING)
LAYOUT_LOCATION(8) flat VARYING highp int instance_index;
highp int logical_instance_index;
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
LAYOUT_LOCATION(9) VARYING mediump vec4 vertex_color;
#endif

#if defined(HAS_ATTRIBUTE_UV0) && !defined(HAS_ATTRIBUTE_UV1)
LAYOUT_LOCATION(10) VARYING highp vec2 vertex_uv01;
#elif defined(HAS_ATTRIBUTE_UV1)
LAYOUT_LOCATION(10) VARYING highp vec4 vertex_uv01;
#endif

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
LAYOUT_LOCATION(11) VARYING highp vec4 vertex_lightSpacePosition;
#endif

// Note that fragColor is an output and is not declared here; see main.fs and depth_main.fs

#if defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_INSTANCED)
#if defined(GL_ES) && defined(FILAMENT_GLSLANG)
// On ES, gl_ClipDistance is not a built-in, so we have to rely on EXT_clip_cull_distance
// However, this extension is not supported by glslang, so we instead write to
// filament_gl_ClipDistance, which will get decorated at the SPIR-V stage to refer to the built-in.
// The location here does not matter, so long as it doesn't conflict with others.
LAYOUT_LOCATION(100) out float filament_gl_ClipDistance[2];
#define FILAMENT_CLIPDISTANCE filament_gl_ClipDistance
#else
// If we're on Desktop GL (or not running shaders through glslang), we're free to use gl_ClipDistance
#define FILAMENT_CLIPDISTANCE gl_ClipDistance
#endif // GL_ES && FILAMENT_GLSLANG
#endif // VARIANT_HAS_STEREO


#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                    
#endif
#if defined(VARIANT_HAS_SHADOWING)
// Adreno drivers seem to ignore precision qualifiers in structs, unless they're used in
// UBOs, which is the case here.
struct ShadowData {
    highp mat4 lightFromWorldMatrix;
    highp vec4 lightFromWorldZ;
    highp vec4 scissorNormalized;
    mediump float texelSizeAtOneMeter;
    mediump float bulbRadiusLs;
    mediump float nearOverFarMinusNear;
    mediump float normalBias;
    bool elvsm;
    mediump uint layer;
    mediump uint reserved1;
    mediump uint reserved2;
};
#endif

#if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
struct BoneData {
    highp mat3x4 transform;    // bone transform is mat4x3 stored in row-major (last row [0,0,0,1])
    highp float3 cof0;         // 3 first cofactor matrix of transform's upper left
    highp float cof1x;         // 4th cofactor
};
#endif

struct PerRenderableData {
    highp mat4 worldFromModelMatrix;
    highp mat3 worldFromModelNormalMatrix;
    highp int morphTargetCount;
    highp int flagsChannels;                   // see packFlags() below (0x00000fll)
    highp int objectId;                        // used for picking
    highp float userData;   // TODO: We need a better solution, this currently holds the average local scale for the renderable
    highp vec4 reserved[8];
};

// Bits for flagsChannels
#define FILAMENT_OBJECT_SKINNING_ENABLED_BIT   0x100
#define FILAMENT_OBJECT_MORPHING_ENABLED_BIT   0x200
#define FILAMENT_OBJECT_CONTACT_SHADOWS_BIT    0x400
#define FILAMENT_OBJECT_INSTANCE_BUFFER_BIT    0x800

LAYOUT_LOCATION(0) VARYING highp vec4 variable_customnormal;

layout(std140) uniform FrameUniforms {
    mat4 viewFromWorldMatrix;
    mat4 worldFromViewMatrix;
    mat4 clipFromViewMatrix;
    mat4 viewFromClipMatrix;
    mat4 clipFromWorldMatrix[4];
    mat4 worldFromClipMatrix;
    mat4 userWorldFromWorldMatrix;
    vec4 clipTransform;
    vec2 clipControl;
    float time;
    float temporalNoise;
    vec4 userTime;
    vec4 resolution;
    vec2 logicalViewportScale;
    vec2 logicalViewportOffset;
    float lodBias;
    float refractionLodOffset;
    lowp vec2 derivativesScale;
    float oneOverFarMinusNear;
    float nearOverFarMinusNear;
    float cameraFar;
    float exposure;
    float ev100;
    float needsAlphaChannel;
    lowp float aoSamplingQualityAndEdgeDistance;
    lowp float aoBentNormals;
    lowp vec4 zParams;
    lowp uvec3 fParams;
    lowp int lightChannels;
    lowp vec2 froxelCountXY;
    float iblLuminance;
    float iblRoughnessOneLevel;
    lowp vec3 iblSH[9];
    vec3 lightDirection;
    lowp float padding0;
    vec4 lightColorIntensity;
    vec4 sun;
    vec2 shadowFarAttenuationParams;
    lowp int directionalShadows;
    lowp float ssContactShadowDistance;
    vec4 cascadeSplits;
    lowp int cascades;
    lowp float shadowPenumbraRatioScale;
    vec2 lightFarAttenuationParams;
    lowp float vsmExponent;
    lowp float vsmDepthScale;
    lowp float vsmLightBleedReduction;
    lowp uint shadowSamplingType;
    vec3 fogDensity;
    float fogStart;
    float fogMaxOpacity;
    uint fogMinMaxMip;
    float fogHeightFalloff;
    float fogCutOffDistance;
    vec3 fogColor;
    float fogColorFromIbl;
    float fogInscatteringStart;
    float fogInscatteringSize;
    float fogOneOverFarMinusNear;
    float fogNearOverFarMinusNear;
    mat3 fogFromWorldMatrix;
    mat4 ssrReprojection;
    mat4 ssrUvFromViewMatrix;
    lowp float ssrThickness;
    lowp float ssrBias;
    lowp float ssrDistance;
    lowp float ssrStride;
    vec4 custom[4];
    int rec709;
    lowp float es2Reserved0;
    lowp float es2Reserved1;
    lowp float es2Reserved2;
    lowp vec4 reserved[40];
} frameUniforms;

layout(std140) uniform ObjectUniforms {
    PerRenderableData data[CONFIG_MAX_INSTANCES];
} objectUniforms;

uniform mediump sampler2DArrayShadow sampler0_shadowMap;
uniform mediump sampler2DArray sampler0_ssao;
uniform highp sampler2D sampler0_structure;
uniform mediump samplerCube sampler0_fog;

float filament_lodBias;
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                   
#endif
//------------------------------------------------------------------------------
// Common math
//------------------------------------------------------------------------------

/** @public-api */
#define PI                 3.14159265359
/** @public-api */
#define HALF_PI            1.570796327

#define MEDIUMP_FLT_MAX    65504.0
#define MEDIUMP_FLT_MIN    0.00006103515625

#ifdef TARGET_MOBILE
#define FLT_EPS            MEDIUMP_FLT_MIN
#define saturateMediump(x) min(x, MEDIUMP_FLT_MAX)
#else
#define FLT_EPS            1e-5
#define saturateMediump(x) x
#endif

#define saturate(x)        clamp(x, 0.0, 1.0)

//------------------------------------------------------------------------------
// Scalar operations
//------------------------------------------------------------------------------

/**
 * Computes x^5 using only multiply operations.
 *
 * @public-api
 */
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

/**
 * Computes x^2 as a single multiplication.
 *
 * @public-api
 */
float sq(float x) {
    return x * x;
}

//------------------------------------------------------------------------------
// Vector operations
//------------------------------------------------------------------------------

/**
 * Returns the maximum component of the specified vector.
 *
 * @public-api
 */
float max3(const vec3 v) {
    return max(v.x, max(v.y, v.z));
}

float vmax(const vec2 v) {
    return max(v.x, v.y);
}

float vmax(const vec3 v) {
    return max(v.x, max(v.y, v.z));
}

float vmax(const vec4 v) {
    return max(max(v.x, v.y), max(v.y, v.z));
}

/**
 * Returns the minimum component of the specified vector.
 *
 * @public-api
 */
float min3(const vec3 v) {
    return min(v.x, min(v.y, v.z));
}

float vmin(const vec2 v) {
    return min(v.x, v.y);
}

float vmin(const vec3 v) {
    return min(v.x, min(v.y, v.z));
}

float vmin(const vec4 v) {
    return min(min(v.x, v.y), min(v.y, v.z));
}

//------------------------------------------------------------------------------
// Trigonometry
//------------------------------------------------------------------------------

/**
 * Approximates acos(x) with a max absolute error of 9.0x10^-3.
 * Valid in the range -1..1.
 */
float acosFast(float x) {
    // Lagarde 2014, "Inverse trigonometric functions GPU optimization for AMD GCN architecture"
    // This is the approximation of degree 1, with a max absolute error of 9.0x10^-3
    float y = abs(x);
    float p = -0.1565827 * y + 1.570796;
    p *= sqrt(1.0 - y);
    return x >= 0.0 ? p : PI - p;
}

/**
 * Approximates acos(x) with a max absolute error of 9.0x10^-3.
 * Valid only in the range 0..1.
 */
float acosFastPositive(float x) {
    float p = -0.1565827 * x + 1.570796;
    return p * sqrt(1.0 - x);
}

//------------------------------------------------------------------------------
// Matrix and quaternion operations
//------------------------------------------------------------------------------

/**
 * Multiplies the specified 3-component vector by the 4x4 matrix (m * v) in
 * high precision.
 *
 * @public-api
 */
highp vec4 mulMat4x4Float3(const highp mat4 m, const highp vec3 v) {
    return v.x * m[0] + (v.y * m[1] + (v.z * m[2] + m[3]));
}

/**
 * Multiplies the specified 3-component vector by the 3x3 matrix (m * v) in
 * high precision.
 *
 * @public-api
 */
highp vec3 mulMat3x3Float3(const highp mat4 m, const highp vec3 v) {
    return v.x * m[0].xyz + (v.y * m[1].xyz + (v.z * m[2].xyz));
}

/**
 * Extracts the normal vector of the tangent frame encoded in the specified quaternion.
 */
void toTangentFrame(const highp vec4 q, out highp vec3 n) {
    n = vec3( 0.0,  0.0,  1.0) +
        vec3( 2.0, -2.0, -2.0) * q.x * q.zwx +
        vec3( 2.0,  2.0, -2.0) * q.y * q.wzy;
}

/**
 * Extracts the normal and tangent vectors of the tangent frame encoded in the
 * specified quaternion.
 */
void toTangentFrame(const highp vec4 q, out highp vec3 n, out highp vec3 t) {
    toTangentFrame(q, n);
    t = vec3( 1.0,  0.0,  0.0) +
        vec3(-2.0,  2.0, -2.0) * q.y * q.yxw +
        vec3(-2.0,  2.0,  2.0) * q.z * q.zwx;
}

highp mat3 cofactor(const highp mat3 m) {
    highp float a = m[0][0];
    highp float b = m[1][0];
    highp float c = m[2][0];
    highp float d = m[0][1];
    highp float e = m[1][1];
    highp float f = m[2][1];
    highp float g = m[0][2];
    highp float h = m[1][2];
    highp float i = m[2][2];

    highp mat3 cof;
    cof[0][0] = e * i - f * h;
    cof[0][1] = c * h - b * i;
    cof[0][2] = b * f - c * e;
    cof[1][0] = f * g - d * i;
    cof[1][1] = a * i - c * g;
    cof[1][2] = c * d - a * f;
    cof[2][0] = d * h - e * g;
    cof[2][1] = b * g - a * h;
    cof[2][2] = a * e - b * d;
    return cof;
}

//------------------------------------------------------------------------------
// Random
//------------------------------------------------------------------------------

/*
 * Random number between 0 and 1, using interleaved gradient noise.
 * w must not be normalized (e.g. window coordinates)
 */
float interleavedGradientNoise(highp vec2 w) {
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(m.z * fract(dot(w, m.xy)));
}
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                         
#endif
//------------------------------------------------------------------------------
// Instancing
// ------------------------------------------------------------------------------------------------

highp mat4 object_uniforms_worldFromModelMatrix;
highp mat3 object_uniforms_worldFromModelNormalMatrix;
highp int object_uniforms_morphTargetCount;
highp int object_uniforms_flagsChannels;                   // see packFlags() below (0x00000fll)
highp int object_uniforms_objectId;                        // used for picking
highp float object_uniforms_userData;   // TODO: We need a better solution, this currently holds the average local scale for the renderable

//------------------------------------------------------------------------------
// Instance access
//------------------------------------------------------------------------------

void initObjectUniforms() {
    // Adreno drivers workarounds:
    // - We need to copy each field separately because non-const array access in a UBO fails
    //    e.g.: this fails `p = objectUniforms.data[instance_index];`
    // - We can't use a struct to hold the result because Adreno driver ignore precision qualifiers
    //   on fields of structs, unless they're in a UBO (which we just copied out of).

#if defined(FILAMENT_HAS_FEATURE_INSTANCING)
    highp int i;
#   if defined(MATERIAL_HAS_INSTANCES)
    // instancing handled by the material
    if ((objectUniforms.data[0].flagsChannels & FILAMENT_OBJECT_INSTANCE_BUFFER_BIT) != 0) {
        // hybrid instancing, we have a instance buffer per object
        i = logical_instance_index;
    } else {
        // fully manual instancing
        i = 0;
    }
#   else
    // automatic instancing
    i = logical_instance_index;
#   endif
#else
    // we don't support instancing (e.g. ES2)
    const int i = 0;
#endif
    object_uniforms_worldFromModelMatrix        = objectUniforms.data[i].worldFromModelMatrix;
    object_uniforms_worldFromModelNormalMatrix  = objectUniforms.data[i].worldFromModelNormalMatrix;
    object_uniforms_morphTargetCount            = objectUniforms.data[i].morphTargetCount;
    object_uniforms_flagsChannels               = objectUniforms.data[i].flagsChannels;
    object_uniforms_objectId                    = objectUniforms.data[i].objectId;
    object_uniforms_userData                    = objectUniforms.data[i].userData;
}

#if defined(FILAMENT_HAS_FEATURE_INSTANCING) && defined(MATERIAL_HAS_INSTANCES)
/** @public-api */
highp int getInstanceIndex() {
    return logical_instance_index;
}
#endif
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                        
#endif
//------------------------------------------------------------------------------
// Shadowing
//------------------------------------------------------------------------------

#if defined(VARIANT_HAS_SHADOWING)
/**
 * Computes the light space position of the specified world space point.
 * The returned point may contain a bias to attempt to eliminate common
 * shadowing artifacts such as "acne". To achieve this, the world space
 * normal at the point must also be passed to this function.
 * Normal bias is not used for VSM.
 */

highp vec4 computeLightSpacePosition(highp vec3 p, const highp vec3 n,
        const highp vec3 dir, const float b, highp_mat4 lightFromWorldMatrix) {

#if !defined(VARIANT_HAS_VSM)
    highp float cosTheta = saturate(dot(n, dir));
    highp float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    p += n * (sinTheta * b);
#endif

    return mulMat4x4Float3(lightFromWorldMatrix, p);
}

#endif // VARIANT_HAS_SHADOWING
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                    
#endif
// These variables should be in a struct but some GPU drivers ignore the
// precision qualifier on individual struct members
highp mat3  shading_tangentToWorld;   // TBN matrix
highp vec3  shading_position;         // position of the fragment in world space
      vec3  shading_view;             // normalized vector from the fragment to the eye
#if defined(HAS_ATTRIBUTE_TANGENTS)
      vec3  shading_normal;           // normalized transformed normal, in world space
      vec3  shading_geometricNormal;  // normalized geometric normal, in world space
      vec3  shading_reflected;        // reflection of view about normal
      float shading_NoV;              // dot(normal, view), always strictly >= MIN_N_DOT_V

#if defined(MATERIAL_HAS_BENT_NORMAL)
      vec3  shading_bentNormal;       // normalized transformed normal, in world space
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT)
      vec3  shading_clearCoatNormal;  // normalized clear coat layer normal, in world space
#endif
#endif

highp vec2 shading_normalizedViewportCoord;
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                     
#endif
//------------------------------------------------------------------------------
// Common color operations
//------------------------------------------------------------------------------

/**
 * Computes the luminance of the specified linear RGB color using the
 * luminance coefficients from Rec. 709.
 *
 * @public-api
 */
float luminance(const vec3 linear) {
    return dot(linear, vec3(0.2126, 0.7152, 0.0722));
}

/**
 * Computes the pre-exposed intensity using the specified intensity and exposure.
 * This function exists to force highp precision on the two parameters
 */
float computePreExposedIntensity(const highp float intensity, const highp float exposure) {
    return intensity * exposure;
}

void unpremultiply(inout vec4 color) {
    color.rgb /= max(color.a, FLT_EPS);
}

/**
 * Applies a full range YCbCr to sRGB conversion and returns an RGB color.
 *
 * @public-api
 */
vec3 ycbcrToRgb(float luminance, vec2 cbcr) {
    // Taken from https://developer.apple.com/documentation/arkit/arframe/2867984-capturedimage
    const mat4 ycbcrToRgbTransform = mat4(
         1.0000,  1.0000,  1.0000,  0.0000,
         0.0000, -0.3441,  1.7720,  0.0000,
         1.4020, -0.7141,  0.0000,  0.0000,
        -0.7010,  0.5291, -0.8860,  1.0000
    );
    return (ycbcrToRgbTransform * vec4(luminance, cbcr, 1.0)).rgb;
}

//------------------------------------------------------------------------------
// Tone mapping operations
//------------------------------------------------------------------------------

/*
 * The input must be in the [0, 1] range.
 */
vec3 Inverse_Tonemap_Filmic(const vec3 x) {
    return (0.03 - 0.59 * x - sqrt(0.0009 + 1.3702 * x - 1.0127 * x * x)) / (-5.02 + 4.86 * x);
}

/**
 * Applies the inverse of the tone mapping operator to the specified HDR or LDR
 * sRGB (non-linear) color and returns a linear sRGB color. The inverse tone mapping
 * operator may be an approximation of the real inverse operation.
 *
 * @public-api
 */
vec3 inverseTonemapSRGB(vec3 color) {
    // sRGB input
    color = clamp(color, 0.0, 1.0);
    return Inverse_Tonemap_Filmic(pow(color, vec3(2.2)));
}

/**
 * Applies the inverse of the tone mapping operator to the specified HDR or LDR
 * linear RGB color and returns a linear RGB color. The inverse tone mapping operator
 * may be an approximation of the real inverse operation.
 *
 * @public-api
 */
vec3 inverseTonemap(vec3 linear) {
    // Linear input
    return Inverse_Tonemap_Filmic(clamp(linear, 0.0, 1.0));
}

//------------------------------------------------------------------------------
// Common texture operations
//------------------------------------------------------------------------------

/**
 * Decodes the specified RGBM value to linear HDR RGB.
 */
vec3 decodeRGBM(vec4 c) {
    c.rgb *= (c.a * 16.0);
    return c.rgb * c.rgb;
}

//------------------------------------------------------------------------------
// Common screen-space operations
//------------------------------------------------------------------------------

// returns the frag coord in the GL convention with (0, 0) at the bottom-left
// resolution : width, height
highp vec2 getFragCoord(const highp vec2 resolution) {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    return vec2(gl_FragCoord.x, resolution.y - gl_FragCoord.y);
#else
    return gl_FragCoord.xy;
#endif
}

//------------------------------------------------------------------------------
// Common debug
//------------------------------------------------------------------------------

vec3 heatmap(float v) {
    vec3 r = v * 2.1 - vec3(1.8, 1.14, 0.3);
    return 1.0 - r * r;
}
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                     
#endif
#if defined(TARGET_MOBILE)
    // min roughness such that (MIN_PERCEPTUAL_ROUGHNESS^4) > 0 in fp16 (i.e. 2^(-14/4), rounded up)
    #define MIN_PERCEPTUAL_ROUGHNESS 0.089
    #define MIN_ROUGHNESS            0.007921
#else
    #define MIN_PERCEPTUAL_ROUGHNESS 0.045
    #define MIN_ROUGHNESS            0.002025
#endif

#define MIN_N_DOT_V 1e-4

float clampNoV(float NoV) {
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
    return max(NoV, MIN_N_DOT_V);
}

vec3 computeDiffuseColor(const vec4 baseColor, float metallic) {
    return baseColor.rgb * (1.0 - metallic);
}

vec3 computeF0(const vec4 baseColor, float metallic, float reflectance) {
    return baseColor.rgb * metallic + (reflectance * (1.0 - metallic));
}

float computeDielectricF0(float reflectance) {
    return 0.16 * reflectance * reflectance;
}

float computeMetallicFromSpecularColor(const vec3 specularColor) {
    return max3(specularColor);
}

float computeRoughnessFromGlossiness(float glossiness) {
    return 1.0 - glossiness;
}

float perceptualRoughnessToRoughness(float perceptualRoughness) {
    return perceptualRoughness * perceptualRoughness;
}

float roughnessToPerceptualRoughness(float roughness) {
    return sqrt(roughness);
}

float iorToF0(float transmittedIor, float incidentIor) {
    return sq((transmittedIor - incidentIor) / (transmittedIor + incidentIor));
}

float f0ToIor(float f0) {
    float r = sqrt(f0);
    return (1.0 + r) / (1.0 - r);
}

vec3 f0ClearCoatToSurface(const vec3 f0) {
    // Approximation of iorTof0(f0ToIor(f0), 1.5)
    // This assumes that the clear coat layer has an IOR of 1.5
#if FILAMENT_QUALITY == FILAMENT_QUALITY_LOW
    return saturate(f0 * (f0 * 0.526868 + 0.529324) - 0.0482256);
#else
    return saturate(f0 * (f0 * (0.941892 - 0.263008 * f0) + 0.346479) - 0.0285998);
#endif
}
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                      
#endif
//------------------------------------------------------------------------------
// Uniforms access
//------------------------------------------------------------------------------

/** @public-api */
highp mat4 getViewFromWorldMatrix() {
    return frameUniforms.viewFromWorldMatrix;
}

/** @public-api */
highp mat4 getWorldFromViewMatrix() {
    return frameUniforms.worldFromViewMatrix;
}

/** @public-api */
highp mat4 getClipFromViewMatrix() {
    return frameUniforms.clipFromViewMatrix;
}

/** @public-api */
highp mat4 getViewFromClipMatrix() {
    return frameUniforms.viewFromClipMatrix;
}

/** @public-api */
highp mat4 getClipFromWorldMatrix() {
#if defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_INSTANCED)
    int eye = instance_index % CONFIG_STEREO_EYE_COUNT;
    return frameUniforms.clipFromWorldMatrix[eye];
#elif defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_MULTIVIEW)
    return frameUniforms.clipFromWorldMatrix[gl_ViewID_OVR];
#else
    return frameUniforms.clipFromWorldMatrix[0];
#endif
}

/** @public-api */
highp mat4 getWorldFromClipMatrix() {
    return frameUniforms.worldFromClipMatrix;
}

/** @public-api */
highp mat4 getUserWorldFromWorldMatrix() {
    return frameUniforms.userWorldFromWorldMatrix;
}

/** @public-api */
float getTime() {
    return frameUniforms.time;
}

/** @public-api */
highp vec4 getUserTime() {
    return frameUniforms.userTime;
}

/** @public-api **/
highp float getUserTimeMod(float m) {
    return mod(mod(frameUniforms.userTime.x, m) + mod(frameUniforms.userTime.y, m), m);
}

/**
 * Transforms a texture UV to make it suitable for a render target attachment.
 *
 * In Vulkan and Metal, texture coords are Y-down but in OpenGL they are Y-up. This wrapper function
 * accounts for these differences. When sampling from non-render targets (i.e. uploaded textures)
 * these differences do not matter because OpenGL has a second piece of backwardness, which is that
 * the first row of texels in glTexImage2D is interpreted as the bottom row.
 *
 * To protect users from these differences, we recommend that materials in the SURFACE domain
 * leverage this wrapper function when sampling from offscreen render targets.
 *
 * @public-api
 */
highp vec2 uvToRenderTargetUV(const highp vec2 uv) {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    return vec2(uv.x, 1.0 - uv.y);
#else
    return uv;
#endif
}

// TODO: below shouldn't be accessible from post-process materials

/** @public-api */
highp vec4 getResolution() {
    return frameUniforms.resolution;
}

/** @public-api */
highp vec3 getWorldCameraPosition() {
    return frameUniforms.worldFromViewMatrix[3].xyz;
}

/** @public-api, @deprecated use getUserWorldPosition() or getUserWorldFromWorldMatrix() instead  */
highp vec3 getWorldOffset() {
    return getUserWorldFromWorldMatrix()[3].xyz;
}

/** @public-api */
float getExposure() {
    // NOTE: this is a highp uniform only to work around #3602 (qualcomm)
    // We are intentionally casting it to mediump here, as per the Materials doc.
    return frameUniforms.exposure;
}

/** @public-api */
float getEV100() {
    return frameUniforms.ev100;
}

//------------------------------------------------------------------------------
// user defined globals
//------------------------------------------------------------------------------

highp vec4 getMaterialGlobal0() {
    return frameUniforms.custom[0];
}

highp vec4 getMaterialGlobal1() {
    return frameUniforms.custom[1];
}

highp vec4 getMaterialGlobal2() {
    return frameUniforms.custom[2];
}

highp vec4 getMaterialGlobal3() {
    return frameUniforms.custom[3];
}
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0             
#endif
//------------------------------------------------------------------------------
// Uniforms access
//------------------------------------------------------------------------------

/** sort-of public */
float getObjectUserData() {
    return object_uniforms_userData;
}

//------------------------------------------------------------------------------
// Attributes access
//------------------------------------------------------------------------------

#if defined(HAS_ATTRIBUTE_COLOR)
/** @public-api */
vec4 getColor() {
    return vertex_color;
}
#endif

#if defined(HAS_ATTRIBUTE_UV0)
/** @public-api */
highp vec2 getUV0() {
    return vertex_uv01.xy;
}
#endif

#if defined(HAS_ATTRIBUTE_UV1)
/** @public-api */
highp vec2 getUV1() {
    return vertex_uv01.zw;
}
#endif

#if defined(BLEND_MODE_MASKED)
/** @public-api */
float getMaskThreshold() {
    return materialParams._maskThreshold;
}
#endif

/** @public-api */
highp mat3 getWorldTangentFrame() {
    return shading_tangentToWorld;
}

/** @public-api */
highp vec3 getWorldPosition() {
    return shading_position;
}

/** @public-api */
highp vec3 getUserWorldPosition() {
    return mulMat4x4Float3(getUserWorldFromWorldMatrix(), getWorldPosition()).xyz;
}

/** @public-api */
vec3 getWorldViewVector() {
    return shading_view;
}

bool isPerspectiveProjection() {
    return frameUniforms.clipFromViewMatrix[2].w != 0.0;
}

#if defined(HAS_ATTRIBUTE_TANGENTS)

/** @public-api */
vec3 getWorldNormalVector() {
    return shading_normal;
}

/** @public-api */
vec3 getWorldGeometricNormalVector() {
    return shading_geometricNormal;
}

/** @public-api */
vec3 getWorldReflectedVector() {
    return shading_reflected;
}

/** @public-api */
float getNdotV() {
    return shading_NoV;
}

#endif

highp vec3 getNormalizedPhysicalViewportCoord() {
    // make sure to handle our reversed-z
    return vec3(shading_normalizedViewportCoord, gl_FragCoord.z);
}

/**
 * Returns the normalized [0, 1] logical viewport coordinates with the origin at the viewport's
 * bottom-left. Z coordinate is in the [1, 0] range (reversed).
 *
 * @public-api
 */
highp vec3 getNormalizedViewportCoord() {
    // make sure to handle our reversed-z
    highp vec2 scale = frameUniforms.logicalViewportScale;
    highp vec2 offset = frameUniforms.logicalViewportOffset;
    highp vec2 logicalUv = shading_normalizedViewportCoord * scale + offset;
    return vec3(logicalUv, gl_FragCoord.z);
}

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DYNAMIC_LIGHTING)
highp vec4 getSpotLightSpacePosition(int index, highp vec3 dir, highp float zLight) {
    highp mat4 lightFromWorldMatrix = shadowUniforms.shadows[index].lightFromWorldMatrix;

    // for spotlights, the bias depends on z
    float bias = shadowUniforms.shadows[index].normalBias * zLight;

    return computeLightSpacePosition(getWorldPosition(), getWorldGeometricNormalVector(),
            dir, bias, lightFromWorldMatrix);
}
#endif

#if defined(MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY)
bool isDoubleSided() {
    return materialParams._doubleSided;
}
#endif

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)

/**
 * Returns the cascade index for this fragment (between 0 and CONFIG_MAX_SHADOW_CASCADES - 1).
 */
int getShadowCascade() {
    highp float z = mulMat4x4Float3(getViewFromWorldMatrix(), getWorldPosition()).z;
    ivec4 greaterZ = ivec4(greaterThan(frameUniforms.cascadeSplits, vec4(z)));
    int cascadeCount = frameUniforms.cascades & 0xF;
    return clamp(greaterZ.x + greaterZ.y + greaterZ.z + greaterZ.w, 0, cascadeCount - 1);
}

highp vec4 getCascadeLightSpacePosition(int cascade) {
    // For the first cascade, return the interpolated light space position.
    // This branch will be coherent (mostly) for neighboring fragments, and it's worth avoiding
    // the matrix multiply inside computeLightSpacePosition.
    if (cascade == 0) {
        // Note: this branch may cause issues with derivatives
        return vertex_lightSpacePosition;
    }

    return computeLightSpacePosition(getWorldPosition(), getWorldGeometricNormalVector(),
        frameUniforms.lightDirection,
        shadowUniforms.shadows[cascade].normalBias,
        shadowUniforms.shadows[cascade].lightFromWorldMatrix);
}

#endif

#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                     
#endif
// Decide if we can skip lighting when dot(n, l) <= 0.0
#if defined(SHADING_MODEL_CLOTH)
#if !defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    #define MATERIAL_CAN_SKIP_LIGHTING
#endif
#elif defined(SHADING_MODEL_SUBSURFACE) || defined(MATERIAL_HAS_CUSTOM_SURFACE_SHADING)
    // Cannot skip lighting
#else
    #define MATERIAL_CAN_SKIP_LIGHTING
#endif

struct MaterialInputs {
    vec4  baseColor;
#if !defined(SHADING_MODEL_UNLIT)
#if !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    float roughness;
#endif
#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    float metallic;
    float reflectance;
#endif
    float ambientOcclusion;
#endif
    vec4  emissive;

#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SUBSURFACE) && !defined(SHADING_MODEL_UNLIT)
    vec3 sheenColor;
    float sheenRoughness;
#endif

    float clearCoat;
    float clearCoatRoughness;

    float anisotropy;
    vec3  anisotropyDirection;

#if defined(SHADING_MODEL_SUBSURFACE) || defined(MATERIAL_HAS_REFRACTION)
    float thickness;
#endif
#if defined(SHADING_MODEL_SUBSURFACE)
    float subsurfacePower;
    vec3  subsurfaceColor;
#endif

#if defined(SHADING_MODEL_CLOTH)
    vec3  sheenColor;
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    vec3  subsurfaceColor;
#endif
#endif

#if defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    vec3  specularColor;
    float glossiness;
#endif

#if defined(MATERIAL_HAS_NORMAL)
    vec3  normal;
#endif
#if defined(MATERIAL_HAS_BENT_NORMAL)
    vec3  bentNormal;
#endif
#if defined(MATERIAL_HAS_CLEAR_COAT) && defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    vec3  clearCoatNormal;
#endif

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
    vec4  postLightingColor;
    float postLightingMixFactor;
#endif

#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SUBSURFACE) && !defined(SHADING_MODEL_UNLIT)
#if defined(MATERIAL_HAS_REFRACTION)
#if defined(MATERIAL_HAS_ABSORPTION)
    vec3 absorption;
#endif
#if defined(MATERIAL_HAS_TRANSMISSION)
    float transmission;
#endif
#if defined(MATERIAL_HAS_IOR)
    float ior;
#endif
#if defined(MATERIAL_HAS_MICRO_THICKNESS) && (REFRACTION_TYPE == REFRACTION_TYPE_THIN)
    float microThickness;
#endif
#elif !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
#if defined(MATERIAL_HAS_IOR)
    float ior;
#endif
#endif
#endif

#if defined(MATERIAL_HAS_SPECULAR_FACTOR)
    float specularFactor;
#endif

#if defined(MATERIAL_HAS_SPECULAR_COLOR_FACTOR)
    vec3 specularColorFactor;
#endif

};

void initMaterial(out MaterialInputs material) {
    material.baseColor = vec4(1.0);
#if !defined(SHADING_MODEL_UNLIT)
#if !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    material.roughness = 1.0;
#endif
#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    material.metallic = 0.0;
    material.reflectance = 0.5;
#endif
    material.ambientOcclusion = 1.0;
#endif
    material.emissive = vec4(vec3(0.0), 1.0);

#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SUBSURFACE) && !defined(SHADING_MODEL_UNLIT)
#if defined(MATERIAL_HAS_SHEEN_COLOR)
    material.sheenColor = vec3(0.0);
    material.sheenRoughness = 0.0;
#endif
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT)
    material.clearCoat = 1.0;
    material.clearCoatRoughness = 0.0;
#endif

#if defined(MATERIAL_HAS_ANISOTROPY)
    material.anisotropy = 0.0;
    material.anisotropyDirection = vec3(1.0, 0.0, 0.0);
#endif

#if defined(SHADING_MODEL_SUBSURFACE) || defined(MATERIAL_HAS_REFRACTION)
    material.thickness = 0.5;
#endif
#if defined(SHADING_MODEL_SUBSURFACE)
    material.subsurfacePower = 12.234;
    material.subsurfaceColor = vec3(1.0);
#endif

#if defined(SHADING_MODEL_CLOTH)
    material.sheenColor = sqrt(material.baseColor.rgb);
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    material.subsurfaceColor = vec3(0.0);
#endif
#endif

#if defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    material.glossiness = 0.0;
    material.specularColor = vec3(0.0);
#endif

#if defined(MATERIAL_HAS_NORMAL)
    material.normal = vec3(0.0, 0.0, 1.0);
#endif
#if defined(MATERIAL_HAS_BENT_NORMAL)
    material.bentNormal = vec3(0.0, 0.0, 1.0);
#endif
#if defined(MATERIAL_HAS_CLEAR_COAT) && defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    material.clearCoatNormal = vec3(0.0, 0.0, 1.0);
#endif

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
    material.postLightingColor = vec4(0.0);
    material.postLightingMixFactor = 1.0;
#endif

#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SUBSURFACE) && !defined(SHADING_MODEL_UNLIT)
#if defined(MATERIAL_HAS_REFRACTION)
#if defined(MATERIAL_HAS_ABSORPTION)
    material.absorption = vec3(0.0);
#endif
#if defined(MATERIAL_HAS_TRANSMISSION)
    material.transmission = 1.0;
#endif
#if defined(MATERIAL_HAS_IOR)
    material.ior = 1.5;
#endif
#if defined(MATERIAL_HAS_MICRO_THICKNESS) && (REFRACTION_TYPE == REFRACTION_TYPE_THIN)
    material.microThickness = 0.0;
#endif
#elif !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
#if defined(MATERIAL_HAS_IOR)
    material.ior = 1.5;
#endif
#endif
#endif

#if defined(MATERIAL_HAS_SPECULAR_FACTOR)
    material.specularFactor = 1.0;
#endif

#if defined(MATERIAL_HAS_SPECULAR_COLOR_FACTOR)
    material.specularColorFactor = vec3(1.0);
#endif

}

#if defined(MATERIAL_HAS_CUSTOM_SURFACE_SHADING)
/** @public-api */
struct LightData {
    vec4  colorIntensity;
    vec3  l;
    float NdotL;
    vec3  worldPosition;
    float attenuation;
    float visibility;
};

/** @public-api */
struct ShadingData {
    vec3  diffuseColor;
    float perceptualRoughness;
    vec3  f0;
    float roughness;
};
#endif
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                        
#endif
//------------------------------------------------------------------------------
// Material evaluation
//------------------------------------------------------------------------------

/**
 * Computes global shading parameters used to apply lighting, such as the view
 * vector in world space, the tangent frame at the shading point, etc.
 */
void computeShadingParams() {
#if defined(HAS_ATTRIBUTE_TANGENTS)
    highp vec3 n = vertex_worldNormal;
#if defined(MATERIAL_NEEDS_TBN)
    highp vec3 t = vertex_worldTangent.xyz;
    highp vec3 b = cross(n, t) * sign(vertex_worldTangent.w);
#endif

#if defined(MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY)
    if (isDoubleSided()) {
        n = gl_FrontFacing ? n : -n;
#if defined(MATERIAL_NEEDS_TBN)
        t = gl_FrontFacing ? t : -t;
        b = gl_FrontFacing ? b : -b;
#endif
    }
#endif

    shading_geometricNormal = normalize(n);

#if defined(MATERIAL_NEEDS_TBN)
    // We use unnormalized post-interpolation values, assuming mikktspace tangents
    shading_tangentToWorld = mat3(t, b, n);
#endif
#endif

    shading_position = vertex_worldPosition.xyz;

    // With perspective camera, the view vector is cast from the fragment pos to the eye position,
    // With ortho camera, however, the view vector is the same for all fragments:
    highp vec3 sv = isPerspectiveProjection() ?
        (frameUniforms.worldFromViewMatrix[3].xyz - shading_position) :
         frameUniforms.worldFromViewMatrix[2].xyz; // ortho camera backward dir
    shading_view = normalize(sv);

    // we do this so we avoid doing (matrix multiply), but we burn 4 varyings:
    //    p = clipFromWorldMatrix * shading_position;
    //    shading_normalizedViewportCoord = p.xy * 0.5 / p.w + 0.5
    shading_normalizedViewportCoord = vertex_position.xy * (0.5 / vertex_position.w) + 0.5;
}

/**
 * Computes global shading parameters that the material might need to access
 * before lighting: N dot V, the reflected vector and the shading normal (before
 * applying the normal map). These parameters can be useful to material authors
 * to compute other material properties.
 *
 * This function must be invoked by the user's material code (guaranteed by
 * the material compiler) after setting a value for MaterialInputs.normal.
 */
void prepareMaterial(const MaterialInputs material) {
#if defined(HAS_ATTRIBUTE_TANGENTS)
#if defined(MATERIAL_HAS_NORMAL)
    shading_normal = normalize(shading_tangentToWorld * material.normal);
#else
    shading_normal = getWorldGeometricNormalVector();
#endif
    shading_NoV = clampNoV(dot(shading_normal, shading_view));
    shading_reflected = reflect(-shading_view, shading_normal);

#if defined(MATERIAL_HAS_BENT_NORMAL)
    shading_bentNormal = normalize(shading_tangentToWorld * material.bentNormal);
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT)
#if defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    shading_clearCoatNormal = normalize(shading_tangentToWorld * material.clearCoatNormal);
#else
    shading_clearCoatNormal = getWorldGeometricNormalVector();
#endif
#endif
#endif
}
#line 20
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 19                  
#endif

    void material(inout MaterialInputs material) {
        prepareMaterial(material);
        vec3 c=vec3(variable_customnormal.xyz+1.0)/2.0;
        material.baseColor = vec4(c,1.0);
    }
#line 1419
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                   
#endif
void addEmissive(const MaterialInputs material, inout vec4 color) {
#if defined(MATERIAL_HAS_EMISSIVE)
    highp vec4 emissive = material.emissive;
    highp float attenuation = mix(1.0, getExposure(), emissive.w);
#if defined(BLEND_MODE_TRANSPARENT) || defined(BLEND_MODE_FADE)
    attenuation *= color.a;
#endif
    color.rgb += emissive.rgb * attenuation;
#endif
}

vec4 fixupAlpha(vec4 color) {
#if defined(BLEND_MODE_MASKED)
    // If we reach this point in the code, we already know that the fragment is not discarded due
    // to the threshold factor. Therefore we can just output 1.0, which prevents a "punch through"
    // effect from occuring. We do this only for TRANSLUCENT views in order to prevent breakage
    // of ALPHA_TO_COVERAGE.
    return vec4(color.rgb, (frameUniforms.needsAlphaChannel == 1.0) ? 1.0 : color.a);
#else
    return color;
#endif
}

/**
 * Evaluates unlit materials. In this lighting model, only the base color and
 * emissive properties are taken into account:
 *
 * finalColor = baseColor + emissive
 *
 * The emissive factor is only applied if the fragment passes the optional
 * alpha test.
 *
 * When the shadowMultiplier property is enabled on the material, the final
 * color is multiplied by the inverse light visibility to apply a shadow.
 * This is mostly useful in AR to cast shadows on unlit transparent shadow
 * receiving planes.
 */
vec4 evaluateMaterial(const MaterialInputs material) {
    vec4 color = material.baseColor;

#if defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
#if defined(VARIANT_HAS_SHADOWING)
    float visibility = 1.0;
    int cascade = getShadowCascade();
    bool cascadeHasVisibleShadows = bool(frameUniforms.cascades & ((1 << cascade) << 8));
    bool hasDirectionalShadows = bool(frameUniforms.directionalShadows & 1);
    if (hasDirectionalShadows && cascadeHasVisibleShadows) {
        highp vec4 shadowPosition = getShadowPosition(cascade);
        visibility = shadow(true, sampler0_shadowMap, cascade, shadowPosition, 0.0);
        // shadow far attenuation
        highp vec3 v = getWorldPosition() - getWorldCameraPosition();
        // (viewFromWorld * v).z == dot(transpose(viewFromWorld), v)
        highp float z = dot(transpose(getViewFromWorldMatrix())[2].xyz, v);
        highp vec2 p = frameUniforms.shadowFarAttenuationParams;
        visibility = 1.0 - ((1.0 - visibility) * saturate(p.x - z * z * p.y));
    }
    if ((frameUniforms.directionalShadows & 0x2) != 0 && visibility > 0.0) {
        if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_CONTACT_SHADOWS_BIT) != 0) {
            visibility *= (1.0 - screenSpaceContactShadow(frameUniforms.lightDirection));
        }
    }
    color *= 1.0 - visibility;
#else
    color = vec4(0.0);
#endif
#elif defined(MATERIAL_HAS_SHADOW_MULTIPLIER)
    color = vec4(0.0);
#endif

    addEmissive(material, color);

    return fixupAlpha(color);
}
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0          
#endif
#if __VERSION__ == 100
vec4 fragColor;
#else
layout(location = 0) out vec4 fragColor;
#endif

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
void blendPostLightingColor(const MaterialInputs material, inout vec4 color) {
    vec4 blend = color;
#if defined(POST_LIGHTING_BLEND_MODE_OPAQUE)
    blend = material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_TRANSPARENT)
    blend = material.postLightingColor + color * (1.0 - material.postLightingColor.a);
#elif defined(POST_LIGHTING_BLEND_MODE_ADD)
    blend += material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_MULTIPLY)
    blend *= material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_SCREEN)
    blend += material.postLightingColor * (1.0 - color);
#endif
    color = mix(color, blend, material.postLightingMixFactor);
}
#endif

#if defined(BLEND_MODE_MASKED)
void applyAlphaMask(inout vec4 baseColor) {
    // Use derivatives to sharpen alpha tested edges, combined with alpha to
    // coverage to smooth the result
    baseColor.a = (baseColor.a - getMaskThreshold()) / max(fwidth(baseColor.a), 1e-3) + 0.5;
    if (baseColor.a <= getMaskThreshold()) {
        discard;
    }
}
#else
void applyAlphaMask(inout vec4 baseColor) {}
#endif

void main() {
    filament_lodBias = frameUniforms.lodBias;
#if defined(FILAMENT_HAS_FEATURE_INSTANCING)
    logical_instance_index = instance_index;
#endif

    initObjectUniforms();

    // See shading_parameters.fs
    // Computes global variables we need to evaluate material and lighting
    computeShadingParams();

    // Initialize the inputs to sensible default values, see material_inputs.fs
    MaterialInputs inputs;
    initMaterial(inputs);

    // Invoke user code
    material(inputs);

    applyAlphaMask(inputs.baseColor);

    fragColor = evaluateMaterial(inputs);

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR) && !defined(MATERIAL_HAS_REFLECTIONS)
    blendPostLightingColor(inputs, fragColor);
#endif

#if defined(VARIANT_HAS_FOG)
    highp vec3 view = getWorldPosition() - getWorldCameraPosition();
    view = frameUniforms.fogFromWorldMatrix * view;
    fragColor = fog(fragColor, view);
#endif

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
    if (CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP) {
        float a = fragColor.a;
        highp vec4 p = getShadowPosition(getShadowCascade());
        p.xy = p.xy * (1.0 / p.w);
        if (p.xy != saturate(p.xy)) {
            vec4 c = vec4(1.0, 0, 1.0, 1.0) * a;
            fragColor = mix(fragColor, c, 0.2);
        } else {
            highp vec2 size = vec2(textureSize(sampler0_shadowMap, 0));
            highp int ix = int(floor(p.x * size.x));
            highp int iy = int(floor(p.y * size.y));
            float t = float((ix ^ iy) & 1) * 0.2;
            vec4 c = vec4(vec3(t * a), a);
            fragColor = mix(fragColor, c, 0.5);
        }
    }
#endif

#if MATERIAL_FEATURE_LEVEL == 0
    if (CONFIG_SRGB_SWAPCHAIN_EMULATION) {
        if (frameUniforms.rec709 != 0) {
            fragColor.r = pow(fragColor.r, 0.45454);
            fragColor.g = pow(fragColor.g, 0.45454);
            fragColor.b = pow(fragColor.b, 0.45454);
        }
    }
#endif

#if __VERSION__ == 100
    gl_FragData[0] = fragColor;
#endif
}

