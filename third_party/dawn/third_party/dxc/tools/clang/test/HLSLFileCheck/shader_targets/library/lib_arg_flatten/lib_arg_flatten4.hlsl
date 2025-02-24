// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure function call on external function has correct type.

// CHECK: call float @"\01?test_extern{{[@$?.A-Za-z0-9_]+}}"(%struct.Foo* {{.*}})

struct Foo {
  float a;
};

struct Bar {
  Foo foo;
  float b;
};

float test_extern(Foo foo);

float test(Bar b)
{
  float x = test_extern(b.foo);
  return x + b.b;
}
