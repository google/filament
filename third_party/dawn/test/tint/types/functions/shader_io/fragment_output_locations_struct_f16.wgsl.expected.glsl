#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct FragmentOutputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  float16_t loc4;
  f16vec3 loc5;
};

layout(location = 0) out int main_loc0_Output;
layout(location = 1) out uint main_loc1_Output;
layout(location = 2) out float main_loc2_Output;
layout(location = 3) out vec4 main_loc3_Output;
layout(location = 4) out float16_t main_loc4_Output;
layout(location = 5) out f16vec3 main_loc5_Output;
FragmentOutputs main_inner() {
  return FragmentOutputs(1, 1u, 1.0f, vec4(1.0f, 2.0f, 3.0f, 4.0f), 2.25hf, f16vec3(3.0hf, 5.0hf, 8.0hf));
}
void main() {
  FragmentOutputs v = main_inner();
  main_loc0_Output = v.loc0;
  main_loc1_Output = v.loc1;
  main_loc2_Output = v.loc2;
  main_loc3_Output = v.loc3;
  main_loc4_Output = v.loc4;
  main_loc5_Output = v.loc5;
}
