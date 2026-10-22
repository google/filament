// RUN: %dxilver 1.1 | %dxc -E main -T vs_6_1 %s | FileCheck %s

// CHECK: Number of inputs: 7, outputs: 7
// CHECK: Outputs dependent on ViewId: { 4, 5, 6 }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 0 }
// CHECK:   output 1 depends on inputs: { 1 }
// CHECK:   output 2 depends on inputs: { 2 }

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
	uint viewID : SV_ViewID;
};
struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	output.pos = float4(input.pos, 1.0f);
	if (input.viewID % 2) {
		output.color = float3(1.0f, 0.0f, 1.0f);
	} else {
		output.color = float3(0.0f, 1.0f, 0.0f);
	}
	return output;
}
