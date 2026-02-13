// RUN: %dxc -spirv -fcgl -T ps_6_0 %s | FileCheck %s

struct S {
  uint u;
};

typedef vk::BufferPointer<S> BP;

struct PC {
  BP bp;
};

[[vk::push_constant]] PC pc;

// CHECK: [[UINT:%[_0-9A-Za-z]*]] = OpTypeInt 32 0
// CHECK: [[U0:%[_0-9A-Za-z]*]] = OpConstant [[UINT]] 0
// CHECK: [[INT:%[_0-9A-Za-z]*]] = OpTypeInt 32 1
// CHECK: [[I0:%[_0-9A-Za-z]*]] = OpConstant [[INT]] 0
// CHECK: [[S:%[_0-9A-Za-z]*]] = OpTypeStruct [[UINT]]
// CHECK: [[PS:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[S]]
// CHECK: [[PU:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[UINT]]
// CHECK: [[U1:%[_0-9A-Za-z]*]] = OpConstant [[UINT]] 1
// CHECK: [[PC:%[_0-9A-Za-z]*]] = OpVariable %{{[_0-9A-Za-z]*}} PushConstant

void main()
{
// CHECK: [[IN:%[_0-9A-Za-z]*]] = OpVariable
// CHECK: [[OUT:%[_0-9A-Za-z]*]] = OpVariable
  uint u0, u1;

// CHECK: [[X1:%[_0-9]+]] = OpAccessChain %{{[_0-9A-Za-z]*}} [[PC]] [[I0]]
// CHECK: [[X2:%[_0-9]+]] = OpLoad [[PS]] [[X1]]
// CHECK: [[X3:%[_0-9]+]] = OpAccessChain [[PU]] [[X2]] [[I0]]
// CHECK: [[X4:%[_0-9]+]] = OpLoad [[UINT]] [[IN]]
// CHECK: [[X5:%[_0-9]+]] = OpAtomicExchange [[UINT]] [[X3]] [[U1]] [[U0]] [[X4]]
// CHECK: OpStore [[OUT]] [[X5]]
  InterlockedExchange(pc.bp.Get().u, u0, u1);
}

