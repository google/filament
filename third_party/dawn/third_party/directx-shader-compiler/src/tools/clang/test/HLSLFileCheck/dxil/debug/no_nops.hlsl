// RUN: %dxc -Od -E main -T ps_6_0 %s -opt-disable debug-nops | FileCheck %s

// Test for disabling generation of debug nops.

// CHECK-NOT: @dx.nothing

struct MyStruct {
  float1 foo;
};

[RootSignature("")]
float main(float a : A) : SV_Target {
  MyStruct my_s;
  my_s.foo.x = a;
  return my_s.foo.x;
}

