// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-constantColor | %FileCheck %s

// Check the write to the UAVs were unaffected:
// CHECK: %floatRWUAV_UAV_structbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 1, i32 1, i1 false)
// CHECK: %uav0_UAV_2d = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 0, i1 false)
// CHECK: call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %uav0_UAV_2d, i32 0, i32 0, i32 undef, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, i8 15)

// Added override output color:
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)

RWTexture2D<float4> uav0 : register(u0);
RWStructuredBuffer<float> floatRWUAV: register(u1);

[RootSignature(
  "DescriptorTable(UAV(u0, numDescriptors = 1, space = 0, offset = DESCRIPTOR_RANGE_OFFSET_APPEND)), "
  "UAV(u1)"
)]
float main() : SV_Target {
  floatRWUAV[0] = 3.5;
  uav0[uint2(0,0)] = float4(1,2,3,4);
  return 2.5;
}
