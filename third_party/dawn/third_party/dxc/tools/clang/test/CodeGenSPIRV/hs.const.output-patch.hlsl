// RUN: %dxc -T hs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct HSCtrlPt {
  float4 ctrlPt : CONTROLPOINT;
};

struct HSPatchConstData {
  float tessFactor[3] : SV_TessFactor;
  float insideTessFactor[1] : SV_InsideTessFactor;
  float4 constData : CONSTANTDATA;
};

// CHECK: OpFunctionCall %HSPatchConstData %HSPatchConstantFunc %temp_var_hullMainRetVal

HSPatchConstData HSPatchConstantFunc(const OutputPatch<HSCtrlPt, 3> input) {
  HSPatchConstData data;

// CHECK: %input = OpFunctionParameter %_ptr_Function__arr_HSCtrlPt_uint_3

// CHECK: [[OutCtrl0:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %input %uint_0 %int_0
// CHECK:   [[input0:%[0-9]+]] = OpLoad %v4float [[OutCtrl0]]
// CHECK: [[OutCtrl1:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %input %uint_1 %int_0
// CHECK:   [[input1:%[0-9]+]] = OpLoad %v4float [[OutCtrl1]]
// CHECK:      [[add:%[0-9]+]] = OpFAdd %v4float [[input0]] [[input1]]
// CHECK: [[OutCtrl2:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %input %uint_2 %int_0
// CHECK:   [[input2:%[0-9]+]] = OpLoad %v4float [[OutCtrl2]]
// CHECK:                     OpFAdd %v4float [[add]] [[input2]]
  data.constData = input[0].ctrlPt + input[1].ctrlPt + input[2].ctrlPt;

  data.tessFactor[0] = 3.0;
  data.tessFactor[1] = 3.0;
  data.tessFactor[2] = 3.0;
  data.insideTessFactor[0] = 3.0;
  return data;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPatchConstantFunc")]
[maxtessfactor(15)]
HSCtrlPt main(InputPatch<HSCtrlPt, 3> input, uint CtrlPtID : SV_OutputControlPointID) {
  HSCtrlPt data;
  data.ctrlPt = input[CtrlPtID].ctrlPt;
  return data;
}
