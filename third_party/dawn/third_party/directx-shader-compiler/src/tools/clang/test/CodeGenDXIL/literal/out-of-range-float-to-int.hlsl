// RUN: %dxc -T cs_6_0 %s | FileCheck %s

// CHECK: void @main()
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 0, i32 undef, i32 -2147483648
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 4, i32 undef, i32 -2147483648
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 8, i32 undef, i32 -2147483648
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 12, i32 undef, i32 2147483647
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 16, i32 undef, i32 2147483647
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 20, i32 undef, i32 2147483647
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 24, i32 undef, i32 -2147483648
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 28, i32 undef, i32 2147483647
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 32, i32 undef, i32 0
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 36, i32 undef, i32 0
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 40, i32 undef, i32 0
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 44, i32 undef, i32 -1
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 48, i32 undef, i32 -1
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 52, i32 undef, i32 -1
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 56, i32 undef, i32 0
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 60, i32 undef, i32 -1

RWByteAddressBuffer buff;
uint to_uint(uint a) { return a; }
int to_int(int a) { return a; }

static uint o = 0;

void store(uint u) {
  buff.Store(4 * (o++), u);
}

[numthreads(1, 1, 1)]
void main() {
  // to_int
  store(to_int(-2147483648.0)); // MaxNegative int: -2147483648
  store(to_int(-2147483648.0 - 1.0)); // -2147483648 (clamp int)
  store(to_int(-2147483648.0 - 2.0)); // -2147483648 (clamp int)

  store(to_int(2147483647.0)); // MaxPositive int: 2147483647
  store(to_int(2147483647.0 + 1.0)); // 2147483647 (clamp int)
  store(to_int(2147483647.0 + 2.0)); // 2147483647 (clamp int)

  store(to_int(-1.7976931348623158e+308)); // MaxNegative double: -2147483648 (clamp int)
  store(to_int(1.7976931348623158e+308)); // MaxPositive double: 2147483647 (clamp int)

  // to_uint
  store(to_uint(0.0)); // MaxNegative uint: 0
  store(to_uint(0.0 - 1.0)); // 0 (clamp uint)
  store(to_uint(0.0 - 2.0)); // 0 (clamp uint)

  store(to_uint(4294967295.0)); // MaxPositive uint: 4294967295 (clamp uint)
  store(to_uint(4294967295.0 + 1.0)); // 4294967295 (clamp uint)
  store(to_uint(4294967295.0 + 2.0)); // 4294967295 (clamp uint)

  store(to_uint(-1.7976931348623158e+308)); // MaxNegative double: 0 (clamp uint)
  store(to_uint(1.7976931348623158e+308)); // MaxPositive double: 4294967295 (clamp uint)
}
