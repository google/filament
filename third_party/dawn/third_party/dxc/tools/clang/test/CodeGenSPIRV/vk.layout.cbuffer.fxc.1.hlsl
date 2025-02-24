// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

struct S {
  float a;
};

// CHECK: OpMemberDecorate %T 0 Offset 0
// CHECK: OpMemberDecorate %T 1 Offset 16
// CHECK: OpMemberDecorate %T 2 Offset 20
// CHECK: OpMemberDecorate %T 3 Offset 32

struct T {
  float a;
  S     b;
  float c;
  S     d;
};

// CHECK: %type_CB = OpTypeStruct %T

cbuffer CB {
  T CB_T;
};

// CHECK: %type_TB = OpTypeStruct %T

tbuffer TB {
  T TB_T;
};

float4 main() : SV_Target {
  return CB_T.a + TB_T.a;
}
