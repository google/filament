// FXC command line: fxc /Tps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm | %FileCheck %s -check-prefix=DXIL

// DXIL: !{i32 0, %dx.types.i8x28 addrspace(1)* undef, !"U0", i32 0, i32 1, i32 1, i32 12, i1 false, i1 true

struct Foo {
  float4 a;
  float3 b;
};


RWStructuredBuffer<Foo> buf2;

int main() : SV_Target
{
  return buf2.IncrementCounter();
}