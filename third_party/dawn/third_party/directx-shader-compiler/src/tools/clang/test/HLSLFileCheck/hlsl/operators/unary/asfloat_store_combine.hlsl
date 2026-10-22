// RUN: %dxc /T cs_6_0 /E main %s | FileCheck %s

// Make sure asfloat and store not crash.
// CHECK:define void @main

groupshared float2 s[2];

[numthreads(8, 8, 1)]
void main(uint2 DTid : SV_DispatchThreadID) {
    for (int j = 0; j < 2; j++)
    {
        s[j] = float2(asfloat(DTid.x), asfloat((0xFFFFFFFF << DTid.y) << j));
    }
}