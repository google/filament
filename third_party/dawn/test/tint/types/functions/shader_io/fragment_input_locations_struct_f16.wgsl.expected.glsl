#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct FragmentInputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  float16_t loc4;
  f16vec3 loc5;
};

layout(location = 0) flat in int tint_interstage_location0;
layout(location = 1) flat in uint tint_interstage_location1;
layout(location = 2) in float tint_interstage_location2;
layout(location = 3) in vec4 tint_interstage_location3;
layout(location = 4) in float16_t tint_interstage_location4;
layout(location = 5) in f16vec3 tint_interstage_location5;
void main_inner(FragmentInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  vec4 v = inputs.loc3;
  float16_t x = inputs.loc4;
  f16vec3 y = inputs.loc5;
}
void main() {
  main_inner(FragmentInputs(tint_interstage_location0, tint_interstage_location1, tint_interstage_location2, tint_interstage_location3, tint_interstage_location4, tint_interstage_location5));
}
