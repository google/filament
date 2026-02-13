// RUN: %dxc -spirv -HV 202x -Od -T cs_6_9 %s | FileCheck %s --check-prefix=CHECK --check-prefix=NOFUN
// RUN: %dxc -spirv -HV 202x -Od -T cs_6_9 -DFUN %s | FileCheck %s --check-prefix=CHECK --check-prefix=FUN

// Issue #7302: implicit object argument of Get() evaluates to rvalue

template<class T, class U>
[[vk::ext_instruction(/*spv::OpBitcast*/124)]]
T bitcast(U);

struct Content
{
  int a;
};

// CHECK: [[INT:%[_0-9A-Za-z]*]] = OpTypeInt 32 1
// CHECK-DAG: [[I1:%[_0-9A-Za-z]*]] = OpConstant [[INT]] 1
// CHECK-DAG: [[IO:%[_0-9A-Za-z]*]] = OpConstant [[INT]] 0
// CHECK: [[UINT:%[_0-9A-Za-z]*]] = OpTypeInt 32 0
// CHECK-DAG: [[UDEADBEEF:%[_0-9A-Za-z]*]] = OpConstant [[UINT]] 3735928559
// CHECK-DAG: [[U0:%[_0-9A-Za-z]*]] = OpConstant [[UINT]] 0
// CHECK: [[V2UINT:%[_0-9A-Za-z]*]] = OpTypeVector [[UINT]] 2
// CHECK: [[VECTOR:%[_0-9A-Za-z]*]] = OpConstantComposite [[V2UINT]] [[UDEADBEEF]] [[U0]]
// CHECK: [[CONTENT:%[_0-9A-Za-z]*]] = OpTypeStruct [[INT]]
// FUN: [[PFCONTENT:%[_0-9A-Za-z]*]] = OpTypePointer Function [[CONTENT]]
// FUN: [[PFINT:%[_0-9A-Za-z]*]] = OpTypePointer Function [[INT]]
// FUN: [[CONTENT0:%[_0-9A-Za-z]*]] = OpTypeStruct [[INT]]
// FUN: [[PPCONTENT:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[CONTENT0]]
// NOFUN: [[PPCONTENT:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[CONTENT]]
// NOFUN: [[PPINT:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[INT]]

Content f() {
  return bitcast<vk::BufferPointer<Content> >(uint32_t2(0xdeadbeefu,0x0u)).Get();
}

[numthreads(1, 1, 1)]
void main()
{
#ifdef FUN
  Content c = f();
  c.a = 1;
#else
  bitcast<vk::BufferPointer<Content> >(uint32_t2(0xdeadbeefu,0x0u)).Get().a = 1;
#endif
}

// NOFUN: [[BITCAST:%[0-9]*]] = OpBitcast [[PPCONTENT]] [[VECTOR]]
// NOFUN: [[PTR:%[0-9]*]] = OpAccessChain [[PPINT]] [[BITCAST]] [[IO]]
// NOFUN: OpStore [[PTR]] [[I1]] Aligned 4

// FUN: [[VAR:%[_0-9A-Za-z]*]] = OpVariable [[PFCONTENT]] Function
// FUN: [[CALL:%[0-9]*]] = OpFunctionCall [[CONTENT]] [[F:%[_0-9A-Za-z]*]]
// FUN: OpStore [[VAR]] [[CALL]]
// FUN: [[PTR:%[0-9]*]] = OpAccessChain [[PFINT]] [[VAR]] [[IO]]
// FUN: OpStore [[PTR]] [[I1]]

// FUN: [[F]] = OpFunction [[CONTENT]]
// FUN: [[VAR:%[_0-9A-Za-z]*]] = OpVariable [[PFCONTENT]] Function
// FUN: [[BITCAST:%[0-9]*]] = OpBitcast [[PPCONTENT]] [[VECTOR]]
// FUN: [[CVAL0:%[0-9]*]] = OpLoad [[CONTENT0]] [[BITCAST]] Aligned 4
// FUN: [[IVAL:%[0-9]*]] = OpCompositeExtract [[INT]] [[CVAL0]] 0
// FUN: [[CVAL1:%[0-9]*]] = OpCompositeConstruct [[CONTENT]] [[IVAL]]
// FUN: OpStore [[VAR]] [[CVAL1]]
// FUN: [[RET:%[0-9]*]] = OpLoad [[CONTENT]] [[VAR]]
// FUN: OpReturnValue [[RET]]

