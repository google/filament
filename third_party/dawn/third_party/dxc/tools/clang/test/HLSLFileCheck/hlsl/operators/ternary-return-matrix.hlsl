// RUN: %dxc -T cs_6_0 -E CSMain -HV 2021 %s -fcgl | FileCheck %s

float2x2 crashingFunction(bool b) {
  float2x2 x = {0.0, 0.0, 0.0, 0.0};
  float2x2 y = {0.0, 0.0, 0.0, 0.0};
  return b ? x : y; // <-- this is the issue
}

[numthreads(1, 1, 1)] void CSMain() {
  if (crashingFunction(true)[0][0] > 0)
    return;
}

// CHECK: define internal %class.matrix.float.2.2 @"\01?crashingFunction{{[@$?.A-Za-z0-9_]+}}"
// CHECK: [[ALLOCA:%[0-9a-z]+]] = alloca %class.matrix.float.2.2
// CHECK: preds = {{%[0-9a-z]+}}
// CHECK: call %class.matrix.float.2.2 @"dx.hl.matldst.colStore.%class.matrix.float.2.2 (i32, %class.matrix.float.2.2*, %class.matrix.float.2.2)"(i32 1, %class.matrix.float.2.2* [[ALLOCA]], %class.matrix.float.2.2 %{{[0-9]+}})
// CHECK: preds = {{%[0-9a-z]+}}
// CHECK: call %class.matrix.float.2.2 @"dx.hl.matldst.colStore.%class.matrix.float.2.2 (i32, %class.matrix.float.2.2*, %class.matrix.float.2.2)"(i32 1, %class.matrix.float.2.2* [[ALLOCA]], %class.matrix.float.2.2 %{{[0-9]+}})
// CHECK: preds = {{%[0-9a-z.]+}}, {{%[0-9a-z.]+}}
// CHECK: call %class.matrix.float.2.2 @"dx.hl.matldst.colLoad.%class.matrix.float.2.2 (i32, %class.matrix.float.2.2*)"(i32 0, %class.matrix.float.2.2* [[ALLOCA]])

