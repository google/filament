#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;


struct FragIn {
  float a;
  uint mask;
};

layout(location = 0) in float tint_interstage_location0;
layout(location = 1) in float tint_interstage_location1;
layout(location = 0) out float main_loc0_Output;
FragIn main_inner(FragIn v, float b) {
  if ((v.mask == 0u)) {
    return v;
  }
  return FragIn(b, 1u);
}
void main() {
  float v_1 = tint_interstage_location0;
  FragIn v_2 = FragIn(v_1, uint(gl_SampleMaskIn[0u]));
  FragIn v_3 = main_inner(v_2, tint_interstage_location1);
  main_loc0_Output = v_3.a;
  gl_SampleMask[0u] = int(v_3.mask);
}
