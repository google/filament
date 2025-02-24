// RUN: %dxc -Tlib_6_3 -verify %s
// RUN: %dxc -Tps_6_0 -verify %s

#include "Header1.hlsli"                                    /* expected-error {{'Header1.hlsli' file not found}} fxc-error {{X1507: failed to open source file: 'Header1.hlsli'}} */

// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
  matrix model;
  matrix view;
  matrix projection;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
  float4 pos : SV_POSITION;
  float3 color : COLOR0;
};

[shader("pixel")]
// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)              /* fxc-error {{X3000: unrecognized identifier 'VertexShaderInput'}} */
{
  PixelShaderInput output;
  float4 pos = float4(input.pos, 1.0f);

  // Transform the vertex position into projected space.
  pos = mul(pos, model);
  pos = mul(pos, view);
  pos = mul(pos, projection);
  output.pos = pos;

  // Pass the color through without modification.
  output.color = input.color;

  return output;
}
