// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

groupshared int sharedInt[1];
groupshared uint sharedUInt[1];

[numthreads(8, 8, 1)]
void main()
{
  int v;
  // CHECK: cmpxchg
  InterlockedCompareExchange(sharedInt[0], -1, -2, v);
  // CHECK: atomicrmw xchg
  InterlockedExchange(sharedInt[0], -1, v);
  // CHECK: atomicrmw add
  InterlockedAdd(sharedInt[0], -1);
  // CHECK: atomicrmw and
  InterlockedAnd(sharedInt[0], -1);
  // CHECK: atomicrmw or
  InterlockedOr(sharedInt[0], -1);
  // CHECK: atomicrmw xor
  InterlockedXor(sharedInt[0], -1);
  // CHECK: atomicrmw max
  InterlockedMax(sharedInt[0], -1); // -1 to workaround GitHub #2283
  // CHECK: atomicrmw min
  InterlockedMin(sharedInt[0], -1); // -1 to workaround GitHub #2283
  // CHECK: atomicrmw umax
  InterlockedMax(sharedUInt[0], (uint)1);
  // CHECK: atomicrmw umin
  InterlockedMin(sharedUInt[0], (uint)1);
}