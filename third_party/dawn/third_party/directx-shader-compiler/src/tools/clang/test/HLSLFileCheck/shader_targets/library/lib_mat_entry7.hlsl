// RUN: %dxc -T lib_6_1 -Vd  %s | FileCheck %s

// Make sure double matrix flattened and have correct offset.
// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 0)
// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 16)
// CHECK: makeDouble.f64(i32 101,

// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 32)
// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 48)
// CHECK: makeDouble.f64(i32 101,


// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 64)
// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 80)
// CHECK: makeDouble.f64(i32 101,

// CHECK: splitDouble.f64(i32 102,
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 0, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 16, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)


// CHECK: splitDouble.f64(i32 102,
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 32, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 48, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)


// CHECK: splitDouble.f64(i32 102,
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 64, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 80, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)


// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 96)
// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 112)
// CHECK: makeDouble.f64(i32 101,

// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 128)
// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 144)
// CHECK: makeDouble.f64(i32 101,


// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 160)
// CHECK:dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 176)
// CHECK: makeDouble.f64(i32 101,

// CHECK: splitDouble.f64(i32 102,
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 96, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 112, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)


// CHECK: splitDouble.f64(i32 102,
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 128, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 144, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)


// CHECK: splitDouble.f64(i32 102,
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 160, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)
// CHECK: dx.op.bufferStore.i32(i32 69, %dx.types.Handle {{.*}}, i32 {{.*}}, i32 176, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, i8 15)



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