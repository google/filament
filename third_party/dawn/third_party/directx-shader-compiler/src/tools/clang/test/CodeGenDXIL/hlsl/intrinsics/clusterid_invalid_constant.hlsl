// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 %s | FileCheck %s

// Test CLUSTER_ID_INVALID constant

RWByteAddressBuffer outbuf : register(u0);

struct [raypayload] Payload {
  float dummy : read(caller, closesthit, miss, anyhit) : write(caller, closesthit, miss, anyhit);
};

// CHECK-LABEL: define void @{{.*}}test_cluster_id_invalid{{.*}}(
// CHECK: %[[CID:[0-9]+]] = call i32 @dx.op.clusterID(i32 -2147483645)
// CHECK: %[[CMP:[0-9]+]] = icmp eq i32 %[[CID]], -1
// CHECK: br i1 %[[CMP]]
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{[0-9]+}}, i32 0, i32 undef, i32 -1, i32 undef, i32 undef, i32 undef, i8 1, i32 4)
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{[0-9]+}}, i32 0, i32 undef, i32 %[[CID]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
[shader("closesthit")]
void test_cluster_id_invalid(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr) {
  uint cid = ClusterID();
  
  // Test the CLUSTER_ID_INVALID constant (0xffffffff = -1 as signed)
  if (cid == CLUSTER_ID_INVALID) {
    outbuf.Store(0, CLUSTER_ID_INVALID);
  } else {
    outbuf.Store(0, cid);
  }
}

// CHECK-LABEL: define void @{{.*}}test_constant_value{{.*}}(
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{[0-9]+}}, i32 0, i32 undef, i32 -1, i32 undef, i32 undef, i32 undef, i8 1, i32 4)
[shader("raygeneration")]
void test_constant_value() {
  // Verify the constant value is 0xffffffff
  outbuf.Store(0, CLUSTER_ID_INVALID);
}

