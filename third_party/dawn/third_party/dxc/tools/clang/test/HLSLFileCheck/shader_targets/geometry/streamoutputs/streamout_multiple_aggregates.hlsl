// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// Regression test for GitHub #1812
// "Crash when using multiple nested structs in GS"
// Due to multiple SROA passes processing the same original Append intrinsic,
// and redundantly queuing it for deletion.

// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float 0.000000e+00)

struct Inner1 { float t : A; };
struct Inner2 { float t : B; };
struct Outer { Inner1 i1; Inner2 i2; };

[maxvertexcount(1)]
void main(point Outer input[1], inout PointStream<Outer> output)
{
    Outer o = (Outer)0;
    output.Append(o);
}