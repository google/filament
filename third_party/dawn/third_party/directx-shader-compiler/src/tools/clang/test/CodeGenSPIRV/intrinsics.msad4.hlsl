// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

uint4 main(uint reference : REF, uint2 source :SOURCE, uint4 accum : ACCUM) : MSAD_RESULT
{

// CHECK:          [[ref:%[0-9]+]] = OpLoad %uint %reference
// CHECK-NEXT:     [[src:%[0-9]+]] = OpLoad %v2uint %source
// CHECK-NEXT:   [[accum:%[0-9]+]] = OpLoad %v4uint %accum
// CHECK-NEXT:    [[src0:%[0-9]+]] = OpCompositeExtract %uint [[src]] 0
// CHECK-NEXT:  [[src0s8:%[0-9]+]] = OpShiftLeftLogical %uint [[src0]] %uint_8
// CHECK-NEXT: [[src0s16:%[0-9]+]] = OpShiftLeftLogical %uint [[src0]] %uint_16
// CHECK-NEXT: [[src0s24:%[0-9]+]] = OpShiftLeftLogical %uint [[src0]] %uint_24
// CHECK-NEXT:    [[src1:%[0-9]+]] = OpCompositeExtract %uint [[src]] 1
// CHECK-NEXT:    [[bfi0:%[0-9]+]] = OpBitFieldInsert %uint [[src0s8]] [[src1]] %uint_24 %uint_8
// CHECK-NEXT:    [[bfi1:%[0-9]+]] = OpBitFieldInsert %uint [[src0s16]] [[src1]] %uint_16 %uint_16
// CHECK-NEXT:    [[bfi2:%[0-9]+]] = OpBitFieldInsert %uint [[src0s24]] [[src1]] %uint_8 %uint_24
// CHECK-NEXT:  [[accum0:%[0-9]+]] = OpCompositeExtract %uint [[accum]] 0
// CHECK-NEXT:  [[accum1:%[0-9]+]] = OpCompositeExtract %uint [[accum]] 1
// CHECK-NEXT:  [[accum2:%[0-9]+]] = OpCompositeExtract %uint [[accum]] 2
// CHECK-NEXT:  [[accum3:%[0-9]+]] = OpCompositeExtract %uint [[accum]] 3

// Now perforoming MSAD four times

// CHECK-NEXT:           [[refByte0:%[0-9]+]] = OpBitFieldUExtract %uint [[ref]] %uint_0 %uint_8
// CHECK-NEXT:        [[intRefByte0:%[0-9]+]] = OpBitcast %int [[refByte0]]
// CHECK-NEXT:     [[isRefByte0Zero:%[0-9]+]] = OpIEqual %bool [[refByte0]] %uint_0
// CHECK-NEXT:           [[refByte1:%[0-9]+]] = OpBitFieldUExtract %uint [[ref]] %uint_8 %uint_8
// CHECK-NEXT:        [[intRefByte1:%[0-9]+]] = OpBitcast %int [[refByte1]]
// CHECK-NEXT:     [[isRefByte1Zero:%[0-9]+]] = OpIEqual %bool [[refByte1]] %uint_0
// CHECK-NEXT:           [[refByte2:%[0-9]+]] = OpBitFieldUExtract %uint [[ref]] %uint_16 %uint_8
// CHECK-NEXT:        [[intRefByte2:%[0-9]+]] = OpBitcast %int [[refByte2]]
// CHECK-NEXT:     [[isRefByte2Zero:%[0-9]+]] = OpIEqual %bool [[refByte2]] %uint_0
// CHECK-NEXT:           [[refByte3:%[0-9]+]] = OpBitFieldUExtract %uint [[ref]] %uint_24 %uint_8
// CHECK-NEXT:        [[intRefByte3:%[0-9]+]] = OpBitcast %int [[refByte3]]
// CHECK-NEXT:     [[isRefByte3Zero:%[0-9]+]] = OpIEqual %bool [[refByte3]] %uint_0

// MSAD 0 Byte 0
// CHECK-NEXT:          [[src0Byte0:%[0-9]+]] = OpBitFieldUExtract %uint [[src0]] %uint_0 %uint_8
// CHECK-NEXT:       [[intSrc0Byte0:%[0-9]+]] = OpBitcast %int [[src0Byte0]]
// CHECK-NEXT:               [[sub0:%[0-9]+]] = OpISub %int [[intRefByte0]] [[intSrc0Byte0]]
// CHECK-NEXT:            [[absSub0:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub0]]
// CHECK-NEXT:        [[uintAbsSub0:%[0-9]+]] = OpBitcast %uint [[absSub0]]
// CHECK-NEXT:              [[diff0:%[0-9]+]] = OpSelect %uint [[isRefByte0Zero]] %uint_0 [[uintAbsSub0]]
// CHECK-NEXT:    [[accum0PlusDiff0:%[0-9]+]] = OpIAdd %uint [[accum0]] [[diff0]]

// MSAD 0 Byte 1
// CHECK-NEXT:          [[src0Byte1:%[0-9]+]] = OpBitFieldUExtract %uint [[src0]] %uint_8 %uint_8
// CHECK-NEXT:       [[intSrc0Byte1:%[0-9]+]] = OpBitcast %int [[src0Byte1]]
// CHECK-NEXT:               [[sub1:%[0-9]+]] = OpISub %int [[intRefByte1]] [[intSrc0Byte1]]
// CHECK-NEXT:            [[absSub1:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub1]]
// CHECK-NEXT:        [[uintAbsSub1:%[0-9]+]] = OpBitcast %uint [[absSub1]]
// CHECK-NEXT:              [[diff1:%[0-9]+]] = OpSelect %uint [[isRefByte1Zero]] %uint_0 [[uintAbsSub1]]
// CHECK-NEXT:   [[accum0PlusDiff01:%[0-9]+]] = OpIAdd %uint [[accum0PlusDiff0]] [[diff1]]

// MSAD 0 Byte 2
// CHECK-NEXT:          [[src0Byte2:%[0-9]+]] = OpBitFieldUExtract %uint [[src0]] %uint_16 %uint_8
// CHECK-NEXT:       [[intSrc0Byte2:%[0-9]+]] = OpBitcast %int [[src0Byte2]]
// CHECK-NEXT:               [[sub2:%[0-9]+]] = OpISub %int [[intRefByte2]] [[intSrc0Byte2]]
// CHECK-NEXT:            [[absSub2:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub2]]
// CHECK-NEXT:        [[uintAbsSub2:%[0-9]+]] = OpBitcast %uint [[absSub2]]
// CHECK-NEXT:              [[diff2:%[0-9]+]] = OpSelect %uint [[isRefByte2Zero]] %uint_0 [[uintAbsSub2]]
// CHECK-NEXT:  [[accum0PlusDiff012:%[0-9]+]] = OpIAdd %uint [[accum0PlusDiff01]] [[diff2]]

// MSAD 0 Byte 3
// CHECK-NEXT:          [[src0Byte3:%[0-9]+]] = OpBitFieldUExtract %uint [[src0]] %uint_24 %uint_8
// CHECK-NEXT:       [[intSrc0Byte3:%[0-9]+]] = OpBitcast %int [[src0Byte3]]
// CHECK-NEXT:               [[sub3:%[0-9]+]] = OpISub %int [[intRefByte3]] [[intSrc0Byte3]]
// CHECK-NEXT:            [[absSub3:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub3]]
// CHECK-NEXT:        [[uintAbsSub3:%[0-9]+]] = OpBitcast %uint [[absSub3]]
// CHECK-NEXT:              [[diff3:%[0-9]+]] = OpSelect %uint [[isRefByte3Zero]] %uint_0 [[uintAbsSub3]]
// CHECK-NEXT: [[accum0PlusDiff0123:%[0-9]+]] = OpIAdd %uint [[accum0PlusDiff012]] [[diff3]]


// MSAD 1 Byte 0
// CHECK-NEXT:          [[src1Byte0:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi0]] %uint_0 %uint_8
// CHECK-NEXT:       [[intSrc1Byte0:%[0-9]+]] = OpBitcast %int [[src1Byte0]]
// CHECK-NEXT:               [[sub0_0:%[0-9]+]] = OpISub %int [[intRefByte0]] [[intSrc1Byte0]]
// CHECK-NEXT:            [[absSub0_0:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub0_0]]
// CHECK-NEXT:        [[uintAbsSub0_0:%[0-9]+]] = OpBitcast %uint [[absSub0_0]]
// CHECK-NEXT:              [[diff0_0:%[0-9]+]] = OpSelect %uint [[isRefByte0Zero]] %uint_0 [[uintAbsSub0_0]]
// CHECK-NEXT:    [[accum1PlusDiff0:%[0-9]+]] = OpIAdd %uint [[accum1]] [[diff0_0]]

// MSAD 1 Byte 1
// CHECK-NEXT:          [[src1Byte1:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi0]] %uint_8 %uint_8
// CHECK-NEXT:       [[intSrc1Byte1:%[0-9]+]] = OpBitcast %int [[src1Byte1]]
// CHECK-NEXT:               [[sub1_0:%[0-9]+]] = OpISub %int [[intRefByte1]] [[intSrc1Byte1]]
// CHECK-NEXT:            [[absSub1_0:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub1_0]]
// CHECK-NEXT:        [[uintAbsSub1_0:%[0-9]+]] = OpBitcast %uint [[absSub1_0]]
// CHECK-NEXT:              [[diff1_0:%[0-9]+]] = OpSelect %uint [[isRefByte1Zero]] %uint_0 [[uintAbsSub1_0]]
// CHECK-NEXT:   [[accum1PlusDiff01:%[0-9]+]] = OpIAdd %uint [[accum1PlusDiff0]] [[diff1_0]]

// MSAD 1 Byte 2
// CHECK-NEXT:          [[src1Byte2:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi0]] %uint_16 %uint_8
// CHECK-NEXT:       [[intSrc1Byte2:%[0-9]+]] = OpBitcast %int [[src1Byte2]]
// CHECK-NEXT:               [[sub2_0:%[0-9]+]] = OpISub %int [[intRefByte2]] [[intSrc1Byte2]]
// CHECK-NEXT:            [[absSub2_0:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub2_0]]
// CHECK-NEXT:        [[uintAbsSub2_0:%[0-9]+]] = OpBitcast %uint [[absSub2_0]]
// CHECK-NEXT:              [[diff2_0:%[0-9]+]] = OpSelect %uint [[isRefByte2Zero]] %uint_0 [[uintAbsSub2_0]]
// CHECK-NEXT:  [[accum1PlusDiff012:%[0-9]+]] = OpIAdd %uint [[accum1PlusDiff01]] [[diff2_0]]

// MSAD 1 Byte 3
// CHECK-NEXT:          [[src1Byte3:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi0]] %uint_24 %uint_8
// CHECK-NEXT:       [[intSrc1Byte3:%[0-9]+]] = OpBitcast %int [[src1Byte3]]
// CHECK-NEXT:               [[sub3_0:%[0-9]+]] = OpISub %int [[intRefByte3]] [[intSrc1Byte3]]
// CHECK-NEXT:            [[absSub3_0:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub3_0]]
// CHECK-NEXT:        [[uintAbsSub3_0:%[0-9]+]] = OpBitcast %uint [[absSub3_0]]
// CHECK-NEXT:              [[diff3_0:%[0-9]+]] = OpSelect %uint [[isRefByte3Zero]] %uint_0 [[uintAbsSub3_0]]
// CHECK-NEXT: [[accum1PlusDiff0123:%[0-9]+]] = OpIAdd %uint [[accum1PlusDiff012]] [[diff3_0]]


// MSAD 2 Byte 0
// CHECK-NEXT:          [[src2Byte0:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi1]] %uint_0 %uint_8
// CHECK-NEXT:       [[intSrc2Byte0:%[0-9]+]] = OpBitcast %int [[src2Byte0]]
// CHECK-NEXT:               [[sub0_1:%[0-9]+]] = OpISub %int [[intRefByte0]] [[intSrc2Byte0]]
// CHECK-NEXT:            [[absSub0_1:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub0_1]]
// CHECK-NEXT:        [[uintAbsSub0_1:%[0-9]+]] = OpBitcast %uint [[absSub0_1]]
// CHECK-NEXT:              [[diff0_1:%[0-9]+]] = OpSelect %uint [[isRefByte0Zero]] %uint_0 [[uintAbsSub0_1]]
// CHECK-NEXT:    [[accum2PlusDiff0:%[0-9]+]] = OpIAdd %uint [[accum2]] [[diff0_1]]

// MSAD 2 Byte 1
// CHECK-NEXT:          [[src2Byte1:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi1]] %uint_8 %uint_8
// CHECK-NEXT:       [[intSrc2Byte1:%[0-9]+]] = OpBitcast %int [[src2Byte1]]
// CHECK-NEXT:               [[sub1_1:%[0-9]+]] = OpISub %int [[intRefByte1]] [[intSrc2Byte1]]
// CHECK-NEXT:            [[absSub1_1:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub1_1]]
// CHECK-NEXT:        [[uintAbsSub1_1:%[0-9]+]] = OpBitcast %uint [[absSub1_1]]
// CHECK-NEXT:              [[diff1_1:%[0-9]+]] = OpSelect %uint [[isRefByte1Zero]] %uint_0 [[uintAbsSub1_1]]
// CHECK-NEXT:   [[accum2PlusDiff01:%[0-9]+]] = OpIAdd %uint [[accum2PlusDiff0]] [[diff1_1]]

// MSAD 2 Byte 2
// CHECK-NEXT:          [[src2Byte2:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi1]] %uint_16 %uint_8
// CHECK-NEXT:       [[intSrc2Byte2:%[0-9]+]] = OpBitcast %int [[src2Byte2]]
// CHECK-NEXT:               [[sub2_1:%[0-9]+]] = OpISub %int [[intRefByte2]] [[intSrc2Byte2]]
// CHECK-NEXT:            [[absSub2_1:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub2_1]]
// CHECK-NEXT:        [[uintAbsSub2_1:%[0-9]+]] = OpBitcast %uint [[absSub2_1]]
// CHECK-NEXT:              [[diff2_1:%[0-9]+]] = OpSelect %uint [[isRefByte2Zero]] %uint_0 [[uintAbsSub2_1]]
// CHECK-NEXT:  [[accum2PlusDiff012:%[0-9]+]] = OpIAdd %uint [[accum2PlusDiff01]] [[diff2_1]]

// MSAD 2 Byte 3
// CHECK-NEXT:          [[src2Byte3:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi1]] %uint_24 %uint_8
// CHECK-NEXT:       [[intSrc2Byte3:%[0-9]+]] = OpBitcast %int [[src2Byte3]]
// CHECK-NEXT:               [[sub3_1:%[0-9]+]] = OpISub %int [[intRefByte3]] [[intSrc2Byte3]]
// CHECK-NEXT:            [[absSub3_1:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub3_1]]
// CHECK-NEXT:        [[uintAbsSub3_1:%[0-9]+]] = OpBitcast %uint [[absSub3_1]]
// CHECK-NEXT:              [[diff3_1:%[0-9]+]] = OpSelect %uint [[isRefByte3Zero]] %uint_0 [[uintAbsSub3_1]]
// CHECK-NEXT: [[accum2PlusDiff0123:%[0-9]+]] = OpIAdd %uint [[accum2PlusDiff012]] [[diff3_1]]


// MSAD 3 Byte 0
// CHECK-NEXT:          [[src3Byte0:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi2]] %uint_0 %uint_8
// CHECK-NEXT:       [[intSrc3Byte0:%[0-9]+]] = OpBitcast %int [[src3Byte0]]
// CHECK-NEXT:               [[sub0_2:%[0-9]+]] = OpISub %int [[intRefByte0]] [[intSrc3Byte0]]
// CHECK-NEXT:            [[absSub0_2:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub0_2]]
// CHECK-NEXT:        [[uintAbsSub0_2:%[0-9]+]] = OpBitcast %uint [[absSub0_2]]
// CHECK-NEXT:              [[diff0_2:%[0-9]+]] = OpSelect %uint [[isRefByte0Zero]] %uint_0 [[uintAbsSub0_2]]
// CHECK-NEXT:    [[accum3PlusDiff0:%[0-9]+]] = OpIAdd %uint [[accum3]] [[diff0_2]]

// MSAD 3 Byte 1
// CHECK-NEXT:          [[src3Byte1:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi2]] %uint_8 %uint_8
// CHECK-NEXT:       [[intSrc3Byte1:%[0-9]+]] = OpBitcast %int [[src3Byte1]]
// CHECK-NEXT:               [[sub1_2:%[0-9]+]] = OpISub %int [[intRefByte1]] [[intSrc3Byte1]]
// CHECK-NEXT:            [[absSub1_2:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub1_2]]
// CHECK-NEXT:        [[uintAbsSub1_2:%[0-9]+]] = OpBitcast %uint [[absSub1_2]]
// CHECK-NEXT:              [[diff1_2:%[0-9]+]] = OpSelect %uint [[isRefByte1Zero]] %uint_0 [[uintAbsSub1_2]]
// CHECK-NEXT:   [[accum3PlusDiff01:%[0-9]+]] = OpIAdd %uint [[accum3PlusDiff0]] [[diff1_2]]

// MSAD 3 Byte 2
// CHECK-NEXT:          [[src3Byte2:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi2]] %uint_16 %uint_8
// CHECK-NEXT:       [[intSrc3Byte2:%[0-9]+]] = OpBitcast %int [[src3Byte2]]
// CHECK-NEXT:               [[sub2_2:%[0-9]+]] = OpISub %int [[intRefByte2]] [[intSrc3Byte2]]
// CHECK-NEXT:            [[absSub2_2:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub2_2]]
// CHECK-NEXT:        [[uintAbsSub2_2:%[0-9]+]] = OpBitcast %uint [[absSub2_2]]
// CHECK-NEXT:              [[diff2_2:%[0-9]+]] = OpSelect %uint [[isRefByte2Zero]] %uint_0 [[uintAbsSub2_2]]
// CHECK-NEXT:  [[accum3PlusDiff012:%[0-9]+]] = OpIAdd %uint [[accum3PlusDiff01]] [[diff2_2]]

// MSAD 3 Byte 3
// CHECK-NEXT:          [[src3Byte3:%[0-9]+]] = OpBitFieldUExtract %uint [[bfi2]] %uint_24 %uint_8
// CHECK-NEXT:       [[intSrc3Byte3:%[0-9]+]] = OpBitcast %int [[src3Byte3]]
// CHECK-NEXT:               [[sub3_2:%[0-9]+]] = OpISub %int [[intRefByte3]] [[intSrc3Byte3]]
// CHECK-NEXT:            [[absSub3_2:%[0-9]+]] = OpExtInst %int [[glsl]] SAbs [[sub3_2]]
// CHECK-NEXT:        [[uintAbsSub3_2:%[0-9]+]] = OpBitcast %uint [[absSub3_2]]
// CHECK-NEXT:              [[diff3_2:%[0-9]+]] = OpSelect %uint [[isRefByte3Zero]] %uint_0 [[uintAbsSub3_2]]
// CHECK-NEXT: [[accum3PlusDiff0123:%[0-9]+]] = OpIAdd %uint [[accum3PlusDiff012]] [[diff3_2]]

// CHECK-NEXT: {{%[0-9]+}} = OpCompositeConstruct %v4uint [[accum0PlusDiff0123]] [[accum1PlusDiff0123]] [[accum2PlusDiff0123]] [[accum3PlusDiff0123]]

  uint4 result = msad4(reference, source, accum);
  return result;
}
