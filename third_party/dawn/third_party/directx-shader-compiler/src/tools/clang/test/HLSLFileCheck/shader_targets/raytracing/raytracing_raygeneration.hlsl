// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: ; RTAS                              texture     i32         ras      T0             t5     1

// CHECK:@"\01?RTAS{{[@$?.A-Za-z0-9_]+}}" = external constant %struct.RaytracingAccelerationStructure, align 4

// CHECK: define void [[raygen1:@"\\01\?raygen1@[^\"]+"]]() #0 {
// CHECK:   %[[i_0:[0-9]+]] = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS{{[@$?.A-Za-z0-9_]+}}", align 4
// CHECK:   call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 0)
// CHECK:   call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 1)
// CHECK:   call i32 @dx.op.dispatchRaysDimensions.i32(i32 146, i8 0)
// CHECK:   %[[i_8:[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RaytracingAccelerationStructure(i32 160, %struct.RaytracingAccelerationStructure %[[i_0]])
// CHECK:   call void @dx.op.traceRay.struct.MyPayload(i32 157, %dx.types.Handle %[[i_8]], i32 0, i32 0, i32 0, i32 1, i32 0, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.250000e-01, float {{.*}}, float {{.*}}, float {{.*}}, float 1.280000e+02, %struct.MyPayload* nonnull {{.*}})
// CHECK:   ret void

struct MyPayload {
  float4 color;
  uint2 pos;
};

RaytracingAccelerationStructure RTAS : register(t5);

[shader("raygeneration")]
void raygen1()
{
  MyPayload p = (MyPayload)0;
  p.pos = DispatchRaysIndex();
  float3 origin = {0, 0, 0};
  float3 dir = normalize(float3(p.pos / (float)DispatchRaysDimensions(), 1));
  RayDesc ray = { origin, 0.125, dir, 128.0};
  TraceRay(RTAS, RAY_FLAG_NONE, 0, 0, 1, 0, ray, p);
}
