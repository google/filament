struct main_out {
  float4 glFragColor_1;
};

struct main_outputs {
  float4 main_out_glFragColor_1 : SV_Target0;
};

struct main_inputs {
  float fClipDistance3_param : TEXCOORD2;
  float fClipDistance4_param : TEXCOORD3;
};


static float fClipDistance3 = 0.0f;
static float fClipDistance4 = 0.0f;
cbuffer cbuffer_x_29 : register(b0) {
  uint4 x_29[1];
};
cbuffer cbuffer_x_49 : register(b1) {
  uint4 x_49[3];
};
cbuffer cbuffer_x_137 : register(b2) {
  uint4 x_137[1];
};
static float4 glFragColor = (0.0f).xxxx;
void main_1() {
  float3 viewDirectionW = (0.0f).xxx;
  float4 baseColor = (0.0f).xxxx;
  float3 diffuseColor = (0.0f).xxx;
  float alpha = 0.0f;
  float3 normalW = (0.0f).xxx;
  float2 uvOffset = (0.0f).xx;
  float3 baseAmbientColor = (0.0f).xxx;
  float glossiness = 0.0f;
  float3 diffuseBase = (0.0f).xxx;
  float shadow = 0.0f;
  float4 refractionColor = (0.0f).xxxx;
  float4 reflectionColor = (0.0f).xxxx;
  float3 emissiveColor = (0.0f).xxx;
  float3 finalDiffuse = (0.0f).xxx;
  float3 finalSpecular = (0.0f).xxx;
  float4 color = (0.0f).xxxx;
  float x_9 = fClipDistance3;
  if ((x_9 > 0.0f)) {
    discard;
  }
  float x_17 = fClipDistance4;
  if ((x_17 > 0.0f)) {
    discard;
  }
  float4 x_34 = asfloat(x_29[0u]);
  float3 x_38 = (0.0f).xxx;
  viewDirectionW = normalize((float3(x_34.x, x_34.y, x_34.z) - x_38));
  baseColor = (1.0f).xxxx;
  float4 x_52 = asfloat(x_49[0u]);
  diffuseColor = float3(x_52.x, x_52.y, x_52.z);
  float x_60 = asfloat(x_49[0u].w);
  alpha = x_60;
  float3 x_62 = (0.0f).xxx;
  float3 x_64 = (0.0f).xxx;
  uvOffset = (0.0f).xx;
  float4 x_74 = (0.0f).xxxx;
  float4 x_76 = baseColor;
  float3 v = float3(x_76.x, x_76.y, x_76.z);
  float3 x_78 = (v * float3(x_74.x, x_74.y, x_74.z));
  float4 x_79 = baseColor;
  baseColor = float4(x_78.x, x_78.y, x_78.z, x_79.w);
  baseAmbientColor = (1.0f).xxx;
  glossiness = 0.0f;
  diffuseBase = (0.0f).xxx;
  shadow = 1.0f;
  refractionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
  reflectionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
  float3 x_94 = asfloat(x_49[2u].xyz);
  emissiveColor = x_94;
  float3 x_96 = diffuseBase;
  float3 x_97 = diffuseColor;
  float3 x_99 = emissiveColor;
  float3 x_103 = asfloat(x_49[1u].xyz);
  float4 x_108 = baseColor;
  finalDiffuse = (clamp((((x_96 * x_97) + x_99) + x_103), (0.0f).xxx, (1.0f).xxx) * float3(x_108.x, x_108.y, x_108.z));
  finalSpecular = (0.0f).xxx;
  float3 x_113 = finalDiffuse;
  float3 x_114 = baseAmbientColor;
  float3 x_116 = finalSpecular;
  float4 x_118 = reflectionColor;
  float4 x_121 = refractionColor;
  float3 v_1 = (((x_113 * x_114) + x_116) + float3(x_118.x, x_118.y, x_118.z));
  float3 x_123 = (v_1 + float3(x_121.x, x_121.y, x_121.z));
  float x_124 = alpha;
  color = float4(x_123.x, x_123.y, x_123.z, x_124);
  float4 x_129 = color;
  float3 x_132 = max(float3(x_129.x, x_129.y, x_129.z), (0.0f).xxx);
  float4 x_133 = color;
  color = float4(x_132.x, x_132.y, x_132.z, x_133.w);
  float x_140 = asfloat(x_137[0u].x);
  float x_142 = color.w;
  color.w = (x_142 * x_140);
  float4 x_147 = color;
  glFragColor = x_147;
}

main_out main_inner(float fClipDistance3_param, float fClipDistance4_param) {
  fClipDistance3 = fClipDistance3_param;
  fClipDistance4 = fClipDistance4_param;
  main_1();
  main_out v_2 = {glFragColor};
  return v_2;
}

main_outputs main(main_inputs inputs) {
  main_out v_3 = main_inner(inputs.fClipDistance3_param, inputs.fClipDistance4_param);
  main_outputs v_4 = {v_3.glFragColor_1};
  return v_4;
}

