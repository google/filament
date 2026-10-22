// RUN: %dxc -T ps_6_0 -E main -fspv-debug=vulkan -fcgl  %s -spirv | FileCheck %s

struct PSInput
{
    float4 color : COLOR;
};

Texture2D bindless[];

sampler DummySampler;

// CHECK: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeArray {{%[0-9]+}} %uint_0

float4 main(PSInput input) : SV_TARGET
{
    return input.color * bindless[4].Sample(DummySampler, float2(1,1));
}

