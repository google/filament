// RUN: %dxc -T hs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct ControlPoint {
  float4 position[1000] : POSITION;
};

struct HullPatchOut {
    float edge [3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

HullPatchOut HullConst () {
  return (HullPatchOut)0;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HullConst")]
[outputcontrolpoints(3)]
ControlPoint main(InputPatch<ControlPoint, 1> large_input,
                  uint id : SV_OutputControlPointID) {
  return large_input[0];
}

// CHECK: OpCapability Tessellation

// CHECK: OpEntryPoint TessellationControl %main "main" [[var:%[a-zA-Z0-9_]+]]
// CHECK: [[array_1:%[a-zA-Z0-9_]+]] = OpTypeArray %v4float %uint_1000
// CHECK: [[array_2:%[a-zA-Z0-9_]+]] = OpTypeArray [[array_1]] %uint_1
// CHECK:     [[ptr:%[a-zA-Z0-9_]+]] = OpTypePointer Input [[array_2]]
// CHECK:                    [[var]] = OpVariable [[ptr]] Input
