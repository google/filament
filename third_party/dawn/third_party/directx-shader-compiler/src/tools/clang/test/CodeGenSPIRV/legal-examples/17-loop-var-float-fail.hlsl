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

  [unroll]
  for( float j = 0; j < 2; j++ ) {  // Can't infer floating point induction values
    if (constant > j) {
      lSBuffer = gSBuffer1;
    } else {
      lSBuffer = gSBuffer2;
    }
    gRWSBuffer[j] = lSBuffer[j];
  }
}
