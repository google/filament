// RUN: %dxc -T cs_6_6 -spirv -fspv-target-env=vulkan1.3 -fcgl %s | FileCheck %s

static const uint Reduce = 0;

#define InclusiveScan 1

enum
{
    ExclusiveScan = 2,
};

[[vk::ext_instruction(/* OpGroupNonUniformBallotBitCount */ 342)]]
uint OpGroupNonUniformBallotBitCount(uint scope, [[vk::ext_literal]] uint groupOperation, uint4 ballot);

[numthreads(1, 1, 1)]
void main()
{
// CHECK: [[ballot:%[0-9]+]] = OpConstantComposite %v4uint %uint_0 %uint_0 %uint_0 %uint_0

// CHECK: {{%[0-9]+}} = OpGroupNonUniformBallotBitCount %uint %uint_3 Reduce [[ballot]]
	OpGroupNonUniformBallotBitCount(vk::SubgroupScope, Reduce, 0);
// CHECK: {{%[0-9]+}} = OpGroupNonUniformBallotBitCount %uint %uint_3 InclusiveScan [[ballot]]
	OpGroupNonUniformBallotBitCount(vk::SubgroupScope, InclusiveScan, 0);
// CHECK: {{%[0-9]+}} = OpGroupNonUniformBallotBitCount %uint %uint_3 ExclusiveScan [[ballot]]
	OpGroupNonUniformBallotBitCount(vk::SubgroupScope, ExclusiveScan, 0);
}
