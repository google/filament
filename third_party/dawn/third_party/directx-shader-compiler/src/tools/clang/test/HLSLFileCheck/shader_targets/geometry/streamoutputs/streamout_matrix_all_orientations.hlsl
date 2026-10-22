// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// CHECK: call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 0)
// CHECK: call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 0)
// CHECK: call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0, i32 0)
// CHECK: call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 1, i32 0)
// CHECK: call float @dx.op.loadInput.f32(i32 4, i32 2, i32 0, i8 0, i32 0)
// CHECK: call float @dx.op.loadInput.f32(i32 4, i32 2, i32 1, i8 0, i32 0)
// CHECK: call float @dx.op.loadInput.f32(i32 4, i32 3, i32 0, i8 0, i32 0)
// CHECK: call float @dx.op.loadInput.f32(i32 4, i32 3, i32 1, i8 0, i32 0)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float {{.*}})
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float {{.*}})
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float {{.*}})
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 0, float {{.*}})
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 2, i32 0, i8 0, float {{.*}})
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 2, i32 0, i8 1, float {{.*}})
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 3, i32 0, i8 0, float {{.*}})
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 3, i32 1, i8 0, float {{.*}})

struct GSIn
{
    row_major float1x2 a : A;
    row_major float1x2 b : B;
    column_major float1x2 c : C;
    column_major float1x2 d : D;
};

struct GSOut
{
    row_major float1x2 a : A;
    column_major float1x2 b : B;
    row_major float1x2 c : C;
    column_major float1x2 d : D;
};

[maxvertexcount(1)]
void main(point GSIn input[1], inout PointStream<GSOut> output)
{
    GSOut result;
    result.a = input[0].a;
    result.b = input[0].b;
    result.c = input[0].c;
    result.d = input[0].d;
    output.Append(result);
}