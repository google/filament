// RUN: not %dxc -T cs_6_6 -E main -fcgl -spirv %s 2>&1 | FileCheck %s

groupshared uint value;

uint getValue() {
  return 0;
}

uint2 getVector() {
  uint2 output;
  return output;
}

int getArray()[2] {
  int array[2];
  return array;
}

[numthreads(1, 1, 1)]
void main() {
  InterlockedCompareExchange(value, 1, 2, 3);
// CHECK: error: InterlockedCompareExchange requires a reference as output parameter

  InterlockedAdd(value, 1, getValue());
// CHECK: error: InterlockedCompareExchange requires a reference as output parameter

  InterlockedAdd(value, 1, getVector().x);
// CHECK: error: InterlockedCompareExchange requires a reference as output parameter

  InterlockedAdd(value, 1, getArray()[0]);
// CHECK: error: InterlockedCompareExchange requires a reference as output parameter
}
