// RUN: %dxc -T vs_6_0 -E main -fspv-reflect -fcgl  %s -spirv | FileCheck %s


// CHECK: OpExtension "SPV_GOOGLE_hlsl_functionality1"

// CHECK: OpName %in_var_POSITION "in.var.POSITION"
// CHECK: OpName %in_var_TEXCOORD4 "in.var.TEXCOORD4"
// CHECK: OpName %in_var_TEXCOORD5 "in.var.TEXCOORD5"

// CHECK: OpDecorateString %in_var_POSITION UserSemantic "POSITION"
// CHECK: OpDecorateString %in_var_TEXCOORD4 UserSemantic "TEXCOORD4"
// CHECK: OpDecorateString %in_var_TEXCOORD5 UserSemantic "TEXCOORD5"

// CHECK: %in_var_POSITION = OpVariable %_ptr_Input_v3float Input
// CHECK: %in_var_TEXCOORD4 = OpVariable %_ptr_Input_mat4v3float Input
// CHECK: %in_var_TEXCOORD5 = OpVariable %_ptr_Input_v4float Input

struct SInstanceData {
	float4x3 VisualToWorld;
	float4 Output;
};

struct VS_INPUT {
	float3 Position : POSITION;
	SInstanceData InstanceData : TEXCOORD4;
};

float4 main(const VS_INPUT v) : SV_Position {
	const SInstanceData	I = v.InstanceData;
  return I.Output;
}
