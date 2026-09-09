// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv -Vd | FileCheck %s

RWStructuredBuffer<float4> a;

[[vk::ext_builtin_input(/* PointCoord */ 16)]]
static const float2 gl_PointCoord;

// CHECK: [[instSet:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void setValue(float4 input);

// CHECK: %src_main = OpFunction
void main(float4 color : COLOR, float instance : SV_InstanceID) {
// CHECK: [[val:%[0-9]+]] = OpLoad %v4float %color
// CHECK:                   OpStore %temp_var_v4float [[val]]
// CHECK:                   OpExtInst %v4float [[instSet]] InterpolateAtSample %temp_var_v4float %uint_0
	a[0] = EvaluateAttributeAtSample(color, 0);

	setValue(color);

// CHECK: [[val:%[0-9]+]] = OpLoad %v4float %c2
// CHECK:                   OpStore %temp_var_v4float_0 [[val]]
// CHECK:                   OpExtInst %v4float [[instSet]] InterpolateAtSample %temp_var_v4float_0 %uint_2
	float4 c2 = color;
	a[2] = EvaluateAttributeAtSample(c2, 2);

// CHECK: [[val:%[0-9]+]] = OpLoad %float %instance
// CHECK:                   OpStore %temp_var_float [[val]]
// CHECK:                   OpExtInst %float [[instSet]] InterpolateAtSample %temp_var_float %uint_3
	a[3] = EvaluateAttributeAtSample(instance, 3);

// CHECK: [[val:%[0-9]+]] = OpLoad %v2float %gl_PointCoord
// CHECK:                   OpStore %temp_var_v2float [[val]]
// CHECK:                   OpExtInst %v2float [[instSet]] InterpolateAtSample %temp_var_v2float %uint_4
	a[4] = float4(EvaluateAttributeAtSample(gl_PointCoord, 4), 0, 0);
}

// CHECK: %setValue = OpFunction
void setValue(float4 input) {
// CHECK: [[val:%[0-9]+]] = OpLoad %v4float %input
// CHECK:                   OpStore %temp_var_v4float_1 [[val]]
// CHECK:                   OpExtInst %v4float [[instSet]] InterpolateAtSample %temp_var_v4float_1 %uint_1
	a[1] = EvaluateAttributeAtSample(input, 1);
}
