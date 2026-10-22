// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure function call on external function has correct type.

// CHECK: call float @"\01?test_extern{{[@$?.A-Za-z0-9_]+}}"(%struct.T* nonnull {{.*}})

struct T {
  float a;
  float b;
};

float test_extern(T t);

float test(T t)
{
  T tmp = t;
  float x = test_extern(tmp);
  return x + tmp.b;
}
