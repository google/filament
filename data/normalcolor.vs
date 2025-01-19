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
#define FLIP_UV_ATTRIBUTE
#define VARYING out
#define ATTRIBUTE in
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
#define texture2DLod textureLod
#define texture2DProjLod textureProjLod
#define texture3DLod textureLod
#define texture3DProjLod textureProjLod
#define textureCubeLod textureLod


#define MATERIAL_FEATURE_LEVEL 1
#define MATERIAL_HAS_BASE_COLOR
#define SHADING_INTERPOLATION 

#define HAS_ATTRIBUTE_POSITION
#define HAS_ATTRIBUTE_COLOR
#define HAS_ATTRIBUTE_CUSTOM0

layout(location = 0) in vec4 mesh_position;
layout(location = 2) in vec4 mesh_color;
layout(location = 8) in vec4 mesh_custom0;

struct Constants {
int morphingBufferOffset;
};
LAYOUT_LOCATION(32) uniform Constants pushConstants;

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

#define VARIABLE_CUSTOM0 customnormal

#define VARIABLE_CUSTOM_AT0 variable_customnormal
LAYOUT_LOCATION(0) VARYING  vec4 variable_customnormal;
#define VERTEX_DOMAIN_OBJECT

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

/** @public-api */
mat4 getWorldFromModelMatrix() {
    return object_uniforms_worldFromModelMatrix;
}

/** @public-api */
mat3 getWorldFromModelNormalMatrix() {
    return object_uniforms_worldFromModelNormalMatrix;
}

/** sort-of public */
float getObjectUserData() {
    return object_uniforms_userData;
}

//------------------------------------------------------------------------------
// Attributes access
//------------------------------------------------------------------------------

#if __VERSION__ >= 300
/** @public-api */
int getVertexIndex() {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    return gl_VertexIndex;
#else
    return gl_VertexID;
#endif
}
#endif

#if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
#define MAX_SKINNING_BUFFER_WIDTH 2048u
vec3 mulBoneNormal(vec3 n, uint j) {

    highp mat3 cof;

    // the last element must be computed by hand
    highp float a = bonesUniforms.bones[j].transform[0][0];
    highp float b = bonesUniforms.bones[j].transform[0][1];
    highp float c = bonesUniforms.bones[j].transform[0][2];
    highp float d = bonesUniforms.bones[j].transform[1][0];
    highp float e = bonesUniforms.bones[j].transform[1][1];
    highp float f = bonesUniforms.bones[j].transform[1][2];
    highp float g = bonesUniforms.bones[j].transform[2][0];
    highp float h = bonesUniforms.bones[j].transform[2][1];
    highp float i = bonesUniforms.bones[j].transform[2][2];

    cof[0] = bonesUniforms.bones[j].cof0;

    cof[1].xyz = vec3(
            bonesUniforms.bones[j].cof1x,
            a * i - c * g,
            c * d - a * f);

    cof[2].xyz = vec3(
            d * h - e * g,
            b * g - a * h,
            a * e - b * d);

    return normalize(cof * n);
}

vec3 mulBoneVertex(vec3 v, uint i) {
    // last row of bonesUniforms.transform[i] (row major) is assumed to be [0,0,0,1]
    highp mat4x3 m = transpose(bonesUniforms.bones[i].transform);
    return v.x * m[0].xyz + (v.y * m[1].xyz + (v.z * m[2].xyz + m[3].xyz));
}

void skinPosition(inout vec3 p, const uvec4 ids, const vec4 weights) {
    // standard skinning for 4 weights, some of them could be zero
    if (weights.w >= 0.0) {
        p = weights.x * mulBoneVertex(p, uint(ids.x))
            + weights.y * mulBoneVertex(p, uint(ids.y))
            + weights.z * mulBoneVertex(p, uint(ids.z))
            + weights.w * mulBoneVertex(p, uint(ids.w));
        return;
    }
    // skinning for >4 weights
    vec3 posSum = weights.x * mulBoneVertex(p, uint(ids.x));
    posSum += weights.y * mulBoneVertex(p, uint(ids.y));
    posSum += weights.z * mulBoneVertex(p, uint(ids.z));
    uint pairIndex = uint(-weights.w - 1.);
    uint pairStop = pairIndex + uint(ids.w - 3u);
    for (uint i = pairIndex; i < pairStop; ++i) {
        ivec2 texcoord = ivec2(i % MAX_SKINNING_BUFFER_WIDTH, i / MAX_SKINNING_BUFFER_WIDTH);
        vec2 indexWeight = texelFetch(sampler1_indicesAndWeights, texcoord, 0).rg;
        posSum += mulBoneVertex(p, uint(indexWeight.r)) * indexWeight.g;
    }
    p = posSum;
}

void skinNormal(inout vec3 n, const uvec4 ids, const vec4 weights) {
    // standard skinning for 4 weights, some of them could be zero
    if (weights.w >= 0.0) {
        n = weights.x * mulBoneNormal(n, uint(ids.x))
            + weights.y * mulBoneNormal(n, uint(ids.y))
            + weights.z * mulBoneNormal(n, uint(ids.z))
            + weights.w * mulBoneNormal(n, uint(ids.w));
        return;
    }
    // skinning for >4 weights
    vec3 normSum = weights.x * mulBoneNormal(n, uint(ids.x));
    normSum += weights.y * mulBoneNormal(n, uint(ids.y));
    normSum += weights.z * mulBoneNormal(n, uint(ids.z));
    uint pairIndex = uint(-weights.w - 1.);
    uint pairStop = pairIndex + uint(ids.w - 3u);
    for (uint i = pairIndex; i < pairStop; i = i + 1u) {
        ivec2 texcoord = ivec2(i % MAX_SKINNING_BUFFER_WIDTH, i / MAX_SKINNING_BUFFER_WIDTH);
        vec2 indexWeight = texelFetch(sampler1_indicesAndWeights, texcoord, 0).rg;

        normSum += mulBoneNormal(n, uint(indexWeight.r)) * indexWeight.g;
    }
    n = normSum;
}

void skinNormalTangent(inout vec3 n, inout vec3 t, const uvec4 ids, const vec4 weights) {
    // standard skinning for 4 weights, some of them could be zero
    if (weights.w >= 0.0) {
        n = weights.x * mulBoneNormal(n, uint(ids.x))
            + weights.y * mulBoneNormal(n, uint(ids.y))
            + weights.z * mulBoneNormal(n, uint(ids.z))
            + weights.w * mulBoneNormal(n, uint(ids.w));
        t = weights.x * mulBoneNormal(t, uint(ids.x))
            + weights.y * mulBoneNormal(t, uint(ids.y))
            + weights.z * mulBoneNormal(t, uint(ids.z))
            + weights.w * mulBoneNormal(t, uint(ids.w));
        return;
    }
    // skinning for >4 weights
    vec3 normSum = weights.x * mulBoneNormal(n, uint(ids.x));
    normSum += weights.y * mulBoneNormal(n, uint(ids.y)) ;
    normSum += weights.z * mulBoneNormal(n, uint(ids.z));
    vec3 tangSum = weights.x * mulBoneNormal(t, uint(ids.x));
    tangSum += weights.y * mulBoneNormal(t, uint(ids.y));
    tangSum += weights.z * mulBoneNormal(t, uint(ids.z));
    uint pairIndex = uint(-weights.w - 1.);
    uint pairStop = pairIndex + uint(ids.w - 3u);
    for (uint i = pairIndex; i < pairStop; i = i + 1u) {
        ivec2 texcoord = ivec2(i % MAX_SKINNING_BUFFER_WIDTH, i / MAX_SKINNING_BUFFER_WIDTH);
        vec2 indexWeight = texelFetch(sampler1_indicesAndWeights, texcoord, 0).rg;

        normSum += mulBoneNormal(n, uint(indexWeight.r)) * indexWeight.g;
        tangSum += mulBoneNormal(t, uint(indexWeight.r)) * indexWeight.g;
    }
    n = normSum;
    t = tangSum;
}

#define MAX_MORPH_TARGET_BUFFER_WIDTH 2048

void morphPosition(inout vec4 p) {
    int index = getVertexIndex() + pushConstants.morphingBufferOffset;
    ivec3 texcoord = ivec3(index % MAX_MORPH_TARGET_BUFFER_WIDTH, index / MAX_MORPH_TARGET_BUFFER_WIDTH, 0);
    int c = object_uniforms_morphTargetCount;
    for (int i = 0; i < c; ++i) {
        float w = morphingUniforms.weights[i][0];
        if (w != 0.0) {
            texcoord.z = i;
            p += w * texelFetch(sampler1_positions, texcoord, 0);
        }
    }
}

void morphNormal(inout vec3 n) {
    vec3 baseNormal = n;
    int index = getVertexIndex() + pushConstants.morphingBufferOffset;
    ivec3 texcoord = ivec3(index % MAX_MORPH_TARGET_BUFFER_WIDTH, index / MAX_MORPH_TARGET_BUFFER_WIDTH, 0);
    int c = object_uniforms_morphTargetCount;
    for (int i = 0; i < c; ++i) {
        float w = morphingUniforms.weights[i][0];
        if (w != 0.0) {
            texcoord.z = i;
            ivec4 tangent = texelFetch(sampler1_tangents, texcoord, 0);
            vec3 normal;
            toTangentFrame(float4(tangent) * (1.0 / 32767.0), normal);
            n += w * (normal - baseNormal);
        }
    }
}
#endif

/** @public-api */
vec4 getPosition() {
    vec4 pos = mesh_position;

#if defined(VARIANT_HAS_SKINNING_OR_MORPHING)

    if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_MORPHING_ENABLED_BIT) != 0) {
#if defined(LEGACY_MORPHING)
        pos += morphingUniforms.weights[0] * mesh_custom0;
        pos += morphingUniforms.weights[1] * mesh_custom1;
        pos += morphingUniforms.weights[2] * mesh_custom2;
        pos += morphingUniforms.weights[3] * mesh_custom3;
#else
        morphPosition(pos);
#endif
    }

    if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_SKINNING_ENABLED_BIT) != 0) {
        skinPosition(pos.xyz, mesh_bone_indices, mesh_bone_weights);
    }

#endif

    return pos;
}

#if defined(HAS_ATTRIBUTE_CUSTOM0)
vec4 getCustom0() { return mesh_custom0; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM1)
vec4 getCustom1() { return mesh_custom1; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM2)
vec4 getCustom2() { return mesh_custom2; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM3)
vec4 getCustom3() { return mesh_custom3; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM4)
vec4 getCustom4() { return mesh_custom4; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM5)
vec4 getCustom5() { return mesh_custom5; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM6)
vec4 getCustom6() { return mesh_custom6; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM7)
vec4 getCustom7() { return mesh_custom7; }
#endif

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------

/**
 * Computes and returns the position in world space of the current vertex.
 * The world position computation depends on the current vertex domain. This
 * function optionally applies vertex skinning if needed.
 *
 * NOTE: the "transform" and "position" temporaries are necessary to work around
 * an issue with Adreno drivers (b/110851741).
 */
vec4 computeWorldPosition() {
#if defined(VERTEX_DOMAIN_OBJECT)
    mat4 transform = getWorldFromModelMatrix();
    vec3 position = getPosition().xyz;
    return mulMat4x4Float3(transform, position);
#elif defined(VERTEX_DOMAIN_WORLD)
    return vec4(getPosition().xyz, 1.0);
#elif defined(VERTEX_DOMAIN_VIEW)
    mat4 transform = getWorldFromViewMatrix();
    vec3 position = getPosition().xyz;
    return mulMat4x4Float3(transform, position);
#elif defined(VERTEX_DOMAIN_DEVICE)
    mat4 transform = getWorldFromClipMatrix();
    vec4 p = getPosition();
    // GL convention to inverted DX convention
    p.z = p.z * -0.5 + 0.5;
    vec4 position = transform * p;
    // w could be zero (e.g.: with the skybox) which corresponds to an infinite distance in
    // world-space. However, we want to avoid infinites and divides-by-zero, so we use a very
    // small number instead in that case (2^-63 seem to work well).
    const highp float ALMOST_ZERO_FLT = 1.08420217249e-19;
    if (abs(position.w) < ALMOST_ZERO_FLT) {
        position.w = position.w < 0.0 ? -ALMOST_ZERO_FLT : ALMOST_ZERO_FLT;
    }
    return position * (1.0 / position.w);
#else
#error Unknown Vertex Domain
#endif
}

/**
 * Index of the eye being rendered, starting at 0.
 * @public-api
 */
int getEyeIndex() {
#if defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_INSTANCED)
    return instance_index % CONFIG_STEREO_EYE_COUNT;
#elif defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_MULTIVIEW)
    // gl_ViewID_OVR is of uint type, which needs an explicit conversion.
    return int(gl_ViewID_OVR);
#endif
    return 0;
}
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0                     
#endif
struct MaterialVertexInputs {
#ifdef HAS_ATTRIBUTE_COLOR
    vec4 color;
#endif
#ifdef HAS_ATTRIBUTE_UV0
    vec2 uv0;
#endif
#ifdef HAS_ATTRIBUTE_UV1
    vec2 uv1;
#endif
#ifdef VARIABLE_CUSTOM0
    vec4 VARIABLE_CUSTOM0;
#endif
#ifdef VARIABLE_CUSTOM1
    vec4 VARIABLE_CUSTOM1;
#endif
#ifdef VARIABLE_CUSTOM2
    vec4 VARIABLE_CUSTOM2;
#endif
#ifdef VARIABLE_CUSTOM3
    vec4 VARIABLE_CUSTOM3;
#endif
#ifdef HAS_ATTRIBUTE_TANGENTS
    vec3 worldNormal;
#endif
    vec4 worldPosition;
#ifdef VERTEX_DOMAIN_DEVICE
#ifdef MATERIAL_HAS_CLIP_SPACE_TRANSFORM
    mat4 clipSpaceTransform;
#endif // MATERIAL_HAS_CLIP_SPACE_TRANSFORM
#endif // VERTEX_DOMAIN_DEVICE
};

// Workaround for a driver bug on ARM Bifrost GPUs. Assigning a structure member
// (directly or inside an expression) to an invariant causes a driver crash.
vec4 getWorldPosition(const MaterialVertexInputs material) {
    return material.worldPosition;
}

#ifdef VERTEX_DOMAIN_DEVICE
#ifdef MATERIAL_HAS_CLIP_SPACE_TRANSFORM
mat4 getMaterialClipSpaceTransform(const MaterialVertexInputs material) {
    return material.clipSpaceTransform;
}
#endif // MATERIAL_HAS_CLIP_SPACE_TRANSFORM
#endif // VERTEX_DOMAIN_DEVICE

void initMaterialVertex(out MaterialVertexInputs material) {
#ifdef HAS_ATTRIBUTE_COLOR
    material.color = mesh_color;
#endif
#ifdef HAS_ATTRIBUTE_UV0
    #ifdef FLIP_UV_ATTRIBUTE
    material.uv0 = vec2(mesh_uv0.x, 1.0 - mesh_uv0.y);
    #else
    material.uv0 = mesh_uv0;
    #endif
#endif
#ifdef HAS_ATTRIBUTE_UV1
    #ifdef FLIP_UV_ATTRIBUTE
    material.uv1 = vec2(mesh_uv1.x, 1.0 - mesh_uv1.y);
    #else
    material.uv1 = mesh_uv1;
    #endif
#endif
#ifdef VARIABLE_CUSTOM0
    material.VARIABLE_CUSTOM0 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM1
    material.VARIABLE_CUSTOM1 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM2
    material.VARIABLE_CUSTOM2 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM3
    material.VARIABLE_CUSTOM3 = vec4(0.0);
#endif
    material.worldPosition = computeWorldPosition();
#ifdef VERTEX_DOMAIN_DEVICE
#ifdef MATERIAL_HAS_CLIP_SPACE_TRANSFORM
    material.clipSpaceTransform = mat4(1.0);
#endif
#endif
}
#line 14
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 13                  
#endif

    void materialVertex(inout MaterialVertexInputs material) {
        material.customnormal =getCustom0();
    }
#line 1138
#if defined(GL_GOOGLE_cpp_style_line_directive)
#line 0          
#endif
/*
 * This is the main vertex shader of surface materials. It can be invoked with
 * USE_OPTIMIZED_DEPTH_VERTEX_SHADER defined, and in this case we are guaranteed that the
 * DEPTH variant is active *AND* there is no custom vertex shader (i.e.: materialVertex() is
 * empty).
 * We can use this to remove all code that doesn't participate in the depth computation.
 */

void main() {
#if defined(FILAMENT_HAS_FEATURE_INSTANCING)
#   if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    instance_index = gl_InstanceIndex;
#   else
    // PowerVR drivers don't initialize gl_InstanceID correctly if it's assigned to the varying
    // directly and early in the shader. Adding a bit of extra integer math, works around it.
    // Using an intermediate variable doesn't work because of spirv-opt.
    if (CONFIG_POWER_VR_SHADER_WORKAROUNDS) {
        instance_index = (1 + gl_InstanceID) - 1;
    } else {
        instance_index = gl_InstanceID;
    }
#   endif
    logical_instance_index = instance_index;
#endif

#if defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_INSTANCED)
#if !defined(FILAMENT_HAS_FEATURE_INSTANCING)
#error Instanced stereo not supported at this feature level
#endif
    // Calculate the logical instance index, which is the instance index within a single eye.
    logical_instance_index = instance_index / CONFIG_STEREO_EYE_COUNT;
#endif

    initObjectUniforms();

    // Initialize the inputs to sensible default values, see material_inputs.vs
#if defined(USE_OPTIMIZED_DEPTH_VERTEX_SHADER)

    // In USE_OPTIMIZED_DEPTH_VERTEX_SHADER mode, we can even skip this if we're already in
    // VERTEX_DOMAIN_DEVICE and we don't have VSM.
#if !defined(VERTEX_DOMAIN_DEVICE) || defined(VARIANT_HAS_VSM)
    // Run initMaterialVertex to compute material.worldPosition.
    MaterialVertexInputs material;
    initMaterialVertex(material);
    // materialVertex() is guaranteed to be empty here, but we keep it to workaround some problem
    // in NVIDA drivers related to depth invariance.
    materialVertex(material);
#endif

#else // defined(USE_OPTIMIZED_DEPTH_VERTEX_SHADER)

    MaterialVertexInputs material;
    initMaterialVertex(material);

#if defined(HAS_ATTRIBUTE_TANGENTS)
    // If the material defines a value for the "normal" property, we need to output
    // the full orthonormal basis to apply normal mapping
    #if defined(MATERIAL_NEEDS_TBN)
        // Extract the normal and tangent in world space from the input quaternion
        // We encode the orthonormal basis as a quaternion to save space in the attributes
        toTangentFrame(mesh_tangents, material.worldNormal, vertex_worldTangent.xyz);

        #if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
        if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_MORPHING_ENABLED_BIT) != 0) {
            #if defined(LEGACY_MORPHING)
            vec3 normal0, normal1, normal2, normal3;
            toTangentFrame(mesh_custom4, normal0);
            toTangentFrame(mesh_custom5, normal1);
            toTangentFrame(mesh_custom6, normal2);
            toTangentFrame(mesh_custom7, normal3);
            vec3 baseNormal = material.worldNormal;
            material.worldNormal += morphingUniforms.weights[0].xyz * (normal0 - baseNormal);
            material.worldNormal += morphingUniforms.weights[1].xyz * (normal1 - baseNormal);
            material.worldNormal += morphingUniforms.weights[2].xyz * (normal2 - baseNormal);
            material.worldNormal += morphingUniforms.weights[3].xyz * (normal3 - baseNormal);
            #else
            morphNormal(material.worldNormal);
            material.worldNormal = normalize(material.worldNormal);
            #endif
        }

        if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_SKINNING_ENABLED_BIT) != 0) {
            skinNormalTangent(material.worldNormal, vertex_worldTangent.xyz, mesh_bone_indices, mesh_bone_weights);
        }
        #endif

        // We don't need to normalize here, even if there's a scale in the matrix
        // because we ensure the worldFromModelNormalMatrix pre-scales the normal such that
        // all its components are < 1.0. This prevents the bitangent to exceed the range of fp16
        // in the fragment shader, where we renormalize after interpolation
        vertex_worldTangent.xyz = getWorldFromModelNormalMatrix() * vertex_worldTangent.xyz;
        vertex_worldTangent.w = mesh_tangents.w;
        material.worldNormal = getWorldFromModelNormalMatrix() * material.worldNormal;
    #else // MATERIAL_NEEDS_TBN
        // Without anisotropy or normal mapping we only need the normal vector
        toTangentFrame(mesh_tangents, material.worldNormal);

        #if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
        if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_MORPHING_ENABLED_BIT) != 0) {
            #if defined(LEGACY_MORPHING)
            vec3 normal0, normal1, normal2, normal3;
            toTangentFrame(mesh_custom4, normal0);
            toTangentFrame(mesh_custom5, normal1);
            toTangentFrame(mesh_custom6, normal2);
            toTangentFrame(mesh_custom7, normal3);
            vec3 baseNormal = material.worldNormal;
            material.worldNormal += morphingUniforms.weights[0].xyz * (normal0 - baseNormal);
            material.worldNormal += morphingUniforms.weights[1].xyz * (normal1 - baseNormal);
            material.worldNormal += morphingUniforms.weights[2].xyz * (normal2 - baseNormal);
            material.worldNormal += morphingUniforms.weights[3].xyz * (normal3 - baseNormal);
            #else
            morphNormal(material.worldNormal);
            material.worldNormal = normalize(material.worldNormal);
            #endif
        }

        if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_SKINNING_ENABLED_BIT) != 0) {
            skinNormal(material.worldNormal, mesh_bone_indices, mesh_bone_weights);
        }
        #endif

        material.worldNormal = getWorldFromModelNormalMatrix() * material.worldNormal;

    #endif // MATERIAL_HAS_ANISOTROPY || MATERIAL_HAS_NORMAL || MATERIAL_HAS_CLEAR_COAT_NORMAL
#endif // HAS_ATTRIBUTE_TANGENTS

    // Invoke user code
    materialVertex(material);

    // Handle built-in interpolated attributes
#if defined(HAS_ATTRIBUTE_COLOR)
    vertex_color = material.color;
#endif
#if defined(HAS_ATTRIBUTE_UV0)
    vertex_uv01.xy = material.uv0;
#endif
#if defined(HAS_ATTRIBUTE_UV1)
    vertex_uv01.zw = material.uv1;
#endif

    // Handle user-defined interpolated attributes
#if defined(VARIABLE_CUSTOM0)
    VARIABLE_CUSTOM_AT0 = material.VARIABLE_CUSTOM0;
#endif
#if defined(VARIABLE_CUSTOM1)
    VARIABLE_CUSTOM_AT1 = material.VARIABLE_CUSTOM1;
#endif
#if defined(VARIABLE_CUSTOM2)
    VARIABLE_CUSTOM_AT2 = material.VARIABLE_CUSTOM2;
#endif
#if defined(VARIABLE_CUSTOM3)
    VARIABLE_CUSTOM_AT3 = material.VARIABLE_CUSTOM3;
#endif

    // The world position can be changed by the user in materialVertex()
    vertex_worldPosition.xyz = material.worldPosition.xyz;

#ifdef HAS_ATTRIBUTE_TANGENTS
    vertex_worldNormal = material.worldNormal;
#endif

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
    vertex_lightSpacePosition = computeLightSpacePosition(
            vertex_worldPosition.xyz, vertex_worldNormal,
            frameUniforms.lightDirection,
            shadowUniforms.shadows[0].normalBias,
            shadowUniforms.shadows[0].lightFromWorldMatrix);
#endif

#endif // !defined(USE_OPTIMIZED_DEPTH_VERTEX_SHADER)

    vec4 position;

#if defined(VERTEX_DOMAIN_DEVICE)
    // The other vertex domains are handled in initMaterialVertex()->computeWorldPosition()
    position = getPosition();

#if !defined(USE_OPTIMIZED_DEPTH_VERTEX_SHADER)
#if defined(MATERIAL_HAS_CLIP_SPACE_TRANSFORM)
    position = getMaterialClipSpaceTransform(material) * position;
#endif
#endif // !USE_OPTIMIZED_DEPTH_VERTEX_SHADER

#if defined(MATERIAL_HAS_VERTEX_DOMAIN_DEVICE_JITTERED)
    // Apply the clip-space transform which is normally part of the projection
    position.xy = position.xy * frameUniforms.clipTransform.xy + (position.w * frameUniforms.clipTransform.zw);
#endif
#else
    position = getClipFromWorldMatrix() * getWorldPosition(material);
#endif

#if defined(VERTEX_DOMAIN_DEVICE)
    // GL convention to inverted DX convention (must happen after clipSpaceTransform)
    position.z = position.z * -0.5 + 0.5;
#endif

#if defined(VARIANT_HAS_VSM)
    // For VSM, we use the linear light-space Z coordinate as the depth metric, which works for both
    // directional and spot lights and can be safely interpolated.
    // The value is guaranteed to be between [-znear, -zfar] by construction of viewFromWorldMatrix,
    // (see ShadowMap.cpp).
    // Use vertex_worldPosition.w which is otherwise not used to store the interpolated
    // light-space depth.
    highp float z = (getViewFromWorldMatrix() * getWorldPosition(material)).z;

    // rescale [near, far] to [0, 1]
    highp float depth = -z * frameUniforms.oneOverFarMinusNear - frameUniforms.nearOverFarMinusNear;

    // remap depth between -1 and 1
    depth = depth * 2.0 - 1.0;

    vertex_worldPosition.w = depth;
#endif

    // this must happen before we compensate for vulkan below
    vertex_position = position;

#if defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_INSTANCED)
    // We're transforming a vertex whose x coordinate is within the range (-w to w).
    // To move it to the correct portion of the viewport, we need to modify the x coordinate.
    // It's important to do this after computing vertex_position.
    int eyeIndex = instance_index % CONFIG_STEREO_EYE_COUNT;

    float ndcViewportWidth = 2.0 / float(CONFIG_STEREO_EYE_COUNT);  // the width of ndc space is 2
    float eyeZeroMidpoint = -1.0f + ndcViewportWidth / 2.0;

    float transform = eyeZeroMidpoint + ndcViewportWidth * float(eyeIndex);
    position.x *= 1.0 / float(CONFIG_STEREO_EYE_COUNT);
    position.x += transform * position.w;

    // A fragment is clipped when gl_ClipDistance is negative (outside the clip plane).

    float leftClip  = position.x +
            (1.0 - ndcViewportWidth * float(eyeIndex)) * position.w;
    float rightClip = position.x +
            (1.0 - ndcViewportWidth * float(eyeIndex + 1)) * position.w;
    FILAMENT_CLIPDISTANCE[0] =  leftClip;
    FILAMENT_CLIPDISTANCE[1] = -rightClip;
#endif

#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    position.y = -position.y;
#endif

#if !defined(TARGET_VULKAN_ENVIRONMENT) && !defined(TARGET_METAL_ENVIRONMENT)
    // This is not needed in Vulkan or Metal because clipControl is always (1, 0)
    // (We don't use a dot() here because it workaround a spirv-opt optimization that in turn
    //  causes a crash on PowerVR, see #5118)
    position.z = position.z * frameUniforms.clipControl.x + position.w * frameUniforms.clipControl.y;
#endif

    // some PowerVR drivers crash when gl_Position is written more than once
    gl_Position = position;

#if defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_INSTANCED)
    // Fragment shaders filter out the stereo variant, so we need to set instance_index here.
    instance_index = logical_instance_index;
#endif
}

