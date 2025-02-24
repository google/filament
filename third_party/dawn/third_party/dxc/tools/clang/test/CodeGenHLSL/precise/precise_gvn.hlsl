// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Make sure that gvn preserves precise metadata when replacing instructions.
// CHECK: @dx.op.dot4.f32{{.*}} !dx.precise


struct VSIn
{
    float4 Pos : P;
    float4 A   : A;
};

struct VSOut
{
    precise float4 Pos : SV_Position;
    float4 N : A;
};

[RootSignature("")]
VSOut main(VSIn input)
{
    float4 X  = input.A * input.A;
    float4 Y  = input.A + input.A;
    float4 R1 = mul(X, Y);
    float4 R2 = mul(X, Y);

    VSOut O;
    O.Pos = R1 * R1;
    O.N   = R2;
    return O;
}