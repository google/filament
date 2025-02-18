SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 a;
  vec4 b;
  ivec4 fbf;
};

layout(location = 0) in vec4 tint_interstage_location0;
layout(location = 1) flat in vec4 tint_interstage_location1;
in ivec4 f_Input;
void g(float a, float b, int c) {
}
void f_inner(In v) {
  g(v.a.x, v.b.y, v.fbf.x);
}
void main() {
  f_inner(In(tint_interstage_location0, tint_interstage_location1, f_Input));
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'int' : must be qualified as flat in
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
