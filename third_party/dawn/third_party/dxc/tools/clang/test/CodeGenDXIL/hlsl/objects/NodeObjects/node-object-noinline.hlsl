// RUN: %dxc -T lib_6_8 %s | FileCheck %s

// Not supported yet.
// https://github.com/microsoft/DirectXShaderCompiler/issues/6258
// XFAIL: *

[noinline]
void loadStressEmptyRecWorker(
EmptyNodeOutput outputNode)
{
	outputNode.GroupIncrementOutputCount(1);
}

[Shader("node")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_EmptyNodeOutput(
	[MaxRecords(1)] EmptyNodeOutput loadStressChild
)
{
	loadStressEmptyRecWorker(wrapper(loadStressChild));
}
