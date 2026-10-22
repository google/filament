// RUN: %dxc -E main -T ps_6_0 %s -ast-dump | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s -check-prefix=IR

// CHECK: VarDecl {{0x[0-9a-fA-F]+}} {{<.*>}} col:14 used Tex 'Texture2D':'Texture2D<vector<float, 4> >'

// IR: !{i32 0, %"class.Texture2D<vector<float, 4> >"* undef, !"Tex", i32 0, i32 0, i32 1, i32 2, i32 0, ![[EXTRA:[0-9]+]]}
// IR: ![[EXTRA]] = !{i32 0, i32 9}

Texture2D    Tex;
SamplerState Samp;

float4 main(uint val : A) : SV_Target
{
  return Tex.Sample(Samp, float2(0.1, 0.2));
}
