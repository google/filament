// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 %s | FileCheck %s

// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16

// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16
// CHECK: trunc i32 %{{[a-zA-Z0-9]+}} to i16

// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32
// CHECK: fptrunc float %{{[a-zA-Z0-9]+}} to half
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32
// CHECK: fptrunc float %{{[a-zA-Z0-9]+}} to half
// CHECK: fptrunc float %{{[a-zA-Z0-9]+}} to half
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32
// CHECK: fptrunc float %{{[a-zA-Z0-9]+}} to half
// CHECK: fptrunc float %{{[a-zA-Z0-9]+}} to half
// CHECK: fptrunc float %{{[a-zA-Z0-9]+}} to half
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32
// CHECK: fptrunc float %{{[a-zA-Z0-9]+}} to half
// CHECK: fptrunc float %{{[a-zA-Z0-9]+}} to half
// CHECK: fptrunc float %{{[a-zA-Z0-9]+}} to half
// CHECK: fptrunc float %{{[a-zA-Z0-9]+}} to half

// CHECK: sext i16 %{{.*}} to i32
// CHECK: call void @dx.op.rawBufferStore.i32
// CHECK: zext i16 %{{.*}} to i32
// CHECK: call void @dx.op.rawBufferStore.i32

struct MyStruct {
  min16int mi1;
  min16int2 mi2;
  min16int3 mi3;
  min16int4 mi4;
  min16uint mu1;
  min16uint2 mu2;
  min16uint3 mu3;
  min16uint4 mu4;
  min16float mf1;
  min16float2 mf2;
  min16float3 mf3;
  min16float4 mf4;
};
StructuredBuffer<MyStruct> buf1;
RWStructuredBuffer<MyStruct> buf2;
int4 main(float idx1 : IDX1, float idx2 : IDX2) : SV_Target {
  uint status;
  min16int4 r = 0;
  r.x += buf2.Load(idx2, status).mi1;
  r.xy += buf2.Load(idx2, status).mi2;
  r.xyz += buf2.Load(idx2, status).mi3;
  r.xyzw += buf2.Load(idx2, status).mi4;
  r.x += buf2.Load(idx2, status).mu1;
  r.xy += buf2.Load(idx2, status).mu2;
  r.xyz += buf2.Load(idx2, status).mu3;
  r.xyzw += buf2.Load(idx2, status).mu4;
  r.x += buf2.Load(idx2, status).mf1;
  r.xy += buf2.Load(idx2, status).mf2;
  r.xyz += buf2.Load(idx2, status).mf3;
  r.xyzw += buf2.Load(idx2, status).mf4;

  buf2[0].mi4 = r;
  buf2[0].mu4 = (min16uint4)r;
  return r;
}