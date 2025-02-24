SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 pos;
};

in ivec4 f_Input;
layout(location = 0) in vec4 tint_interstage_location0;
in uvec4 f_Input_1;
void g(int a, float b, float c, uint d) {
}
void f_inner(ivec4 fbf_2, In v, vec4 uv, uvec4 fbf_0) {
  g(fbf_2.z, v.pos.x, uv.x, fbf_0.y);
}
void main() {
  ivec4 v_1 = f_Input;
  In v_2 = In(gl_FragCoord);
  f_inner(v_1, v_2, tint_interstage_location0, f_Input_1);
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'int' : must be qualified as flat in
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
