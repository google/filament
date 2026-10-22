// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// CHECK: Offsets to texture access operations must be immediate values

// Regression test that DxilValueCache (DVC) isn't so over-zealous.

// There was a bug where DVC decides for a PHI node, the predecessor that is
// always reachable must always be the block that branched to it, and could
// therefore take on its incoming value (assuming the value dominates the PHI
// node)

// In the case below, the value of 'x' at the sample statement is not decidable
// at compile time. This test makes sure the compilation fails and displays the
// correct message.

Texture2D tex0 : register(t0);
SamplerState samp0 : register(s0);

[RootSignature("DescriptorTable(SRV(t0)), DescriptorTable(Sampler(s0))")]
float4 main(float2 uv : TEXCOORD, int a : A, int b : B) : SV_Target {
  int x = 0;
  if (a > b) {
    x = 1;
  }
  return tex0.Sample(samp0, uv, int2(x,x));
}

