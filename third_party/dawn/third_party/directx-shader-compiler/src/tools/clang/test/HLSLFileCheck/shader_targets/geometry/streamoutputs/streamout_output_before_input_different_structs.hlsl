// RUN: %dxc -E main -T gs_6_0 %s -HV 2018 | FileCheck %s

// Regression test for an SROA bug where the flattening the output stream argument
// would not handle the case where its input had already been SROA'd.

// CHECK: define void @main()
// CHECK: call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 0)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float {{.*}})
// CHECK: call void @dx.op.emitStream(i32 97, i8 0)
// CHECK: ret void

struct GSIn { float value : TEXCOORD0; };
struct GSOut { float value : TEXCOORD0; };

[maxvertexcount(1)]
void main(inout PointStream<GSOut> output, point GSIn input[1])
{
    output.Append(input[0]);
}
