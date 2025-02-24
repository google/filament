// RUN: not %dxc -T cs_6_0 -E main -O3 %s -spirv 2>&1 | FileCheck %s

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

int i;

#define constant 0

void foo1() {
  StructuredBuffer<S> lSBuffer;

  [unroll]
  for( int j = i; j < 2; j++ ) {  // Compiler can't determine start iteration
    if (constant > j) {
      lSBuffer = gSBuffer1;
    } else {
      lSBuffer = gSBuffer2;
    }
    gRWSBuffer[j] = lSBuffer[j];
  }
}

void foo2() {
  StructuredBuffer<S> lSBuffer;

  [unroll]
  for( int j = 0; j < i; j++ ) {  // Compiler can't determine end iteration
    if (constant > j) {
      lSBuffer = gSBuffer1;
    } else {
      lSBuffer = gSBuffer2;
    }
    gRWSBuffer[j] = lSBuffer[j];
  }
}

void foo3() {
  StructuredBuffer<S> lSBuffer;

  [unroll]
  for( int j = 0; j < 2; j += i ) { // Compiler can't determine step count
    if (constant > j) {
      lSBuffer = gSBuffer1;
    } else {
      lSBuffer = gSBuffer2;
    }
    gRWSBuffer[j] = lSBuffer[j];
  }
}


[numthreads(1,1,1)]
void main() {
  foo1(); foo2(); foo3();
}
