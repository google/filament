#version 310 es
#extension GL_EXT_blend_func_extended: require
precision highp float;
precision highp int;


struct FragOutput {
  vec4 color;
  vec4 blend;
};

layout(location = 0, index = 0) out vec4 frag_main_loc0_idx0_Output;
layout(location = 0, index = 1) out vec4 frag_main_loc0_idx1_Output;
FragOutput frag_main_inner() {
  FragOutput v = FragOutput(vec4(0.0f), vec4(0.0f));
  v.color = vec4(0.5f, 0.5f, 0.5f, 1.0f);
  v.blend = vec4(0.5f, 0.5f, 0.5f, 1.0f);
  return v;
}
void main() {
  FragOutput v_1 = frag_main_inner();
  frag_main_loc0_idx0_Output = v_1.color;
  frag_main_loc0_idx1_Output = v_1.blend;
}
