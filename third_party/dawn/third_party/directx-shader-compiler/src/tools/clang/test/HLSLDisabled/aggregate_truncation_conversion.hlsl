// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Repro of GitHub #1799

struct S1 { int a; };
struct S2 { int a, b; };

void main()
{
    S2 s2;
    (S1)s2;
}