#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 frag_main_loc0_Output;
vec4 frag_main_inner() {
  float b = 0.0f;
  vec3 v = vec3(b);
  return vec4(v, 1.0f);
}
void main() {
  frag_main_loc0_Output = frag_main_inner();
}
