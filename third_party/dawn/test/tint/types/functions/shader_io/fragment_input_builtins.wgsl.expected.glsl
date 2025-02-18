#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;

void main_inner(vec4 position, bool front_facing, uint sample_index, uint sample_mask) {
  if (front_facing) {
    vec4 foo = position;
    uint bar = (sample_index + sample_mask);
  }
}
void main() {
  vec4 v = gl_FragCoord;
  bool v_1 = gl_FrontFacing;
  uint v_2 = uint(gl_SampleID);
  main_inner(v, v_1, v_2, uint(gl_SampleMaskIn[0u]));
}
