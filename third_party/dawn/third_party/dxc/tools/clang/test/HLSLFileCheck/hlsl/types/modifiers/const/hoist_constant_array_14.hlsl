// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK-DAG:     internal unnamed_addr constant [3 x i32]
// CHECK-DAG:     internal unnamed_addr constant [3 x float]
// CHECK-NOT: alloca [3 x i32]
// CHECK-NOT: alloca [3 x float]

struct S {
    int x;
    float y;
};

int foo(int i) {
    S A[3];
    S s;
    s.x = 1;
    s.y = 2;
    A[0] = s;
    s.x = 3;
    s.y = 4;
    A[1] = s;
    s.x = 5;
    s.y = 6;
    A[2] = s;

    return A[i].x + A[i+1].y;
}

int main(int i : I) : SV_Target {
    return foo(i);
}
