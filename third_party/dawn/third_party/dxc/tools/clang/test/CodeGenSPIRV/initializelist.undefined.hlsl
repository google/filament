// RUN: not %dxc -T cs_6_2 -E main -enable-16bit-types %s -spirv  2>&1 | FileCheck %s

typedef vector<uint32_t,3> uint32_t3;
typedef vector<uint16_t,3> uint16_t3;
uint32_t3 gl_WorkGroupSize();

struct S {
  float3 a;
};

float3 foo();
RWStructuredBuffer<float> bar();
float baz();

struct Empty {};

struct B {
  float a;
  Empty b;
};

[numthreads(1, 1, 1)]
void main() {

  // createInitForVectorType
// CHECK: 27:20: error: found undefined function
  int4 v = int4(1, foo());
// CHECK: 29:36: error: found undefined function
  const uint16_t3 dims = uint16_t3(gl_WorkGroupSize());

  // createInitForMatrixType
// CHECK: 33:21: error: found undefined function
  float2x2 m = { 0, foo() };

  // createInitForStructType
// CHECK: 37:13: error: found undefined function
  S s  = (S)foo();

  // createInitForConstantArrayType
// CHECK: 41:27: error: found undefined function
  const float data[3] = { foo() };

  // createInitForBufferOrImageType
// CHECK: 46:38: error: found undefined function
// CHECK: 46:36: fatal error: cannot find the associated counter variable
  RWStructuredBuffer<float> buff = { bar() };

  // Valid empty initializer list.
// CHECK-NOT: error
  B b = { 1.f };
}
