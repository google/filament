// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s

// CHECK: define void @main()
// CHECK: %[[pos:[^ ]+]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: %[[a:[^ ]+]] = fptosi float %[[pos]] to i32
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %[[a]])

struct Number {
    int n;

    int operator()() {
        return n;
    }
};

int main(float4 pos: SV_Position) : SV_Target {
    Number a = {pos.x};
    return a();
}
