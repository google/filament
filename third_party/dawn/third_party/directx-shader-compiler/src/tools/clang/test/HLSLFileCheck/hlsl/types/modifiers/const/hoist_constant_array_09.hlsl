// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK:     internal unnamed_addr constant [3 x float]
// CHECK-NOT: alloca [3 x float]

int main(int i : I, int b : B) : SV_Target {
    float A[3] = {1,2,3};
    return asuint(A[i]);
}
