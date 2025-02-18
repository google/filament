#version 310 es
#extension GL_OES_sample_variables: require
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct FragmentInputs0 {
  vec4 position;
  int loc0;
};

struct FragmentInputs1 {
  vec4 loc3;
  f16vec3 loc5;
  uint sample_mask;
};

layout(location = 0) flat in int tint_interstage_location0;
layout(location = 1) flat in uint tint_interstage_location1;
layout(location = 3) in vec4 tint_interstage_location3;
layout(location = 5) in f16vec3 tint_interstage_location5;
layout(location = 2) in float tint_interstage_location2;
layout(location = 4) in float16_t tint_interstage_location4;
void main_inner(FragmentInputs0 inputs0, bool front_facing, uint loc1, uint sample_index, FragmentInputs1 inputs1, float loc2, float16_t loc4) {
  if (front_facing) {
    vec4 foo = inputs0.position;
    uint bar = (sample_index + inputs1.sample_mask);
    int i = inputs0.loc0;
    uint u = loc1;
    float f = loc2;
    vec4 v = inputs1.loc3;
    float16_t x = loc4;
    f16vec3 y = inputs1.loc5;
  }
}
void main() {
  FragmentInputs0 v_1 = FragmentInputs0(gl_FragCoord, tint_interstage_location0);
  bool v_2 = gl_FrontFacing;
  uint v_3 = tint_interstage_location1;
  uint v_4 = uint(gl_SampleID);
  vec4 v_5 = tint_interstage_location3;
  f16vec3 v_6 = tint_interstage_location5;
  FragmentInputs1 v_7 = FragmentInputs1(v_5, v_6, uint(gl_SampleMaskIn[0u]));
  main_inner(v_1, v_2, v_3, v_4, v_7, tint_interstage_location2, tint_interstage_location4);
}
