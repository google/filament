// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK: alloca [3 x float]

float main(int i : I, int b : B) : SV_Target {
    float A[] = {
        1, 2, 3
    };
    if (b)
        A[2] = 4;
    else 
        A[2] = 5;
    return A[i];
}

