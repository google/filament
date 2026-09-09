// RUN: %dxc -E main -T vs_6_2 -HV 2018 -enable-16bit-types %s | FileCheck %s

// Tests that ByteAddressBuffer.Store<T> works with all type shapes

struct S
{
  int16_t i;
  // 2-byte padding here, to test offsets.
  struct { float f; } s; // Nested struct, to test recursion
  struct {} _; // 0-byte field, to test offsets
};
RWByteAddressBuffer buf;

void main() {
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 104, i32 undef, i32 42, i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  buf.Store(104, (int)42);
  // CHECK: call void @dx.op.rawBufferStore.i16(i32 140, {{.*}}, i32 108, i32 undef, i16 42, i16 undef, i16 undef, i16 undef, i8 1, i32 2)
  buf.Store(108, (int16_t)42);
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 112, i32 undef, i32 42, i32 0, i32 undef, i32 undef, i8 3, i32 4)
  buf.Store(112, (int64_t)42);

  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 200, i32 undef, i32 42, i32 42, i32 undef, i32 undef, i8 3, i32 4)
  buf.Store(200, (int2)42);

  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 300, i32 undef, i32 42, i32 42, i32 42, i32 42, i8 15, i32 4)
  buf.Store(300, (int2x2)42);
  
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 400, i32 undef, i32 42, i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 404, i32 undef, i32 42, i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  buf.Store(400, (int[2])42);
  
  // CHECK: call void @dx.op.rawBufferStore.i16(i32 140, {{.*}}, i32 500, i32 undef, i16 42, i16 undef, i16 undef, i16 undef, i8 1, i32 2)
  // CHECK: call void @dx.op.rawBufferStore.f32(i32 140, {{.*}}, i32 504, i32 undef, float 4.200000e+01, float undef, float undef, float undef, i8 1, i32 4)
  buf.Store(500, (S)42);

  // Test arrays of structs because of the SROA behavior that turns them into per-element arrays
  // CHECK: call void @dx.op.rawBufferStore.i16(i32 140, {{.*}}, i32 600, i32 undef, i16 42, i16 undef, i16 undef, i16 undef, i8 1, i32 2)
  // CHECK: call void @dx.op.rawBufferStore.f32(i32 140, {{.*}}, i32 604, i32 undef, float 4.200000e+01, float undef, float undef, float undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.i16(i32 140, {{.*}}, i32 608, i32 undef, i16 42, i16 undef, i16 undef, i16 undef, i8 1, i32 2)
  // CHECK: call void @dx.op.rawBufferStore.f32(i32 140, {{.*}}, i32 612, i32 undef, float 4.200000e+01, float undef, float undef, float undef, i8 1, i32 4)
  buf.Store(600, (S[2])42);
}
