// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Test that individual fields can be loaded from a typed buffer based on a struct type.
// The buffer load offset should be aligned to element sizes.
// Regression test for GitHub #2258

AppendStructuredBuffer<int4> output;

struct Scalars { int a, b; };
Buffer<Scalars> buf_scalars;

struct Vectors { int2 a, b; };
Buffer<Vectors> buf_vectors;

struct Array { int a; int b[3]; };
Buffer<Array> buf_array;

int i;

void main()
{
  // Float at offset > 0
  // CHECK: %[[scalretres:.*]] = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, {{.*}}, i32 0, i32 undef)
  // CHECK: %[[scalval:.*]] = extractvalue %dx.types.ResRet.i32 %[[scalretres]], 1
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 {{.*}}, i32 0, i32 %[[scalval]], i32 0, i32 0, i32 0, i8 15, i32 4)
  output.Append(int4(buf_scalars[0].b, 0, 0, 0));

  // Vector at offset > 0
  // CHECK: %[[vecretres:.*]] = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, {{.*}}, i32 0, i32 undef)
  // CHECK: %[[vecvalx:.*]] = extractvalue %dx.types.ResRet.i32 %[[vecretres]], 2
  // CHECK: %[[vecvaly:.*]] = extractvalue %dx.types.ResRet.i32 %[[vecretres]], 3
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, {{.*}}, i32 {{.*}}, i32 0, i32 %[[vecvalx]], i32 %[[vecvaly]], i32 0, i32 0, i8 15, i32 4)
  output.Append(int4(buf_vectors[0].b, 0, 0));

  // Array at offset > 0, dynamically indexed

  // Convert index to offset in struct
  // CHECK: shl i32 %{{.*}}, 2
  // CHECK: add i32 %{{.*}}, 4

  // Load entire struct elements
  // CHECK: call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32
  // CHECK: extractvalue %dx.types.ResRet.i32 %{{.*}}, 0
  // CHECK: extractvalue %dx.types.ResRet.i32 %{{.*}}, 1
  // CHECK: extractvalue %dx.types.ResRet.i32 %{{.*}}, 2
  // CHECK: extractvalue %dx.types.ResRet.i32 %{{.*}}, 3

  // Fill temporary array
  // CHECK: getelementptr [4 x i32]
  // CHECK: store i32
  // CHECK: getelementptr [4 x i32]
  // CHECK: store i32
  // CHECK: getelementptr [4 x i32]
  // CHECK: store i32
  // CHECK: getelementptr [4 x i32]
  // CHECK: store i32

  // Index into array
  // CHECK: lshr exact i32 %{{.*}}, 2
  // CHECK: getelementptr [4 x i32]
  // CHECK: load i32

  // Store result
  // CHECK: call void @dx.op.rawBufferStore.i32
  output.Append(int4(buf_array[0].b[i], 0, 0, 0));
}