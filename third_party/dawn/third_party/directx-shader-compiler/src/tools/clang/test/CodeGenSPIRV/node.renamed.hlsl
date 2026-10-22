// RUN: %dxc -spirv -Od -T lib_6_8 -fspv-target-env=vulkan1.3 external %s | FileCheck %s

// Renamed node, unnamed index defaults to 0

struct RECORD {
  uint i;
};

[Shader("node")]
[NodeLaunch("thread")]
[NodeID("new_node_name")]
[NodeIsProgramEntry]
void node017_renamed_node([NodeID("output_node_name", 2)] NodeOutput<RECORD> r)
{
  ThreadNodeOutputRecords<RECORD> records = r.GetThreadNodeOutputRecords(1);
  records.OutputComplete();
}

// CHECK: OpEntryPoint GLCompute %{{[^ ]*}} "node017_renamed_node"
// CHECK-DAG: OpDecorateId [[TYPE:%[^ ]*]] PayloadNodeNameAMDX [[STR:%[0-9A-Za-z_]*]]
// CHECK-DAG: OpDecorateId [[TYPE]] PayloadNodeBaseIndexAMDX [[U2:%[0-9A-Za-z_]*]]
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[STR]] = OpConstantStringAMDX "output_node_name"
// CHECK-DAG: [[U0:%[_0-9A-Za-z]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[U1:%[_0-9A-Za-z]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[U2]] = OpConstant [[UINT]] 2
// CHECK-DAG: [[U4:%[_0-9A-Za-z]*]] = OpConstant [[UINT]] 4
// CHECK: OpAllocateNodePayloadsAMDX %{{[^ ]*}} [[U4]] [[U1]] [[U0]]
