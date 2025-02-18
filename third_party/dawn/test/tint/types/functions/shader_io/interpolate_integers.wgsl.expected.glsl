//
// vert_main
//
#version 310 es


struct Interface {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};

layout(location = 0) flat out int tint_interstage_location0;
layout(location = 1) flat out uint tint_interstage_location1;
layout(location = 2) flat out ivec4 tint_interstage_location2;
layout(location = 3) flat out uvec4 tint_interstage_location3;
Interface vert_main_inner() {
  return Interface(0, 0u, ivec4(0), uvec4(0u), vec4(0.0f));
}
void main() {
  Interface v = vert_main_inner();
  tint_interstage_location0 = v.i;
  tint_interstage_location1 = v.u;
  tint_interstage_location2 = v.vi;
  tint_interstage_location3 = v.vu;
  gl_Position = vec4(v.pos.x, -(v.pos.y), ((2.0f * v.pos.z) - v.pos.w), v.pos.w);
  gl_PointSize = 1.0f;
}
//
// frag_main
//
#version 310 es
precision highp float;
precision highp int;


struct Interface {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};

layout(location = 0) flat in int tint_interstage_location0;
layout(location = 1) flat in uint tint_interstage_location1;
layout(location = 2) flat in ivec4 tint_interstage_location2;
layout(location = 3) flat in uvec4 tint_interstage_location3;
layout(location = 0) out int frag_main_loc0_Output;
int frag_main_inner(Interface inputs) {
  return inputs.i;
}
void main() {
  frag_main_loc0_Output = frag_main_inner(Interface(tint_interstage_location0, tint_interstage_location1, tint_interstage_location2, tint_interstage_location3, gl_FragCoord));
}
