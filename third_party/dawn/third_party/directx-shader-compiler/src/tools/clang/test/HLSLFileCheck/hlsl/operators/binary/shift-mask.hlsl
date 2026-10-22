// RUN: %dxc -T lib_6_3 %s -fcgl | FileCheck %s

int shl32(int V, int S) {
  return V << S;
}

// CHECK: define internal i32 @"\01?shl32
// CHECK-DAG:  %[[Masked:.*]] = and i32 %{{.*}}, 31
// CHECK-DAG:  %{{.*}} = shl i32 %{{.*}}, %[[Masked]]

int shr32(int V, int S) {
  return V >> S;
}

// CHECK: define internal i32 @"\01?shr32
// CHECK-DAG:  %[[Masked:.*]] = and i32 %{{.*}}, 31
// CHECK-DAG:  %{{.*}} = ashr i32 %{{.*}}, %[[Masked]]

int64_t shl64(int64_t V, int64_t S) {
  return V << S;
}

// CHECK define internal i64 @"\01?shl64
// CHECK-DAG:  %[[Masked:.*]] = and i64 %{{.*}}, 63
// CHECK-DAG:  %{{.*}} = shl i64 %{{.*}}, %[[Masked]]

int64_t shr64(int64_t V, int64_t S) {
  return V >> S;
}

// CHECK: define internal i64 @"\01?shr64{{[@$?.A-Za-z0-9_]+}}"(i64 %V, i64 %S) #0 {
// CHECK-DAG:  %[[Masked:.*]] = and i64 %{{.*}}, 63
// CHECK-DAG:  %{{.*}} = ashr i64 %{{.*}}, %[[Masked]]
