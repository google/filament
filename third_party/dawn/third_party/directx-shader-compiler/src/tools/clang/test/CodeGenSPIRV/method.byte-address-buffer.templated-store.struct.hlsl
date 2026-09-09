// RUN: %dxc -T cs_6_2 -E main -enable-16bit-types -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

ByteAddressBuffer buf;
RWByteAddressBuffer buf2;

struct T {
  float16_t x[5];
};

struct U {
  float16_t v[3];
  uint w;
};

struct S {
  float16_t3 a[3];
  double c;
  T t;
  double b;
  float16_t d;
  T e[2];
  U f[2];
  float16_t z;
};

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadId) {
  S sArr[2] = buf.Load<S[2]>(tid.x);
  buf2.Store<S[2]>(tid.x, sArr);
}

// Note: the following indices are taken from the DXIL compilation:
//
//
//                           // sArr[0] starts
//
//  %3 = 0                    // a[0] starts at byte offset 0
//  %8 = add i32 %3, 6        // a[1] starts at byte offset 6
// %13 = add i32 %3, 12       // a[2] starts at byte offset 12
//                            // since the next member is a 'double' it does not
//                            // start at offset 18 or 20. It starts at offset 24.
//                            // byte [18-23] inclusive are PADDING.
// %18 = add i32 %3, 24       // c starts at offset 24 (6 words)
// %23 = add i32 %3, 32       // t.x[0] starts at byte offset 32 (8 words)
// %26 = add i32 %3, 34       // t.x[1] starts at byte offset 34
// %29 = add i32 %3, 36       // t.x[2] starts at byte offset 36
// %32 = add i32 %3, 38       // t.x[2] starts at byte offset 38
// %35 = add i32 %3, 40       // t.x[2] starts at byte offset 40
//                            // byte [42-47] inclusive are PADDING.
// %38 = add i32 %3, 48       // b starts at byte offset 48 (12 words)
// %43 = add i32 %3, 56       // d starts at byte offset 56 (14 words)
//                            // even though 'e' is the next struct member,
//                            // it does NOT start at an aligned address (does not start at 64 byte offset).
// %46 = add i32 %3, 58       // e[0].x[0] starts at byte offset 58
// %49 = add i32 %3, 60       // e[0].x[1] starts at byte offset 60
// %52 = add i32 %3, 62       // e[0].x[2] starts at byte offset 62
// %55 = add i32 %3, 64       // e[0].x[3] starts at byte offset 64
// %58 = add i32 %3, 66       // e[0].x[4] starts at byte offset 66
// %61 = add i32 %3, 68       // e[1].x[0] starts at byte offset 68
// %64 = add i32 %3, 70       // e[1].x[1] starts at byte offset 70
// %67 = add i32 %3, 72       // e[1].x[2] starts at byte offset 72
// %70 = add i32 %3, 74       // e[1].x[3] starts at byte offset 74
// %73 = add i32 %3, 76       // e[1].x[4] starts at byte offset 76
//                            // 'f' starts at the next aligned address
//                            // byte [78-79] inclusive are PADDING
// %76 = add i32 %3, 80       // f[0].v[0] starts at byte offset 80 (20 words)
// %79 = add i32 %3, 82       // f[0].v[1] starts at byte offset 82
// %82 = add i32 %3, 84       // f[0].v[2] starts at byte offset 84
//                            // byte [86-87] inclusive are PADDING
// %85 = add i32 %3, 88       // f[0].w starts at byte offset 88 (22 words)
// %88 = add i32 %3, 92       // f[1].v[0] starts at byte offset 92
// %91 = add i32 %3, 94       // f[1].v[1] starts at byte offset 94
// %94 = add i32 %3, 96       // f[1].v[2] starts at byte offset 96
//                            // byte [98-99] inclusive are PADDING
// %97 = add i32 %3, 100      // f[1].w starts at byte offset 100 (25 words)
// %100 = add i32 %3, 104     // z starts at byte offset 104 (26 words)
//
//                           // sArr[1] starts
//
//                           // byte [106-111] inclusive are PADDING
//
//                           // ALL the following offsets are similar to offsets
//                           // of sArr[0], shifted by 112 bytes.
//
// %103 = add i32 %3, 112
// %108 = add i32 %3, 118
// %113 = add i32 %3, 124
// %118 = add i32 %3, 136
// %123 = add i32 %3, 144
// %126 = add i32 %3, 146
// %129 = add i32 %3, 148
// %132 = add i32 %3, 150
// %135 = add i32 %3, 152
// %138 = add i32 %3, 160
// %143 = add i32 %3, 168
// %146 = add i32 %3, 170
// %149 = add i32 %3, 172
// %152 = add i32 %3, 174
// %155 = add i32 %3, 176
// %158 = add i32 %3, 178
// %161 = add i32 %3, 180
// %164 = add i32 %3, 182
// %167 = add i32 %3, 184
// %170 = add i32 %3, 186
// %173 = add i32 %3, 188
// %176 = add i32 %3, 192
// %179 = add i32 %3, 194
// %182 = add i32 %3, 196
// %185 = add i32 %3, 200
// %188 = add i32 %3, 204
// %191 = add i32 %3, 206
// %194 = add i32 %3, 208
// %197 = add i32 %3, 212
// %200 = add i32 %3, 216

// Initialization of sArr array.
// CHECK: OpStore %sArr {{%[0-9]+}}
//
// Check for templated 'Store' method.
//
// CHECK:          [[tidx_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %tid %int_0
// CHECK:         [[base_addr:%[0-9]+]] = OpLoad %uint [[tidx_ptr]]
// CHECK:              [[sArr:%[0-9]+]] = OpLoad %_arr_S_uint_2 %sArr
// CHECK:                [[s0:%[0-9]+]] = OpCompositeExtract %S [[sArr]] 0
// CHECK:                [[s1:%[0-9]+]] = OpCompositeExtract %S [[sArr]] 1
// CHECK:                 [[a:%[0-9]+]] = OpCompositeExtract %_arr_v3half_uint_3 [[s0]] 0
// CHECK:                [[a0:%[0-9]+]] = OpCompositeExtract %v3half [[a]] 0
// CHECK:                [[a1:%[0-9]+]] = OpCompositeExtract %v3half [[a]] 1
// CHECK:                [[a2:%[0-9]+]] = OpCompositeExtract %v3half [[a]] 2
// CHECK:               [[a00:%[0-9]+]] = OpCompositeExtract %half [[a0]] 0
// CHECK:               [[a01:%[0-9]+]] = OpCompositeExtract %half [[a0]] 1
// CHECK:               [[a02:%[0-9]+]] = OpCompositeExtract %half [[a0]] 2
// CHECK:               [[a10:%[0-9]+]] = OpCompositeExtract %half [[a1]] 0
// CHECK:               [[a11:%[0-9]+]] = OpCompositeExtract %half [[a1]] 1
// CHECK:               [[a12:%[0-9]+]] = OpCompositeExtract %half [[a1]] 2
// CHECK:               [[a20:%[0-9]+]] = OpCompositeExtract %half [[a2]] 0
// CHECK:               [[a21:%[0-9]+]] = OpCompositeExtract %half [[a2]] 1
// CHECK:               [[a22:%[0-9]+]] = OpCompositeExtract %half [[a2]] 2
// CHECK:         [[a00_16bit:%[0-9]+]] = OpBitcast %ushort [[a00]]
// CHECK:         [[a00_32bit:%[0-9]+]] = OpUConvert %uint [[a00_16bit]]
// CHECK:          [[a01_addr:%[0-9]+]] = OpIAdd %uint [[base_addr]] %uint_2
// CHECK:         [[a02_index:%[0-9]+]] = OpShiftRightLogical %uint [[a01_addr]] %uint_2
// CHECK:        [[byteOffset:%[0-9]+]] = OpUMod %uint [[a01_addr]] %uint_4
// CHECK:         [[bitOffset:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset]] %uint_3
// CHECK:               [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[a02_index]]
// CHECK:         [[a01_16bit:%[0-9]+]] = OpBitcast %ushort [[a01]]
// CHECK:         [[a01_32bit:%[0-9]+]] = OpUConvert %uint [[a01_16bit]]
// CHECK: [[a01_32bit_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[a01_32bit]] [[bitOffset]]
// CHECK:        [[maskOffset:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset]]
// CHECK:              [[mask:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset]]
// CHECK:           [[oldWord:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:            [[masked:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord]] [[mask]]
// CHECK:              [[word:%[0-9]+]] = OpBitwiseOr %uint [[masked]] [[a01_32bit_shifted]]
// CHECK:                              OpStore [[ptr]] [[word]]

// CHECK:          [[a02_addr:%[0-9]+]] = OpIAdd %uint [[a01_addr]] %uint_2
// CHECK:         [[a02_16bit:%[0-9]+]] = OpBitcast %ushort [[a02]]
// CHECK:         [[a02_32bit:%[0-9]+]] = OpUConvert %uint [[a02_16bit]]
// CHECK:          [[a10_addr:%[0-9]+]] = OpIAdd %uint [[a02_addr]] %uint_2
// CHECK:         [[a10_index:%[0-9]+]] = OpShiftRightLogical %uint [[a10_addr]] %uint_2
// CHECK:        [[byteOffset_0:%[0-9]+]] = OpUMod %uint [[a10_addr]] %uint_4
// CHECK:         [[bitOffset_0:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_0]] %uint_3
// CHECK:               [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[a10_index]]
// CHECK:         [[a10_16bit:%[0-9]+]] = OpBitcast %ushort [[a10]]
// CHECK:         [[a10_32bit:%[0-9]+]] = OpUConvert %uint [[a10_16bit]]
// CHECK: [[a10_32bit_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[a10_32bit]] [[bitOffset_0]]
// CHECK:        [[maskOffset_0:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_0]]
// CHECK:              [[mask_0:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_0]]
// CHECK:           [[oldWord_0:%[0-9]+]] = OpLoad %uint [[ptr_0]]
// CHECK:            [[masked_0:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_0]] [[mask_0]]
// CHECK:              [[word_0:%[0-9]+]] = OpBitwiseOr %uint [[masked_0]] [[a10_32bit_shifted]]
// CHECK:                              OpStore [[ptr_0]] [[word_0]]

// CHECK:          [[a11_addr:%[0-9]+]] = OpIAdd %uint [[a10_addr]] %uint_2
// CHECK:         [[a11_16bit:%[0-9]+]] = OpBitcast %ushort [[a11]]
// CHECK:         [[a11_32bit:%[0-9]+]] = OpUConvert %uint [[a11_16bit]]
// CHECK:          [[a12_addr:%[0-9]+]] = OpIAdd %uint [[a11_addr]] %uint_2
// CHECK:         [[a12_index:%[0-9]+]] = OpShiftRightLogical %uint [[a12_addr]] %uint_2
// CHECK:        [[byteOffset_1:%[0-9]+]] = OpUMod %uint [[a12_addr]] %uint_4
// CHECK:         [[bitOffset_1:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_1]] %uint_3
// CHECK:               [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[a12_index]]
// CHECK:         [[a12_16bit:%[0-9]+]] = OpBitcast %ushort [[a12]]
// CHECK:         [[a12_32bit:%[0-9]+]] = OpUConvert %uint [[a12_16bit]]
// CHECK: [[a12_32bit_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[a12_32bit]] [[bitOffset_1]]
// CHECK:        [[maskOffset_1:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_1]]
// CHECK:              [[mask_1:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_1]]
// CHECK:           [[oldWord_1:%[0-9]+]] = OpLoad %uint [[ptr_1]]
// CHECK:            [[masked_1:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_1]] [[mask_1]]
// CHECK:              [[word_1:%[0-9]+]] = OpBitwiseOr %uint [[masked_1]] [[a12_32bit_shifted]]
// CHECK:                              OpStore [[ptr_1]] [[word_1]]

// CHECK:          [[a20_addr:%[0-9]+]] = OpIAdd %uint [[a12_addr]] %uint_2
// CHECK:         [[a20_16bit:%[0-9]+]] = OpBitcast %ushort [[a20]]
// CHECK:         [[a20_32bit:%[0-9]+]] = OpUConvert %uint [[a20_16bit]]
// CHECK:          [[a21_addr:%[0-9]+]] = OpIAdd %uint [[a20_addr]] %uint_2
// CHECK:         [[a21_index:%[0-9]+]] = OpShiftRightLogical %uint [[a21_addr]] %uint_2
// CHECK:        [[byteOffset_2:%[0-9]+]] = OpUMod %uint [[a21_addr]] %uint_4
// CHECK:         [[bitOffset_2:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_2]] %uint_3
// CHECK:               [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[a21_index]]
// CHECK:         [[a21_16bit:%[0-9]+]] = OpBitcast %ushort [[a21]]
// CHECK:         [[a21_32bit:%[0-9]+]] = OpUConvert %uint [[a21_16bit]]
// CHECK: [[a21_32bit_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[a21_32bit]] [[bitOffset_2]]
// CHECK:        [[maskOffset_2:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_2]]
// CHECK:              [[mask_2:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_2]]
// CHECK:           [[oldWord_2:%[0-9]+]] = OpLoad %uint [[ptr_2]]
// CHECK:            [[masked_2:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_2]] [[mask_2]]
// CHECK:              [[word_2:%[0-9]+]] = OpBitwiseOr %uint [[masked_2]] [[a21_32bit_shifted]]
// CHECK:                              OpStore [[ptr_2]] [[word_2]]

// CHECK:          [[a22_addr:%[0-9]+]] = OpIAdd %uint [[a21_addr]] %uint_2
// CHECK:         [[a22_index:%[0-9]+]] = OpShiftRightLogical %uint [[a22_addr]] %uint_2
// CHECK:        [[byteOffset_3:%[0-9]+]] = OpUMod %uint [[a22_addr]] %uint_4
// CHECK:         [[bitOffset_3:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_3]] %uint_3
// CHECK:               [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[a22_index]]
// CHECK:         [[a22_16bit:%[0-9]+]] = OpBitcast %ushort [[a22]]
// CHECK:         [[a22_32bit:%[0-9]+]] = OpUConvert %uint [[a22_16bit]]
// CHECK: [[a22_32bit_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[a22_32bit]] [[bitOffset_3]]
// CHECK:        [[maskOffset_3:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_3]]
// CHECK:              [[mask_3:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_3]]
// CHECK:           [[oldWord_3:%[0-9]+]] = OpLoad %uint [[ptr_3]]
// CHECK:            [[masked_3:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_3]] [[mask_3]]
// CHECK:              [[word_3:%[0-9]+]] = OpBitwiseOr %uint [[masked_3]] [[a22_32bit_shifted]]
// CHECK:                              OpStore [[ptr_3]] [[word_3]]

//
// The second member of S starts at byte offset 24 (6 words)
//
// CHECK:        [[c_addr:%[0-9]+]] = OpIAdd %uint [[base_addr]] %uint_24

// CHECK:             [[c:%[0-9]+]] = OpCompositeExtract %double [[s0]] 1
// CHECK:         [[merge:%[0-9]+]] = OpBitcast %v2uint [[c]]
// CHECK:       [[c_word0:%[0-9]+]] = OpCompositeExtract %uint [[merge]] 0
// CHECK:       [[c_word1:%[0-9]+]] = OpCompositeExtract %uint [[merge]] 1

// CHECK:       [[c_index:%[0-9]+]] = OpShiftRightLogical %uint [[c_addr]] %uint_2
// CHECK:         [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[c_index]]
// CHECK:                             OpStore [[ptr_4]] [[c_word0]]
// CHECK:   [[c_msb_index:%[0-9]+]] = OpIAdd %uint [[c_index]] %uint_1

// CHECK:         [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[c_msb_index]]
// CHECK:                             OpStore [[ptr_5]] [[c_word1]]
// CHECK:    [[next_index:%[0-9]+]] = OpIAdd %uint [[c_msb_index]] %uint_1

//
// The third member of S starts at byte offset 32 (8 words)
//
// CHECK:         [[t_addr:%[0-9]+]] = OpIAdd %uint [[base_addr]] %uint_32
//
// CHECK:              [[t:%[0-9]+]] = OpCompositeExtract %T [[s0]] 2
// CHECK:              [[x:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_5 [[t]] 0
// CHECK:             [[x0:%[0-9]+]] = OpCompositeExtract %half [[x]] 0
// CHECK:             [[x1:%[0-9]+]] = OpCompositeExtract %half [[x]] 1
// CHECK:             [[x2:%[0-9]+]] = OpCompositeExtract %half [[x]] 2
// CHECK:             [[x3:%[0-9]+]] = OpCompositeExtract %half [[x]] 3
// CHECK:             [[x4:%[0-9]+]] = OpCompositeExtract %half [[x]] 4
// CHECK:         [[x0_u16:%[0-9]+]] = OpBitcast %ushort [[x0]]
// CHECK:         [[x0_u32:%[0-9]+]] = OpUConvert %uint [[x0_u16]]
// CHECK:        [[x1_addr:%[0-9]+]] = OpIAdd %uint [[t_addr]] %uint_2
// CHECK:       [[x1_index:%[0-9]+]] = OpShiftRightLogical %uint [[x1_addr]] %uint_2
// CHECK:     [[byteOffset_4:%[0-9]+]] = OpUMod %uint [[x1_addr]] %uint_4
// CHECK:      [[bitOffset_4:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_4]] %uint_3
// CHECK:            [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x1_index]]
// CHECK:         [[x1_u16:%[0-9]+]] = OpBitcast %ushort [[x1]]
// CHECK:         [[x1_u32:%[0-9]+]] = OpUConvert %uint [[x1_u16]]
// CHECK: [[x1_u32_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[x1_u32]] [[bitOffset_4]]
// CHECK:     [[maskOffset_4:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_4]]
// CHECK:           [[mask_4:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_4]]
// CHECK:        [[oldWord_4:%[0-9]+]] = OpLoad %uint [[ptr_6]]
// CHECK:         [[masked_4:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_4]] [[mask_4]]
// CHECK:           [[word_4:%[0-9]+]] = OpBitwiseOr %uint [[masked_4]] [[x1_u32_shifted]]
// CHECK:                           OpStore [[ptr_6]] [[word_4]]
// CHECK:        [[x2_addr:%[0-9]+]] = OpIAdd %uint [[x1_addr]] %uint_2
// CHECK:         [[x2_u16:%[0-9]+]] = OpBitcast %ushort [[x2]]
// CHECK:         [[x2_u32:%[0-9]+]] = OpUConvert %uint [[x2_u16]]
// CHECK:        [[x3_addr:%[0-9]+]] = OpIAdd %uint [[x2_addr]] %uint_2
// CHECK:       [[x3_index:%[0-9]+]] = OpShiftRightLogical %uint [[x3_addr]] %uint_2
// CHECK:     [[byteOffset_5:%[0-9]+]] = OpUMod %uint [[x3_addr]] %uint_4
// CHECK:      [[bitOffset_5:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_5]] %uint_3
// CHECK:            [[ptr_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x3_index]]
// CHECK:         [[x3_u16:%[0-9]+]] = OpBitcast %ushort [[x3]]
// CHECK:         [[x3_u32:%[0-9]+]] = OpUConvert %uint [[x3_u16]]
// CHECK: [[x3_u32_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[x3_u32]] [[bitOffset_5]]
// CHECK:     [[maskOffset_5:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_5]]
// CHECK:           [[mask_5:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_5]]
// CHECK:        [[oldWord_5:%[0-9]+]] = OpLoad %uint [[ptr_7]]
// CHECK:         [[masked_5:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_5]] [[mask_5]]
// CHECK:           [[word_5:%[0-9]+]] = OpBitwiseOr %uint [[masked_5]] [[x3_u32_shifted]]
// CHECK:                           OpStore [[ptr_7]] [[word_5]]
// CHECK:        [[x4_addr:%[0-9]+]] = OpIAdd %uint [[x3_addr]] %uint_2
// CHECK:       [[x4_index:%[0-9]+]] = OpShiftRightLogical %uint [[x4_addr]] %uint_2
// CHECK:     [[byteOffset_6:%[0-9]+]] = OpUMod %uint [[x4_addr]] %uint_4
// CHECK:      [[bitOffset_6:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_6]] %uint_3
// CHECK:            [[ptr_8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x4_index]]
// CHECK:         [[x4_u16:%[0-9]+]] = OpBitcast %ushort [[x4]]
// CHECK:         [[x4_u32:%[0-9]+]] = OpUConvert %uint [[x4_u16]]
// CHECK: [[x4_u32_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[x4_u32]] [[bitOffset_6]]
// CHECK:     [[maskOffset_6:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_6]]
// CHECK:           [[mask_6:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_6]]
// CHECK:        [[oldWord_6:%[0-9]+]] = OpLoad %uint [[ptr_8]]
// CHECK:         [[masked_6:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_6]] [[mask_6]]
// CHECK:           [[word_6:%[0-9]+]] = OpBitwiseOr %uint [[masked_6]] [[x4_u32_shifted]]
// CHECK:                           OpStore [[ptr_8]] [[word_6]]

//
// The fourth member of S starts at byte offset 48 (12 words)
//
// CHECK:        [[b_addr:%[0-9]+]] = OpIAdd %uint [[base_addr]] %uint_48
//
// CHECK:             [[b:%[0-9]+]] = OpCompositeExtract %double [[s0]] 3
// CHECK:         [[merge:%[0-9]+]] = OpBitcast %v2uint [[b]]
// CHECK:       [[b_word0:%[0-9]+]] = OpCompositeExtract %uint [[merge]] 0
// CHECK:       [[b_word1:%[0-9]+]] = OpCompositeExtract %uint [[merge]] 1

// CHECK:       [[b_index:%[0-9]+]] = OpShiftRightLogical %uint [[b_addr]] %uint_2
// CHECK:         [[ptr_9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[b_index]]
// CHECK:                             OpStore [[ptr_9]] [[b_word0]]
// CHECK:   [[b_msb_index:%[0-9]+]] = OpIAdd %uint [[b_index]] %uint_1
// CHECK:        [[ptr_10:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[b_msb_index]]
// CHECK:                             OpStore [[ptr_10]] [[b_word1]]
// CHECK:    [[next_index:%[0-9]+]] = OpIAdd %uint [[b_msb_index]] %uint_1

//
// The fifth member of S starts at byte offset 56 (14 words)
//
// CHECK:        [[d_addr:%[0-9]+]] = OpIAdd %uint [[base_addr]] %uint_56
//
// CHECK:             [[d:%[0-9]+]] = OpCompositeExtract %half [[s0]] 4
// CHECK:       [[d_index:%[0-9]+]] = OpShiftRightLogical %uint [[d_addr]] %uint_2
// CHECK:    [[byteOffset_7:%[0-9]+]] = OpUMod %uint [[d_addr]] %uint_4
// CHECK:     [[bitOffset_7:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_7]] %uint_3
// CHECK:           [[ptr_11:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[d_index]]
// CHECK:         [[d_u16:%[0-9]+]] = OpBitcast %ushort [[d]]
// CHECK:         [[d_u32:%[0-9]+]] = OpUConvert %uint [[d_u16]]
// CHECK: [[d_u32_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[d_u32]] [[bitOffset_7]]
// CHECK:    [[maskOffset_7:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_7]]
// CHECK:          [[mask_7:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_7]]
// CHECK:       [[oldWord_7:%[0-9]+]] = OpLoad %uint [[ptr_11]]
// CHECK:        [[masked_7:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_7]] [[mask_7]]
// CHECK:          [[word_7:%[0-9]+]] = OpBitwiseOr %uint [[masked_7]] [[d_u32_shifted]]
// CHECK:                          OpStore [[ptr_11]] [[word_7]]

//
// The sixth member of S starts at byte offset 58 (14 words + 16bit offset)
// This is an extraordinary case of alignment. Since the sixth member only
// contains fp16, and the fifth member was also fp16, DX packs them tightly.
// As a result, store must occur at non-aligned offset.
// e[0] takes the following byte offsets: 58, 60, 62, 64, 66.
// e[1] takes the following byte offsets: 68, 70, 72, 74, 76.
// (60-64 = index 15. 64-68 = index 16)
// (68-72 = index 17. 72-76 = index 18)
// (76-78 = first half of index 19)
//
// CHECK:        [[e_addr:%[0-9]+]] = OpIAdd %uint [[base_addr]] %uint_58
// CHECK:             [[e:%[0-9]+]] = OpCompositeExtract %_arr_T_uint_2 [[s0]] 5
// CHECK:            [[e0:%[0-9]+]] = OpCompositeExtract %T [[e]] 0
// CHECK:            [[e1:%[0-9]+]] = OpCompositeExtract %T [[e]] 1
// CHECK:             [[x_0:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_5 [[e0]] 0
// CHECK:            [[x0_0:%[0-9]+]] = OpCompositeExtract %half [[x_0]] 0
// CHECK:            [[x1_0:%[0-9]+]] = OpCompositeExtract %half [[x_0]] 1
// CHECK:            [[x2_0:%[0-9]+]] = OpCompositeExtract %half [[x_0]] 2
// CHECK:            [[x3_0:%[0-9]+]] = OpCompositeExtract %half [[x_0]] 3
// CHECK:            [[x4_0:%[0-9]+]] = OpCompositeExtract %half [[x_0]] 4
// CHECK:      [[x0_index:%[0-9]+]] = OpShiftRightLogical %uint [[e_addr]] %uint_2
// CHECK:    [[byteOffset_8:%[0-9]+]] = OpUMod %uint [[e_addr]] %uint_4
// CHECK:     [[bitOffset_8:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_8]] %uint_3
// CHECK:           [[ptr_12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x0_index]]
// CHECK:         [[x0u16:%[0-9]+]] = OpBitcast %ushort [[x0_0]]
// CHECK:         [[x0u32:%[0-9]+]] = OpUConvert %uint [[x0u16]]
// CHECK: [[x0u32_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[x0u32]] [[bitOffset_8]]
// CHECK:    [[maskOffset_8:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_8]]
// CHECK:          [[mask_8:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_8]]
// CHECK:       [[oldWord_8:%[0-9]+]] = OpLoad %uint [[ptr_12]]
// CHECK:        [[masked_8:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_8]] [[mask_8]]
// CHECK:       [[newWord:%[0-9]+]] = OpBitwiseOr %uint [[masked_8]] [[x0u32_shifted]]
// CHECK:                          OpStore [[ptr_12]] [[newWord]]

// CHECK:       [[x1_addr_0:%[0-9]+]] = OpIAdd %uint [[e_addr]] %uint_2
// CHECK:      [[x1_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[x1_addr_0]] %uint_2
// CHECK:    [[byteOffset_9:%[0-9]+]] = OpUMod %uint [[x1_addr_0]] %uint_4
// CHECK:     [[bitOffset_9:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_9]] %uint_3
// CHECK:           [[ptr_13:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x1_index_0]]
// CHECK:         [[x1u16:%[0-9]+]] = OpBitcast %ushort [[x1_0]]
// CHECK:         [[x1u32:%[0-9]+]] = OpUConvert %uint [[x1u16]]
// CHECK:[[x1_u32_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[x1u32]] [[bitOffset_9]]
// CHECK:    [[maskOffset_9:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_9]]
// CHECK:          [[mask_9:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_9]]
// CHECK:       [[oldWord_9:%[0-9]+]] = OpLoad %uint [[ptr_13]]
// CHECK:        [[masked_9:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_9]] [[mask_9]]
// CHECK:          [[word_8:%[0-9]+]] = OpBitwiseOr %uint [[masked_9]] [[x1_u32_shifted_0]]
// CHECK:                          OpStore [[ptr_13]] [[word_8]]

// CHECK:       [[x2_addr_0:%[0-9]+]] = OpIAdd %uint [[x1_addr_0]] %uint_2
// CHECK:         [[x2u16:%[0-9]+]] = OpBitcast %ushort [[x2_0]]
// CHECK:         [[x2u32:%[0-9]+]] = OpUConvert %uint [[x2u16]]
// CHECK:       [[x3_addr_0:%[0-9]+]] = OpIAdd %uint [[x2_addr_0]] %uint_2
// CHECK:      [[x3_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[x3_addr_0]] %uint_2
// CHECK:    [[byteOffset_10:%[0-9]+]] = OpUMod %uint [[x3_addr_0]] %uint_4
// CHECK:     [[bitOffset_10:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_10]] %uint_3
// CHECK:           [[ptr_14:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x3_index_0]]
// CHECK:         [[x3u16:%[0-9]+]] = OpBitcast %ushort [[x3_0]]
// CHECK:         [[x3u32:%[0-9]+]] = OpUConvert %uint [[x3u16]]
// CHECK:[[x3_u32_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[x3u32]] [[bitOffset_10]]
// CHECK:    [[maskOffset_10:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_10]]
// CHECK:          [[mask_10:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_10]]
// CHECK:       [[oldWord_10:%[0-9]+]] = OpLoad %uint [[ptr_14]]
// CHECK:        [[masked_10:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_10]] [[mask_10]]
// CHECK:          [[word_9:%[0-9]+]] = OpBitwiseOr %uint [[masked_10]] [[x3_u32_shifted_0]]
// CHECK:                          OpStore [[ptr_14]] [[word_9]]

// CHECK:       [[x4_addr_0:%[0-9]+]] = OpIAdd %uint [[x3_addr_0]] %uint_2
// CHECK:      [[x4_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[x4_addr_0]] %uint_2
// CHECK:    [[byteOffset_11:%[0-9]+]] = OpUMod %uint [[x4_addr_0]] %uint_4
// CHECK:     [[bitOffset_11:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_11]] %uint_3
// CHECK:           [[ptr_15:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x4_index_0]]
// CHECK:         [[x4u16:%[0-9]+]] = OpBitcast %ushort [[x4_0]]
// CHECK:         [[x4u32:%[0-9]+]] = OpUConvert %uint [[x4u16]]
// CHECK:[[x4_u32_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[x4u32]] [[bitOffset_11]]
// CHECK:    [[maskOffset_11:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_11]]
// CHECK:          [[mask_11:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_11]]
// CHECK:       [[oldWord_11:%[0-9]+]] = OpLoad %uint [[ptr_15]]
// CHECK:        [[masked_11:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_11]] [[mask_11]]
// CHECK:          [[word_10:%[0-9]+]] = OpBitwiseOr %uint [[masked_11]] [[x4_u32_shifted_0]]
// CHECK:                          OpStore [[ptr_15]] [[word_10]]

//
// The seventh member of S starts at byte offset 80 (20 words), so:
// for f[0]:
// v should start at byte offset 80 (20 words)
// w should start at byte offset 88 (22 words)
// for f[1]:
// v should start at byte offset 92 (23 words)
// w should start at byte offset 100 (25 words)
//
// CHECK:        [[f_addr:%[0-9]+]] = OpIAdd %uint [[base_addr]] %uint_80
// CHECK:             [[f:%[0-9]+]] = OpCompositeExtract %_arr_U_uint_2 [[s0]] 6
// CHECK:            [[u0:%[0-9]+]] = OpCompositeExtract %U [[f]] 0
// CHECK:            [[u1:%[0-9]+]] = OpCompositeExtract %U [[f]] 1
// CHECK:             [[v:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_3 [[u0]] 0
// CHECK:            [[v0:%[0-9]+]] = OpCompositeExtract %half [[v]] 0
// CHECK:            [[v1:%[0-9]+]] = OpCompositeExtract %half [[v]] 1
// CHECK:            [[v2:%[0-9]+]] = OpCompositeExtract %half [[v]] 2
// CHECK:         [[v0u16:%[0-9]+]] = OpBitcast %ushort [[v0]]
// CHECK:         [[v0u32:%[0-9]+]] = OpUConvert %uint [[v0u16]]
// CHECK:       [[v1_addr:%[0-9]+]] = OpIAdd %uint [[f_addr]] %uint_2
// CHECK:      [[v1_index:%[0-9]+]] = OpShiftRightLogical %uint [[v1_addr]] %uint_2
// CHECK:    [[byteOffset_12:%[0-9]+]] = OpUMod %uint [[v1_addr]] %uint_4
// CHECK:     [[bitOffset_12:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_12]] %uint_3
// CHECK:           [[ptr_16:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[v1_index]]
// CHECK:         [[v1u16:%[0-9]+]] = OpBitcast %ushort [[v1]]
// CHECK:         [[v1u32:%[0-9]+]] = OpUConvert %uint [[v1u16]]
// CHECK: [[v1u32_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[v1u32]] [[bitOffset_12]]
// CHECK:    [[maskOffset_12:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_12]]
// CHECK:          [[mask_12:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_12]]
// CHECK:       [[oldWord_12:%[0-9]+]] = OpLoad %uint [[ptr_16]]
// CHECK:        [[masked_12:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_12]] [[mask_12]]
// CHECK:          [[word_11:%[0-9]+]] = OpBitwiseOr %uint [[masked_12]] [[v1u32_shifted]]
// CHECK:                          OpStore [[ptr_16]] [[word_11]]

// CHECK:       [[v2_addr:%[0-9]+]] = OpIAdd %uint [[v1_addr]] %uint_2
// CHECK:      [[v2_index:%[0-9]+]] = OpShiftRightLogical %uint [[v2_addr]] %uint_2
// CHECK:    [[byteOffset_13:%[0-9]+]] = OpUMod %uint [[v2_addr]] %uint_4
// CHECK:     [[bitOffset_13:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_13]] %uint_3
// CHECK:           [[ptr_17:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[v2_index]]
// CHECK:         [[v2u16:%[0-9]+]] = OpBitcast %ushort [[v2]]
// CHECK:         [[v2u32:%[0-9]+]] = OpUConvert %uint [[v2u16]]
// CHECK: [[v2u32_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[v2u32]] [[bitOffset_13]]
// CHECK:    [[maskOffset_13:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_13]]
// CHECK:          [[mask_13:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_13]]
// CHECK:       [[oldWord_13:%[0-9]+]] = OpLoad %uint [[ptr_17]]
// CHECK:        [[masked_13:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_13]] [[mask_13]]
// CHECK:          [[word_12:%[0-9]+]] = OpBitwiseOr %uint [[masked_13]] [[v2u32_shifted]]
// CHECK:                          OpStore [[ptr_17]] [[word_12]]

// CHECK:    [[w_addr:%[0-9]+]] = OpIAdd %uint [[f_addr]] %uint_8
// CHECK:         [[w:%[0-9]+]] = OpCompositeExtract %uint [[u0]] 1
// CHECK:   [[w_index:%[0-9]+]] = OpShiftRightLogical %uint [[w_addr]] %uint_2
// CHECK:       [[ptr_18:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[w_index]]
// CHECK:                      OpStore [[ptr_18]] [[w]]

// CHECK:       [[f1_addr:%[0-9]+]] = OpIAdd %uint [[f_addr]] %uint_12
// CHECK:             [[v_0:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_3 [[u1]] 0
// CHECK:            [[v0_0:%[0-9]+]] = OpCompositeExtract %half [[v_0]] 0
// CHECK:            [[v1_0:%[0-9]+]] = OpCompositeExtract %half [[v_0]] 1
// CHECK:            [[v2_0:%[0-9]+]] = OpCompositeExtract %half [[v_0]] 2
// CHECK:         [[v0u16_0:%[0-9]+]] = OpBitcast %ushort [[v0_0]]
// CHECK:         [[v0u32_0:%[0-9]+]] = OpUConvert %uint [[v0u16_0]]
// CHECK:       [[v1_addr_0:%[0-9]+]] = OpIAdd %uint [[f1_addr]] %uint_2
// CHECK:      [[v1_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[v1_addr_0]] %uint_2
// CHECK:    [[byteOffset_14:%[0-9]+]] = OpUMod %uint [[v1_addr_0]] %uint_4
// CHECK:     [[bitOffset_14:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_14]] %uint_3
// CHECK:           [[ptr_19:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[v1_index_0]]
// CHECK:         [[v1u16_0:%[0-9]+]] = OpBitcast %ushort [[v1_0]]
// CHECK:         [[v1u32_0:%[0-9]+]] = OpUConvert %uint [[v1u16_0]]
// CHECK: [[v1u32_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[v1u32_0]] [[bitOffset_14]]
// CHECK:    [[maskOffset_14:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_14]]
// CHECK:          [[mask_14:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_14]]
// CHECK:       [[oldWord_14:%[0-9]+]] = OpLoad %uint [[ptr_19]]
// CHECK:        [[masked_14:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_14]] [[mask_14]]
// CHECK:          [[word_13:%[0-9]+]] = OpBitwiseOr %uint [[masked_14]] [[v1u32_shifted_0]]
// CHECK:                          OpStore [[ptr_19]] [[word_13]]

// CHECK:       [[v2_addr_0:%[0-9]+]] = OpIAdd %uint [[v1_addr_0]] %uint_2
// CHECK:      [[v2_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[v2_addr_0]] %uint_2
// CHECK:    [[byteOffset_15:%[0-9]+]] = OpUMod %uint [[v2_addr_0]] %uint_4
// CHECK:     [[bitOffset_15:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_15]] %uint_3
// CHECK:           [[ptr_20:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[v2_index_0]]
// CHECK:         [[v2u16_0:%[0-9]+]] = OpBitcast %ushort [[v2_0]]
// CHECK:         [[v2u32_0:%[0-9]+]] = OpUConvert %uint [[v2u16_0]]
// CHECK: [[v2u32_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[v2u32_0]] [[bitOffset_15]]
// CHECK:    [[maskOffset_15:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_15]]
// CHECK:          [[mask_15:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_15]]
// CHECK:       [[oldWord_15:%[0-9]+]] = OpLoad %uint [[ptr_20]]
// CHECK:        [[masked_15:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_15]] [[mask_15]]
// CHECK:          [[word_14:%[0-9]+]] = OpBitwiseOr %uint [[masked_15]] [[v2u32_shifted_0]]
// CHECK:                          OpStore [[ptr_20]] [[word_14]]

// CHECK:        [[w_addr_0:%[0-9]+]] = OpIAdd %uint [[f1_addr]] %uint_8
// CHECK:             [[w_0:%[0-9]+]] = OpCompositeExtract %uint [[u1]] 1
// CHECK:       [[w_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[w_addr_0]] %uint_2
// CHECK:           [[ptr_21:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[w_index_0]]
// CHECK:                          OpStore [[ptr_21]] [[w_0]]

//
// The eighth member of S starts at byte offset 104 (26 words)
//
// CHECK:       [[z_addr:%[0-9]+]] = OpIAdd %uint [[base_addr]] %uint_104
// CHECK:            [[z:%[0-9]+]] = OpCompositeExtract %half [[s0]] 7
// CHECK:      [[z_index:%[0-9]+]] = OpShiftRightLogical %uint [[z_addr]] %uint_2
// CHECK:   [[byteOffset_16:%[0-9]+]] = OpUMod %uint [[z_addr]] %uint_4
// CHECK:    [[bitOffset_16:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_16]] %uint_3
// CHECK:          [[ptr_22:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[z_index]]
// CHECK:         [[zu16:%[0-9]+]] = OpBitcast %ushort [[z]]
// CHECK:         [[zu32:%[0-9]+]] = OpUConvert %uint [[zu16]]
// CHECK: [[zu32_shifted:%[0-9]+]] = OpShiftLeftLogical %uint [[zu32]] [[bitOffset_16]]
// CHECK:   [[maskOffset_16:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_16]]
// CHECK:         [[mask_16:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_16]]
// CHECK:      [[oldWord_16:%[0-9]+]] = OpLoad %uint [[ptr_22]]
// CHECK:       [[masked_16:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_16]] [[mask_16]]
// CHECK:         [[word_15:%[0-9]+]] = OpBitwiseOr %uint [[masked_16]] [[zu32_shifted]]
// CHECK:                         OpStore [[ptr_22]] [[word_15]]

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//
// We have an array of S structures (sArr). The second member (sArr[1]) should
// start at an aligned address. A structure aligment is the maximum alignment
// of its members.
// In this example, sArr[1] should start at byte offset 112 (28 words)
// It should *NOT* start at byte offset 108 (27 words).
//
//
// CHECK: [[s1_addr:%[0-9]+]] = OpIAdd %uint [[base_addr]] %uint_112
//
// CHECK:                 [[a_0:%[0-9]+]] = OpCompositeExtract %_arr_v3half_uint_3 [[s1]] 0
// CHECK:                [[a0_0:%[0-9]+]] = OpCompositeExtract %v3half [[a_0]] 0
// CHECK:                [[a1_0:%[0-9]+]] = OpCompositeExtract %v3half [[a_0]] 1
// CHECK:                [[a2_0:%[0-9]+]] = OpCompositeExtract %v3half [[a_0]] 2
// CHECK:               [[a00_0:%[0-9]+]] = OpCompositeExtract %half [[a0_0]] 0
// CHECK:               [[a01_0:%[0-9]+]] = OpCompositeExtract %half [[a0_0]] 1
// CHECK:               [[a02_0:%[0-9]+]] = OpCompositeExtract %half [[a0_0]] 2
// CHECK:               [[a10_0:%[0-9]+]] = OpCompositeExtract %half [[a1_0]] 0
// CHECK:               [[a11_0:%[0-9]+]] = OpCompositeExtract %half [[a1_0]] 1
// CHECK:               [[a12_0:%[0-9]+]] = OpCompositeExtract %half [[a1_0]] 2
// CHECK:               [[a20_0:%[0-9]+]] = OpCompositeExtract %half [[a2_0]] 0
// CHECK:               [[a21_0:%[0-9]+]] = OpCompositeExtract %half [[a2_0]] 1
// CHECK:               [[a22_0:%[0-9]+]] = OpCompositeExtract %half [[a2_0]] 2
// CHECK:         [[a00_16bit_0:%[0-9]+]] = OpBitcast %ushort [[a00_0]]
// CHECK:         [[a00_32bit_0:%[0-9]+]] = OpUConvert %uint [[a00_16bit_0]]
// CHECK:          [[a01_addr_0:%[0-9]+]] = OpIAdd %uint [[s1_addr]] %uint_2
// CHECK:         [[a02_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[a01_addr_0]] %uint_2
// CHECK:        [[byteOffset_17:%[0-9]+]] = OpUMod %uint [[a01_addr_0]] %uint_4
// CHECK:         [[bitOffset_17:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_17]] %uint_3
// CHECK:               [[ptr_23:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[a02_index_0]]
// CHECK:         [[a01_16bit_0:%[0-9]+]] = OpBitcast %ushort [[a01_0]]
// CHECK:         [[a01_32bit_0:%[0-9]+]] = OpUConvert %uint [[a01_16bit_0]]
// CHECK: [[a01_32bit_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[a01_32bit_0]] [[bitOffset_17]]
// CHECK:        [[maskOffset_17:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_17]]
// CHECK:              [[mask_17:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_17]]
// CHECK:           [[oldWord_17:%[0-9]+]] = OpLoad %uint [[ptr_23]]
// CHECK:            [[masked_17:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_17]] [[mask_17]]
// CHECK:              [[word_16:%[0-9]+]] = OpBitwiseOr %uint [[masked_17]] [[a01_32bit_shifted_0]]
// CHECK:                              OpStore [[ptr_23]] [[word_16]]

// CHECK:          [[a02_addr_0:%[0-9]+]] = OpIAdd %uint [[a01_addr_0]] %uint_2
// CHECK:         [[a02_16bit_0:%[0-9]+]] = OpBitcast %ushort [[a02_0]]
// CHECK:         [[a02_32bit_0:%[0-9]+]] = OpUConvert %uint [[a02_16bit_0]]
// CHECK:          [[a10_addr_0:%[0-9]+]] = OpIAdd %uint [[a02_addr_0]] %uint_2
// CHECK:         [[a10_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[a10_addr_0]] %uint_2
// CHECK:        [[byteOffset_18:%[0-9]+]] = OpUMod %uint [[a10_addr_0]] %uint_4
// CHECK:         [[bitOffset_18:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_18]] %uint_3
// CHECK:               [[ptr_24:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[a10_index_0]]
// CHECK:         [[a10_16bit_0:%[0-9]+]] = OpBitcast %ushort [[a10_0]]
// CHECK:         [[a10_32bit_0:%[0-9]+]] = OpUConvert %uint [[a10_16bit_0]]
// CHECK: [[a10_32bit_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[a10_32bit_0]] [[bitOffset_18]]
// CHECK:        [[maskOffset_18:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_18]]
// CHECK:              [[mask_18:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_18]]
// CHECK:           [[oldWord_18:%[0-9]+]] = OpLoad %uint [[ptr_24]]
// CHECK:            [[masked_18:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_18]] [[mask_18]]
// CHECK:              [[word_17:%[0-9]+]] = OpBitwiseOr %uint [[masked_18]] [[a10_32bit_shifted_0]]
// CHECK:                              OpStore [[ptr_24]] [[word_17]]

// CHECK:          [[a11_addr_0:%[0-9]+]] = OpIAdd %uint [[a10_addr_0]] %uint_2
// CHECK:         [[a11_16bit_0:%[0-9]+]] = OpBitcast %ushort [[a11_0]]
// CHECK:         [[a11_32bit_0:%[0-9]+]] = OpUConvert %uint [[a11_16bit_0]]
// CHECK:          [[a12_addr_0:%[0-9]+]] = OpIAdd %uint [[a11_addr_0]] %uint_2
// CHECK:         [[a12_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[a12_addr_0]] %uint_2
// CHECK:        [[byteOffset_19:%[0-9]+]] = OpUMod %uint [[a12_addr_0]] %uint_4
// CHECK:         [[bitOffset_19:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_19]] %uint_3
// CHECK:               [[ptr_25:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[a12_index_0]]
// CHECK:         [[a12_16bit_0:%[0-9]+]] = OpBitcast %ushort [[a12_0]]
// CHECK:         [[a12_32bit_0:%[0-9]+]] = OpUConvert %uint [[a12_16bit_0]]
// CHECK: [[a12_32bit_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[a12_32bit_0]] [[bitOffset_19]]
// CHECK:        [[maskOffset_19:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_19]]
// CHECK:              [[mask_19:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_19]]
// CHECK:           [[oldWord_19:%[0-9]+]] = OpLoad %uint [[ptr_25]]
// CHECK:            [[masked_19:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_19]] [[mask_19]]
// CHECK:              [[word_18:%[0-9]+]] = OpBitwiseOr %uint [[masked_19]] [[a12_32bit_shifted_0]]
// CHECK:                              OpStore [[ptr_25]] [[word_18]]

// CHECK:          [[a20_addr_0:%[0-9]+]] = OpIAdd %uint [[a12_addr_0]] %uint_2
// CHECK:         [[a20_16bit_0:%[0-9]+]] = OpBitcast %ushort [[a20_0]]
// CHECK:         [[a20_32bit_0:%[0-9]+]] = OpUConvert %uint [[a20_16bit_0]]
// CHECK:          [[a21_addr_0:%[0-9]+]] = OpIAdd %uint [[a20_addr_0]] %uint_2
// CHECK:         [[a21_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[a21_addr_0]] %uint_2
// CHECK:        [[byteOffset_20:%[0-9]+]] = OpUMod %uint [[a21_addr_0]] %uint_4
// CHECK:         [[bitOffset_20:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_20]] %uint_3
// CHECK:               [[ptr_26:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[a21_index_0]]
// CHECK:         [[a21_16bit_0:%[0-9]+]] = OpBitcast %ushort [[a21_0]]
// CHECK:         [[a21_32bit_0:%[0-9]+]] = OpUConvert %uint [[a21_16bit_0]]
// CHECK: [[a21_32bit_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[a21_32bit_0]] [[bitOffset_20]]
// CHECK:        [[maskOffset_20:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_20]]
// CHECK:              [[mask_20:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_20]]
// CHECK:           [[oldWord_20:%[0-9]+]] = OpLoad %uint [[ptr_26]]
// CHECK:            [[masked_20:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_20]] [[mask_20]]
// CHECK:              [[word_19:%[0-9]+]] = OpBitwiseOr %uint [[masked_20]] [[a21_32bit_shifted_0]]
// CHECK:                              OpStore [[ptr_26]] [[word_19]]

// CHECK:          [[a22_addr_0:%[0-9]+]] = OpIAdd %uint [[a21_addr_0]] %uint_2
// CHECK:         [[a22_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[a22_addr_0]] %uint_2
// CHECK:        [[byteOffset_21:%[0-9]+]] = OpUMod %uint [[a22_addr_0]] %uint_4
// CHECK:         [[bitOffset_21:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_21]] %uint_3
// CHECK:               [[ptr_27:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[a22_index_0]]
// CHECK:         [[a22_16bit_0:%[0-9]+]] = OpBitcast %ushort [[a22_0]]
// CHECK:         [[a22_32bit_0:%[0-9]+]] = OpUConvert %uint [[a22_16bit_0]]
// CHECK: [[a22_32bit_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[a22_32bit_0]] [[bitOffset_21]]
// CHECK:        [[maskOffset_21:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_21]]
// CHECK:              [[mask_21:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_21]]
// CHECK:           [[oldWord_21:%[0-9]+]] = OpLoad %uint [[ptr_27]]
// CHECK:            [[masked_21:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_21]] [[mask_21]]
// CHECK:              [[word_20:%[0-9]+]] = OpBitwiseOr %uint [[masked_21]] [[a22_32bit_shifted_0]]
// CHECK:                              OpStore [[ptr_27]] [[word_20]]

//
// The second member of S starts at byte offset 24 (6 words)
//
// CHECK:      [[c_addr_0:%[0-9]+]] = OpIAdd %uint [[s1_addr]] %uint_24
//
// CHECK:           [[c_0:%[0-9]+]] = OpCompositeExtract %double [[s1]] 1
// CHECK:         [[merge:%[0-9]+]] = OpBitcast %v2uint [[c_0]]
// CHECK:     [[c_word0_0:%[0-9]+]] = OpCompositeExtract %uint [[merge]] 0
// CHECK:     [[c_word1_0:%[0-9]+]] = OpCompositeExtract %uint [[merge]] 1

// CHECK:     [[c_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[c_addr_0]] %uint_2
// CHECK:        [[ptr_28:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[c_index_0]]
// CHECK:                             OpStore [[ptr_28]] [[c_word0_0]]
// CHECK: [[c_msb_index_0:%[0-9]+]] = OpIAdd %uint [[c_index_0]] %uint_1
// CHECK:        [[ptr_29:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[c_msb_index_0]]
// CHECK:                             OpStore [[ptr_29]] [[c_word1_0]]
// CHECK:    [[index_next:%[0-9]+]] = OpIAdd %uint [[c_msb_index_0]] %uint_1

//
// The third member of S starts at byte offset 32 (8 words)
//
// CHECK:         [[t_addr_0:%[0-9]+]] = OpIAdd %uint [[s1_addr]] %uint_32
//
// CHECK:              [[t_0:%[0-9]+]] = OpCompositeExtract %T [[s1]] 2
// CHECK:              [[x_1:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_5 [[t_0]] 0
// CHECK:             [[x0_1:%[0-9]+]] = OpCompositeExtract %half [[x_1]] 0
// CHECK:             [[x1_1:%[0-9]+]] = OpCompositeExtract %half [[x_1]] 1
// CHECK:             [[x2_1:%[0-9]+]] = OpCompositeExtract %half [[x_1]] 2
// CHECK:             [[x3_1:%[0-9]+]] = OpCompositeExtract %half [[x_1]] 3
// CHECK:             [[x4_1:%[0-9]+]] = OpCompositeExtract %half [[x_1]] 4
// CHECK:         [[x0_u16_0:%[0-9]+]] = OpBitcast %ushort [[x0_1]]
// CHECK:         [[x0_u32_0:%[0-9]+]] = OpUConvert %uint [[x0_u16_0]]
// CHECK:        [[x1_addr_1:%[0-9]+]] = OpIAdd %uint [[t_addr_0]] %uint_2
// CHECK:       [[x1_index_1:%[0-9]+]] = OpShiftRightLogical %uint [[x1_addr_1]] %uint_2
// CHECK:     [[byteOffset_22:%[0-9]+]] = OpUMod %uint [[x1_addr_1]] %uint_4
// CHECK:      [[bitOffset_22:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_22]] %uint_3
// CHECK:            [[ptr_30:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x1_index_1]]
// CHECK:         [[x1_u16_0:%[0-9]+]] = OpBitcast %ushort [[x1_1]]
// CHECK:         [[x1_u32_0:%[0-9]+]] = OpUConvert %uint [[x1_u16_0]]
// CHECK: [[x1_u32_shifted_1:%[0-9]+]] = OpShiftLeftLogical %uint [[x1_u32_0]] [[bitOffset_22]]
// CHECK:     [[maskOffset_22:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_22]]
// CHECK:           [[mask_22:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_22]]
// CHECK:        [[oldWord_22:%[0-9]+]] = OpLoad %uint [[ptr_30]]
// CHECK:         [[masked_22:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_22]] [[mask_22]]
// CHECK:           [[word_21:%[0-9]+]] = OpBitwiseOr %uint [[masked_22]] [[x1_u32_shifted_1]]
// CHECK:                           OpStore [[ptr_30]] [[word_21]]
// CHECK:        [[x2_addr_1:%[0-9]+]] = OpIAdd %uint [[x1_addr_1]] %uint_2
// CHECK:         [[x2_u16_0:%[0-9]+]] = OpBitcast %ushort [[x2_1]]
// CHECK:         [[x2_u32_0:%[0-9]+]] = OpUConvert %uint [[x2_u16_0]]
// CHECK:        [[x3_addr_1:%[0-9]+]] = OpIAdd %uint [[x2_addr_1]] %uint_2
// CHECK:       [[x3_index_1:%[0-9]+]] = OpShiftRightLogical %uint [[x3_addr_1]] %uint_2
// CHECK:     [[byteOffset_23:%[0-9]+]] = OpUMod %uint [[x3_addr_1]] %uint_4
// CHECK:      [[bitOffset_23:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_23]] %uint_3
// CHECK:            [[ptr_31:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x3_index_1]]
// CHECK:         [[x3_u16_0:%[0-9]+]] = OpBitcast %ushort [[x3_1]]
// CHECK:         [[x3_u32_0:%[0-9]+]] = OpUConvert %uint [[x3_u16_0]]
// CHECK: [[x3_u32_shifted_1:%[0-9]+]] = OpShiftLeftLogical %uint [[x3_u32_0]] [[bitOffset_23]]
// CHECK:     [[maskOffset_23:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_23]]
// CHECK:           [[mask_23:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_23]]
// CHECK:        [[oldWord_23:%[0-9]+]] = OpLoad %uint [[ptr_31]]
// CHECK:         [[masked_23:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_23]] [[mask_23]]
// CHECK:           [[word_22:%[0-9]+]] = OpBitwiseOr %uint [[masked_23]] [[x3_u32_shifted_1]]
// CHECK:                           OpStore [[ptr_31]] [[word_22]]
// CHECK:        [[x4_addr_1:%[0-9]+]] = OpIAdd %uint [[x3_addr_1]] %uint_2
// CHECK:       [[x4_index_1:%[0-9]+]] = OpShiftRightLogical %uint [[x4_addr_1]] %uint_2
// CHECK:     [[byteOffset_24:%[0-9]+]] = OpUMod %uint [[x4_addr_1]] %uint_4
// CHECK:      [[bitOffset_24:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_24]] %uint_3
// CHECK:            [[ptr_32:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x4_index_1]]
// CHECK:         [[x4_u16_0:%[0-9]+]] = OpBitcast %ushort [[x4_1]]
// CHECK:         [[x4_u32_0:%[0-9]+]] = OpUConvert %uint [[x4_u16_0]]
// CHECK: [[x4_u32_shifted_1:%[0-9]+]] = OpShiftLeftLogical %uint [[x4_u32_0]] [[bitOffset_24]]
// CHECK:     [[maskOffset_24:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_24]]
// CHECK:           [[mask_24:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_24]]
// CHECK:        [[oldWord_24:%[0-9]+]] = OpLoad %uint [[ptr_32]]
// CHECK:         [[masked_24:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_24]] [[mask_24]]
// CHECK:           [[word_23:%[0-9]+]] = OpBitwiseOr %uint [[masked_24]] [[x4_u32_shifted_1]]
// CHECK:                           OpStore [[ptr_32]] [[word_23]]

//
// The fourth member of S starts at byte offset 48 (12 words)
//
// CHECK:      [[b_addr_0:%[0-9]+]] = OpIAdd %uint [[s1_addr]] %uint_48
//
// CHECK:           [[b_0:%[0-9]+]] = OpCompositeExtract %double [[s1]] 3
// CHECK:         [[merge:%[0-9]+]] = OpBitcast %v2uint [[b_0]]
// CHECK:     [[b_word0_0:%[0-9]+]] = OpCompositeExtract %uint [[merge]] 0
// CHECK:     [[b_word1_0:%[0-9]+]] = OpCompositeExtract %uint [[merge]] 1
// CHECK:     [[b_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[b_addr_0]] %uint_2
// CHECK:        [[ptr_33:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[b_index_0]]
// CHECK:                             OpStore [[ptr_33]] [[b_word0_0]]
// CHECK: [[b_msb_index_0:%[0-9]+]] = OpIAdd %uint [[b_index_0]] %uint_1
// CHECK:        [[ptr_34:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[b_msb_index_0]]
// CHECK:                             OpStore [[ptr_34]] [[b_word1_0]]
// CHECK:    [[next_index:%[0-9]+]] = OpIAdd %uint [[b_msb_index_0]] %uint_1

//
// The fifth member of S starts at byte offset 56 (14 words)
//
// CHECK:        [[d_addr_0:%[0-9]+]] = OpIAdd %uint [[s1_addr]] %uint_56
//
// CHECK:             [[d_0:%[0-9]+]] = OpCompositeExtract %half [[s1]] 4
// CHECK:       [[d_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[d_addr_0]] %uint_2
// CHECK:    [[byteOffset_25:%[0-9]+]] = OpUMod %uint [[d_addr_0]] %uint_4
// CHECK:     [[bitOffset_25:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_25]] %uint_3
// CHECK:           [[ptr_35:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[d_index_0]]
// CHECK:         [[d_u16_0:%[0-9]+]] = OpBitcast %ushort [[d_0]]
// CHECK:         [[d_u32_0:%[0-9]+]] = OpUConvert %uint [[d_u16_0]]
// CHECK: [[d_u32_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[d_u32_0]] [[bitOffset_25]]
// CHECK:    [[maskOffset_25:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_25]]
// CHECK:          [[mask_25:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_25]]
// CHECK:       [[oldWord_25:%[0-9]+]] = OpLoad %uint [[ptr_35]]
// CHECK:        [[masked_25:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_25]] [[mask_25]]
// CHECK:          [[word_24:%[0-9]+]] = OpBitwiseOr %uint [[masked_25]] [[d_u32_shifted_0]]
// CHECK:                          OpStore [[ptr_35]] [[word_24]]

//
// The sixth member of S starts at byte offset 58 (14 words + 16bit offset)
// This is an extraordinary case of alignment. Since the sixth member only
// contains fp16, and the fifth member was also fp16, DX packs them tightly.
// As a result, store must occur at non-aligned offset.
// e[0] takes the following byte offsets: 58, 60, 62, 64, 66.
// e[1] takes the following byte offsets: 68, 70, 72, 74, 76.
// (60-64 = index 15. 64-68 = index 16)
// (68-72 = index 17. 72-76 = index 18)
// (76-78 = first half of index 19)
//
// CHECK:        [[e_addr_0:%[0-9]+]] = OpIAdd %uint [[s1_addr]] %uint_58
// CHECK:             [[e_0:%[0-9]+]] = OpCompositeExtract %_arr_T_uint_2 [[s1]] 5
// CHECK:            [[e0_0:%[0-9]+]] = OpCompositeExtract %T [[e_0]] 0
// CHECK:            [[e1_0:%[0-9]+]] = OpCompositeExtract %T [[e_0]] 1
// CHECK:             [[x_2:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_5 [[e0_0]] 0
// CHECK:            [[x0_2:%[0-9]+]] = OpCompositeExtract %half [[x_2]] 0
// CHECK:            [[x1_2:%[0-9]+]] = OpCompositeExtract %half [[x_2]] 1
// CHECK:            [[x2_2:%[0-9]+]] = OpCompositeExtract %half [[x_2]] 2
// CHECK:            [[x3_2:%[0-9]+]] = OpCompositeExtract %half [[x_2]] 3
// CHECK:            [[x4_2:%[0-9]+]] = OpCompositeExtract %half [[x_2]] 4
// CHECK:      [[x0_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[e_addr_0]] %uint_2
// CHECK:    [[byteOffset_26:%[0-9]+]] = OpUMod %uint [[e_addr_0]] %uint_4
// CHECK:     [[bitOffset_26:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_26]] %uint_3
// CHECK:           [[ptr_36:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x0_index_0]]
// CHECK:         [[x0u16_0:%[0-9]+]] = OpBitcast %ushort [[x0_2]]
// CHECK:         [[x0u32_0:%[0-9]+]] = OpUConvert %uint [[x0u16_0]]
// CHECK: [[x0u32_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[x0u32_0]] [[bitOffset_26]]
// CHECK:    [[maskOffset_26:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_26]]
// CHECK:          [[mask_26:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_26]]
// CHECK:       [[oldWord_26:%[0-9]+]] = OpLoad %uint [[ptr_36]]
// CHECK:        [[masked_26:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_26]] [[mask_26]]
// CHECK:       [[newWord_0:%[0-9]+]] = OpBitwiseOr %uint [[masked_26]] [[x0u32_shifted_0]]
// CHECK:                          OpStore [[ptr_36]] [[newWord_0]]

// CHECK:       [[x1_addr_2:%[0-9]+]] = OpIAdd %uint [[e_addr_0]] %uint_2
// CHECK:      [[x1_index_2:%[0-9]+]] = OpShiftRightLogical %uint [[x1_addr_2]] %uint_2
// CHECK:    [[byteOffset_27:%[0-9]+]] = OpUMod %uint [[x1_addr_2]] %uint_4
// CHECK:     [[bitOffset_27:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_27]] %uint_3
// CHECK:           [[ptr_37:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x1_index_2]]
// CHECK:         [[x1u16_0:%[0-9]+]] = OpBitcast %ushort [[x1_2]]
// CHECK:         [[x1u32_0:%[0-9]+]] = OpUConvert %uint [[x1u16_0]]
// CHECK:[[x1_u32_shifted_2:%[0-9]+]] = OpShiftLeftLogical %uint [[x1u32_0]] [[bitOffset_27]]
// CHECK:    [[maskOffset_27:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_27]]
// CHECK:          [[mask_27:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_27]]
// CHECK:       [[oldWord_27:%[0-9]+]] = OpLoad %uint [[ptr_37]]
// CHECK:        [[masked_27:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_27]] [[mask_27]]
// CHECK:          [[word_25:%[0-9]+]] = OpBitwiseOr %uint [[masked_27]] [[x1_u32_shifted_2]]
// CHECK:                          OpStore [[ptr_37]] [[word_25]]

// CHECK:       [[x2_addr_2:%[0-9]+]] = OpIAdd %uint [[x1_addr_2]] %uint_2
// CHECK:         [[x2u16_0:%[0-9]+]] = OpBitcast %ushort [[x2_2]]
// CHECK:         [[x2u32_0:%[0-9]+]] = OpUConvert %uint [[x2u16_0]]
// CHECK:       [[x3_addr_2:%[0-9]+]] = OpIAdd %uint [[x2_addr_2]] %uint_2
// CHECK:      [[x3_index_2:%[0-9]+]] = OpShiftRightLogical %uint [[x3_addr_2]] %uint_2
// CHECK:    [[byteOffset_28:%[0-9]+]] = OpUMod %uint [[x3_addr_2]] %uint_4
// CHECK:     [[bitOffset_28:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_28]] %uint_3
// CHECK:           [[ptr_38:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x3_index_2]]
// CHECK:         [[x3u16_0:%[0-9]+]] = OpBitcast %ushort [[x3_2]]
// CHECK:         [[x3u32_0:%[0-9]+]] = OpUConvert %uint [[x3u16_0]]
// CHECK:[[x3_u32_shifted_2:%[0-9]+]] = OpShiftLeftLogical %uint [[x3u32_0]] [[bitOffset_28]]
// CHECK:    [[maskOffset_28:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_28]]
// CHECK:          [[mask_28:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_28]]
// CHECK:       [[oldWord_28:%[0-9]+]] = OpLoad %uint [[ptr_38]]
// CHECK:        [[masked_28:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_28]] [[mask_28]]
// CHECK:          [[word_26:%[0-9]+]] = OpBitwiseOr %uint [[masked_28]] [[x3_u32_shifted_2]]
// CHECK:                          OpStore [[ptr_38]] [[word_26]]

// CHECK:       [[x4_addr_2:%[0-9]+]] = OpIAdd %uint [[x3_addr_2]] %uint_2
// CHECK:      [[x4_index_2:%[0-9]+]] = OpShiftRightLogical %uint [[x4_addr_2]] %uint_2
// CHECK:    [[byteOffset_29:%[0-9]+]] = OpUMod %uint [[x4_addr_2]] %uint_4
// CHECK:     [[bitOffset_29:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_29]] %uint_3
// CHECK:           [[ptr_39:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[x4_index_2]]
// CHECK:         [[x4u16_0:%[0-9]+]] = OpBitcast %ushort [[x4_2]]
// CHECK:         [[x4u32_0:%[0-9]+]] = OpUConvert %uint [[x4u16_0]]
// CHECK:[[x4_u32_shifted_2:%[0-9]+]] = OpShiftLeftLogical %uint [[x4u32_0]] [[bitOffset_29]]
// CHECK:    [[maskOffset_29:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_29]]
// CHECK:          [[mask_29:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_29]]
// CHECK:       [[oldWord_29:%[0-9]+]] = OpLoad %uint [[ptr_39]]
// CHECK:        [[masked_29:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_29]] [[mask_29]]
// CHECK:          [[word_27:%[0-9]+]] = OpBitwiseOr %uint [[masked_29]] [[x4_u32_shifted_2]]
// CHECK:                          OpStore [[ptr_39]] [[word_27]]

//
// The seventh member of S starts at byte offset 80 (20 words), so:
// for f[0]:
// v should start at byte offset 80 (20 words)
// w should start at byte offset 88 (22 words)
// for f[1]:
// v should start at byte offset 92 (23 words)
// w should start at byte offset 100 (25 words)
//
// CHECK:        [[f_addr_0:%[0-9]+]] = OpIAdd %uint [[s1_addr]] %uint_80
// CHECK:             [[f_0:%[0-9]+]] = OpCompositeExtract %_arr_U_uint_2 [[s1]] 6
// CHECK:            [[u0_0:%[0-9]+]] = OpCompositeExtract %U [[f_0]] 0
// CHECK:            [[u1_0:%[0-9]+]] = OpCompositeExtract %U [[f_0]] 1
// CHECK:             [[v_1:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_3 [[u0_0]] 0
// CHECK:            [[v0_1:%[0-9]+]] = OpCompositeExtract %half [[v_1]] 0
// CHECK:            [[v1_1:%[0-9]+]] = OpCompositeExtract %half [[v_1]] 1
// CHECK:            [[v2_1:%[0-9]+]] = OpCompositeExtract %half [[v_1]] 2
// CHECK:         [[v0u16_1:%[0-9]+]] = OpBitcast %ushort [[v0_1]]
// CHECK:         [[v0u32_1:%[0-9]+]] = OpUConvert %uint [[v0u16_1]]
// CHECK:       [[v1_addr_1:%[0-9]+]] = OpIAdd %uint [[f_addr_0]] %uint_2
// CHECK:      [[v1_index_1:%[0-9]+]] = OpShiftRightLogical %uint [[v1_addr_1]] %uint_2
// CHECK:    [[byteOffset_30:%[0-9]+]] = OpUMod %uint [[v1_addr_1]] %uint_4
// CHECK:     [[bitOffset_30:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_30]] %uint_3
// CHECK:           [[ptr_40:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[v1_index_1]]
// CHECK:         [[v1u16_1:%[0-9]+]] = OpBitcast %ushort [[v1_1]]
// CHECK:         [[v1u32_1:%[0-9]+]] = OpUConvert %uint [[v1u16_1]]
// CHECK: [[v1u32_shifted_1:%[0-9]+]] = OpShiftLeftLogical %uint [[v1u32_1]] [[bitOffset_30]]
// CHECK:    [[maskOffset_30:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_30]]
// CHECK:          [[mask_30:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_30]]
// CHECK:       [[oldWord_30:%[0-9]+]] = OpLoad %uint [[ptr_40]]
// CHECK:        [[masked_30:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_30]] [[mask_30]]
// CHECK:          [[word_28:%[0-9]+]] = OpBitwiseOr %uint [[masked_30]] [[v1u32_shifted_1]]
// CHECK:                          OpStore [[ptr_40]] [[word_28]]

// CHECK:       [[v2_addr_1:%[0-9]+]] = OpIAdd %uint [[v1_addr_1]] %uint_2
// CHECK:      [[v2_index_1:%[0-9]+]] = OpShiftRightLogical %uint [[v2_addr_1]] %uint_2
// CHECK:    [[byteOffset_31:%[0-9]+]] = OpUMod %uint [[v2_addr_1]] %uint_4
// CHECK:     [[bitOffset_31:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_31]] %uint_3
// CHECK:           [[ptr_41:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[v2_index_1]]
// CHECK:         [[v2u16_1:%[0-9]+]] = OpBitcast %ushort [[v2_1]]
// CHECK:         [[v2u32_1:%[0-9]+]] = OpUConvert %uint [[v2u16_1]]
// CHECK: [[v2u32_shifted_1:%[0-9]+]] = OpShiftLeftLogical %uint [[v2u32_1]] [[bitOffset_31]]
// CHECK:    [[maskOffset_31:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_31]]
// CHECK:          [[mask_31:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_31]]
// CHECK:       [[oldWord_31:%[0-9]+]] = OpLoad %uint [[ptr_41]]
// CHECK:        [[masked_31:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_31]] [[mask_31]]
// CHECK:          [[word_29:%[0-9]+]] = OpBitwiseOr %uint [[masked_31]] [[v2u32_shifted_1]]
// CHECK:                          OpStore [[ptr_41]] [[word_29]]

// CHECK:    [[w_addr_1:%[0-9]+]] = OpIAdd %uint [[f_addr_0]] %uint_8
// CHECK:         [[w_1:%[0-9]+]] = OpCompositeExtract %uint [[u0_0]] 1
// CHECK:   [[w_index_1:%[0-9]+]] = OpShiftRightLogical %uint [[w_addr_1]] %uint_2
// CHECK:       [[ptr_42:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[w_index_1]]
// CHECK:                      OpStore [[ptr_42]] [[w_1]]

// CHECK:       [[f1_addr_0:%[0-9]+]] = OpIAdd %uint [[f_addr_0]] %uint_12
// CHECK:             [[v_2:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_3 [[u1_0]] 0
// CHECK:            [[v0_2:%[0-9]+]] = OpCompositeExtract %half [[v_2]] 0
// CHECK:            [[v1_2:%[0-9]+]] = OpCompositeExtract %half [[v_2]] 1
// CHECK:            [[v2_2:%[0-9]+]] = OpCompositeExtract %half [[v_2]] 2
// CHECK:         [[v0u16_2:%[0-9]+]] = OpBitcast %ushort [[v0_2]]
// CHECK:         [[v0u32_2:%[0-9]+]] = OpUConvert %uint [[v0u16_2]]
// CHECK:       [[v1_addr_2:%[0-9]+]] = OpIAdd %uint [[f1_addr_0]] %uint_2
// CHECK:      [[v1_index_2:%[0-9]+]] = OpShiftRightLogical %uint [[v1_addr_2]] %uint_2
// CHECK:    [[byteOffset_32:%[0-9]+]] = OpUMod %uint [[v1_addr_2]] %uint_4
// CHECK:     [[bitOffset_32:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_32]] %uint_3
// CHECK:           [[ptr_43:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[v1_index_2]]
// CHECK:         [[v1u16_2:%[0-9]+]] = OpBitcast %ushort [[v1_2]]
// CHECK:         [[v1u32_2:%[0-9]+]] = OpUConvert %uint [[v1u16_2]]
// CHECK: [[v1u32_shifted_2:%[0-9]+]] = OpShiftLeftLogical %uint [[v1u32_2]] [[bitOffset_32]]
// CHECK:    [[maskOffset_32:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_32]]
// CHECK:          [[mask_32:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_32]]
// CHECK:       [[oldWord_32:%[0-9]+]] = OpLoad %uint [[ptr_43]]
// CHECK:        [[masked_32:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_32]] [[mask_32]]
// CHECK:          [[word_30:%[0-9]+]] = OpBitwiseOr %uint [[masked_32]] [[v1u32_shifted_2]]
// CHECK:                          OpStore [[ptr_43]] [[word_30]]

// CHECK:       [[v2_addr_2:%[0-9]+]] = OpIAdd %uint [[v1_addr_2]] %uint_2
// CHECK:      [[v2_index_2:%[0-9]+]] = OpShiftRightLogical %uint [[v2_addr_2]] %uint_2
// CHECK:    [[byteOffset_33:%[0-9]+]] = OpUMod %uint [[v2_addr_2]] %uint_4
// CHECK:     [[bitOffset_33:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_33]] %uint_3
// CHECK:           [[ptr_44:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[v2_index_2]]
// CHECK:         [[v2u16_2:%[0-9]+]] = OpBitcast %ushort [[v2_2]]
// CHECK:         [[v2u32_2:%[0-9]+]] = OpUConvert %uint [[v2u16_2]]
// CHECK: [[v2u32_shifted_2:%[0-9]+]] = OpShiftLeftLogical %uint [[v2u32_2]] [[bitOffset_33]]
// CHECK:    [[maskOffset_33:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_33]]
// CHECK:          [[mask_33:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_33]]
// CHECK:       [[oldWord_33:%[0-9]+]] = OpLoad %uint [[ptr_44]]
// CHECK:        [[masked_33:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_33]] [[mask_33]]
// CHECK:          [[word_31:%[0-9]+]] = OpBitwiseOr %uint [[masked_33]] [[v2u32_shifted_2]]
// CHECK:                          OpStore [[ptr_44]] [[word_31]]

// CHECK:        [[w_addr_2:%[0-9]+]] = OpIAdd %uint [[f1_addr_0]] %uint_8
// CHECK:             [[w_2:%[0-9]+]] = OpCompositeExtract %uint [[u1_0]] 1
// CHECK:       [[w_index_2:%[0-9]+]] = OpShiftRightLogical %uint [[w_addr_2]] %uint_2
// CHECK:           [[ptr_45:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[w_index_2]]
// CHECK:                          OpStore [[ptr_45]] [[w_2]]

//
// The eighth member of S starts at byte offset 104 (26 words)
//
// CHECK:       [[z_addr_0:%[0-9]+]] = OpIAdd %uint [[s1_addr]] %uint_104
// CHECK:            [[z_0:%[0-9]+]] = OpCompositeExtract %half [[s1]] 7
// CHECK:      [[z_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[z_addr_0]] %uint_2
// CHECK:   [[byteOffset_34:%[0-9]+]] = OpUMod %uint [[z_addr_0]] %uint_4
// CHECK:    [[bitOffset_34:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_34]] %uint_3
// CHECK:          [[ptr_46:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[z_index_0]]
// CHECK:         [[zu16_0:%[0-9]+]] = OpBitcast %ushort [[z_0]]
// CHECK:         [[zu32_0:%[0-9]+]] = OpUConvert %uint [[zu16_0]]
// CHECK: [[zu32_shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint [[zu32_0]] [[bitOffset_34]]
// CHECK:   [[maskOffset_34:%[0-9]+]] = OpISub %uint %uint_16 [[bitOffset_34]]
// CHECK:         [[mask_34:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOffset_34]]
// CHECK:      [[oldWord_34:%[0-9]+]] = OpLoad %uint [[ptr_46]]
// CHECK:       [[masked_34:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord_34]] [[mask_34]]
// CHECK:         [[word_32:%[0-9]+]] = OpBitwiseOr %uint [[masked_34]] [[zu32_shifted_0]]
// CHECK:                         OpStore [[ptr_46]] [[word_32]]
