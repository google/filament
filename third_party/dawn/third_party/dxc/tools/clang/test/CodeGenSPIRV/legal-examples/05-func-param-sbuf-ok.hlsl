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


int i;

StructuredBuffer<S> gSBuffer;
RWStructuredBuffer<S> gRWSBuffer;

void foo(StructuredBuffer<S> pSBuffer) {
  gRWSBuffer[i] = pSBuffer[i];
}

[numthreads(1,1,1)]
void main() {
  foo(gSBuffer);
}
