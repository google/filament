// FXC command line: fxc /T ps_5_1 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




struct Foo
{
  float4 g1[16];
};

struct Bar
{
  uint3 idx[16];
};

ConstantBuffer<Foo> buf1[32] : register(b77, space3);
ConstantBuffer<Bar> buf2[64] : register(b17);

[RootSignature("DescriptorTable(CBV(b17, numDescriptors=64, space=0), visibility=SHADER_VISIBILITY_ALL),\
                DescriptorTable(CBV(b77, numDescriptors=32, space=3), visibility=SHADER_VISIBILITY_ALL)")]
float4 main(int3 a : A) : SV_TARGET
{
  return buf1[ buf2[a.x].idx[a.y].z ].g1[a.z + 12].wyyy;
}
