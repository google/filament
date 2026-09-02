// RUN: %dxc -T vs_6_2 -E main -fcgl -enable-16bit-types %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'countbits' function can only operate on scalar or vector of uints.

void main() {
// CHECK: [[v4_32:%[0-9]+]] = OpConstantComposite %v4uint %uint_32 %uint_32 %uint_32 %uint_32

  uint16_t u16;
  uint32_t u32;
  uint64_t u64;

// CHECK:     [[tmp:%[0-9]+]] = OpLoad %ushort %u16
// CHECK:     [[ext:%[0-9]+]] = OpUConvert %uint [[tmp]]
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:    [[cast:%[0-9]+]] = OpUConvert %ushort [[res]]
// CHECK:                       OpStore %u16ru16 [[cast]]
  uint16_t u16ru16 = countbits(u16);

// CHECK:     [[tmp:%[0-9]+]] = OpLoad %ushort %u16
// CHECK:     [[ext:%[0-9]+]] = OpUConvert %uint [[tmp]]
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:                       OpStore %u32ru16 [[res]]
  uint32_t u32ru16 = countbits(u16);

// CHECK:     [[tmp:%[0-9]+]] = OpLoad %ushort %u16
// CHECK:     [[ext:%[0-9]+]] = OpUConvert %uint [[tmp]]
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:    [[cast:%[0-9]+]] = OpUConvert %ulong [[res]]
// CHECK:                       OpStore %u64ru16 [[cast]]
  uint64_t u64ru16 = countbits(u16);

// CHECK:     [[ext:%[0-9]+]] = OpLoad %uint %u32
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:    [[cast:%[0-9]+]] = OpUConvert %ushort [[res]]
// CHECK:                       OpStore %u16ru32 [[cast]]
  uint16_t u16ru32 = countbits(u32);
// CHECK:     [[ext:%[0-9]+]] = OpLoad %uint %u32
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:                       OpStore %u32ru32 [[res]]
  uint32_t u32ru32 = countbits(u32);
// CHECK:     [[ext:%[0-9]+]] = OpLoad %uint %u32
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:    [[cast:%[0-9]+]] = OpUConvert %ulong [[res]]
// CHECK:                       OpStore %u64ru32 [[cast]]
  uint64_t u64ru32 = countbits(u32);

// CHECK:         [[ld:%[0-9]+]] = OpLoad %ulong %u64
// CHECK-DAG:     [[lo:%[0-9]+]] = OpUConvert %uint [[ld]]
// CHECK-DAG:     [[sh:%[0-9]+]] = OpShiftRightLogical %ulong [[ld]] %uint_32
// CHECK-DAG:     [[hi:%[0-9]+]] = OpUConvert %uint [[sh]]
// CHECK-DAG:     [[ca:%[0-9]+]] = OpBitCount %uint [[lo]]
// CHECK-DAG:     [[cb:%[0-9]+]] = OpBitCount %uint [[hi]]
// CHECK-DAG:     [[re:%[0-9]+]] = OpIAdd %uint [[cb]] [[ca]]
// CHECK-DAG:   [[cast:%[0-9]+]] = OpUConvert %ushort [[re]]
// CHECK-DAG:                      OpStore %u16ru64 [[cast]]
  uint16_t u16ru64 = countbits(u64);

// CHECK:         [[ld:%[0-9]+]] = OpLoad %ulong %u64
// CHECK-DAG:     [[lo:%[0-9]+]] = OpUConvert %uint [[ld]]
// CHECK-DAG:     [[sh:%[0-9]+]] = OpShiftRightLogical %ulong [[ld]] %uint_32
// CHECK-DAG:     [[hi:%[0-9]+]] = OpUConvert %uint [[sh]]
// CHECK-DAG:     [[ca:%[0-9]+]] = OpBitCount %uint [[lo]]
// CHECK-DAG:     [[cb:%[0-9]+]] = OpBitCount %uint [[hi]]
// CHECK-DAG:     [[re:%[0-9]+]] = OpIAdd %uint [[cb]] [[ca]]
// CHECK-DAG:                      OpStore %u32ru64 [[re]]
  uint32_t u32ru64 = countbits(u64);

// CHECK:         [[ld:%[0-9]+]] = OpLoad %ulong %u64
// CHECK-DAG:     [[lo:%[0-9]+]] = OpUConvert %uint [[ld]]
// CHECK-DAG:     [[sh:%[0-9]+]] = OpShiftRightLogical %ulong [[ld]] %uint_32
// CHECK-DAG:     [[hi:%[0-9]+]] = OpUConvert %uint [[sh]]
// CHECK-DAG:     [[ca:%[0-9]+]] = OpBitCount %uint [[lo]]
// CHECK-DAG:     [[cb:%[0-9]+]] = OpBitCount %uint [[hi]]
// CHECK-DAG:     [[re:%[0-9]+]] = OpIAdd %uint [[cb]] [[ca]]
// CHECK-DAG:   [[cast:%[0-9]+]] = OpUConvert %ulong [[re]]
// CHECK-DAG:                      OpStore %u64ru64 [[cast]]
  uint64_t u64ru64 = countbits(u64);

  int16_t s16;
  int32_t s32;
  int64_t s64;

// CHECK:     [[tmp:%[0-9]+]] = OpLoad %short %s16
// CHECK:     [[ext:%[0-9]+]] = OpUConvert %uint [[tmp]]
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:    [[cast:%[0-9]+]] = OpUConvert %ushort [[res]]
// CHECK:      [[bc:%[0-9]+]] = OpBitcast %short [[cast]]
// CHECK:                       OpStore %s16rs16 [[bc]]
  int16_t s16rs16 = countbits(s16);

// CHECK:     [[tmp:%[0-9]+]] = OpLoad %short %s16
// CHECK:     [[ext:%[0-9]+]] = OpUConvert %uint [[tmp]]
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:      [[bc:%[0-9]+]] = OpBitcast %int [[res]]
// CHECK:                       OpStore %s32rs16 [[bc]]
  int32_t s32rs16 = countbits(s16);

// CHECK:     [[tmp:%[0-9]+]] = OpLoad %short %s16
// CHECK:     [[ext:%[0-9]+]] = OpUConvert %uint [[tmp]]
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:    [[cast:%[0-9]+]] = OpUConvert %ulong [[res]]
// CHECK:      [[bc:%[0-9]+]] = OpBitcast %long [[cast]]
// CHECK:                       OpStore %s64rs16 [[bc]]
  int64_t s64rs16 = countbits(s16);

// CHECK:     [[ext:%[0-9]+]] = OpLoad %int %s32
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:    [[cast:%[0-9]+]] = OpUConvert %ushort [[res]]
// CHECK:      [[bc:%[0-9]+]] = OpBitcast %short [[cast]]
// CHECK:                       OpStore %s16rs32 [[bc]]
  int16_t s16rs32 = countbits(s32);
// CHECK:     [[ext:%[0-9]+]] = OpLoad %int %s32
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:      [[bc:%[0-9]+]] = OpBitcast %int [[res]]
// CHECK:                       OpStore %s32rs32 [[bc]]
  int32_t s32rs32 = countbits(s32);
// CHECK:     [[ext:%[0-9]+]] = OpLoad %int %s32
// CHECK:     [[res:%[0-9]+]] = OpBitCount %uint [[ext]]
// CHECK:    [[cast:%[0-9]+]] = OpUConvert %ulong [[res]]
// CHECK:      [[bc:%[0-9]+]] = OpBitcast %long [[cast]]
// CHECK:                       OpStore %s64rs32 [[bc]]
  int64_t s64rs32 = countbits(s32);

// CHECK:         [[ld:%[0-9]+]] = OpLoad %long %s64
// CHECK-DAG:     [[lo:%[0-9]+]] = OpUConvert %uint [[ld]]
// CHECK-DAG:     [[sh:%[0-9]+]] = OpShiftRightLogical %long [[ld]] %uint_32
// CHECK-DAG:     [[hi:%[0-9]+]] = OpUConvert %uint [[sh]]
// CHECK-DAG:     [[ca:%[0-9]+]] = OpBitCount %uint [[lo]]
// CHECK-DAG:     [[cb:%[0-9]+]] = OpBitCount %uint [[hi]]
// CHECK-DAG:     [[re:%[0-9]+]] = OpIAdd %uint [[cb]] [[ca]]
// CHECK-DAG:   [[cast:%[0-9]+]] = OpUConvert %ushort [[re]]
// CHECK-DAG:     [[bc:%[0-9]+]] = OpBitcast %short [[cast]]
// CHECK-DAG:                      OpStore %s16rs64 [[bc]]
  int16_t s16rs64 = countbits(s64);

// CHECK:         [[ld:%[0-9]+]] = OpLoad %long %s64
// CHECK-DAG:     [[lo:%[0-9]+]] = OpUConvert %uint [[ld]]
// CHECK-DAG:     [[sh:%[0-9]+]] = OpShiftRightLogical %long [[ld]] %uint_32
// CHECK-DAG:     [[hi:%[0-9]+]] = OpUConvert %uint [[sh]]
// CHECK-DAG:     [[ca:%[0-9]+]] = OpBitCount %uint [[lo]]
// CHECK-DAG:     [[cb:%[0-9]+]] = OpBitCount %uint [[hi]]
// CHECK-DAG:     [[re:%[0-9]+]] = OpIAdd %uint [[cb]] [[ca]]
// CHECK-DAG:     [[bc:%[0-9]+]] = OpBitcast %int [[re]]
// CHECK-DAG:                      OpStore %s32rs64 [[bc]]
  int32_t s32rs64 = countbits(s64);

// CHECK:         [[ld:%[0-9]+]] = OpLoad %long %s64
// CHECK-DAG:     [[lo:%[0-9]+]] = OpUConvert %uint [[ld]]
// CHECK-DAG:     [[sh:%[0-9]+]] = OpShiftRightLogical %long [[ld]] %uint_32
// CHECK-DAG:     [[hi:%[0-9]+]] = OpUConvert %uint [[sh]]
// CHECK-DAG:     [[ca:%[0-9]+]] = OpBitCount %uint [[lo]]
// CHECK-DAG:     [[cb:%[0-9]+]] = OpBitCount %uint [[hi]]
// CHECK-DAG:     [[re:%[0-9]+]] = OpIAdd %uint [[cb]] [[ca]]
// CHECK-DAG:   [[cast:%[0-9]+]] = OpUConvert %ulong [[re]]
// CHECK-DAG:     [[bc:%[0-9]+]] = OpBitcast %long [[cast]]
// CHECK-DAG:                      OpStore %s64rs64 [[bc]]
  int64_t s64rs64 = countbits(s64);

  uint16_t4 vu16;
  uint32_t4 vu32;
  uint64_t4 vu64;

// CHECK:      [[tmp:%[0-9]+]] = OpLoad %v4ushort %vu16
// CHECK-DAG:  [[ext:%[0-9]+]] = OpUConvert %v4uint [[tmp]]
// CHECK-NEXT:     {{%[0-9]+}} = OpBitCount %v4uint [[ext]]
  uint4 rvu16 = countbits(vu16);

// CHECK:      [[tmp:%[0-9]+]] = OpLoad %v4uint %vu32
// CHECK-NEXT:     {{%[0-9]+}} = OpBitCount %v4uint [[tmp]]
  uint4 rvu32 = countbits(vu32);

// CHECK:         [[ld:%[0-9]+]] = OpLoad %v4ulong %vu64
// CHECK-DAG:     [[lo:%[0-9]+]] = OpUConvert %v4uint [[ld]]
// CHECK-DAG:     [[sh:%[0-9]+]] = OpShiftRightLogical %v4ulong [[ld]] [[v4_32]]
// CHECK-DAG:     [[hi:%[0-9]+]] = OpUConvert %v4uint [[sh]]
// CHECK-DAG:     [[ca:%[0-9]+]] = OpBitCount %v4uint [[lo]]
// CHECK-DAG:     [[cb:%[0-9]+]] = OpBitCount %v4uint [[hi]]
// CHECK-DAG:        {{%[0-9]+}} = OpIAdd %v4uint [[cb]] [[ca]]
  uint4 rvu64 = countbits(vu64);

  int16_t4 vs16;
  int32_t4 vs32;
  int64_t4 vs64;

// CHECK:      [[tmp:%[0-9]+]] = OpLoad %v4short %vs16
// CHECK-DAG:  [[ext:%[0-9]+]] = OpUConvert %v4uint [[tmp]]
// CHECK-NEXT:     {{%[0-9]+}} = OpBitCount %v4uint [[ext]]
  uint4 rvs16 = countbits(vs16);

// CHECK:      [[tmp:%[0-9]+]] = OpLoad %v4int %vs32
// CHECK-NEXT:     {{%[0-9]+}} = OpBitCount %v4uint [[tmp]]
  uint4 rvs32 = countbits(vs32);

// CHECK:         [[ld:%[0-9]+]] = OpLoad %v4long %vs64
// CHECK-DAG:     [[lo:%[0-9]+]] = OpUConvert %v4uint [[ld]]
// CHECK-DAG:     [[sh:%[0-9]+]] = OpShiftRightLogical %v4long [[ld]] [[v4_32]]
// CHECK-DAG:     [[hi:%[0-9]+]] = OpUConvert %v4uint [[sh]]
// CHECK-DAG:     [[ca:%[0-9]+]] = OpBitCount %v4uint [[lo]]
// CHECK-DAG:     [[cb:%[0-9]+]] = OpBitCount %v4uint [[hi]]
// CHECK-DAG:        {{%[0-9]+}} = OpIAdd %v4uint [[cb]] [[ca]]
  uint4 rvs64 = countbits(vs64);
}
