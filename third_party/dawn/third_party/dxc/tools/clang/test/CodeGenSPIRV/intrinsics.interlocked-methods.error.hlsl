// RUN: not %dxc -T cs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

groupshared float MyFloat;
RWBuffer<float> MyBuffer;

typedef int MyIntegerType;
RWStructuredBuffer<MyIntegerType> MySBuffer;

[numthreads(1, 1, 1)]
void main()
{
  InterlockedAdd(MyFloat, 1);
  InterlockedCompareStore(MyBuffer[0], MySBuffer[0], MySBuffer[1]);
  InterlockedXor(MySBuffer[2], 5);
}

// CHECK:     :12:3: error: no matching function for call to 'InterlockedAdd'
// CHECK:     :13:3: error: no matching function for call to 'InterlockedCompareStore'
// CHECK-NOT:        error: no matching function for call to
