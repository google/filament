// Rewrite unchanged result:
float4 FetchFromIndexMap(uniform Texture2D Tex, uniform SamplerState SS, const float2 RoundedUV, const float LOD) {
  float4 Sample = Tex.SampleLevel(SS, RoundedUV, LOD);
  return Sample * 255.F;
}


struct VS_IN {
  float2 Pos : POSITION;
  uint2 TexAndChannelSelector : TEXCOORD0;
};
struct VS_OUT {
  float4 Position : POSITION;
  float4 Diffuse : COLOR0_center;
  float2 TexCoord0 : TEXCOORD0;
  float4 ChannelSelector : TEXCOORD1;
};
cbuffer OncePerDrawText : register(b0) {
  const float2 TexScale : register(c0);
  const float4 Color : register(c1);
}
;
sampler FontTexture;
VS_OUT FontVertexShader(VS_IN In) {
  VS_OUT Out;
  Out.Position.x = In.Pos.x;
  Out.Position.y = In.Pos.y;
  Out.Diffuse = Color;
  uint texX = (In.TexAndChannelSelector.x >> 0) & 65535;
  uint texY = (In.TexAndChannelSelector.x >> 16) & 65535;
  Out.TexCoord0 = float2(texX, texY) * TexScale;
  Out.ChannelSelector.w = (0 != (In.TexAndChannelSelector.y & 61440)) ? 1 : 0;
  Out.ChannelSelector.x = (0 != (In.TexAndChannelSelector.y & 3840)) ? 1 : 0;
  Out.ChannelSelector.y = (0 != (In.TexAndChannelSelector.y & 240)) ? 1 : 0;
  Out.ChannelSelector.z = (0 != (In.TexAndChannelSelector.y & 15)) ? 1 : 0;
  return Out;
}


float4 FontPixelShader(VS_OUT In) : COLOR0 {
  float4 FontTexel = tex2D(FontTexture, In.TexCoord0).zyxw;
  float4 Color = FontTexel * In.Diffuse;
  if (dot(In.ChannelSelector, 1)) {
    float value = dot(FontTexel, In.ChannelSelector);
    Color.rgb = (value > 0.5F ? 2 * value - 1 : 0.F);
    Color.a = 2 * (value > 0.5F ? 1.F : value);
    Color *= In.Diffuse;
  }
  clip(Color.a - (8.F / 255.F));
  return Color;
}


;
