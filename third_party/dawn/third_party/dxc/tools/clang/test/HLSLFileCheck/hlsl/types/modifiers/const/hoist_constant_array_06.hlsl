// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK:     internal unnamed_addr constant [3 x float]
// CHECK-NOT: alloca [3 x float]

float main(int i : I) : SV_Target {
    float A[3];
    A[0] = 1;
    A[2] = 2;
    return A[i];
}
