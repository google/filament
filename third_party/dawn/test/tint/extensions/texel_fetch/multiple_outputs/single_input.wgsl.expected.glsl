#version 310 es
precision highp float;
precision highp int;


struct Out {
  vec4 x;
  vec4 y;
  vec4 z;
};

in vec4 f_Input;
layout(location = 0) out vec4 f_loc0_Output;
layout(location = 2) out vec4 f_loc2_Output;
layout(location = 3) out vec4 f_loc3_Output;
Out f_inner(vec4 fbf) {
  return Out(vec4(10.0f), fbf, vec4(30.0f));
}
void main() {
  Out v = f_inner(f_Input);
  f_loc0_Output = v.x;
  f_loc2_Output = v.y;
  f_loc3_Output = v.z;
}
