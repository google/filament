// RUN: not %dxc -T cs_6_0 -E main -O3 %s -spirv 2>&1 | FileCheck %s

// CHECK: Using pointers with OpSelect requires capability

struct S {
  float4 f;
};

struct CombinedBuffers {
  StructuredBuffer<S> SBuffer;
  RWStructuredBuffer<S> RWSBuffer;
};


int i;

StructuredBuffer<S> gSBuffer1;
StructuredBuffer<S> gSBuffer2;
RWStructuredBuffer<S> gRWSBuffer;

#define constant 0

[numthreads(1,1,1)]
void main() {
  StructuredBuffer<S> lSBuffer;
  if (constant > i) {          // Condition can't be computed at compile time.
    lSBuffer = gSBuffer1;      // Will produce invalid SPIR-V for Vulkan.
  } else {
    lSBuffer = gSBuffer2;
  }
  gRWSBuffer[i] = lSBuffer[i];
}
