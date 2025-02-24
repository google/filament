#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct Outputs {
  float16_t a;
  f16vec4 b;
};

layout(location = 1) in float16_t tint_interstage_location1;
layout(location = 2) in f16vec4 tint_interstage_location2;
layout(location = 1) out float16_t frag_main_loc1_Output;
layout(location = 2) out f16vec4 frag_main_loc2_Output;
Outputs frag_main_inner(float16_t loc1, f16vec4 loc2) {
  return Outputs((loc1 * 2.0hf), (loc2 * 3.0hf));
}
void main() {
  Outputs v = frag_main_inner(tint_interstage_location1, tint_interstage_location2);
  frag_main_loc1_Output = v.a;
  frag_main_loc2_Output = v.b;
}
