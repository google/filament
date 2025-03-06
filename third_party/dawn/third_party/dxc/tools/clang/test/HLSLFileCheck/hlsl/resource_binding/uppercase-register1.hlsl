// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: dx.types.CBufRet
// CHECK: Texture2D
// CHECK: SamplerState
// CHECK: RWStructuredBuffer

struct Foo {
  float4 g1;
};

struct Bar {
  float2 g2;
  float4 g3;
};

Texture2D<float4> tex1[10] : register(T0);
SamplerState Samp : register(S0);
ConstantBuffer<Foo> buf1[10] : register(B15, space4);
RWStructuredBuffer<Bar> BarRW : register(U0, space1);

[RootSignature ("DescriptorTable(SRV(t0, numDescriptors=10, space=0))," \
                "StaticSampler(s0)," \
                "DescriptorTable(CBV(b15, numDescriptors=15, space=4))," \
                "UAV(u0, space=1)" \
)]


float4 main(int a : A) : SV_Target {
  BarRW[0].g2 = buf1[a].g1.xy;
  BarRW[0].g3 = buf1[a].g1.zyxw;
  return tex1[0].Gather(Samp, float2(1,1)) + buf1[a].g1.xyzx;
}
