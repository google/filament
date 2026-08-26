// RUN: %dxc -T lib_6_6 %s | %opt -S -hlsl-dxil-pix-shader-access-instrumentation,config=.256;512;1024. | %FileCheck %s

// This file is checking for the correct access tracking for a descriptor-heap-indexed TLAS for TraceRay.

// First advance through the output text to where the handle for heap index 7 is created:
// CHECK: call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 7

// The next buffer store should be for this resource.
// See DxilShaderAccessTracking::EmitResourceAccess for how this index is calculated. 
// It's the descriptor heap index (7, as seen in the HLSL below) plus 1 (to skip
// over the "out-of-bounds" entry in the output UAV) times 8 DWORDs per record
// (the first DWORD for write, the second for read), plus 4 to offset to the "read" record.
// Read access for descriptor 7 is therefore at (7+1)*8+4 = 68.
// This is then added to the base address for dynamic writes, which is 256
// (from the config=.256 in the command-line above), for a total of 324.

// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %[[UAV:[0-9+]]], i32 324,

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

  RaytracingAccelerationStructure scene = ResourceDescriptorHeap[7];

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

