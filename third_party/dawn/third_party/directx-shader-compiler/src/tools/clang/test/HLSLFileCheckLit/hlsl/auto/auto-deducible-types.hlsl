// RUN: %dxc -T cs_6_0 -HV 202x -verify %s
// expected-no-diagnostics

struct MyStruct {
    float a;
    int b;
};

Texture2D<float4> tex : register(t0);
SamplerState samp : register(s0);
RWBuffer<float> output : register(u0);

[numthreads(1,1,1)]
void main() {
    // Scalars.
    auto i = 5;
    auto f = 1.5f;
    auto b = true;

    // Vectors.
    auto v = float4(1, 2, 3, 4);


    // Matrices.
    float2x2 matInit = { 1, 2, 3, 4 };
    auto m = matInit;
    auto row = m[0];
    auto elem = m[0][0];

    // User-defined structs.
    MyStruct s = { 1.0f, 2 };
    auto sCopy = s;

    // Resource objects (their handles are copyable, so binding one is allowed).
    auto t = tex;

    // Use every value to prevent dead-code elimination.
    output[0] = (float)i + f + (float)b + v.x + m._11 + row.x + elem + sCopy.a +
                (float)sCopy.b + t.SampleLevel(samp, float2(0, 0), 0).x;
}
