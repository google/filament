// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure no warning when sample is not wave sensitive.
// CHECK-NOT:warning
// CHECK:main
Texture2D<float> tex;
SamplerState s;

float main(float4 a : A) : SV_Target {

   float i=WaveReadLaneFirst(a.z);

   if (i > 3)
    return   sin(tex.Sample(s, a.xy));

   return 1;

}