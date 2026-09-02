// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK:     internal unnamed_addr constant [3 x float]
// CHECK-NOT: alloca [3 x float]

float main(int i : I, int b : B) : SV_Target {
    float A[3];
    A[0] = 1;
    A[1] = 2;
    [branch]
    if (b)
        A[2] = 4;
    return A[i];
}
