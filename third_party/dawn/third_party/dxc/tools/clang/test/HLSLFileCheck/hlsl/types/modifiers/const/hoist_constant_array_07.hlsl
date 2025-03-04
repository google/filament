// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK: alloca [3 x float]

float main(int i : I, int b : B) : SV_Target {
    float A[3] = {1,2,3};
    [branch]
    if (b)
        A[2] = 4;
    return A[i];
}
