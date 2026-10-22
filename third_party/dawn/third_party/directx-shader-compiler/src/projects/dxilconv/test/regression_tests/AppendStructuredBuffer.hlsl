// FXC command line: fxc /Tps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm | %FileCheck %s -check-prefix=DXIL

// DXIL: !{i32 0, %dx.types.i8x28 addrspace(1)* undef, !"U0", i32 0, i32 5, i32 1, i32 12, i1 false, i1 true

struct X {
  float4 a;
  float3 b;
};

AppendStructuredBuffer<X> buf : register(u5);

#ifdef DX12
#define RS "DescriptorTable(" \
             "UAV(u5), "\
             "visibility=SHADER_VISIBILITY_ALL)"

[RootSignature( RS )]
#endif
float4 main(float i : A) : SV_TARGET
{
  X x = (X)0;
  x.a = i;
  x.b = i + 1;
  buf.Append(x);
  return x.a;
}