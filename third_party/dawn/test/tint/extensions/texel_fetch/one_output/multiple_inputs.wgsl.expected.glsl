#version 310 es
precision highp float;
precision highp int;

in vec4 f_Input;
in vec4 f_Input_1;
layout(location = 0) out vec4 f_loc0_Output;
vec4 f_inner(vec4 fbf_1, vec4 fbf_3) {
  return (fbf_1 + fbf_3);
}
void main() {
  f_loc0_Output = f_inner(f_Input, f_Input_1);
}
