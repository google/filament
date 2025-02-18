#version 310 es
precision highp float;
precision highp int;


struct Scene {
  vec4 vEyePosition;
};

struct Material {
  vec4 vDiffuseColor;
  vec3 vAmbientColor;
  float placeholder;
  vec3 vEmissiveColor;
  float placeholder2;
};

struct Mesh {
  float visibility;
};

struct main_out {
  vec4 glFragColor_1;
};

float fClipDistance3 = 0.0f;
float fClipDistance4 = 0.0f;
layout(binding = 0, std140)
uniform f_x_29_block_ubo {
  Scene inner;
} v;
layout(binding = 1, std140)
uniform f_x_49_block_ubo {
  Material inner;
} v_1;
layout(binding = 2, std140)
uniform f_x_137_block_ubo {
  Mesh inner;
} v_2;
vec4 glFragColor = vec4(0.0f);
bool continue_execution = true;
layout(location = 2) in float tint_interstage_location2;
layout(location = 3) in float tint_interstage_location3;
layout(location = 0) out vec4 main_loc0_Output;
void main_1() {
  vec3 viewDirectionW = vec3(0.0f);
  vec4 baseColor = vec4(0.0f);
  vec3 diffuseColor = vec3(0.0f);
  float alpha = 0.0f;
  vec3 normalW = vec3(0.0f);
  vec2 uvOffset = vec2(0.0f);
  vec3 baseAmbientColor = vec3(0.0f);
  float glossiness = 0.0f;
  vec3 diffuseBase = vec3(0.0f);
  float shadow = 0.0f;
  vec4 refractionColor = vec4(0.0f);
  vec4 reflectionColor = vec4(0.0f);
  vec3 emissiveColor = vec3(0.0f);
  vec3 finalDiffuse = vec3(0.0f);
  vec3 finalSpecular = vec3(0.0f);
  vec4 color = vec4(0.0f);
  float x_9 = fClipDistance3;
  if ((x_9 > 0.0f)) {
    continue_execution = false;
  }
  float x_17 = fClipDistance4;
  if ((x_17 > 0.0f)) {
    continue_execution = false;
  }
  vec4 x_34 = v.inner.vEyePosition;
  vec3 x_38 = vec3(0.0f);
  viewDirectionW = normalize((vec3(x_34.x, x_34.y, x_34.z) - x_38));
  baseColor = vec4(1.0f);
  vec4 x_52 = v_1.inner.vDiffuseColor;
  diffuseColor = vec3(x_52.x, x_52.y, x_52.z);
  float x_60 = v_1.inner.vDiffuseColor.w;
  alpha = x_60;
  vec3 x_62 = vec3(0.0f);
  vec3 x_64 = vec3(0.0f);
  uvOffset = vec2(0.0f);
  vec4 x_74 = vec4(0.0f);
  vec4 x_76 = baseColor;
  vec3 v_3 = vec3(x_76.x, x_76.y, x_76.z);
  vec3 x_78 = (v_3 * vec3(x_74.x, x_74.y, x_74.z));
  vec4 x_79 = baseColor;
  baseColor = vec4(x_78.x, x_78.y, x_78.z, x_79.w);
  baseAmbientColor = vec3(1.0f);
  glossiness = 0.0f;
  diffuseBase = vec3(0.0f);
  shadow = 1.0f;
  refractionColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  reflectionColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  vec3 x_94 = v_1.inner.vEmissiveColor;
  emissiveColor = x_94;
  vec3 x_96 = diffuseBase;
  vec3 x_97 = diffuseColor;
  vec3 x_99 = emissiveColor;
  vec3 x_103 = v_1.inner.vAmbientColor;
  vec4 x_108 = baseColor;
  finalDiffuse = (clamp((((x_96 * x_97) + x_99) + x_103), vec3(0.0f), vec3(1.0f)) * vec3(x_108.x, x_108.y, x_108.z));
  finalSpecular = vec3(0.0f);
  vec3 x_113 = finalDiffuse;
  vec3 x_114 = baseAmbientColor;
  vec3 x_116 = finalSpecular;
  vec4 x_118 = reflectionColor;
  vec4 x_121 = refractionColor;
  vec3 v_4 = (((x_113 * x_114) + x_116) + vec3(x_118.x, x_118.y, x_118.z));
  vec3 x_123 = (v_4 + vec3(x_121.x, x_121.y, x_121.z));
  float x_124 = alpha;
  color = vec4(x_123.x, x_123.y, x_123.z, x_124);
  vec4 x_129 = color;
  vec3 x_132 = max(vec3(x_129.x, x_129.y, x_129.z), vec3(0.0f));
  vec4 x_133 = color;
  color = vec4(x_132.x, x_132.y, x_132.z, x_133.w);
  float x_140 = v_2.inner.visibility;
  float x_142 = color.w;
  color.w = (x_142 * x_140);
  vec4 x_147 = color;
  glFragColor = x_147;
}
main_out main_inner(float fClipDistance3_param, float fClipDistance4_param) {
  fClipDistance3 = fClipDistance3_param;
  fClipDistance4 = fClipDistance4_param;
  main_1();
  main_out v_5 = main_out(glFragColor);
  if (!(continue_execution)) {
    discard;
  }
  return v_5;
}
void main() {
  main_loc0_Output = main_inner(tint_interstage_location2, tint_interstage_location3).glFragColor_1;
}
