// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// CHECK: %struct.BuiltInTriangleIntersectionAttributes

// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 0
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 1
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 2
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 4
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 8
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 16
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 32
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 64
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 128
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 256
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 512
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 254
// CHECK: call void @dx.op.rawBufferStore.i32({{.*}}, i32 255

RWByteAddressBuffer g_buf;

void check(BuiltInTriangleIntersectionAttributes attr) {
    g_buf.Store(0, RAY_FLAG_NONE);
    g_buf.Store(4, RAY_FLAG_FORCE_OPAQUE);
    g_buf.Store(8, RAY_FLAG_FORCE_NON_OPAQUE);
    g_buf.Store(12, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH);
    g_buf.Store(16, RAY_FLAG_SKIP_CLOSEST_HIT_SHADER);
    g_buf.Store(20, RAY_FLAG_CULL_BACK_FACING_TRIANGLES);
    g_buf.Store(24, RAY_FLAG_CULL_FRONT_FACING_TRIANGLES);
    g_buf.Store(28, RAY_FLAG_CULL_OPAQUE);
    g_buf.Store(32, RAY_FLAG_CULL_NON_OPAQUE);
    g_buf.Store(36, RAY_FLAG_SKIP_TRIANGLES);
    g_buf.Store(40, RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES);
    g_buf.Store(44, HIT_KIND_TRIANGLE_FRONT_FACE);
    g_buf.Store(48, HIT_KIND_TRIANGLE_BACK_FACE);
    g_buf.Store(52, attr.barycentrics.x);
    g_buf.Store(56, attr.barycentrics.y);
}