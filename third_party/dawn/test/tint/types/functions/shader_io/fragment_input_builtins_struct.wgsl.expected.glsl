#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;


struct FragmentInputs {
  vec4 position;
  bool front_facing;
  uint sample_index;
  uint sample_mask;
};

void main_inner(FragmentInputs inputs) {
  if (inputs.front_facing) {
    vec4 foo = inputs.position;
    uint bar = (inputs.sample_index + inputs.sample_mask);
  }
}
void main() {
  vec4 v = gl_FragCoord;
  bool v_1 = gl_FrontFacing;
  uint v_2 = uint(gl_SampleID);
  main_inner(FragmentInputs(v, v_1, v_2, uint(gl_SampleMaskIn[0u])));
}
