#version 310 es
precision highp float;
precision highp int;


struct Out {
  vec4 x;
  vec4 y;
  vec4 z;
};

in vec4 f_Input;
in vec4 f_Input_1;
layout(location = 0) out vec4 f_loc0_Output;
layout(location = 2) out vec4 f_loc2_Output;
layout(location = 4) out vec4 f_loc4_Output;
Out f_inner(vec4 fbf_1, vec4 fbf_3) {
  return Out(fbf_1, vec4(20.0f), fbf_3);
}
void main() {
  Out v = f_inner(f_Input, f_Input_1);
  f_loc0_Output = v.x;
  f_loc2_Output = v.y;
  f_loc4_Output = v.z;
}
