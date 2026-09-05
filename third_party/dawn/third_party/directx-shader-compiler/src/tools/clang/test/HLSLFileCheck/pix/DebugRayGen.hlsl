// RUN: %dxc -T lib_6_3 %s | %opt -S -hlsl-dxil-debug-instrumentation,parameter0=10,parameter1=20,parameter2=30 | %FileCheck %s


// Just check that the selection prolog was added:

// CHECK: call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 0)
// CHECK: call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 1)
// CHECK: call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 2)

// There must be three compares:
// CHECK: = icmp eq i32
// CHECK: = icmp eq i32
// CHECK: = icmp eq i32

// Two ANDs of these bools:
// CHECK: and i1
// CHECK: and i1



RaytracingAccelerationStructure scene : register(t0, space0);

RWTexture2D<float4> RTOutput : register(u0);

struct PayloadData
{
  uint index : INDEX;
};

struct AttributeData
{
  float2 barycentrics;
};

struct ColorConstant
{
  uint3 color;
};

struct AlphaConstant
{
  uint alpha;
};


ConstantBuffer<ColorConstant> color : register(b0);
ConstantBuffer<AlphaConstant> alpha : register(b1);

[shader("raygeneration")]
void RayGenMain()
{
  uint2 index = DispatchRaysIndex().xy;
  uint2 dim = DispatchRaysDimensions().xy;

  PayloadData payload;
  payload.index = index.y * dim.x + index.x;

  RayDesc ray;
  ray.Origin.x = 2.0 * (index.x + 0.5) / dim.x - 1.0;
  ray.Origin.y = 1.0 - 2.0 * (index.y + 0.5) / dim.y;
  ray.Origin.z = 0.0;
  ray.Direction = float3(0, 0, -1);
  ray.TMin = 0.01;
  ray.TMax = 100.0;

  TraceRay(
    scene, // Acceleration structure
    0,     // Ray flags
    0xFF,  // Instance inclusion mask
    0,     // RayContributionToHitGroupIndex
    1,     // MultiplierForGeometryContributionToHitGroupIndex
    0,     // MissShaderIndex
    ray,
    payload);

  RTOutput[index] = float4(color.color.r / 255.0f, color.color.g / 255.0f, color.color.b / 255.0f, alpha.alpha / 255.0f);
}

