// RUN: not %dxc -T cs_6_6 -spirv -fspv-target-env=vulkan1.3 -fcgl %s 2>&1 | FileCheck %s

static uint Reduce;

[[vk::ext_instruction(/* OpGroupNonUniformBallotBitCount */ 342)]]
uint OpGroupNonUniformBallotBitCount(uint scope, [[vk::ext_literal]] uint groupOperation, uint4 ballot);

[numthreads(1, 1, 1)]
void main()
{
// CHECK: error: vk::ext_literal may only be applied to parameters that can be evaluated to a literal value
	OpGroupNonUniformBallotBitCount(vk::SubgroupScope, Reduce, 0);
}
