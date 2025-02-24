// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Regression test for GitHub #1929, where we used the C++ definition
// of an aggregate type and failed to match derived structs.

// CHECK: ret void

struct Base {};
struct Derived : Base {};
void f(inout Derived d) {}
void main()
{
    Derived d;
    f(d);
}