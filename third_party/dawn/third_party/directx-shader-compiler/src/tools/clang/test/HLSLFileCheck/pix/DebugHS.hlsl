// RUN: %dxc -EHSMain -Ths_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation,parameter0=1,parameter1=2 | %FileCheck %s

// Check that the primid detection was emitted:

// CHECK: [[ControlPointId:[^ ]+]] = call i32 @dx.op.outputControlPointID.i32(i32 107)
// CHECK: [[PrimId:[^ ]+]] = call i32 @dx.op.primitiveID.i32(i32 108)
// CHECK: [[CompareToPrimId:[^ ]+]] = icmp eq i32 [[PrimId]], 1
// CHECK: [[CompareToControlPointId:[^ ]+]] = icmp eq i32 [[ControlPointId]], 2
// CHECK: [[CompareBoth:[^ ]+]] = and i1 [[CompareToControlPointId]], [[CompareToPrimId]]

struct HsConstantData {
  float Edges[3] : SV_TessFactor;
  float Inside : SV_InsideTessFactor;
};

struct ControlPoint {
  float3 position : WORLDPOS;
  float2 uv : TEXCOORD0;
};

struct OutputPoint {
  float3 vPosition : BEZIERPOS;
  float4 color : COLOR;
};

#define MAX_POINTS 3

// Patch Constant Function
HsConstantData PatchConstantFunction(
    InputPatch<ControlPoint, MAX_POINTS> ip,
    uint PatchID
    : SV_PrimitiveID) {
  HsConstantData Output;

  Output.Edges[0] = 8;
  Output.Edges[1] = 8;
  Output.Edges[2] = 8;
  Output.Inside = 8;

  return Output;
}

[domain("tri")]
    [partitioning("integer")]
    [outputtopology("triangle_cw")]
    [outputcontrolpoints(MAX_POINTS)]
    [patchconstantfunc("PatchConstantFunction")] OutputPoint
    HSMain(
        InputPatch<ControlPoint, MAX_POINTS> ip,
        uint i
        : SV_OutputControlPointID,
          uint PatchID
        : SV_PrimitiveID) {
  OutputPoint Output;

  // Insert code to compute Output here.
  Output.vPosition = ip[i].position;
  Output.color = float4(i / 16.0, PatchID / 32.0, 0, 0);
  return Output;
}
