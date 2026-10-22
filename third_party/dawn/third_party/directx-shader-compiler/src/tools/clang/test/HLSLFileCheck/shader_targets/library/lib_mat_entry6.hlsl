// RUN: %dxc -T lib_6_3  %s | FileCheck %s

// Make sure double matrix have correct offset.
// CHECK:rawBufferLoad.f64(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 0, i8 15, i32 8)
// CHECK:rawBufferLoad.f64(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 32, i8 15, i32 8)
// CHECK:rawBufferLoad.f64(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 64, i8 15, i32 8)

// CHECK:rawBufferStore.f64(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 0, double {{.*}}, double {{.*}}, double {{.*}}, double {{.*}}, i8 15, i32 8)
// CHECK:rawBufferStore.f64(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 32, double {{.*}}, double {{.*}}, double {{.*}}, double {{.*}}, i8 15, i32 8)
// CHECK:rawBufferStore.f64(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 64, double {{.*}}, double {{.*}}, double {{.*}}, double {{.*}}, i8 15, i32 8)

// CHECK:rawBufferLoad.f64(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 96, i8 15, i32 8)
// CHECK:rawBufferLoad.f64(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 128, i8 15, i32 8)
// CHECK:rawBufferLoad.f64(i32 139, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 160, i8 15, i32 8)

// CHECK:rawBufferStore.f64(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 96, double {{.*}}, double {{.*}}, double {{.*}}, double {{.*}}, i8 15, i32 8)
// CHECK:rawBufferStore.f64(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 128, double {{.*}}, double {{.*}}, double {{.*}}, double {{.*}}, i8 15, i32 8)
// CHECK:rawBufferStore.f64(i32 140, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 160, double {{.*}}, double {{.*}}, double {{.*}}, double {{.*}}, i8 15, i32 8)

double4x3 mat_test(inout double3x4 m);


cbuffer A {
column_major double3x4 cm;
row_major    double3x4 rm;
column_major double3x2 cma[2];
row_major    double3x3 rma[2];
uint3 i;
};

struct matMajor {
  column_major double3x4 cm;
  row_major    double3x4 rm;  
};
RWStructuredBuffer<matMajor> uav0;

[shader("pixel")]
float3 mainx() : SV_Target {
  column_major double3x4 cm;
  row_major    double3x4 rm;
  double4x3 tm = mat_test(uav0[i.x].cm);
  double4x3 tm2 = mat_test(uav0[i.y].rm);
  return tm[i.x] + tm2[i.y];
}