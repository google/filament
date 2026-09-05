// RUN: %dxc -T lib_6_3 %s /Qstrip_reflect | FileCheck %s

// Check that validation doesn't complain about RDAT mismatch with resources
// because names are now gone.

// CHECK: @main

Texture2D tex0;
Texture2D tex1;
Texture2D tex2;

SamplerState samp0;
SamplerState samp1;
SamplerState samp2;

[shader("pixel")]
float4 main(float2 uv : TEXCOORD) : SV_Target  {
  return
    tex0.Sample(samp0, uv) +
    tex1.Sample(samp1, uv) +
    tex2.Sample(samp2, uv);
} 
