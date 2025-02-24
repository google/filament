#version 310 es
#extension GL_EXT_blend_func_extended: require
precision highp float;
precision highp int;


struct FragOutput {
  vec4 color;
  vec4 blend;
};

struct FragInput {
  vec4 a;
  vec4 b;
};

layout(location = 0) in vec4 tint_interstage_location0;
layout(location = 1) in vec4 tint_interstage_location1;
layout(location = 0, index = 0) out vec4 frag_main_loc0_idx0_Output;
layout(location = 0, index = 1) out vec4 frag_main_loc0_idx1_Output;
FragOutput frag_main_inner(FragInput v) {
  FragOutput v_1 = FragOutput(vec4(0.0f), vec4(0.0f));
  v_1.color = v.a;
  v_1.blend = v.b;
  return v_1;
}
void main() {
  FragOutput v_2 = frag_main_inner(FragInput(tint_interstage_location0, tint_interstage_location1));
  frag_main_loc0_idx0_Output = v_2.color;
  frag_main_loc0_idx1_Output = v_2.blend;
}
