// RUN: %dxc -T cs_6_0 -E main -O3 -Vd %s -spirv | FileCheck %s

// CHECK:      [[src:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %gSBuffer
// CHECK-NEXT: [[val:%[0-9]+]] = OpLoad %S [[src]]
// CHECK-NEXT: [[dst:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %gRWSBuffer1
// CHECK-NEXT:                OpStore [[dst]] [[val]]
// CHECK-NEXT: [[val:%[0-9]+]] = OpLoad %S [[src]]
// CHECK-NEXT: [[dst:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %gRWSBuffer2
// CHECK-NEXT:                OpStore [[dst]] [[val]]

struct S {
  float4 f;
};

int i;

StructuredBuffer<S> gSBuffer1;
StructuredBuffer<S> gSBuffer2;
RWStructuredBuffer<S> gRWSBuffer1;
RWStructuredBuffer<S> gRWSBuffer2;

#define constant 0

StructuredBuffer<S> bar() {
  if (constant > 2) {
    return gSBuffer1;
  } else {
    return gSBuffer2;
  }
}

void foo(RWStructuredBuffer<S> pRWSBuffer) {
  StructuredBuffer<S> lSBuffer = bar();
  pRWSBuffer[i] = lSBuffer[i];
}

[numthreads(1,1,1)]
void main() {
  foo(gRWSBuffer1);
  foo(gRWSBuffer2);
}
