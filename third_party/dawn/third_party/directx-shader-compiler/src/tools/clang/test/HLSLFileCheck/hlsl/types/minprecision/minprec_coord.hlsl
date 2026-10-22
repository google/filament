// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Minimum-precision data types
// CHECK: fpext half {{.*}} to float

Texture2D<float4> tex;
SamplerState      ss;
float4 main(
              min16float2  UV : X
              ) : SV_Target
{
    min16float w;
    min16float h;
    tex.GetDimensions(w,h);
    return tex.Sample(ss, UV) + w + h;
}
