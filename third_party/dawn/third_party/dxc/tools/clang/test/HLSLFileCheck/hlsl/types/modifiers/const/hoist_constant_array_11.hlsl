// RUN: %dxc -Emain -Tps_6_0 %s | %FileCheck %s
// CHECK-NOT: alloca [3 x i32]

// We disabled alloca merge when inline, now the allocas will be removed.

int foo(int i) {
    int A[] = {1,2,3};
    return A[i];
}

int bar(int i) {
    int A[] = {4,5,6};
    return A[i];
}

int main(int i : I) : SV_Target {
    return foo(i) + bar(i);
}
