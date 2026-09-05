// FXC command line: fxc /T ps_5_1 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




cbuffer Foo1 : register(b5)
{
  float4 g1;
}
cbuffer Foo2 : register(b5)
{
  float4 g2;
}

[RootSignature("DescriptorTable(CBV(b5, numDescriptors=1), visibility=SHADER_VISIBILITY_ALL)")]
float4 main() : SV_TARGET
{
  return g2;
}
