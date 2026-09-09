// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK-DAG: internal unnamed_addr constant [3 x i32]
// CHECK-DAG: internal unnamed_addr constant [4 x i32]
// CHECK-NOT: alloca [

int foo(int i) {
    int A[] = {1,2,3};
    return A[i];
}

int bar(int i) {
    int A[] = {4,5,6,7};
    return A[i];
}

int main(int i : I) : SV_Target {
    int B[] = {1,2,3};
    return foo(i) + bar(i) + B[i];
}
