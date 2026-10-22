// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Regression test for github issue #3579

// CHECK: define void @main


cbuffer C
{
 float1x1 foo[1];
}

static const float1x1 bar[1] = foo;

void main() {}

