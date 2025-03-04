// RUN: %dxc -T cs_6_0 -E main -O3 -Vd %s -spirv | FileCheck %s

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %gSBuffer
// CHECK-NEXT: [[val:%[0-9]+]] = OpLoad %S [[ptr]]
// CHECK-NEXT: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %gRWSBuffer
// CHECK-NEXT:                OpStore [[ptr]] [[val]]

struct S {
  float4 f;
};

int i;

StructuredBuffer<S> gSBuffer;
RWStructuredBuffer<S> gRWSBuffer;

RWStructuredBuffer<S> foo() {
  return gRWSBuffer;
}

[numthreads(1,1,1)]
void main() {
  RWStructuredBuffer<S> lRWSBuffer = foo();
  lRWSBuffer[i] = gSBuffer[i];
}
