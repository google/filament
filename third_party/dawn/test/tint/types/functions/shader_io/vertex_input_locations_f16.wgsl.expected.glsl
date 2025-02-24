#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(location = 0) in int main_loc0_Input;
layout(location = 1) in uint main_loc1_Input;
layout(location = 2) in float main_loc2_Input;
layout(location = 3) in vec4 main_loc3_Input;
layout(location = 4) in float16_t main_loc4_Input;
layout(location = 5) in f16vec3 main_loc5_Input;
vec4 main_inner(int loc0, uint loc1, float loc2, vec4 loc3, float16_t loc4, f16vec3 loc5) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  vec4 v = loc3;
  float16_t x = loc4;
  f16vec3 y = loc5;
  return vec4(0.0f);
}
void main() {
  vec4 v_1 = main_inner(main_loc0_Input, main_loc1_Input, main_loc2_Input, main_loc3_Input, main_loc4_Input, main_loc5_Input);
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
