//
// main0
//
#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out int main0_loc0_Output;
int main0_inner() {
  return 1;
}
void main() {
  main0_loc0_Output = main0_inner();
}
//
// main1
//
#version 310 es
precision highp float;
precision highp int;

layout(location = 1) out uint main1_loc1_Output;
uint main1_inner() {
  return 1u;
}
void main() {
  main1_loc1_Output = main1_inner();
}
//
// main2
//
#version 310 es
precision highp float;
precision highp int;

layout(location = 2) out float main2_loc2_Output;
float main2_inner() {
  return 1.0f;
}
void main() {
  main2_loc2_Output = main2_inner();
}
//
// main3
//
#version 310 es
precision highp float;
precision highp int;

layout(location = 3) out vec4 main3_loc3_Output;
vec4 main3_inner() {
  return vec4(1.0f, 2.0f, 3.0f, 4.0f);
}
void main() {
  main3_loc3_Output = main3_inner();
}
//
// main4
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(location = 4) out float16_t main4_loc4_Output;
float16_t main4_inner() {
  return 2.25hf;
}
void main() {
  main4_loc4_Output = main4_inner();
}
//
// main5
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(location = 5) out f16vec3 main5_loc5_Output;
f16vec3 main5_inner() {
  return f16vec3(3.0hf, 5.0hf, 8.0hf);
}
void main() {
  main5_loc5_Output = main5_inner();
}
