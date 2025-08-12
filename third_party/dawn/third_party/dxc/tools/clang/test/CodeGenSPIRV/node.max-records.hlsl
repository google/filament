// RUN: %dxc -spirv -T lib_6_8  -fspv-target-env=vulkan1.3 %s | FileCheck %s

// Test referencing params with MaxOutputRecordsSharedWith

struct rec0
{
    int i0;
    float f0;
};

struct rec1
{
    float f1;
    int i1;
};

[Shader("node")]
[NodeLaunch("thread")]
void BackwardRef(
  RWThreadNodeInputRecord<rec0> InputyMcInputFace,
  [MaxRecords(5)] NodeOutput<rec1> Output1,
  [MaxRecordsSharedWith(Output1)] NodeOutput<rec1> Output2)
{
}

// CHECK: OpDecorateId [[TYPE1:%[^ ]*]] PayloadNodeNameAMDX [[STR1:%[^ ]*]]
// CHECK: OpDecorateId [[TYPE1]] NodeMaxPayloadsAMDX [[U5:%[^ ]*]]
// CHECK: OpDecorateId [[TYPE2:%[^ ]*]] PayloadNodeNameAMDX [[STR2:%[^ ]*]]
// CHECK: OpDecorateId [[TYPE2]] NodeSharesPayloadLimitsWithAMDX [[TYPE1]]
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U5]] = OpConstant [[UINT]] 5
// CHECK-DAG: [[STR1]] = OpConstantStringAMDX "Output1"
// CHECK-DAG: [[STR2]] = OpConstantStringAMDX "Output2"

#if 0
// copied from DXIL test but doesn't seem to conform to spec
[Shader("node")]
[NodeLaunch("thread")]
void ForwardRef(
  RWThreadNodeInputRecord<rec0> InputyMcInputFace,
  [MaxRecordsSharedWith(Output2)] NodeOutput<rec1> Output1,
  [MaxRecords(5)] NodeOutput<rec1> Output2)
{
}
#endif
