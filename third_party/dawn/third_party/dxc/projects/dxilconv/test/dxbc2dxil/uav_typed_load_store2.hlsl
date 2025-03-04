// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




RWBuffer<min16int3> uav1 : register(u3);

int3 main(uint a : A, uint b : B) : SV_Target
{
  int3 r = 0;
  uint status;
  r += uav1[b];
  r += uav1.Load(a);
  uav1.Load(b, status); r += status;
  r += uav1.Load(a, status); r += status;
  uav1[b] = (min16int3)r;
  return r;
}
