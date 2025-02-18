#version 310 es
precision highp float;
precision highp int;

in vec4 f_Input;
layout(location = 0) out vec4 f_loc0_Output;
vec4 f_inner(vec4 fbf) {
  return fbf;
}
void main() {
  f_loc0_Output = f_inner(f_Input);
}
