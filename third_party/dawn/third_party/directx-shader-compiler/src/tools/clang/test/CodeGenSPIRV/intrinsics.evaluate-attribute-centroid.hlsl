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
// CHECK:                   OpExtInst %v4float [[instSet]] InterpolateAtCentroid %temp_var_v4float
	a[0] = EvaluateAttributeCentroid(color);

	setValue(color);

// CHECK: [[val:%[0-9]+]] = OpLoad %v4float %c2
// CHECK:                   OpStore %temp_var_v4float_0 [[val]]
// CHECK:                   OpExtInst %v4float [[instSet]] InterpolateAtCentroid %temp_var_v4float_0
	float4 c2 = color;
	a[2] = EvaluateAttributeCentroid(c2);

// CHECK: [[val:%[0-9]+]] = OpLoad %float %instance
// CHECK:                   OpStore %temp_var_float [[val]]
// CHECK:                   OpExtInst %float [[instSet]] InterpolateAtCentroid %temp_var_float
	a[3] = EvaluateAttributeCentroid(instance);

// CHECK: [[val:%[0-9]+]] = OpLoad %v2float %gl_PointCoord
// CHECK:                   OpStore %temp_var_v2float [[val]]
// CHECK:                   OpExtInst %v2float [[instSet]] InterpolateAtCentroid %temp_var_v2float
	a[4] = float4(EvaluateAttributeCentroid(gl_PointCoord), 0, 0);
}


// CHECK: %setValue = OpFunction
void setValue(float4 input) {
// CHECK: [[val:%[0-9]+]] = OpLoad %v4float %input
// CHECK:                   OpStore %temp_var_v4float_1 [[val]]
// CHECK:                   OpExtInst %v4float [[instSet]] InterpolateAtCentroid %temp_var_v4float_1
	a[1] = EvaluateAttributeCentroid(input);
}
