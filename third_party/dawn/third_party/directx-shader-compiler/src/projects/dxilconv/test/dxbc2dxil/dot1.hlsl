// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float3 main(float4 a : A, float4 b : B) : SV_Target
{
  float3 r = 0;
  r += dot(a, b);
  r += dot(a.wyz, b.wyz);
  r += dot(a.zz, b.xy);
  return r;
}
