// RUN: %dxc -E main -T ps_6_0 -HV 2017 %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 10)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 -2)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 2, i32 48)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 3, i32 -25)

enum Vertex : int {
    FIRST = 10,
    SECOND = -2,
    THIRD = 48,
    FOURTH = -25,
};

int4 main(float4 col : COLOR) : SV_Target {
    return float4(FIRST, SECOND, THIRD, FOURTH);
}