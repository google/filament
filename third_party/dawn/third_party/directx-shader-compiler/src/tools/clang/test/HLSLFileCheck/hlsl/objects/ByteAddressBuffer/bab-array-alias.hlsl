// RUN: %dxc -E main -T ps_6_3  %s  | FileCheck %s

// Make sure there's only one rawBufferLoad, meaning local copy aliased
// through memcpy to buffer load, rather than loading to local array
// and indexing that.

// CHECK: @dx.op.rawBufferLoad.f32(i32 139,
// CHECK-NOT: @dx.op.rawBufferLoad.f32(i32 139,
// CHECK: ret void

struct Foo {
  float4 farr[16];
};

ByteAddressBuffer BAB;

uint offset;

Foo babload(uint o) {
  return BAB.Load<Foo>(o);
}

float4 main(uint i : IN) : SV_Target {
  Foo foo = babload(offset);
  return foo.farr[i];
}
