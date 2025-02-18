#version 310 es
precision highp float;
precision highp int;


struct Uniforms {
  mat4 worldView;
  mat4 proj;
  uint numPointLights;
  uint color_source;
  uint tint_pad_0;
  uint tint_pad_1;
  vec4 color;
};

struct PointLight {
  vec4 position;
};

struct FragmentOutput {
  vec4 color;
};

struct FragmentInput {
  vec4 position;
  vec4 view_position;
  vec4 normal;
  vec2 uv;
  vec4 color;
};

layout(binding = 0, std140)
uniform f_uniforms_block_ubo {
  Uniforms inner;
} v;
layout(binding = 1, std430)
buffer f_PointLights_ssbo {
  PointLight values[];
} pointLights;
uniform highp sampler2D myTexture;
layout(location = 0) in vec4 tint_interstage_location0;
layout(location = 1) in vec4 tint_interstage_location1;
layout(location = 2) in vec2 tint_interstage_location2;
layout(location = 3) in vec4 tint_interstage_location3;
layout(location = 0) out vec4 main_loc0_Output;
FragmentOutput main_inner(FragmentInput fragment) {
  FragmentOutput v_1 = FragmentOutput(vec4(0.0f));
  v_1.color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  return v_1;
}
void main() {
  main_loc0_Output = main_inner(FragmentInput(gl_FragCoord, tint_interstage_location0, tint_interstage_location1, tint_interstage_location2, tint_interstage_location3)).color;
}
