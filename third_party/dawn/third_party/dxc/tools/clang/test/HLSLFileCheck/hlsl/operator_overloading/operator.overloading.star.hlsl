// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s

// CHECK: [[pos_x:%[^ ]+]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: [[pos_y:%[^ ]+]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)
// CHECK: [[a:%[^ ]+]] = fptosi float [[pos_x]] to i32
// CHECK: [[b:%[^ ]+]] = fptosi float [[pos_y]] to i32
// CHECK: [[star:%[^ ]+]] = mul nsw i32 [[b]], [[a]]
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 [[star]])

struct Number {
    int n;

    int operator*(Number x) {
        return x.n * n;
    }
};

int main(float4 pos: SV_Position) : SV_Target {
    Number a = {pos.x};
    Number b = {pos.y};
    return a * b;
}
