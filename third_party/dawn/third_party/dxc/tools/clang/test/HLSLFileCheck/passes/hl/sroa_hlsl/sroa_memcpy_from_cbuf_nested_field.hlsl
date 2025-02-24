// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Regression test for a crash in SROA when replacing a memcpy
// whose source is a CBuffer value at a deep nesting level (multiple GEPs).

// CHECK: ret void

struct A { float f1[1]; };
struct B { A a1[1]; };
struct C { B b; };
float one(B b) { return 1; }
B getB(C c) { return c.b; }
cbuffer CB { C g_c; }
float main() : SV_Target { return one(getB(g_c)); }