// RUN: %dxc -T ps_6_2 -E main -fcgl -enable-16bit-types %s -spirv | FileCheck %s

RWStructuredBuffer<half> buffer;
RWStructuredBuffer<half2x3> buffer_mat;
RWByteAddressBuffer byte_buffer;

// Since OpIsNOrmal needs the Kernel capability, translation is done by checking
// that the exponent bits are neither all 1s nor all 0s.
// isNormal = !(isNan || isInf || Zero || Subnormal)

void main() {
  half    a;
  half4   b;
  half2x3 c;

// 31744 = 0x7C00
// CHECK: [[Mask:%.*]] = OpConstant %ushort 31744
// CHECK: [[Zero:%.*]] = OpConstant %ushort 0
// CHECK: [[Mask4:%.*]] = OpConstantComposite %v4ushort [[Mask]] [[Mask]] [[Mask]] [[Mask]]
// CHECK: [[Zero4:%.*]] = OpConstantComposite %v4ushort [[Zero]] [[Zero]] [[Zero]] [[Zero]]
// CHECK: [[Mask3:%.*]] = OpConstantComposite %v3ushort [[Mask]] [[Mask]] [[Mask]]
// CHECK: [[Zero3:%.*]] = OpConstantComposite %v3ushort [[Zero]] [[Zero]] [[Zero]]


// CHECK: [[A:%[0-9]+]] = OpLoad %half %a
// CHECK-NEXT: [[A_cast:%[0-9]+]] = OpBitcast %ushort [[A]]
// CHECK-NEXT: [[A_and:%[0-9]+]] = OpBitwiseAnd %ushort [[A_cast]] [[Mask]]
// CHECK-NEXT: [[A_not1:%[0-9]+]] = OpINotEqual %bool [[A_and]] [[Zero]]
// CHECK-NEXT: [[A_not2:%[0-9]+]] = OpINotEqual %bool [[A_and]] [[Mask]]
// CHECK-NEXT: OpLogicalAnd %bool [[A_not1]] [[A_not2]]
  bool    isn_a = isnormal(a);

// CHECK: [[B:%[0-9]+]] = OpLoad %v4half %b
// CHECK-NEXT: [[B_cast:%[0-9]+]] = OpBitcast %v4ushort [[B]]
// CHECK-NEXT: [[B_and:%[0-9]+]] = OpBitwiseAnd %v4ushort [[B_cast]] [[Mask4]]
// CHECK-NEXT: [[B_not1:%[0-9]+]] = OpINotEqual %v4bool [[B_and]] [[Zero4]]
// CHECK-NEXT: [[B_not2:%[0-9]+]] = OpINotEqual %v4bool [[B_and]] [[Mask4]]
// CHECK-NEXT: OpLogicalAnd %v4bool [[B_not1]] [[B_not2]]
  bool4   isn_b = isnormal(b);

// CHECK: [[Ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_half %buffer %int_0 %uint_0
// CHECK-NEXT: [[Tmp:%[0-9]+]] = OpLoad %half [[Ptr]]
// CHECK-NEXT: [[Cast:%[0-9]+]] = OpBitcast %ushort [[Tmp]]
// CHECK-NEXT: [[And:%[0-9]+]] = OpBitwiseAnd %ushort [[Cast]] [[Mask]]
// CHECK-NEXT: [[Not1:%[0-9]+]] = OpINotEqual %bool [[And]] [[Zero]]
// CHECK-NEXT: [[Not2:%[0-9]+]] = OpINotEqual %bool [[And]] [[Mask]]
// CHECK-NEXT: [[And2:%[0-9]+]] = OpLogicalAnd %bool [[Not1]] [[Not2]]
// CHECK-NEXT: OpStore %res [[And2]]
// CHECK-NEXT: [[Tmp2:%[0-9]+]] = OpLoad %bool %res
// CHECK-NEXT: [[Sel:%[0-9]+]] = OpSelect %half [[Tmp2]] %half_0x1p_0 %half_0x0p_0
// CHECK-NEXT: [[Ptr2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_half %buffer %int_0 %uint_0
// CHECK-NEXT: OpStore [[Ptr2]] [[Sel]]
  bool res = isnormal(buffer[0]);
  buffer[0] = (half)res;

// CHECK: [[C:%[0-9]+]] = OpLoad %mat2v3half %c
// CHECK-NEXT: [[R0:%[0-9]+]] = OpCompositeExtract %v3half [[C]] 0
// CHECK-NEXT: [[R0_cast:%[0-9]+]] = OpBitcast %v3ushort [[R0]]
// CHECK-NEXT: [[R0_and:%[0-9]+]] = OpBitwiseAnd %v3ushort [[R0_cast]] [[Mask3]]
// CHECK-NEXT: [[R0_Not1:%[0-9]+]] = OpINotEqual %v3bool [[R0_and]] [[Zero3]]
// CHECK-NEXT: [[R0_Not2:%[0-9]+]] = OpINotEqual %v3bool [[R0_and]] [[Mask3]]
// CHECK-NEXT: [[R0_And2:%[0-9]+]] = OpLogicalAnd %v3bool [[R0_Not1]] [[R0_Not2]]
// CHECK-NEXT: [[R1:%[0-9]+]] = OpCompositeExtract %v3half [[C]] 1
// CHECK-NEXT: [[R1_cast:%[0-9]+]] = OpBitcast %v3ushort [[R1]]
// CHECK-NEXT: [[R1_and:%[0-9]+]] = OpBitwiseAnd %v3ushort [[R1_cast]] [[Mask3]]
// CHECK-NEXT: [[R1_not1:%[0-9]+]] = OpINotEqual %v3bool [[R1_and]] [[Zero3]]
// CHECK-NEXT: [[R1_not2:%[0-9]+]] = OpINotEqual %v3bool [[R1_and]] [[Mask3]]
// CHECK-NEXT: [[R1_and2:%[0-9]+]] = OpLogicalAnd %v3bool [[R1_not1]] [[R1_not2]]
// CHECK-NEXT: [[Tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[R0_And2]] [[R1_and2]]
// CHECK-NEXT: OpStore %isnormal_c [[Tmp]]
  bool2x3 isnormal_c = isnormal(c);

// CHECK: [[Ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat2v3half %buffer_mat %int_0 %uint_0
// CHECK: [[Tmp:%[0-9]+]] = OpLoad %mat2v3half [[Ptr]]
// CHECK: [[R0:%[0-9]+]] = OpCompositeExtract %v3half [[Tmp]] 0
// CHECK-NEXT: [[R0_cast:%[0-9]+]] = OpBitcast %v3ushort [[R0]]
// CHECK-NEXT: [[R0_and:%[0-9]+]] = OpBitwiseAnd %v3ushort [[R0_cast]] [[Mask3]]
// CHECK-NEXT: [[R0_Not1:%[0-9]+]] = OpINotEqual %v3bool [[R0_and]] [[Zero3]]
// CHECK-NEXT: [[R0_Not2:%[0-9]+]] = OpINotEqual %v3bool [[R0_and]] [[Mask3]]
// CHECK-NEXT: [[R0_And2:%[0-9]+]] = OpLogicalAnd %v3bool [[R0_Not1]] [[R0_Not2]]
// CHECK-NEXT: [[R1:%[0-9]+]] = OpCompositeExtract %v3half [[Tmp]] 1
// CHECK-NEXT: [[R1_cast:%[0-9]+]] = OpBitcast %v3ushort [[R1]]
// CHECK-NEXT: [[R1_and:%[0-9]+]] = OpBitwiseAnd %v3ushort [[R1_cast]] [[Mask3]]
// CHECK-NEXT: [[R1_not1:%[0-9]+]] = OpINotEqual %v3bool [[R1_and]] [[Zero3]]
// CHECK-NEXT: [[R1_not2:%[0-9]+]] = OpINotEqual %v3bool [[R1_and]] [[Mask3]]
// CHECK-NEXT: [[R1_and2:%[0-9]+]] = OpLogicalAnd %v3bool [[R1_not1]] [[R1_not2]]
// CHECK-NEXT: [[Tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[R0_And2]] [[R1_and2]]
// CHECK-NEXT: OpStore %isnormal_d [[Tmp]]
  bool2x3 isnormal_d = isnormal(buffer_mat[0]);

// CHECK: [[Addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK: [[Ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[Addr]]
// CHECK-NEXT: [[Tmp:%[0-9]+]] = OpLoad %uint [[Ptr]]
// CHECK-NEXT: [[Tmp2:%[0-9]+]] = OpShiftRightLogical %uint [[Tmp]] %117
// CHECK-NEXT: [[Tmp3:%[0-9]+]] = OpUConvert %ushort [[Tmp2]]
// CHECK-NEXT: [[Tmp_cast:%[0-9]+]] = OpBitcast %half [[Tmp3]]
// CHECK: [[Cast:%[0-9]+]] = OpBitcast %ushort [[Tmp_cast]]
// CHECK-NEXT: [[And:%[0-9]+]] = OpBitwiseAnd %ushort [[Cast]] [[Mask]]
// CHECK-NEXT: [[Not1:%[0-9]+]] = OpINotEqual %bool [[And]] [[Zero]]
// CHECK-NEXT: [[Not2:%[0-9]+]] = OpINotEqual %bool [[And]] [[Mask]]
// CHECK-NEXT: [[And2:%[0-9]+]] = OpLogicalAnd %bool [[Not1]] [[Not2]]
// CHECK-NEXT: OpStore %isnormal_e [[And2]]
  bool isnormal_e = isnormal(byte_buffer.Load<half>(0));

// CHECK: [[Res:%[0-9]+]] = OpLoad %bool %isnormal_e
// CHECK: [[Addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK: [[Ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[Addr]]
// CHECK: [[Tmp:%[0-9]+]] = OpSelect %uint [[Res]] %uint_1 %uint_0
// CHECK: OpStore [[Ptr]] [[Tmp]]
  byte_buffer.Store(0, isnormal_e);
}
