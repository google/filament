// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float4 main(float4 a : A, float4 b : B) : SV_TARGET
{
  return a + b + float4(0,1,2,3);
}
