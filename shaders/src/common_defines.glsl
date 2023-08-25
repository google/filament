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
