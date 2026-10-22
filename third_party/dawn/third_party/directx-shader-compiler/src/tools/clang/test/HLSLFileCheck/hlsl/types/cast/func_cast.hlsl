// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s
// CHECK: cannot initialize a variable of type 'int' with an lvalue of type 'int ()'
// CHECK:  non-object type 'int ()' is not assignable
int f() {
    return 1;
}
void main() {
    int x = f;
    int y = 0;
    f = y;
}