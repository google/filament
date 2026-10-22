// RUN: %dxc -T lib_6_3 -enable-16bit-types  %s | FileCheck %s

// Make sure half matrix have correct offset.
// CHECK:rawBufferLoad.f16(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 0, i8 15, i32 2)
// CHECK:rawBufferLoad.f16(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 8, i8 15, i32 2)
// CHECK:rawBufferLoad.f16(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 16, i8 15, i32 2)

// CHECK:rawBufferStore.f16(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 0, half {{.*}}, half {{.*}}, half {{.*}}, half {{.*}}, i8 15, i32 2)
// CHECK:rawBufferStore.f16(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 8, half {{.*}}, half {{.*}}, half {{.*}}, half {{.*}}, i8 15, i32 2)
// CHECK:rawBufferStore.f16(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 16, half {{.*}}, half {{.*}}, half {{.*}}, half {{.*}}, i8 15, i32 2)

// CHECK:rawBufferLoad.f16(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 24, i8 15, i32 2)
// CHECK:rawBufferLoad.f16(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 32, i8 15, i32 2)
// CHECK:rawBufferLoad.f16(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 40, i8 15, i32 2)

// CHECK:rawBufferStore.f16(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 24, half {{.*}}, half {{.*}}, half {{.*}}, half {{.*}}, i8 15, i32 2)
// CHECK:rawBufferStore.f16(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 32, half {{.*}}, half {{.*}}, half {{.*}}, half {{.*}}, i8 15, i32 2)
// CHECK:rawBufferStore.f16(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 40, half {{.*}}, half {{.*}}, half {{.*}}, half {{.*}}, i8 15, i32 2)

half4x3 mat_test(inout half3x4 m);


cbuffer A {
column_major half3x4 cm;
row_major    half3x4 rm;
column_major half3x2 cma[2];
row_major    half3x3 rma[2];
uint3 i;
};

struct matMajor {
  column_major half3x4 cm;
  row_major    half3x4 rm;  
};
RWStructuredBuffer<matMajor> uav0;

[shader("pixel")]
half3 mainx() : SV_Target {
  column_major half3x4 cm;
  row_major    half3x4 rm;
  half4x3 tm = mat_test(uav0[i.x].cm);
  half4x3 tm2 = mat_test(uav0[i.y].rm);
  return tm[i.x] + tm2[i.y];
}