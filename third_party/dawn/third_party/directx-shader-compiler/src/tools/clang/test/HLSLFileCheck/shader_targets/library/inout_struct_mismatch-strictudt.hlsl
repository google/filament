// RUN: %dxc -T lib_6_x -default-linkage external -HV 2021 %s | FileCheck %s

// CHECK: define <4 x float>
// CHECK-SAME: main
// CHECK-NOT: bitcast
// CHECK-NOT: CallStruct
// CHECK: ParamStruct
// CHECK: call void @llvm.lifetime.start
// CHECK-NOT: bitcast
// CHECK-NOT: CallStruct
// CHECK-LABEL: ret <4 x float>

struct ParamStruct {
  int i;
  float f;
};

struct CallStruct {
  int i;
  float f;
};

void modify(inout ParamStruct s) {
  s.f += 1;
}

void modify_ext(inout ParamStruct s);

CallStruct g_struct;

float4 main() : SV_Target {
  CallStruct local = g_struct;
  modify((ParamStruct)local);
  modify_ext((ParamStruct)local);
  return local.f;
}
