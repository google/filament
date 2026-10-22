// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Regression test for GitHub #1843, where derived-to-base
// conversions would cause a crash while emitting arguments,
// probably due to lvalue/rvalue handling while casting.

// CHECK: ret void

struct Base {};
struct Derived : Base { int a; };
void f(Base b) {}
void main() { Derived d; f(d); }