// RUN: %dxc -T ps_6_0 %s | FileCheck %s

// Regression test to make sure we can compile a global constant with
// initializer. Previously this shader would crash in the compiler.

// CHECK: define void @main
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0x3FE14A2800000000)

static const float val = cos(1.0f);

float main() : SV_Target
{
    return val;
}