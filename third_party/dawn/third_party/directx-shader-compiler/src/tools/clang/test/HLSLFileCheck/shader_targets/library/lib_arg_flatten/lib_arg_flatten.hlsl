// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure function call on external function has correct type.
// CHECK: call float @"\01?test_extern{{[@$?.A-Za-z0-9_]+}}"(%struct.T* {{.*}}, [2 x %struct.T]* {{.*}}, %struct.T* {{.*}}, %class.matrix.float.2.2* dereferenceable(16) {{.*}})

struct T {
  float a;
  float b;
};

float test_extern(T t, T t2[2], out T t3, inout float2x2 m);

float test(T t, T t2[2], out T t3, inout float2x2 m)
{
  return test_extern(t, t2, t3, m);
}
