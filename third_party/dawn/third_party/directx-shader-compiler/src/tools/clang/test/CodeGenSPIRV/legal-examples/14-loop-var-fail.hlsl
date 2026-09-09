// RUN: not %dxc -T cs_6_0 -E main -O3  %s -spirv 2>&1 | FileCheck %s

// CHECK: Using pointers with OpSelect requires capability

struct S {
  float4 f;
};

struct CombinedBuffers {
  StructuredBuffer<S> SBuffer;
  RWStructuredBuffer<S> RWSBuffer;
};

StructuredBuffer<S> gSBuffer1;
StructuredBuffer<S> gSBuffer2;
RWStructuredBuffer<S> gRWSBuffer;

#define constant 0

[numthreads(1,1,1)]
void main() {

  StructuredBuffer<S> lSBuffer;

  for( int j = 0; j < 2; j++ ) {
    if (constant > j) {         // Condition is different for different iterations
      lSBuffer = gSBuffer1;     // Will produces invalid SPIR-V for Vulkan.
    } else {
      lSBuffer = gSBuffer2;
    }
    gRWSBuffer[j] = lSBuffer[j];
  }
}
