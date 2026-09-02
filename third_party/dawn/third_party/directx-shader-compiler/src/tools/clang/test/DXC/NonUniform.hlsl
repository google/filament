// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: createHandle
// CHECK: i1 true)

// CHECK: createHandle
// CHECK: i1 true)

// CHECK: createHandle
// CHECK: i1 true)

// RUN: %dxc %s /T ps_6_0 /DDX12 /Fo %t.NonUniform.cso
// RUN: %dxc %t.NonUniform.cso /dumpbin /Qstrip_rootsignature /Fo %t.NonUniformNoRootSig.cso
// RUN: cat %t.NonUniformNoRootSig.cso
// RUN:%dxa  -listparts %t.NonUniformNoRootSig.cso | FileCheck %s --check-prefix=NO_RS

// NO_RS:Part count
// NO_RS-NOT:RTS0

// RUN: %dxc  %t.NonUniform.cso /dumpbin /extractrootsignature /Fo %t.NonUniformRootSig.cso
// RUN: cat  %t.NonUniformRootSig.cso
// RUN:%dxa  -listparts %t.NonUniformRootSig.cso | FileCheck %s --check-prefix=RS_PART

// RS_PART:Part count: 1
// RS_PART:RTS0

// Verify root signature for NonUniform.cso
// RUN: %dxc  %t.NonUniform.cso /dumpbin /verifyrootsignature %t.NonUniformRootSig.cso
// RUN: %dxc  %t.NonUniformNoRootSig.cso /dumpbin /verifyrootsignature %t.NonUniformRootSig.cso

// Verify mismatched root signatures fail verification

// RUN: %dxc %S/Inputs/smoke.hlsl  /D "semantic = SV_Position" /T vs_6_0 /Zi /Qembed_debug /DDX12 /Fo %t.smoke.define.cso
// RUN: %dxc %t.smoke.define.cso /dumpbin /Qstrip_rootsignature /Fo %t.norootsignature.cso
// RUN: %dxc %t.smoke.define.cso /dumpbin /extractrootsignature /Fo %t.rootsig.cso

// RUN: not %dxc %t.NonUniformNoRootSig.cso /dumpbin /verifyrootsignature %t.rootsig.cso > %t 2>&1
// RUN: FileCheck --input-file=%t %s --check-prefix=RS_MISMATCH

// RS_MISMATCH:Shader sampler descriptor range (RegisterSpace=0, NumDescriptors=3, BaseShaderRegister=2) is not fully bound in root signature.

// RUN: not %dxc %t.norootsignature.cso /dumpbin /verifyrootsignature %t.NonUniformRootSig.cso > %t.mismatch2 2>&1
// RUN: FileCheck --input-file=%t.mismatch2 %s --check-prefix=RS_MISMATCH2

// RS_MISMATCH2:Shader CBV descriptor range (RegisterSpace=0, NumDescriptors=1, BaseShaderRegister=0) is not fully bound in root signature


Texture1D<float4> tex[5] : register(t3);
SamplerState SS[3] : register(s2);

[RootSignature("DescriptorTable(SRV(t3, numDescriptors=5)),\
                DescriptorTable(Sampler(s2, numDescriptors=3))")]
float4 main(int i : A, float j : B) : SV_TARGET
{
  float4 r = tex[NonUniformResourceIndex(i)].Sample(SS[NonUniformResourceIndex(i)], i);
  r += tex[NonUniformResourceIndex(j)].Sample(SS[i], j+2);
  return r;
}
