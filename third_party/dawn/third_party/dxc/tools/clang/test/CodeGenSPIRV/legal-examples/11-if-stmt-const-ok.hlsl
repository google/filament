// RUN: %dxc -T cs_6_0 -E main -O3 -Vd %s -spirv | FileCheck %s

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %gSBuffer2
// CHECK-NEXT: [[val:%[0-9]+]] = OpLoad %S [[ptr]]
// CHECK-NEXT: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %gRWSBuffer
// CHECK-NEXT:                OpStore [[ptr]] [[val]]

struct S {
  float4 f;
};

int i;

StructuredBuffer<S> gSBuffer1;
StructuredBuffer<S> gSBuffer2;
RWStructuredBuffer<S> gRWSBuffer;

#define constant 0

[numthreads(1,1,1)]
void main() {

  StructuredBuffer<S> lSBuffer;
  if (constant > 2) {
    lSBuffer = gSBuffer1;
  } else {
    lSBuffer = gSBuffer2;
  }
  gRWSBuffer[i] = lSBuffer[i];
}
