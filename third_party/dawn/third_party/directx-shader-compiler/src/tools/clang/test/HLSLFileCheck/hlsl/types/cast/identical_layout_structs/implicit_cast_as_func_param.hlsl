// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// github issue #1842
// Test implicit cast scenario between structs of identical layout
// when struct is passed as a function parameter. This test 
// simply checks if this program compiles successfully w/o crashing.

// CHECK: main

struct S1 { int a, b; };
struct S2 { int a, b; };
void foo(S2 s) {}
void main() { S1 s; foo(s); }