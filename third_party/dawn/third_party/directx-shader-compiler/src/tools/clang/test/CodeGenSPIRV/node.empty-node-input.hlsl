// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// Coalescing launch node declares EmptyNodeInput

RWBuffer<uint> buf0;

[Shader("node")]
[NodeLaunch("coalescing")]
[NodeIsProgramEntry]
[NumThreads(2,1,1)]
void emptynodeinput(EmptyNodeInput input)
{
  // input.Count should always return 1 here, so there is
  // an opportunity for an optimization.
  buf0[0] = input.Count();
}

// CHECK-DAG: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[IMG:%[^ ]*]] = OpTypeImage [[UINT]] Buffer 2 0 0 2 R32ui
// CHECK-DAG: [[IMGPTR:%[^ ]*]] = OpTypePointer UniformConstant [[IMG]]
// CHECK-DAG: [[BUF:%[^ ]*]] = OpVariable [[IMGPTR]] UniformConstant

// CHECK: [[COUNT:%[^ ]*]] = OpNodePayloadArrayLengthAMDX [[UINT]]
// CHECK: [[IMAGE:%[^ ]*]] = OpLoad [[IMG]] [[BUF]]
// CHECK: OpImageWrite [[IMAGE]] [[U0]] [[COUNT]] None
