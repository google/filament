// RUN: not %dxc -T ps_6_0 -E main %s -spirv 2>&1 | FileCheck %s

struct PSInput {
  float4 a : SV_Position;
  float4 b : SV_Position;
// CHECK: :5:14: error: input semantic 'SV_Position' used more than once
};

float4 main(PSInput input) : SV_Target
{
	return input.a + input.b;
}
