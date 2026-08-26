// RUN: %dxc -T vs_6_0 %s | FileCheck %s

// Previously, this would trigger an assert.  The scenario:
// - Empty struct field in MyStruct2 causes init list cast path in CodeGen
// - DXASSERT that the value type matches the ConvertType for the bool
// - value type is i32 based on field in MyStruct from RecordLayout
// - ConvertType for bool is i1, while ConvertTypeForMem will be i32.
// The assert should instead check both ConvertType and ConvertTypeForMem.

// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)

struct Empty {
};
struct MyStruct {
  bool b;
};
struct MyStruct2 {
  MyStruct s;
  Empty e;
};

bool main() : OUT {
  MyStruct s = { true };
  MyStruct2 s2 = { s };
  return s2.s.b;
}
