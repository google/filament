// RUN: %dxc -T cs_6_0 -E main -O3 -Vd %s -spirv | FileCheck %s

// CHECK:      [[val:%[0-9]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %gRWSBuffer
// CHECK-NEXT:                OpStore [[ptr]] [[val]]

struct S {
  float4 f;
};

int i;

RWStructuredBuffer<S> gRWSBuffer;

static RWStructuredBuffer<S> sRWSBuffer = gRWSBuffer;

[numthreads(1,1,1)]
void main() {
  sRWSBuffer[i].f = 0.0;
}
