// RUN: %dxc -T ds_6_5 -E main -fspv-target-env=vulkan1.2 -spirv %s | FileCheck %s

// CHECK:  OpCapability RayQueryKHR
// CHECK:  OpExtension "SPV_KHR_ray_query"

struct VSSceneIn {
  float3 pos : POSITION;
};

struct PSSceneIn {
  float4 pos : SV_Position;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Simple forwarding Tessellation shaders

struct HSPerVertexData {
  // This is just the original vertex verbatim. In many real life cases this would be a
  // control point instead
  PSSceneIn v;
};

struct HSPerPatchData {
  // We at least have to specify tess factors per patch
  // As we're tesselating triangles, there will be 4 tess factors
  // In real life case this might contain face normal, for example
  float edges[3] : SV_TessFactor;
  float inside : SV_InsideTessFactor;
};

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
void doInitialize(RayQuery<RAY_FLAG_FORCE_OPAQUE> query, RayDesc ray)
{
    query.TraceRayInline(AccelerationStructure,RAY_FLAG_FORCE_NON_OPAQUE,0xFF,ray);
}

// domain shader that actually outputs the triangle vertices
[domain("tri")] PSSceneIn main(const float3 bary
                               : SV_DomainLocation,
                                 const OutputPatch<HSPerVertexData, 3> patch,
                                 const HSPerPatchData perPatchData) {
  PSSceneIn v;
  v.pos = patch[0].v.pos * bary.x + patch[1].v.pos * bary.y + patch[2].v.pos * bary.z + perPatchData.edges[1];

    RayQuery<RAY_FLAG_FORCE_OPAQUE> q;
    RayDesc ray = MakeRayDesc();
// CHECK:  [[rayquery:%[0-9]+]] = OpVariable %_ptr_Function_rayQueryKHR Function
// CHECK:  [[accel:%[0-9]+]] = OpLoad %accelerationStructureNV %AccelerationStructure
// CHECK:  OpRayQueryInitializeKHR [[rayquery]] [[accel]] %uint_1 %uint_255 {{%[0-9]+}} %float_0 {{%[0-9]+}} %float_9999

    q.TraceRayInline(AccelerationStructure,RAY_FLAG_FORCE_OPAQUE, 0xFF, ray);
// CHECK: OpRayQueryInitializeKHR [[rayquery]] [[accel]] %uint_3 %uint_255 {{%[0-9]+}} %float_0 {{%[0-9]+}} %float_9999
    doInitialize(q, ray);

  return v;
}
