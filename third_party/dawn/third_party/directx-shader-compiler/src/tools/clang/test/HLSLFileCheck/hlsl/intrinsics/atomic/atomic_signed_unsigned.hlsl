// RUN: %dxc /Tcs_6_2 /Emain  %s | FileCheck %s

// Make sure choose correct signed unsigned.
// CHECK:atomicrmw max
// CHECK:atomicrmw umin

groupshared int sharedInt[1];
groupshared uint sharedUInt[1];
[numthreads(8, 8, 1)]
void main()
{
  InterlockedMax(sharedInt[0], 1);
  InterlockedMin(sharedUInt[0], 1);
}