#if defined(TARGET_MOBILE)
#define HIGHP highp
#define MEDIUMP mediump
#else
#define HIGHP
#define MEDIUMP
#endif

#if !defined(TARGET_MOBILE) || defined(TARGET_LANGUAGE_SPIRV)
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
