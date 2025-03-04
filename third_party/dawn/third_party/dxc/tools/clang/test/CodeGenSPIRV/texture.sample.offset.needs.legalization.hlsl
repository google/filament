// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerState      gSampler  : register(s5);
Texture2D<float4> t         : register(t1);

// This shader uses a variable offset for texture sampling that is supposed to
// be converted to a constant value after the legalization. Since we set
// needsLegalization as true when we find a variable offset for texture
// sampling, `--before-legalize-hlsl` option for spirv-val should be enabled
// because of `-fcgl`.

// CHECK:      OpImageSparseSampleImplicitLod
// CHECK-SAME: Offset

float4 sample(int offset, float clamp) {
    uint status;
    return t.Sample(gSampler, float2(0.1, 0.2), offset, clamp, status);
}

float4 main(int2 offset: A) : SV_Target {
    float4 val = 0;
    [unroll] for (int i = 0; i < 3; ++i)
        val = sample(i, offset.x);
    return val;
}
