// RUN: %dxc -T hs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

#define NumOutPoints 2

// CHECK: %gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output

struct HsPcfOut
{
  float tessOuter[3] : SV_TessFactor;
  // According to HLSL doc, this should actually be a scalar float.
  // But developers sometimes use float[1].
  float tessInner[1] : SV_InsideTessFactor;
};

struct HsCpOut
{
    float4   pos : SV_Position;
};


// Patch Constant Function
HsPcfOut pcf() {
  HsPcfOut output;
  output = (HsPcfOut)0;
  return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(NumOutPoints)]
[patchconstantfunc("pcf")]
HsCpOut main(uint cpId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID) {
// CHECK:      [[ret:%[0-9]+]] = OpFunctionCall %HsPcfOut %pcf
// CHECK:      [[itf:%[0-9]+]] = OpCompositeExtract %_arr_float_uint_1 [[ret]] 1
// CHECK-NEXT: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %uint_0
// CHECK-NEXT:  [[e0:%[0-9]+]] = OpCompositeExtract %float [[itf]] 0
// CHECK-NEXT: OpStore [[ptr]] [[e0]]
    HsCpOut output;
    output = (HsCpOut)0;
    return output;
}
