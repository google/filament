#version 310 es
precision highp float;
precision highp int;


struct FragmentOutput {
  vec4 color;
};

struct FragmentInput {
  vec2 vUv;
};

uniform highp sampler2DShadow depthMap_texSampler;
layout(location = 2) in vec2 tint_interstage_location2;
layout(location = 0) out vec4 main_loc0_Output;
FragmentOutput main_inner(FragmentInput fIn) {
  float v = texture(depthMap_texSampler, vec3(fIn.vUv, 0.0f));
  vec3 color = vec3(v, v, v);
  FragmentOutput fOut = FragmentOutput(vec4(0.0f));
  fOut.color = vec4(color, 1.0f);
  return fOut;
}
void main() {
  main_loc0_Output = main_inner(FragmentInput(tint_interstage_location2)).color;
}
