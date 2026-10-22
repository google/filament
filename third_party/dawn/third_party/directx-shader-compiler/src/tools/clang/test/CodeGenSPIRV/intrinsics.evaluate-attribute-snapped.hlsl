// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv -Vd | FileCheck %s

RWStructuredBuffer<float4> a;

[[vk::ext_builtin_input(/* PointCoord */ 16)]]
static const float2 gl_PointCoord;

// CHECK: [[instSet:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

// CHECK: [[v2int_0_0:%[0-9]+]] = OpConstantComposite %v2int %int_0 %int_0
// CHECK: [[v2int_2_2:%[0-9]+]] = OpConstantComposite %v2int %int_2 %int_2
// CHECK: [[v2int_3_3:%[0-9]+]] = OpConstantComposite %v2int %int_3 %int_3
// CHECK: [[v2int_4_4:%[0-9]+]] = OpConstantComposite %v2int %int_4 %int_4
// CHECK: [[v2int_1_1:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_1

void setValue(float4 input);

// CHECK: %src_main = OpFunction
void main(float4 color : COLOR, float instance : SV_InstanceID) {
// CHECK:    [[val:%[0-9]+]] = OpLoad %v4float %color
// CHECK:                      OpStore %temp_var_v4float [[val]]
// CHECK: [[offset:%[0-9]+]] = OpConvertSToF %v2float [[v2int_0_0]]
// CHECK:                      OpExtInst %v4float [[instSet]] InterpolateAtOffset %temp_var_v4float [[offset]]
	a[0] = EvaluateAttributeSnapped(color, int2(0,0));

	setValue(color);

// CHECK:    [[val:%[0-9]+]] = OpLoad %v4float %c2
// CHECK:                      OpStore %temp_var_v4float_0 [[val]]
// CHECK: [[offset:%[0-9]+]] = OpConvertSToF %v2float [[v2int_2_2]]
// CHECK:                      OpExtInst %v4float [[instSet]] InterpolateAtOffset %temp_var_v4float_0 [[offset]]
	float4 c2 = color;
	a[2] = EvaluateAttributeSnapped(c2, int2(2,2));

// CHECK:    [[val:%[0-9]+]] = OpLoad %float %instance
// CHECK:                      OpStore %temp_var_float [[val]]
// CHECK: [[offset:%[0-9]+]] = OpConvertSToF %v2float [[v2int_3_3]]
// CHECK:                      OpExtInst %float [[instSet]] InterpolateAtOffset %temp_var_float [[offset]]
	a[3] = EvaluateAttributeSnapped(instance, int2(3,3));

// CHECK:    [[val:%[0-9]+]] = OpLoad %v2float %gl_PointCoord
// CHECK:                      OpStore %temp_var_v2float [[val]]
// CHECK: [[offset:%[0-9]+]] = OpConvertSToF %v2float [[v2int_4_4]]
// CHECK:                      OpExtInst %v2float [[instSet]] InterpolateAtOffset %temp_var_v2float [[offset]]
	a[4] = float4(EvaluateAttributeSnapped(gl_PointCoord, int2(4, 4)), 0, 0);
}

// CHECK: %setValue = OpFunction
void setValue(float4 input) {
// CHECK:    [[val:%[0-9]+]] = OpLoad %v4float %input
// CHECK:                      OpStore %temp_var_v4float_1 [[val]]
// CHECK: [[offset:%[0-9]+]] = OpConvertSToF %v2float [[v2int_1_1]]
// CHECK:                      OpExtInst %v4float [[instSet]] InterpolateAtOffset %temp_var_v4float_1 [[offset]]
	a[1] = EvaluateAttributeSnapped(input, int2(1,1));
}
