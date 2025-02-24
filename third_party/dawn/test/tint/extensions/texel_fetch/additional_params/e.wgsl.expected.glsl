SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


struct In {
  ivec4 fbf;
  vec4 pos;
};

in ivec4 f_Input;
void g(int a, float b) {
}
void f_inner(In v) {
  g(v.fbf.w, v.pos.x);
}
void main() {
  f_inner(In(f_Input, gl_FragCoord));
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'int' : must be qualified as flat in
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
