// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK:     internal unnamed_addr constant [3 x float]
// CHECK-NOT: alloca [3 x float]

float foo(int i) {
    float A[] = {
        1, 2, 3
    };
    return A[i];
}

float main(int i : I) : SV_Target {

    return foo(i) + foo(i+1);
}

