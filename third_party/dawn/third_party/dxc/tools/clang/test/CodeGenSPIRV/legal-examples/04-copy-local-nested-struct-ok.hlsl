// RUN: %dxc -T cs_6_0 -E main -O3 -Vd %s -spirv | FileCheck %s

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %gSBuffer
// CHECK-NEXT: [[val:%[0-9]+]] = OpLoad %S [[ptr]]
// CHECK-NEXT: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %gRWSBuffer
// CHECK-NEXT:                OpStore [[ptr]] [[val]]

struct S {
  float4 f;
};

struct CombinedBuffers {
  StructuredBuffer<S> SBuffer;
  RWStructuredBuffer<S> RWSBuffer;
};

struct S2 {
  CombinedBuffers cb;
};

struct S1 {
  S2 s2;
};

int i;

StructuredBuffer<S> gSBuffer;
RWStructuredBuffer<S> gRWSBuffer;

[numthreads(1,1,1)]
void main() {
  S1 s1;
  s1.s2.cb.SBuffer = gSBuffer;
  s1.s2.cb.RWSBuffer = gRWSBuffer;
  s1.s2.cb.RWSBuffer[i] = s1.s2.cb.SBuffer[i];
}
