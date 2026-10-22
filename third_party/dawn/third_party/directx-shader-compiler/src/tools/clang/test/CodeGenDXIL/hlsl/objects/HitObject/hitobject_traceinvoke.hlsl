// RUN: %dxc -T lib_6_9 -E main %s -fcgl | FileCheck %s --check-prefix FCGL
// RUN: %dxc -T lib_6_9 -E main %s | FileCheck %s --check-prefix DXIL

// FCGL:  %[[HANDLE:[^ ]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %{{[^ ]+}}, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure undef)
// FCGL-NEXT:  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, %struct.RayDesc*, %struct.Payload*)"(i32 389, %dx.types.HitObject* %{{[^ ]+}}, %dx.types.Handle %[[HANDLE]], i32 513, i32 1, i32 2, i32 4, i32 0, %struct.RayDesc* %{{[^ ]+}}, %struct.Payload* %{{[^ ]+}})
// FCGL-NEXT:  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %struct.Payload*)"(i32 382, %dx.types.HitObject* %{{[^ ]+}}, %struct.Payload* %{{[^ ]+}})

// DXIL:  %[[RTAS:[^ ]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{[^ ]+}}, %dx.types.ResourceProperties { i32 16, i32 0 })  ; AnnotateHandle(res,props)  resource: RTAccelerationStructure
// DXIL:  %[[HIT:[^ ]+]] = call %dx.types.HitObject @dx.op.hitObject_TraceRay.struct.Payload(i32 262, %dx.types.Handle %[[RTAS]], i32 513, i32 1, i32 2, i32 4, i32 0, float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, %struct.Payload* nonnull %{{[^ ]+}})  ; HitObject_TraceRay(accelerationStructure,rayFlags,instanceInclusionMask,rayContributionToHitGroupIndex,multiplierForGeometryContributionToHitGroupIndex,missShaderIndex,Origin_X,Origin_Y,Origin_Z,TMin,Direction_X,Direction_Y,Direction_Z,TMax,payload)
// DXIL:  call void @dx.op.hitObject_Invoke.struct.Payload(i32 267, %dx.types.HitObject %[[HIT]], %struct.Payload* nonnull %{{[^ ]+}})  ; HitObject_Invoke(hitObject,payload)

// DXIL: !dx.dxrPayloadAnnotations = !{![[MDPLD:[^ ]+]]}
// DXIL: ![[MDPLD]] = !{i32 0, %struct.Payload undef, !{{[^ ]+}}}

RaytracingAccelerationStructure RTAS;
RWStructuredBuffer<float> UAV : register(u0);

struct [raypayload]
Payload {
  float3 dummy : read(closesthit) : write(caller, anyhit);
};

[shader("raygeneration")]
void main() {
  RayDesc rayDesc;
  rayDesc.Origin = float3(0.0, 1.0, 2.0);
  rayDesc.TMin = 3.0f;
  rayDesc.Direction = float3(4.0, 5.0, 6.0);
  rayDesc.TMax = 7.0f;

  Payload pld;
  pld.dummy = float3(7.0, 8.0, 9.0);

  dx::HitObject hit = dx::HitObject::TraceRay(
      RTAS,
      RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES,
      1,
      2,
      4,
      0,
      rayDesc,
      pld);

  dx::HitObject::Invoke(hit, pld);
}
