// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Tests that ByteAddressBuffer.Load<T> works with all type shapes with SM 6.2

struct S { int i; float f; };
ByteAddressBuffer buf;
RWStructuredBuffer<int> out_scalar;
RWStructuredBuffer<int2> out_vector;
RWStructuredBuffer<int2x2> out_matrix;
RWStructuredBuffer<int[2]> out_array;
RWStructuredBuffer<S> out_struct;
RWStructuredBuffer<S[2]> out_struct_array;

void main() {
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle {{.*}}, i32 100, i32 undef, i8 1, i32 4)
  out_scalar[0] = buf.Load<int>(100);
  
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle {{.*}}, i32 200, i32 undef, i8 3, i32 4)
  out_vector[0] = buf.Load<int2>(200);
  
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle {{.*}}, i32 300, i32 undef, i8 15, i32 4)
  out_matrix[0] = buf.Load<int2x2>(300);

  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle {{.*}}, i32 400, i32 undef, i8 1, i32 4)
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle {{.*}}, i32 404, i32 undef, i8 1, i32 4)
  out_array[0] = buf.Load<int[2]>(400);
  
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle {{.*}}, i32 500, i32 undef, i8 1, i32 4)
  // CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle {{.*}}, i32 504, i32 undef, i8 1, i32 4)
  out_struct[0] = buf.Load<S>(500);
  
  // Test loads of arrays of structs because of the SROA behavior that turns them into per-element arrays
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle {{.*}}, i32 600, i32 undef, i8 1, i32 4)
  // CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle {{.*}}, i32 604, i32 undef, i8 1, i32 4)
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle {{.*}}, i32 608, i32 undef, i8 1, i32 4)
  // CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle {{.*}}, i32 612, i32 undef, i8 1, i32 4)
  out_struct_array[0] = buf.Load<S[2]>(600);
}
