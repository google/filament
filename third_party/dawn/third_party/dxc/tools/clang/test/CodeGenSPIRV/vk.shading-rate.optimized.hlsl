// RUN: %dxc -T vs_6_6 -E main -spirv -fspv-target-env=vulkan1.3 %s | FileCheck %s

// CHECK-NOT: OpDecorate %unused DescriptorSet
// CHECK-NOT: OpDecorate %unused Binding
RWStructuredBuffer<uint> unused;

struct in0
{
	float4 position : POSITION;
	float2 uv : TEXCOORD0;
};

struct out0
{
	float4 pos : SV_Position;
	uint sr : SV_ShadingRate;
};

// CHECK-NOT: OpEntryPoint Vertex %main "main" %in_var_POSITION %in_var_TEXCOORD0 %gl_Position %{{.*}} %unused
// CHECK:     OpEntryPoint Vertex %main "main" %gl_Position %{{.*}}
out0 main(in0 i)
{
	return (out0) 0;
}
