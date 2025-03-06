// RUN: not %dxc -T vs_6_1 -E main -spirv -Od %s 2>&1 | FileCheck %s

float4 main(nointerpolation float4 p : SV_Position) : SV_Position
{
// CHECK: 6:12: error: GetAttributeAtVertex only allowed in pixel shader
    return GetAttributeAtVertex(p, 0);
}
