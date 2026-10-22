// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Make sure different address space arg works.

// CHECK: @"\01?sT{{[@$?.A-Za-z0-9_]+}}" = addrspace(3) global [8 x float]
// CHECK: @"\01?sT{{[@$?.A-Za-z0-9_]+}}" = addrspace(3) global [8 x i32]

struct T {
  float a;
  int b;
};

StructuredBuffer<T> buf;
RWStructuredBuffer<T> buf2;

T test ( T t1) {
  T t;
  t.a = t1.a + 1;
  t.b = t1.b + 2;
  return t;
}

groupshared T sT[ 8 ];


[numthreads(8, 8, 1)]
void main( uint3 tid : SV_DispatchThreadID) {
  if (tid.x == 0) {
    for (uint i=0;i<8;i++)
      sT[i] = buf[i];
  }
  T t = test(sT[tid.y%8]);
  buf2[tid.x] = t;
}

