// RUN: %dxc -T cs_6_2 -E main -enable-16bit-types -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

ByteAddressBuffer buf;
RWByteAddressBuffer buf2;

struct S {
  float16_t3 a[3];
  float b;
  double c;
  float16_t d;
};

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadId) {
  S sArr[2] = buf.Load<S[2]>(tid.x);
}

// Here is the DXIL output with the load offsets:
//
// define void @main() {
//   COMMENT: Load at address tid.x (offset = 0):
//   %RawBufferLoad36 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %1, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: Load at address tid.x + 6 (offset = 6 bytes):
//   %5 = add i32 %1, 6
//   %RawBufferLoad35 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %5, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: Load at address tid.x + 12 (offset = 12 bytes):
//   %9 = add i32 %1, 12
//   %RawBufferLoad = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %9, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: Load at address tid.x + 20 (offset = 20 bytes):
//   %13 = add i32 %1, 20
//   %RawBufferLoad45 = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %13, i32 undef, i8 1, i32 4)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: Load at address tid.x + 24 (offset = 24 bytes):
//   %15 = add i32 %1, 24
//   %16 = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %15, i32 undef, i8 3, i32 8)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: Load at address tid.x + 32 (offset = 32 bytes):
//   %20 = add i32 %1, 32
//   %RawBufferLoad43 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %20, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: The load of the first struct in sArr has finished.
//   COMMENT: Padding must be applied until we reach the struct alignment.
//   COMMENT: The second struct in sArr starts at offset 40.
//   COMMENT: Load at address tid.x + 40 (offset = 40 bytes):
//   %22 = add i32 %1, 40
//   %RawBufferLoad39 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %22, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: Load at address tid.x + 46 (offset = 46 bytes):
//   %26 = add i32 %1, 46
//   %RawBufferLoad38 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %26, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: Load at address tid.x + 52 (offset = 52 bytes):
//   %30 = add i32 %1, 52
//   %RawBufferLoad37 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %30, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: Load at address tid.x + 60 (offset = 60 bytes):
//   %34 = add i32 %1, 60
//   %RawBufferLoad42 = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %34, i32 undef, i8 1, i32 4)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: Load at address tid.x + 64 (offset = 64 bytes):
//   %36 = add i32 %1, 64
//   %37 = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %36, i32 undef, i8 3, i32 8)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
//   COMMENT: Load at address tid.x + 72 (offset = 72 bytes):
//   %41 = add i32 %1, 72
//   %RawBufferLoad40 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %41, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   ...
// }


// CHECK: [[index_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr0:%[0-9]+]] %uint_2
// CHECK:     [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_0]]
// CHECK:   [[addr1:%[0-9]+]] = OpIAdd %uint [[addr0]] %uint_2
// CHECK: [[index_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr1]] %uint_2
// CHECK:     [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1]]
// CHECK:   [[addr2:%[0-9]+]] = OpIAdd %uint [[addr1]] %uint_2
// CHECK: [[index_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr2]] %uint_2
// CHECK:     [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1]]
// CHECK:   [[addr3:%[0-9]+]] = OpIAdd %uint [[addr2]] %uint_2
// CHECK:   [[aVec0:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK: [[index_1_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr3]] %uint_2
// CHECK:     [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_0]]
// CHECK:   [[addr4:%[0-9]+]] = OpIAdd %uint [[addr3]] %uint_2
// CHECK: [[index_2:%[0-9]+]] = OpShiftRightLogical %uint [[addr4]] %uint_2
// CHECK:     [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2]]
// CHECK:   [[addr5:%[0-9]+]] = OpIAdd %uint [[addr4]] %uint_2
// CHECK: [[index_2_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr5]] %uint_2
// CHECK:     [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2_0]]
// CHECK:   [[addr6:%[0-9]+]] = OpIAdd %uint [[addr5]] %uint_2
// CHECK:   [[aVec1:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK: [[index_3:%[0-9]+]] = OpShiftRightLogical %uint [[addr6]] %uint_2
// CHECK:     [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3]]
// CHECK:   [[addr7:%[0-9]+]] = OpIAdd %uint [[addr6]] %uint_2
// CHECK: [[index_3_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr7]] %uint_2
// CHECK:     [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3_0]]
// CHECK:   [[addr8:%[0-9]+]] = OpIAdd %uint [[addr7]] %uint_2
// CHECK: [[index_4:%[0-9]+]] = OpShiftRightLogical %uint [[addr8]] %uint_2
// CHECK:     [[ptr_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_4]]
// CHECK:   [[aVec2:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK:       [[a:%[0-9]+]] = OpCompositeConstruct %_arr_v3half_uint_3 [[aVec0]] [[aVec1]] [[aVec2]]
//
// COMMENT: Going to start loading 'b'. Must start at byte offset 20
// COMMENT: 20 byte offset is equivalent to 5 words offset (32-bit words).
// COMMENT: 'b' is 32-bit wide, so it will consume 1 word.
//
// CHECK:    [[addr:%[0-9]+]] = OpIAdd %uint [[addr0]] %uint_20
// CHECK: [[index_5:%[0-9]+]] = OpShiftRightLogical %uint [[addr]] %uint_2
// CHECK:     [[ptr_8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_5]]
//
// COMMENT: Going to start loading 'c'. Must start at byte offset 24
// COMMENT: 24 byte offset is equivalent to 6 words offset (32-bit words).
// COMMENT: 'c' is 64-bit wide, so it will consume 2 words.
//
// CHECK:    [[addr_0:%[0-9]+]] = OpIAdd %uint [[addr0]] %uint_24
// CHECK: [[index_6:%[0-9]+]] = OpShiftRightLogical %uint [[addr_0]] %uint_2
// CHECK:     [[ptr_9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_6]]
// CHECK: [[index_7:%[0-9]+]] = OpIAdd %uint [[index_6]] %uint_1
// CHECK:     [[ptr_10:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_7]]
//
// COMMENT: Going to start loading 'd'. Must start at byte offset 32.
// COMMENT: 32 byte offset is equivalent to 8 words offset (32-bit words).
// COMMENT: 'd' is 16-bit wide, so it will consume half of one word.
//
// CHECK:     [[addr_1:%[0-9]+]] = OpIAdd %uint [[addr0]] %uint_32
// CHECK:  [[index_8:%[0-9]+]] = OpShiftRightLogical %uint [[addr_1]] %uint_2
// CHECK:      [[ptr_11:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_8]]
// CHECK:  [[newAddr:%[0-9]+]] = OpIAdd %uint [[addr0]] %uint_40
// CHECK:      [[s_0:%[0-9]+]] = OpCompositeConstruct %S
//
// COMMENT: Going to start loading the second struct in sArr.
// COMMENT: The structure is 8-byte aligned (due to the existence of 'double')
// COMMENT: The last struct in the array ended at offset 34.
// COMMENT: We need to add padding up to offset 40 (to be 8-byte aligned).
// COMMENT: 40 byte offset is equivalent to 10 words offset (32-bit words).
//
// CHECK: [[index_10:%[0-9]+]] = OpShiftRightLogical %uint [[newAddr]] %uint_2
// CHECK:      [[ptr_12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_10]]
// CHECK:    [[addr1_0:%[0-9]+]] = OpIAdd %uint [[newAddr]] %uint_2
// CHECK: [[index_10_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr1_0]] %uint_2
// CHECK:      [[ptr_13:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_10_0]]
// CHECK:    [[addr2_0:%[0-9]+]] = OpIAdd %uint [[addr1_0]] %uint_2
// CHECK: [[index_11:%[0-9]+]] = OpShiftRightLogical %uint [[addr2_0]] %uint_2
// CHECK:      [[ptr_14:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_11]]
// CHECK:    [[addr3_0:%[0-9]+]] = OpIAdd %uint [[addr2_0]] %uint_2
// CHECK:    [[aVec0_0:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK: [[index_11_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr3_0]] %uint_2
// CHECK:      [[ptr_15:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_11_0]]
// CHECK:    [[addr4_0:%[0-9]+]] = OpIAdd %uint [[addr3_0]] %uint_2
// CHECK: [[index_12:%[0-9]+]] = OpShiftRightLogical %uint [[addr4_0]] %uint_2
// CHECK:      [[ptr_16:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_12]]
// CHECK:    [[addr5_0:%[0-9]+]] = OpIAdd %uint [[addr4_0]] %uint_2
// CHECK: [[index_12_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr5_0]] %uint_2
// CHECK:      [[ptr_17:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_12_0]]
// CHECK:    [[addr6_0:%[0-9]+]] = OpIAdd %uint [[addr5_0]] %uint_2
// CHECK:    [[aVec1_0:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK: [[index_13:%[0-9]+]] = OpShiftRightLogical %uint [[addr6_0]] %uint_2
// CHECK:      [[ptr_18:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_13]]
// CHECK:    [[addr7_0:%[0-9]+]] = OpIAdd %uint [[addr6_0]] %uint_2
// CHECK: [[index_13_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr7_0]] %uint_2
// CHECK:      [[ptr_19:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_13_0]]
// CHECK:    [[addr8_0:%[0-9]+]] = OpIAdd %uint [[addr7_0]] %uint_2
// CHECK: [[index_14:%[0-9]+]] = OpShiftRightLogical %uint [[addr8_0]] %uint_2
// CHECK:      [[ptr_20:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_14]]
// CHECK:    [[aVec2_0:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK:        [[a_0:%[0-9]+]] = OpCompositeConstruct %_arr_v3half_uint_3 [[aVec0_0]] [[aVec1_0]] [[aVec2_0]]
//
// COMMENT: Similar to the load of the first struct, member 'b' is at offset 20
// COMMENT: from the beginning of the struct. The second struct starts at offset 40
// COMMENT: Therefore the second struct's 'b' starts at offset 60.
// COMMENT: 60 byte offset is equivalent to 15 words offset (32-bit words).
//
// CHECK: [[addr_2:%[0-9]+]] = OpIAdd %uint [[newAddr]] %uint_20
// CHECK: [[index_15:%[0-9]+]] = OpShiftRightLogical %uint [[addr_2]] %uint_2
// CHECK:      [[ptr_21:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_15]]
//
// COMMENT: member 'c' must start at offset 40 + 24 = 64
// COMMENT: 64 byte offset is equivalent to 16 words offset (32-bit words).
// COMMENT: 'c' is 64-bit wide, so it will consume 2 words.
//
// CHECK: [[addr_3:%[0-9]+]] = OpIAdd %uint [[newAddr]] %uint_24
// CHECK: [[index_16:%[0-9]+]] = OpShiftRightLogical %uint [[addr_3]] %uint_2
// CHECK:      [[ptr_22:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_16]]
// CHECK: [[index_17:%[0-9]+]] = OpIAdd %uint [[index_16]] %uint_1
// CHECK:      [[ptr_23:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_17]]
//
// COMMENT: member 'd' must start at offset 40 + 32 = 72
// COMMENT: 72 byte offset is equivalent to 18 words offset (32-bit words).
// COMMENT: 'c' is 16-bit wide, so it will consume half of one word.
//
// CHECK:     [[addr_4:%[0-9]+]] = OpIAdd %uint [[newAddr]] %uint_32
// CHECK: [[index_18:%[0-9]+]] = OpShiftRightLogical %uint [[addr_4]] %uint_2
// CHECK:      [[ptr_24:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_18]]
//
// COMMENT: Construct sArr[2]
//
// CHECK:                     OpCompositeConstruct %S
// CHECK:                     OpCompositeConstruct %_arr_S_uint_2
// CHECK:                     OpStore %sArr {{%[0-9]+}}

