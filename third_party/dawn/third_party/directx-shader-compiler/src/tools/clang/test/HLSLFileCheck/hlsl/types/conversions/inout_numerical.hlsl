// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Test that numerical conversions happen when binding to inout parameters,
// both converting into and out of the parameter type.

void inc_i32(inout int val) { val++; }
void main(inout float f32 : F32)
{
    // CHECK: fptosi
    // CHECK: add
    // CHECK: sitofp
    inc_i32(f32);
}