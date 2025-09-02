// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// Check that SV_DispatchGrid in nested struct is recognized

struct INNER {
  uint c;
  uint3 grid : SV_DispatchGrid;
};

struct RECORD
{
  uint a;
  INNER b;
};

[Shader("node")]
[NodeLaunch("coalescing")]
[numthreads(4,4,4)]
void node01(RWGroupNodeInputRecords<RECORD> input)
{
  input.Get().a = input.Get().b.grid.x;
}

// CHECK: OpName [[RECORD:%[^ ]*]] "RECORD"
// CHECK: OpName [[INNER:%[^ ]*]] "INNER"
// CHECK: OpMemberDecorate [[INNER]] 1 PayloadDispatchIndirectAMDX
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK: [[VECTOR:%[^ ]*]] = OpTypeVector %uint 3
// CHECK: [[INNER]] = OpTypeStruct [[UINT]] [[VECTOR]]
// CHECK: [[RECORD]] = OpTypeStruct [[UINT]] [[INNER]]
