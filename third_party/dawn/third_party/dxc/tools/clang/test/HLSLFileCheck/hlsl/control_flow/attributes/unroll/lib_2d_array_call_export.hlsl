// RUN: %dxc -T lib_6_x %s | FileCheck %s

// Ensure 2d array is supported.

// Ensure BreakUpArrayAllocas doesn't misfire and ignore the function call
// users, causing everything but the calls to be moved to separate scalar
// arrays, then all that code, being dead, getting deleted, leaving just the
// passing of uninitialized arrays to the function calls.

// CHECK-DAG: %[[h0:.+]] = load %dx.types.Handle, %dx.types.Handle* @{{.*}}buf0
// CHECK-DAG: %[[h1:.+]] = load %dx.types.Handle, %dx.types.Handle* @{{.*}}buf1
// CHECK-DAG: %[[h2:.+]] = load %dx.types.Handle, %dx.types.Handle* @{{.*}}buf2
// CHECK-DAG: %[[h3:.+]] = load %dx.types.Handle, %dx.types.Handle* @{{.*}}buf3
// CHECK: %[[buffers:.+]] = alloca [2 x [2 x %dx.types.Handle]]
// CHECK-DAG: %[[gep0:.+]] = getelementptr inbounds [2 x [2 x %dx.types.Handle]], [2 x [2 x %dx.types.Handle]]* %[[buffers]], i32 0, i32 0, i32 0
// CHECK: store %dx.types.Handle %[[h0]], %dx.types.Handle* %[[gep0]], align 8
// CHECK-DAG: %[[gep1:.+]] = getelementptr inbounds [2 x [2 x %dx.types.Handle]], [2 x [2 x %dx.types.Handle]]* %[[buffers]], i32 0, i32 0, i32 1
// CHECK: store %dx.types.Handle %[[h1]], %dx.types.Handle* %[[gep1]], align 8
// CHECK-DAG: %[[gep2:.+]] = getelementptr inbounds [2 x [2 x %dx.types.Handle]], [2 x [2 x %dx.types.Handle]]* %[[buffers]], i32 0, i32 1, i32 0
// CHECK: store %dx.types.Handle %[[h2]], %dx.types.Handle* %[[gep2]], align 8
// CHECK-DAG: %[[gep3:.+]] = getelementptr inbounds [2 x [2 x %dx.types.Handle]], [2 x [2 x %dx.types.Handle]]* %[[buffers]], i32 0, i32 1, i32 1
// CHECK: store %dx.types.Handle %[[h3]], %dx.types.Handle* %[[gep3]], align 8
// CHECK: call void @{{.*}}useArray{{.*}}([2 x [2 x %dx.types.Handle]]* nonnull %[[buffers]])
// CHECK: call void @{{.*}}useArray{{.*}}([2 x [2 x %dx.types.Handle]]* nonnull %[[buffers]])
// CHECK: %[[h10:.+]] = load %dx.types.Handle, %dx.types.Handle* %[[gep2]], align 8
// CHECK: store %dx.types.Handle %[[h10]], %dx.types.Handle* %[[gep1]], align 8
// CHECK: call void @{{.*}}useArray{{.*}}([2 x [2 x %dx.types.Handle]]* nonnull %[[buffers]])
// CHECK: %[[h01:.+]] = load %dx.types.Handle, %dx.types.Handle* %[[gep1]], align 8
// CHECK: store %dx.types.Handle %[[h01]], %dx.types.Handle* %[[gep2]], align 8
// CHECK: call void @{{.*}}useArray{{.*}}([2 x [2 x %dx.types.Handle]]* nonnull %[[buffers]])

void useArray(RWStructuredBuffer<float4> buffers[2][2]);

RWStructuredBuffer<float4> buf0;
RWStructuredBuffer<float4> buf1;
RWStructuredBuffer<float4> buf2;
RWStructuredBuffer<float4> buf3;
uint g_cond;

[shader("pixel")]
float main() : SV_Target {

  RWStructuredBuffer<float4> buffers[2][2] = { buf0, buf1, buf2, buf3, };

  [unroll]
  for (uint j = 0; j < 4; j++) {
    useArray(buffers);
    if (g_cond == j) {
      buffers[j/2][j%2] = buffers[j%2][j/2];
    }
  }

  return 0;
}
