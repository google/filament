// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK: alloca [3 x float]

float main(int i : I, float f : F) : SV_Target {
    float A[] = {
        1, 2, f
    };
    return A[i];
}

