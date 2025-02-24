// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | FileCheck %s

// CHECK-NOT: @dx.op.rawBufferLoad
// CHECK: call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32
// CHECK: call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32
// CHECK: call double @dx.op.makeDouble.f64
// CHECK: call double @dx.op.makeDouble.f64
// CHECK: call double @dx.op.makeDouble.f64
// CHECK: call double @dx.op.makeDouble.f64
// Store by default will store what's passed in
// CHECK: call void @dx.op.bufferStore.f32
// CHECK: call void @dx.op.bufferStore.i32
// CHECK: call void @dx.op.bufferStore.i32
// CHECK: call void @dx.op.bufferStore.i32

ByteAddressBuffer buf1;
RWByteAddressBuffer buf2;

float4 main(uint idx1 : IDX1, uint idx2 : IDX2) : SV_Target {
  uint status;
  float4 r = float4(0,0,0,0);

  r.x += buf1.Load(idx1);
  r.xy += buf1.Load2(idx1, status);
  r.xyz += buf1.Load3(idx1);
  r.xyzw += buf1.Load4(idx1, status);

  r.x += buf2.Load(idx2, status);
  r.xy += buf2.Load2(idx2);
  r.xyz += buf2.Load3(idx2, status);
  r.xyzw += buf2.Load4(idx2);

  r.x += buf1.Load<float>(idx1, status);
  r.xy += buf1.Load<float2>(idx1);
  r.xyz += buf1.Load<float3>(idx1, status);
  r.xyzw += buf1.Load<float4>(idx1);

  r.x += buf2.Load<float>(idx2);
  r.xy += buf2.Load<float2>(idx2, status);
  r.xyz += buf2.Load<float3>(idx2);
  r.xyzw += buf2.Load<float4>(idx2, status);

  r.x += buf1.Load<double>(idx1);
  r.xy += buf1.Load<double2>(idx1, status);

  r.x += buf2.Load<double>(idx2, status);
  r.xy += buf2.Load<double2>(idx2);

  buf2.Store(1, r.x);
  buf2.Store2(1, r.xy);
  buf2.Store3(1, r.xyz);
  buf2.Store4(1, r);

  return r;
}