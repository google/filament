// RUN: %dxc -T cs_6_0 -E main -Zi -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opline.intrinsic.control.barrier.hlsl

groupshared int dest_i;

[numthreads(1,1,1)]void main() {
// CHECK:      OpLine [[file]] 11 3
// CHECK-NEXT: OpControlBarrier %uint_2 %uint_1 %uint_2376
  AllMemoryBarrierWithGroupSync();

// CHECK-NEXT: OpLine [[file]] 15 3
// CHECK-NEXT: OpMemoryBarrier %uint_1 %uint_2120
  DeviceMemoryBarrier();

// CHECK-NEXT: OpLine [[file]] 19 3
// CHECK-NEXT: OpMemoryBarrier %uint_2 %uint_264
  GroupMemoryBarrier();
}
