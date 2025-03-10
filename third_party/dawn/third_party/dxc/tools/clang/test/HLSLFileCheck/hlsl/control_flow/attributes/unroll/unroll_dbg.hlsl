// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: @main
struct X {
   float a;
   float b;
};
float a;
float b;
static X g = { a, b};

float4 loop(float4 position) {
    const uint loopCount = 4;

    [unroll(loopCount)]
    for(uint i = 0; i < loopCount; ++i)
        position.x += 1.0f;
    return position;
}

float4 main() : SV_Position
{
    float4 position = g.b;



    return loop(position) + g.a;
}