// RUN: %dxc -spirv -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s

struct RECORD1
{
  uint value;
  uint value2;
};

// CHECK: OpEntryPoint GLCompute [[NODE10:%[^ ]*]] "node_1_0"
// CHECK: OpEntryPoint GLCompute [[NODE11:%[^ ]*]] "node_1_1"
// CHECK: OpEntryPoint GLCompute [[NODE12:%[^ ]*]] "node_1_2"
// CHECK: OpEntryPoint GLCompute [[NODE20:%[^ ]*]] "node_2_0"
// CHECK: OpEntryPoint GLCompute [[NODE21:%[^ ]*]] "node_2_1"
// CHECK: OpEntryPoint GLCompute [[NODE22:%[^ ]*]] "node_2_2"
// CHECK: OpDecorateId [[A10:%[^ ]*]] PayloadNodeNameAMDX [[S10:%[^ ]*]]
// CHECK: OpDecorateId [[A10]] NodeMaxPayloadsAMDX [[U31:%[^ ]*]]
// CHECK: OpDecorate [[A10]] PayloadNodeSparseArrayAMDX
// CHECK: OpDecorateId [[A10]] PayloadNodeArraySizeAMDX [[U129:%[^ ]*]]
// CHECK: OpDecorateId [[A11:%[^ ]*]] PayloadNodeNameAMDX [[S11:%[^ ]*]]
// CHECK: OpDecorateId [[A11]] NodeMaxPayloadsAMDX [[U37:%[^ ]*]]
// CHECK: OpDecorate [[A11]] PayloadNodeSparseArrayAMDX
// CHECK: OpDecorateId [[A12:%[^ ]*]] PayloadNodeNameAMDX [[S12:%[^ ]*]]
// CHECK: OpDecorateId [[A12]] NodeMaxPayloadsAMDX [[U47:%[^ ]*]]
// CHECK: OpDecorate [[A12]] PayloadNodeSparseArrayAMDX
// CHECK: OpDecorateId [[A20:%[^ ]*]] PayloadNodeNameAMDX [[S20:%[^ ]*]]
// CHECK: OpDecorateId [[A20]] NodeMaxPayloadsAMDX [[U41:%[^ ]*]]
// CHECK: OpDecorate [[A20]] PayloadNodeSparseArrayAMDX
// CHECK: OpDecorateId [[A20]] PayloadNodeArraySizeAMDX [[U131:%[^ ]*]]
// CHECK: OpDecorateId [[A21:%[^ ]*]] PayloadNodeNameAMDX [[S21:%[^ ]*]]
// CHECK: OpDecorateId [[A21]] NodeMaxPayloadsAMDX [[U43:%[^ ]*]]
// CHECK: OpDecorate [[A21]] PayloadNodeSparseArrayAMDX
// CHECK: OpDecorateId [[A22:%[^ ]*]] PayloadNodeNameAMDX [[S22:%[^ ]*]]
// CHECK: OpDecorateId [[A22]] NodeMaxPayloadsAMDX [[U53:%[^ ]*]]
// CHECK: OpDecorate [[A22]] PayloadNodeSparseArrayAMDX
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK: [[RECORD:%[^ ]*]] = OpTypeStruct [[UINT]] [[UINT]]
// CHECK-DAG: [[A10]] = OpTypeNodePayloadArrayAMDX [[RECORD]]
// CHECK-DAG: [[S10]] = OpConstantStringAMDX "OutputArray_1_0"
// CHECK-DAG: [[U31]] = OpConstant [[UINT]] 31
// CHECK-DAG: [[U129]] = OpConstant [[UINT]] 129
// CHECK-DAG: [[A11]] = OpTypeNodePayloadArrayAMDX [[RECORD]]
// CHECK-DAG: [[S11]] = OpConstantStringAMDX "OutputArray_1_1"
// CHECK-DAG: [[U37]] = OpConstant [[UINT]] 37
// CHECK-DAG: [[A12]] = OpTypeNodePayloadArrayAMDX [[RECORD]]
// CHECK-DAG: [[S12]] = OpConstantStringAMDX "Output_1_2"
// CHECK-DAG: [[U47]] = OpConstant [[UINT]] 47
// CHECK-DAG: [[EMPTY:%[^ ]*]] = OpTypeStruct
// CHECK-DAG: [[A20]] = OpTypeNodePayloadArrayAMDX [[EMPTY]]
// CHECK-DAG: [[S20]] = OpConstantStringAMDX "OutputArray_2_0"
// CHECK-DAG: [[U41]] = OpConstant [[UINT]] 41
// CHECK-DAG: [[U131]] = OpConstant [[UINT]] 131
// CHECK-DAG: [[A21]] = OpTypeNodePayloadArrayAMDX [[EMPTY]]
// CHECK-DAG: [[S21]] = OpConstantStringAMDX "OutputArray_2_1"
// CHECK-DAG: [[U43]] = OpConstant [[UINT]] 43
// CHECK-DAG: [[A22]] = OpTypeNodePayloadArrayAMDX [[EMPTY]]
// CHECK-DAG: [[S22]] = OpConstantStringAMDX "Output_2_2"
// CHECK-DAG: [[U53]] = OpConstant [[UINT]] 53

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_1_0(
    [AllowSparseNodes] [NodeArraySize(129)] [MaxRecords(31)]
    NodeOutputArray<RECORD1> OutputArray_1_0) {
  ThreadNodeOutputRecords<RECORD1> outRec = OutputArray_1_0[1].GetThreadNodeOutputRecords(2);
  outRec.OutputComplete();
}

// CHECK: [[NODE10]] = OpFunction %void None
// CHECK: OpFunctionEnd

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_1_1(
    [UnboundedSparseNodes] [MaxRecords(37)]
    NodeOutputArray<RECORD1> OutputArray_1_1) {
  ThreadNodeOutputRecords<RECORD1> outRec = OutputArray_1_1[1].GetThreadNodeOutputRecords(2);
  outRec.OutputComplete();
}

// CHECK: [[NODE11]] = OpFunction %void None
// CHECK: OpFunctionEnd

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_1_2(
    [AllowSparseNodes] [MaxRecords(47)]
    NodeOutput<RECORD1> Output_1_2) {
  ThreadNodeOutputRecords<RECORD1> outRec = Output_1_2.GetThreadNodeOutputRecords(2);
  outRec.OutputComplete();
}

// CHECK: [[NODE12]] = OpFunction %void None
// CHECK: %{{[^ ]*}} = OpAllocateNodePayloadsAMDX %{{[^ ]*}} %{{[^ ]*}} %{{[^ ]*}} [[U0]]
// CHECK: OpFunctionEnd

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_2_0(
    [AllowSparseNodes] [NodeArraySize(131)] [MaxRecords(41)]
    EmptyNodeOutputArray OutputArray_2_0) {
  OutputArray_2_0[1].GroupIncrementOutputCount(10);
}

// CHECK: [[NODE20]] = OpFunction %void None
// CHECK: OpFunctionEnd

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_2_1(
    [UnboundedSparseNodes] [MaxRecords(43)]
    EmptyNodeOutputArray OutputArray_2_1) {
  OutputArray_2_1[1].GroupIncrementOutputCount(10);
}

// CHECK: [[NODE21]] = OpFunction %void None
// CHECK: OpFunctionEnd

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_2_2(
    [AllowSparseNodes] [MaxRecords(53)]
    EmptyNodeOutput Output_2_2) {
  Output_2_2.GroupIncrementOutputCount(10);
}

// CHECK: [[NODE22]] = OpFunction %void None
// CHECK: %{{[^ ]*}} = OpAllocateNodePayloadsAMDX %{{[^ ]*}} %{{[^ ]*}} %{{[^ ]*}} [[U0]]
// CHECK: OpFunctionEnd
