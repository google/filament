SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


struct FBF {
  vec4 c1;
  ivec4 c3;
};

in vec4 f_Input;
in ivec4 f_Input_1;
void g(float a, float b, int c) {
}
void f_inner(vec4 pos, FBF fbf) {
  g(fbf.c1.x, pos.y, fbf.c3.z);
}
void main() {
  vec4 v = gl_FragCoord;
  f_inner(v, FBF(f_Input, f_Input_1));
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'int' : must be qualified as flat in
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
