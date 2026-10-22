// RUN: %dxc -E main -T cs_6_9 %s | FileCheck %s

// Regression test for min precision rawBufferLoad/Store.
// Min precision types should use i32/f32 operations (not i16/f16)
// to match how pre-SM6.9 RawBufferLoad handles min precision.

RWByteAddressBuffer g_buf : register(u0);

[numthreads(1,1,1)]
void main() {
  // === Vector loads/stores (RawBufferVectorLoad/Store) ===

  // min16int: should load as v3i32, not v3i16
  // CHECK: call %dx.types.ResRet.v3i32 @dx.op.rawBufferVectorLoad.v3i32
  min16int3 vi = g_buf.Load< min16int3 >(0);
  // CHECK: call void @dx.op.rawBufferVectorStore.v3i32
  g_buf.Store< min16int3 >(12, vi);

  // min16uint: should load as v3i32, not v3i16
  // CHECK: call %dx.types.ResRet.v3i32 @dx.op.rawBufferVectorLoad.v3i32
  min16uint3 vu = g_buf.Load< min16uint3 >(24);
  // CHECK: call void @dx.op.rawBufferVectorStore.v3i32
  g_buf.Store< min16uint3 >(36, vu);

  // min16float: should load as v3f32, not v3f16
  // CHECK: call %dx.types.ResRet.v3f32 @dx.op.rawBufferVectorLoad.v3f32
  // CHECK: fptrunc <3 x float> {{.*}} to <3 x half>
  min16float3 vf = g_buf.Load< min16float3 >(48);
  // CHECK: fpext <3 x half> {{.*}} to <3 x float>
  // CHECK: call void @dx.op.rawBufferVectorStore.v3f32
  g_buf.Store< min16float3 >(60, vf);

  // Verify i16/f16 ops are NOT used for vector loads/stores.
  // CHECK-NOT: rawBufferVectorLoad.v{{[0-9]+}}i16
  // CHECK-NOT: rawBufferVectorStore.v{{[0-9]+}}i16
  // CHECK-NOT: rawBufferVectorLoad.v{{[0-9]+}}f16
  // CHECK-NOT: rawBufferVectorStore.v{{[0-9]+}}f16
}
