// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: define void @main()
// CHECK-NOT: fast

RWTexture1D<float> u0;
Texture2D<float4> t0;
SamplerState s0;
SamplerState s1;

struct VSIn
{
    float4 Position  : ATTRIBUTE0;
};

struct VSOut
{
    float4 Position  : SV_Position;
};

static int array[4];
int array_case(float f) {
    // TODO: Propagate precise through conditions that control branches
    // that impact incoming values in phi nodes.
    if (f * f > 16.0) {
        return (int) 1.0 / f;
    } else {
        for (int i = 0; i < 4; ++i) {
            array[i] = f + (i * i);
        }
        return array[(int)((f < 0.0) ? -f : f+1) % 4];
    }
}

precise float4 MakePrecise(float4 v) {
    precise float4 pv = v;
    return pv;
}

[RootSignature("DescriptorTable(SRV(t0), UAV(u0), CBV(b0)), DescriptorTable(Sampler(s0, numDescriptors=2))")]
VSOut main(VSIn input)
{
    VSOut o;
    float2 uv = input.Position.xy * input.Position.zw;
    float4 pos = t0.SampleLevel(s0, uv, 0);
    o.Position = MakePrecise(pos * input.Position + array_case(input.Position.x));
    return o;
}
