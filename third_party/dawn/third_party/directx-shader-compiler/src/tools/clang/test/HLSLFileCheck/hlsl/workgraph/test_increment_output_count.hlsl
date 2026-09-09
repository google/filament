// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -Od %s | FileCheck %s

void loadStressEmptyRecWorker(
EmptyNodeOutput outputNode)
{
	// CHECK: call void @dx.op.incrementOutputCount
	outputNode.GroupIncrementOutputCount(1);
}

[Shader("node")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void loadStressEmptyRec_1(
	[MaxRecords(1)] EmptyNodeOutput loadStressChild
)
{
	loadStressEmptyRecWorker(loadStressChild);
}