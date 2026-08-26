// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// Make sure we don't store the initial value (must load from payload after TraceRay)
// CHECK: define void @"\01?RayGenTestMain{{[@$?.A-Za-z0-9_]+}}"()
// CHECK: call void @dx.op.textureStore.f32(i32 67,
// CHECK-NOT: float 0.000000e+00, float 1.000000e+00, float 0.000000e+00, float 1.000000e+00
// CHECK: , i8 15)
// CHECK: ret void

struct Payload {
   float4 abc;
   float4 color;
};

RWTexture2D<float4> RTOutput : register(u0);
RaytracingAccelerationStructure scene : register(t0);

int2 viewportDims;
float3 invView[4];
float tanHalfFovY;

[shader("raygeneration")]
void RayGenTestMain()
{
    uint2 LaunchIndex = DispatchRaysIndex();
    float2 d = ((LaunchIndex.xy / (float2)viewportDims) * 2.f - 1.f);
    float aspectRatio = (float)viewportDims.x / (float)viewportDims.y;

    RayDesc ray;
    ray.Origin = invView[3].xyz;
    ray.Direction = normalize((d.x * invView[0].xyz * tanHalfFovY * aspectRatio) + (-d.y * invView[1].xyz * tanHalfFovY) - invView[2].xyz);
    ray.TMin = 0;
    ray.TMax = 100000;

    Payload payload;
    payload.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    TraceRay(scene, 0 /*rayFlags*/, 0xFF /*rayMask*/, 0 /*sbtRecordOffset*/, 1 /*sbtRecordStride*/, 0 /*missIndex*/, ray, payload);
    RTOutput[LaunchIndex.xy] = payload.color;
}
