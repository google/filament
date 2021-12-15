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

// Adreno drivers seem to ignore precision qualifiers in structs, unless they're used in
// UBOs, which is is the case here.
struct ShadowData {
    highp mat4 lightFromWorldMatrix;
    highp vec3 direction;
    float normalBias;
    highp vec4 lightFromWorldZ;
    float texelSizeAtOneMeter;
    float bulbRadiusLs;
    float nearOverFarMinusNear;
};

struct BoneData {
    highp mat3x4 transform;    // bone transform is mat4x3 stored in row-major (last row [0,0,0,1])
    highp uvec4 cof;           // 8 first cofactor matrix of transform's upper left
};
