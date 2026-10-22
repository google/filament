// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




struct Foo
{
  float4 a;
};

RWStructuredBuffer<Foo> uav1;


void main()
{
  uav1.IncrementCounter();
}
