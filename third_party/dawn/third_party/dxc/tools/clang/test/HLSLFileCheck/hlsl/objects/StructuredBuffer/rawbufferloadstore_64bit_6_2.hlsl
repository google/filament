// // RUN: %dxc -E main -T cs_6_2 %s | FileCheck %s

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
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %srv0_texture_rawbuf, i32 0, i32 undef, i8 15, i32 4)
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 1
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 2
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 3
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %srv0_texture_rawbuf, i32 16, i32 undef, i8 3, i32 4)
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 1
// CHECK: zext i32 %{{[0-9]+}} to i64
// CHECK: zext i32 %{{[0-9]+}} to i64
// CHECK: shl i64 %{{[0-9]+}}, 32
// CHECK: or i64 %{{[0-9]+}}, %{{[0-9]+}}

   uav0.Store(0, vec3);
// CHECK: trunc i64 %{{[0-9]+}} to i32
// CHECK: lshr i64 %{{[0-9]+}}, 32
// CHECK: trunc i64 %{{[0-9]+}} to i32
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %uav0_UAV_rawbuf, i32 0, i32 undef, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i8 15, i32 4)
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %uav0_UAV_rawbuf, i32 16, i32 undef, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 undef, i32 undef, i8 3, i32 4)

  int64_t4 vec4 = srv0.Load<int64_t4>(0);
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %srv0_texture_rawbuf, i32 0, i32 undef, i8 15, i32 4)
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 1
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 2
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 3
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %srv0_texture_rawbuf, i32 16, i32 undef, i8 15, i32 4)
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 1
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 2
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 3
// CHECK: zext i32 %{{[0-9]+}} to i64
// CHECK: zext i32 %{{[0-9]+}} to i64
// CHECK: shl i64 %{{[0-9]+}}, 32
// CHECK: or i64 %{{[0-9]+}}, %{{[0-9]+}}

  uav0.Store(0, vec4);
// CHECK: trunc i64 %{{[0-9]+}} to i32
// CHECK: lshr i64 %{{[0-9]+}}, 32
// CHECK: trunc i64 %{{[0-9]+}} to i32
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %uav0_UAV_rawbuf, i32 0, i32 undef, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i8 15, i32 4)
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %uav0_UAV_rawbuf, i32 16, i32 undef, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i8 15, i32 4)

  int64_t3 svec3 = srv1[0].v3;
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %srv1_texture_structbuf, i32 0, i32 0, i8 15, i32 8)
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 1
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 2
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 3
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %srv1_texture_structbuf, i32 0, i32 16, i8 3, i32 8)
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 1
// CHECK: zext i32 %{{[0-9]+}} to i64
// CHECK: zext i32 %{{[0-9]+}} to i64
// CHECK: shl i64 %{{[0-9]+}}, 32
// CHECK: or i64 %{{[0-9]+}}, %{{[0-9]+}}

  uav1[0].v3 = svec3;
// CHECK: trunc i64 %{{[0-9]+}} to i32
// CHECK: lshr i64 %{{[0-9]+}}, 32
// CHECK: trunc i64 %{{[0-9]+}} to i32
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %uav1_UAV_structbuf, i32 0, i32 0, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i8 15, i32 8) 
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %uav1_UAV_structbuf, i32 0, i32 16, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 undef, i32 undef, i8 3, i32 8) 

  int64_t4 svec4 = srv1[0].v4;
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %srv1_texture_structbuf, i32 0, i32 24, i8 15, i32 8)
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 1
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 2
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 3
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %srv1_texture_structbuf, i32 0, i32 40, i8 15, i32 8)
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 0
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 1
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 2
// CHECK: extractvalue %dx.types.ResRet.i32 %{{[a-zA-Z0-9]+}}, 3
// CHECK: zext i32 %{{[0-9]+}} to i64
// CHECK: zext i32 %{{[0-9]+}} to i64
// CHECK: shl i64 %{{[0-9]+}}, 32
// CHECK: or i64 %{{[0-9]+}}, %{{[0-9]+}}

  uav1[0].v4 = svec4;
// CHECK: trunc i64 %{{[0-9]+}} to i32
// CHECK: lshr i64 %{{[0-9]+}}, 32
// CHECK: trunc i64 %{{[0-9]+}} to i32
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %uav1_UAV_structbuf, i32 0, i32 24, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i8 15, i32 8) 
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %uav1_UAV_structbuf, i32 0, i32 40, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i8 15, i32 8) 
};