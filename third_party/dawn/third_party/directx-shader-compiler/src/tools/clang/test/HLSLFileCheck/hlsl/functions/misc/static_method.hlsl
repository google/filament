// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)

struct Helper
{
    static float GetValue() { return 1; }
};

float main() : SV_Target
{
    return Helper::GetValue();
}