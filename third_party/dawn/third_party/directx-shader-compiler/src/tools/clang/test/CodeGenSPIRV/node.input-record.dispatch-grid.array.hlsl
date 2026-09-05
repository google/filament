// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// Check that SV_DispatchGrid supports array

struct RECORD
{
  uint a[3] : SV_DispatchGrid;
  uint b[3];
};

[Shader("node")]
[NodeLaunch("coalescing")]
[numthreads(4,4,4)]
void node01(RWGroupNodeInputRecords<RECORD> input)
{
  input.Get().a = input.Get().b;
}

// CHECK: OpName [[RECORD:%[^ ]*]] "RECORD"
// CHECK: OpMemberDecorate [[RECORD]] 0 PayloadDispatchIndirectAMDX
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK: [[U3:%[^ ]*]] = OpConstant %uint 3
// CHECK: [[ARRAY:%[^ ]*]] = OpTypeArray [[UINT]] [[U3]]
// CHECK: [[RECORD]] = OpTypeStruct [[ARRAY]] [[ARRAY]]
