// RUN: %dxc -E mainS -T vs_6_0 %s | FileCheck %s
// RUN: %dxc -E mainU -T vs_6_0 %s | FileCheck %s

// The shift for hlsl only use the LSB 5 bits (0-31 range) of src1 for int/uint.
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 -8)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 16777215)

// The shift for hlsl only use the LSB 6 bits (0-63 range) of src1 for int64_t/uint64_t.
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 2, i32 4)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 3, i32 512)

uint4 mainS(int4 i : INT, uint u : UINT) : OUT {
  uint4 result = 0;

// The shift for hlsl only use the LSB 5 bits (0-31 range) of src1 for int/uint.
  result.x = 0xFFFFFFFFL << 35;
  result.y = 0xFFFFFFFFL >> 40;

// The shift for hlsl only use the LSB 6 bits (0-63 range) of src1 for int64_t/uint64_t.
  uint64_t u64 = 1LL << 105; // = 1 << 40 = 1099511627776 (0x10000000000)
  u64 += 16LL >> 66; // + 4 = (0x10000000004)
  result.z = (uint)(u64 & 0xFFFFFFFF);
  result.w = (uint)(u64 >> 32);
  return result;
}

uint4 mainU(int4 i : INT, uint u : UINT) : OUT {
  uint4 result = 0;

// The shift for hlsl only use the LSB 5 bits (0-31 range) of src1 for int/uint.
  result.x = 0xFFFFFFFFU << 35;
  result.y = 0xFFFFFFFFU >> 40;

// The shift for hlsl only use the LSB 6 bits (0-63 range) of src1 for int64_t/uint64_t.
  uint64_t u64 = 1ULL << 105; // = 1 << 40 = 1099511627776 (0x10000000000)
  u64 += 16ULL >> 66; // + 4 = (0x10000000004)
  result.z = (uint)(u64 & 0xFFFFFFFF);
  result.w = (uint)(u64 >> 32);
  return result;
}
