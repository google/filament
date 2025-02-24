// // RUN: %dxc -E main -T cs_6_3 %s | FileCheck %s

struct TestData { 
  int64_t3 v3;
  int64_t4 v4;
};

ByteAddressBuffer srv0 : register(t0); 
RWByteAddressBuffer uav0 : register(u0); 

StructuredBuffer<TestData> srv1 : register(t1);
RWStructuredBuffer<TestData> uav1 : register(u1);

[numthreads(1, 1, 1)]
void main(uint GI : SV_GroupIndex) {

  int64_t3 vec3 = srv0.Load<int64_t3>(0);
// CHECK: call %dx.types.ResRet.i64 @dx.op.rawBufferLoad.i64(i32 139, %dx.types.Handle %srv0_texture_rawbuf, i32 0, i32 undef, i8 7, i32 4)
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 1
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 2

  uav0.Store(0, vec3);
// CHECK: call void @dx.op.rawBufferStore.i64(i32 140, %dx.types.Handle %uav0_UAV_rawbuf, i32 0, i32 undef, i64 %{{[0-9]+}}, i64 %{{[0-9]+}}, i64 %{{[0-9]+}}, i64 undef, i8 7, i32 4)

  int64_t4 vec4 = srv0.Load<int64_t4>(0);
// CHECK: call %dx.types.ResRet.i64 @dx.op.rawBufferLoad.i64(i32 139, %dx.types.Handle %srv0_texture_rawbuf, i32 0, i32 undef, i8 15, i32 4)
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 1
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 2
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 3

  uav0.Store(0, vec4);
// CHECK: call void @dx.op.rawBufferStore.i64(i32 140, %dx.types.Handle %uav0_UAV_rawbuf, i32 0, i32 undef, i64 %{{[0-9]+}}, i64 %{{[0-9]+}}, i64 %{{[0-9]+}}, i64 %{{[0-9]+}}, i8 15, i32 4)

  int64_t3 svec3 = srv1[0].v3;
// CHECK: call %dx.types.ResRet.i64 @dx.op.rawBufferLoad.i64(i32 139, %dx.types.Handle %srv1_texture_structbuf, i32 0, i32 0, i8 7, i32 8)
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 1
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 2

  uav1[0].v3 = svec3;
// CHECK: call void @dx.op.rawBufferStore.i64(i32 140, %dx.types.Handle %uav1_UAV_structbuf, i32 0, i32 0, i64 %{{[0-9]+}}, i64 %{{[0-9]+}}, i64 %{{[0-9]+}}, i64 undef, i8 7, i32 8)

  int64_t4 svec4 = srv1[0].v4;
// CHECK: call %dx.types.ResRet.i64 @dx.op.rawBufferLoad.i64(i32 139, %dx.types.Handle %srv1_texture_structbuf, i32 0, i32 24, i8 15, i32 8)
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 1
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 2
// CHECK: extractvalue %dx.types.ResRet.i64 %{{[0-9a-zA-Z]+}}, 3

  uav1[0].v4 = svec4;
// CHECK: call void @dx.op.rawBufferStore.i64(i32 140, %dx.types.Handle %uav1_UAV_structbuf, i32 0, i32 24, i64 %{{[0-9]+}}, i64 %{{[0-9]+}}, i64 %{{[0-9]+}}, i64 %{{[0-9]+}}, i8 15, i32 8)
};