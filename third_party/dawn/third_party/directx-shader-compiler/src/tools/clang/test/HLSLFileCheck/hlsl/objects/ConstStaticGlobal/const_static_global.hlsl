// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure not split const static globals.
// CHECK:[16 x float]

          static const      float4 referenceColors[4] =
                {
                                // Red               Green
                                float4(1, 0, 0, 1),  float4(0, 1, 0, 1),
                                // Blue              Black
                                float4(0, 0, 1, 1),  float4(0, 0, 0, 1),
                };

float4 main(float2 pos:P) : SV_TARGET0
{

                uint referenceX = (uint)pos.x % 2;
                uint referenceY = (uint)pos.y % 2;
                uint referenceIndex = (referenceY * 2) + referenceX;
                return referenceColors[referenceIndex];
}