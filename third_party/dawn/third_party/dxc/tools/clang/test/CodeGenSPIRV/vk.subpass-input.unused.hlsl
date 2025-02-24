// Test that the we can correctly compile the compute shader when the
// subpass input is used in  PsSubpassTest, but not in the compute shader.

// RUN: %dxc -T cs_6_0 -E CsTest -fcgl  %s -spirv | FileCheck %s

// The subpass input variable should not be in the module.
// CHECK-NOT: %subInput

[[vk::input_attachment_index (0)]] SubpassInput<float4> subInput;

float4 PsSubpassTest() : SV_TARGET
{
	return subInput.SubpassLoad();
}

float4 f()
{
	return subInput.SubpassLoad();
}

[numthreads (8,1,1)]
void CsTest() {}
