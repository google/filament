// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK: alloca [3 x float]

int main(int i : I, int j : J) : SV_Target {
    float A[3];
    A[0] = 1;
    A[2] = 2;
    A[j] = 3;
    return A[i];
}
