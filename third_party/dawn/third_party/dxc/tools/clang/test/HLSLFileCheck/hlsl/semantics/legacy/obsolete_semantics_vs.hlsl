// RUN: %dxc -E main -T vs_6_0 %s -Gec | FileCheck %s

// CHECK: COLOR                    0   xyzw        0     NONE   float
// CHECK: SV_Position              0   xyzw        0      POS   float

struct VSOut
{
 float4 a : POSITION;
};

VSOut main(float4 c : COLOR)
{
    VSOut retValue = { {1, 2, 3, 4} };
    return retValue;
}
