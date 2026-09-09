// RUN: %dxc -EDSMain -Tds_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation,parameter0=1,parameter1=2 | %FileCheck %s

// Check that the primid detection was emitted:

// CHECK: [[PrimId:[^ ]+]] = call i32 @dx.op.primitiveID.i32(i32 108)
// CHECK: [[CompareToPrimId:[^ ]+]] = icmp eq i32 [[PrimId]], 1

struct HsConstantData {
  float Edges[3] : SV_TessFactor;
  float Inside : SV_InsideTessFactor;
};

struct ControlPoint {
  float3 position : WORLDPOS;
  float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
};

[domain("tri")] PSInput DSMain(HsConstantData input,
                               float3 UV
                               : SV_DomainLocation,
                                 const OutputPatch<ControlPoint, 3> patch) {
  PSInput Output;

  Output.position = float4(
      patch[0].position * UV.x +
      patch[1].position * UV.y +
      patch[2].position * UV.z,
      1);

  return Output;
}
