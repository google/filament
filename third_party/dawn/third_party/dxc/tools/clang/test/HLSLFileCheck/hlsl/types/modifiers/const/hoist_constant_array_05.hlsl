// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK: alloca [3 x float]

void foo(inout float f) {
    f += 1;
}

float main(int i : I, int j : J) : SV_Target {
    float A[] = {1,2,3};
    foo(A[i]);
    return A[j];
}

