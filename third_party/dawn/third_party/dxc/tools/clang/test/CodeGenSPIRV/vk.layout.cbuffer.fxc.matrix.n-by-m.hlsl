// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s


// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 0 Offset 0
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 1 Offset 16
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 1 MatrixStride 16
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 1 RowMajor
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 2 Offset 56

cbuffer buffer0 {
  float dummy0;                      // Offset:    0 Size:     4 [unused]
  float2x3 foo;                      // Offset:   16 Size:    40 [unused]
  float end;                         // Offset:   56 Size:     4
};

float4 main(float4 color : COLOR) : SV_TARGET
{
  color.x += end;
  return color;
}
