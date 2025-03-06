// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

RWByteAddressBuffer outBuffer;

struct S
{
    uint32_t x:8;
    uint32_t y:8;
};

[numthreads(1, 1, 1)]
void main() {
  uint addr = 0;
  uint words1 = 1;
  uint2 words2 = uint2(1, 2);
  uint3 words3 = uint3(1, 2, 3);
  uint4 words4 = uint4(1, 2, 3, 4);

// CHECK:      [[byteAddr1:%[0-9]+]] = OpLoad %uint %addr
// CHECK-NEXT: [[baseAddr1:%[0-9]+]] = OpShiftRightLogical %uint [[byteAddr1]] %uint_2
// CHECK-NEXT: [[words1:%[0-9]+]] = OpLoad %uint %words1
// CHECK-NEXT: [[out1_outBufPtr0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[baseAddr1]]
// CHECK-NEXT: OpStore [[out1_outBufPtr0]] [[words1]]
  outBuffer.Store(addr, words1);


// CHECK:      [[byteAddr2:%[0-9]+]] = OpLoad %uint %addr
// CHECK-NEXT: [[baseAddr2:%[0-9]+]] = OpShiftRightLogical %uint [[byteAddr2]] %uint_2
// CHECK-NEXT: [[words2:%[0-9]+]] = OpLoad %v2uint %words2
// CHECK-NEXT: [[words2_0:%[0-9]+]] = OpCompositeExtract %uint [[words2]] 0
// CHECK-NEXT: [[out2_outBufPtr0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[baseAddr2]]
// CHECK-NEXT: OpStore [[out2_outBufPtr0]] [[words2_0]]
// CHECK-NEXT: [[words2_1:%[0-9]+]] = OpCompositeExtract %uint [[words2]] 1
// CHECK-NEXT: [[baseAddr2_plus1:%[0-9]+]] = OpIAdd %uint [[baseAddr2]] %uint_1
// CHECK-NEXT: [[out2_outBufPtr1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[baseAddr2_plus1]]
// CHECK-NEXT: OpStore [[out2_outBufPtr1]] [[words2_1]]
  outBuffer.Store2(addr, words2);


// CHECK:      [[byteAddr3:%[0-9]+]] = OpLoad %uint %addr
// CHECK-NEXT: [[baseAddr3:%[0-9]+]] = OpShiftRightLogical %uint [[byteAddr3]] %uint_2
// CHECK-NEXT: [[words3:%[0-9]+]] = OpLoad %v3uint %words3
// CHECK-NEXT: [[word3_0:%[0-9]+]] = OpCompositeExtract %uint [[words3]] 0
// CHECK-NEXT: [[out3_outBufPtr0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[baseAddr3]]
// CHECK-NEXT: OpStore [[out3_outBufPtr0]] [[word3_0]]
// CHECK-NEXT: [[words3_1:%[0-9]+]] = OpCompositeExtract %uint [[words3]] 1
// CHECK-NEXT: [[baseAddr3_plus1:%[0-9]+]] = OpIAdd %uint [[baseAddr3]] %uint_1
// CHECK-NEXT: [[out3_outBufPtr1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[baseAddr3_plus1]]
// CHECK-NEXT: OpStore [[out3_outBufPtr1]] [[words3_1]]
// CHECK-NEXT: [[word3_2:%[0-9]+]] = OpCompositeExtract %uint [[words3]] 2
// CHECK-NEXT: [[baseAddr3_plus2:%[0-9]+]] = OpIAdd %uint [[baseAddr3]] %uint_2
// CHECK-NEXT: [[out3_outBufPtr2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[baseAddr3_plus2]]
// CHECK-NEXT: OpStore [[out3_outBufPtr2]] [[word3_2]]
  outBuffer.Store3(addr, words3);


// CHECK:      [[byteAddr:%[0-9]+]] = OpLoad %uint %addr
// CHECK-NEXT: [[baseAddr:%[0-9]+]] = OpShiftRightLogical %uint [[byteAddr]] %uint_2
// CHECK-NEXT: [[words4:%[0-9]+]] = OpLoad %v4uint %words4
// CHECK-NEXT: [[word0:%[0-9]+]] = OpCompositeExtract %uint [[words4]] 0
// CHECK-NEXT: [[outBufPtr0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[baseAddr]]
// CHECK-NEXT: OpStore [[outBufPtr0]] [[word0]]
// CHECK-NEXT: [[word1:%[0-9]+]] = OpCompositeExtract %uint [[words4]] 1
// CHECK-NEXT: [[baseAddr_plus1:%[0-9]+]] = OpIAdd %uint [[baseAddr]] %uint_1
// CHECK-NEXT: [[outBufPtr1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[baseAddr_plus1]]
// CHECK-NEXT: OpStore [[outBufPtr1]] [[word1]]
// CHECK-NEXT: [[word2:%[0-9]+]] = OpCompositeExtract %uint [[words4]] 2
// CHECK-NEXT: [[baseAddr_plus2:%[0-9]+]] = OpIAdd %uint [[baseAddr]] %uint_2
// CHECK-NEXT: [[outBufPtr2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[baseAddr_plus2]]
// CHECK-NEXT: OpStore [[outBufPtr2]] [[word2]]
// CHECK-NEXT: [[word3:%[0-9]+]] = OpCompositeExtract %uint [[words4]] 3
// CHECK-NEXT: [[baseAddr_plus3:%[0-9]+]] = OpIAdd %uint [[baseAddr]] %uint_3
// CHECK-NEXT: [[outBufPtr3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[baseAddr_plus3]]
// CHECK-NEXT: OpStore [[outBufPtr3]] [[word3]]
  outBuffer.Store4(addr, words4);

// CHECK: [[s:%[0-9]+]] = OpLoad %S %s
// CHECK: [[bitfield:%[0-9]+]] = OpCompositeExtract %uint [[s]] 0
// CHECK: [[idx:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outBuffer %uint_0 [[idx]]
// CHECK: OpStore [[ac]] [[bitfield]]
  S s  = (S)0;
  s.x = 5;
  outBuffer.Store(0, s);
}
