#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct PerVertex
{
    float4 v_color;
    float2 v_texPos;
    float3 v_worldPosition;
};

struct u_objToWorlds
{
    float4x4 u_objToWorld;
};

struct CameraData
{
    float4x4 u_projectFromView;
    float4x4 u_projectFromWorld;
    float4x4 u_clipFromPixels;
    float4x4 u_viewFromWorld;
    float4x4 u_worldFromView;
    float4x4 u_viewFromProject;
    packed_float3 u_cameraPosition;
    float pad0;
    packed_float3 u_cameraRight;
    float pad1;
    packed_float3 u_cameraUp;
    float pad2;
    packed_float3 u_cameraForward;
    float pad3;
    float2 u_clipPlanes;
    float2 u_invProjParams;
    float2 u_viewportSize;
    float2 u_invViewportSize;
    float2 u_viewportOffset;
    float2 u_fragToLightGrid;
    float2 u_screenToLightGrid;
    float2 pad4;
    float4 u_lightGridZParams;
};

struct LightData
{
    float4 posRadius;
    float3 color;
    float4 axis;
    float3 directionalParams;
    float4 spotParams;
    float4 shadowSize;
    float4x4 cookieFromWorld;
    float4x4 shadowFromWorld;
    uint shadowTextureValid;
    uint lightCookieValid;
    float minLightDistance;
    float pad;
};

struct SceneSettings
{
    CameraData u_cameras[2];
    float3 u_centerPosition;
    float3 u_centerRight;
    float3 u_centerUp;
    float3 u_centerForward;
    float2 u_times;
    uint u_lightFXMask;
    LightData u_lights[8];
    float3 u_globalLightDir;
    float3 u_globalLightDiffuseColor;
    float3 u_globalLightSpecularColor;
    float2 u_distanceFog;
    float2 u_heightFog;
    float4 u_fogColor;
    uint u_debugMode;
};

struct LightContext
{
    float3 u_ambientIBLTint;
    float3 u_specularIBLTint;
    float3 u_iblAABBMin;
    float3 u_iblAABBMax;
    float3 u_iblAABBCenter;
    float LightContext_unusued0;
    float u_exposureMultiplier;
};

struct main0_out
{
    float4 m_4_v_color [[user(locn1)]];
    float2 m_4_v_texPos [[user(locn2)]];
    float3 m_4_v_worldPosition [[user(locn3)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 a_position [[attribute(0)]];
    float2 a_texcoord [[attribute(1)]];
    float4 a_color [[attribute(2)]];
};

static inline __attribute__((always_inline))
void DrawWorldVS(thread PerVertex& _4, thread float4& a_color, thread float2& a_texcoord, thread float3& a_position, thread float4& gl_Position, constant u_objToWorlds& _15, constant SceneSettings& _22)
{
    _4.v_color = a_color;
    _4.v_texPos = a_texcoord;
    _4.v_worldPosition = a_position;
    float4 worldSpacePosition = _15.u_objToWorld * float4(a_position, 1.0);
    gl_Position = _22.u_cameras[0].u_projectFromWorld * worldSpacePosition;
}

vertex main0_out main0(main0_in in [[stage_in]], constant u_objToWorlds& _15 [[buffer(0)]], constant SceneSettings& _22 [[buffer(1)]])
{
    main0_out out = {};
    PerVertex _4 = {};
    DrawWorldVS(_4, in.a_color, in.a_texcoord, in.a_position, out.gl_Position, _15, _22);
    out.m_4_v_color = _4.v_color;
    out.m_4_v_texPos = _4.v_texPos;
    out.m_4_v_worldPosition = _4.v_worldPosition;
    return out;
}

