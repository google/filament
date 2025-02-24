#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 main_loc0_Output;
vec4 main_inner() {
  return vec4(0.0f);
}
void main() {
  main_loc0_Output = main_inner();
}
