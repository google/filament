// RUN: %dxc -E main -T vs_6_2 -HV 2018 -enable-16bit-types %s | FileCheck %s

// Tests that ByteAddressBuffer.Store<T> works with the various ways the
// template type can be specified.

struct S
{
  int16_t i;
  // 2-byte padding here, to test offsets.
  struct { float f; } s; // Nested struct, to test recursion
  struct {} _; // 0-byte field, to test offsets
};
RWByteAddressBuffer buf;

void main() {
  // Explicit template type and matching argument type
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 100, i32 undef, i32 42, i32 42, i32 42, i32 42, i8 15, i32 4)
  buf.Store<int2x2>(100, (int2x2)42);

  // Explicit template type and conversion
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 200, i32 undef, i32 42, i32 42, i32 42, i32 42, i8 15, i32 4)
  buf.Store<int2x2>(200, 42);
  
  // Explicit template type with struct
  // CHECK: call void @dx.op.rawBufferStore.f32(i32 140, {{.*}}, i32 254, i32 undef, float 4.200000e+01, float undef, float undef, float undef, i8 1, i32 4)
  buf.Store<S>(250, (S)42);

  // Deduced template type
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 300, i32 undef, i32 42, i32 42, i32 42, i32 42, i8 15, i32 4)
  buf.Store(300, (int2x2)42);

  // Deduced template type from literal int/float to make sure they become uint/float, not uint64_t/double
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 400, i32 undef, i32 42, i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.f32(i32 140, {{.*}}, i32 404, i32 undef, float 4.200000e+01, float undef, float undef, float undef, i8 1, i32 4)
  buf.Store(400, 42);
  buf.Store(404, 42.0);

}
