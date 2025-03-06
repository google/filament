// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Regression test for a crash when array subscript of ConstantBuffer<My_Struct>[]
// is implicitly converted to T.

// A temp `alloca ConstantBuffer<My_Struct>` was generated which couldn't be
// cleaned up during codegen.

// CHECK: %[[handle:.+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 3
// CHECK: %[[cbufret:.+]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %[[handle]]
// CHECK: %[[comp:.+]] = extractvalue %dx.types.CBufRet.f32 %[[cbufret]], 1

struct My_Struct {
  float a;
  float b;
  float c;
  float d;
};

ConstantBuffer<My_Struct> my_cbuf[10];

float main() : SV_Target {
  My_Struct s = my_cbuf[3];
  return s.b;
}

