// RUN: %dxc -T cs_6_0 -E main -Zi -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opline.intrinsic.vulkan1.1.hlsl

[numthreads(1,1,1)]void main() {
// CHECK:      OpLine [[file]] 11 11
// CHECK:      OpLoad %uint %SubgroupSize
// CHECK-NEXT: OpLine [[file]] 11 32
// CHECK-NEXT: OpLoad %uint %SubgroupLocalInvocationId
  int i = WaveGetLaneCount() + WaveGetLaneIndex();

// CHECK:      OpLine [[file]] 15 3
// CHECK-NEXT: OpGroupNonUniformElect %bool %uint_3
  WaveIsFirstLane();

// CHECK:      OpLine [[file]] 19 3
// CHECK-NEXT: OpGroupNonUniformAll %bool %uint_3
  WaveActiveAllTrue(i == 1);

// CHECK:      OpLine [[file]] 23 3
// CHECK-NEXT: OpGroupNonUniformAny %bool %uint_3
  WaveActiveAnyTrue(i == 0);

// CHECK:      OpLine [[file]] 27 3
// CHECK-NEXT: OpGroupNonUniformBallot %v4uint %uint_3
  WaveActiveBallot(i == 2);

// CHECK:      OpLine [[file]] 31 3
// CHECK-NEXT: OpGroupNonUniformAllEqual %bool %uint_3
  WaveActiveAllEqual(i);

// CHECK:      OpLine [[file]] 36 3
// CHECK-NEXT: OpGroupNonUniformBallot %v4uint %uint_3
// CHECK-NEXT: OpGroupNonUniformBallotBitCount %uint %uint_3 Reduce
  WaveActiveCountBits(i);

// CHECK:      OpLine [[file]] 40 3
// CHECK-NEXT: OpGroupNonUniformIAdd %int %uint_3 Reduce
  WaveActiveSum(i);

// CHECK:      OpLine [[file]] 44 3
// CHECK-NEXT: OpGroupNonUniformIAdd %int %uint_3 ExclusiveScan
  WavePrefixSum(i);

// CHECK:      OpLine [[file]] 49 3
// CHECK-NEXT: OpGroupNonUniformBallot %v4uint %uint_3
// CHECK-NEXT: OpGroupNonUniformBallotBitCount %uint %uint_3 ExclusiveScan
  WavePrefixCountBits(i == 1);

// CHECK:      OpLine [[file]] 53 3
// CHECK-NEXT: OpGroupNonUniformShuffle %int %uint_3
  WaveReadLaneAt(i, 15);

// CHECK:      OpLine [[file]] 57 3
// CHECK-NEXT: OpGroupNonUniformQuadBroadcast %int %uint_3
  QuadReadLaneAt(i, 15);
}
