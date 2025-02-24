// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




cbuffer Foo
{
  float4 g1[16];
};

cbuffer Bar
{
  uint3 idx[8];
};

float4 main(int2 a : A) : SV_TARGET
{
  return g1[idx[a.x].z].wyyy;
}
