// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Regression test for a crash when array subscript of ConstantBuffer<T>
// is assigned to a variable of a wrong type..

// CHECK: error:

struct My_Struct {
  float a;
  float b;
  float c;
  float d;
};
struct My_Struct2 {
  float b;
};

ConstantBuffer<My_Struct> my_cbuf[10];

float main() : SV_Target {
  My_Struct2 s = my_cbuf[3];
  return s.b;
}

