// RUN: %dxc -T hs_6_5 -E main -fspv-target-env=vulkan1.2 -spirv %s | FileCheck %s

// CHECK:  OpCapability RayQueryKHR
// CHECK:  OpExtension "SPV_KHR_ray_query"

#define MAX_POINTS 3

// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
  float3 vPosition : WORLDPOS;
};

// Output control point
struct CONTROL_POINT
{
  float3 vPosition	: BEZIERPOS;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
  float Edges[4]        : SV_TessFactor;
  float Inside[2]       : SV_InsideTessFactor;
};

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT mainConstant(InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip) {
  HS_CONSTANT_DATA_OUTPUT Output;

  // Must initialize Edges and Inside; otherwise HLSL validation will fail.
  Output.Edges[0]  = 1.0;
  Output.Edges[1]  = 2.0;
  Output.Edges[2]  = 3.0;
  Output.Edges[3]  = 4.0;
  Output.Inside[0] = 5.0;
  Output.Inside[1] = 6.0;

  return Output;
}



RaytracingAccelerationStructure AccelerationStructure : register(t0);
RayDesc MakeRayDesc()
{
    RayDesc desc;
    desc.Origin = float3(0,0,0);
    desc.Direction = float3(1,0,0);
    desc.TMin = 0.0f;
    desc.TMax = 9999.0;
    return desc;
}
void doInitialize(RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_SKIP_TRIANGLES> query, RayDesc ray)
{
    query.TraceRayInline(AccelerationStructure,RAY_FLAG_FORCE_NON_OPAQUE,0xFF,ray);
}

[outputtopology("triangle_ccw")]
[outputcontrolpoints(MAX_POINTS)]
[patchconstantfunc("mainConstant")]
CONTROL_POINT main(InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, uint cpid : SV_OutputControlPointID) {
// CHECK:  %rayQueryKHR = OpTypeRayQueryKHR
// CHECK:  %_ptr_Function_rayQueryKHR = OpTypePointer Function %rayQueryKHR
// CHECK:  [[rayquery:%[0-9]+]] = OpVariable %_ptr_Function_rayQueryKHR Function
    RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_SKIP_TRIANGLES> q;
    RayDesc ray = MakeRayDesc();
// CHECK:  [[accel:%[0-9]+]] = OpLoad %accelerationStructureNV %AccelerationStructure
// CHECK:  OpRayQueryInitializeKHR [[rayquery]] [[accel]] %uint_257 %uint_255 {{%[0-9]+}} %float_0 {{%[0-9]+}} %float_9999

    q.TraceRayInline(AccelerationStructure,RAY_FLAG_FORCE_OPAQUE, 0xFF, ray);
// CHECK: OpRayQueryInitializeKHR [[rayquery]] [[accel]] %uint_259 %uint_255 {{%[0-9]+}} %float_0 {{%[0-9]+}} %float_9999
    doInitialize(q, ray);
    CONTROL_POINT result;
    return result;
}
