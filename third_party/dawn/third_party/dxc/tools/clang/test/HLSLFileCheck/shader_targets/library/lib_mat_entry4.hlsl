// RUN: %dxc -T lib_6_3   %s | FileCheck %s

// Make sure major change on function call work and offset is correct.
// CHECK: dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 0, i8 15, i32 4)
// CHECK: dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 16, i8 15, i32 4)
// CHECK: dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 32, i8 15, i32 4)

// CHECK: bitcast <12 x float>* {{.*}} to %class.matrix.float.3.4*
// CHECK: [[RET:%.*]] = call %class.matrix.float.4.3 @"\01?mat_test
// CHECK: bitcast <12 x float>* {{.*}} to %class.matrix.float.4.3*
// CHECK: store %class.matrix.float.4.3 [[RET]], %class.matrix.float.4.3* 

// CHECK: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 0, float {{.*}}, float {{.*}}, float {{.*}}, float {{.*}}, i8 15, i32 4)
// CHECK: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 16, float {{.*}}, float {{.*}}, float {{.*}}, float {{.*}}, i8 15, i32 4)
// CHECK: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 32, float {{.*}}, float {{.*}}, float {{.*}}, float {{.*}}, i8 15, i32 4)

// CHECK: dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 48, i8 15, i32 4)
// CHECK: dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 64, i8 15, i32 4)
// CHECK: dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 80, i8 15, i32 4)

// CHECK: bitcast <12 x float>* {{.*}} to %class.matrix.float.3.4*
// CHECK: [[RET2:%.*]] = call %class.matrix.float.4.3 @"\01?mat_test
// CHECK: bitcast <12 x float>* {{.*}} to %class.matrix.float.4.3*
// CHECK: store %class.matrix.float.4.3 [[RET2]], %class.matrix.float.4.3* 

// CHECK: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 48, float {{.*}}, float {{.*}}, float {{.*}}, float {{.*}}, i8 15, i32 4)
// CHECK: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 64, float {{.*}}, float {{.*}}, float {{.*}}, float {{.*}}, i8 15, i32 4)
// CHECK: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 80, float {{.*}}, float {{.*}}, float {{.*}}, float {{.*}}, i8 15, i32 4)



float4x3 mat_test(inout float3x4 m);


cbuffer A {
column_major float3x4 cm;
row_major    float3x4 rm;
column_major float3x2 cma[2];
row_major    float3x3 rma[2];
uint3 i;
};

struct matMajor {
  column_major float3x4 cm;
  row_major    float3x4 rm;  
};
RWStructuredBuffer<matMajor> uav0;

[shader("pixel")]
float3 mainx() : SV_Target {
  column_major float3x4 cm;
  row_major    float3x4 rm;
  float4x3 tm = mat_test(uav0[i.x].cm);
  row_major float4x3 tm2 = mat_test(uav0[i.y].rm);
  return tm[i.x] + tm2[i.y];
}