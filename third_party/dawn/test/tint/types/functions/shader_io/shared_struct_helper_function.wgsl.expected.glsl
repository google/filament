//
// vert_main1
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  int loc0;
};

layout(location = 0) flat out int tint_interstage_location0;
VertexOutput foo(float x) {
  return VertexOutput(vec4(x, x, x, 1.0f), 42);
}
VertexOutput vert_main1_inner() {
  return foo(0.5f);
}
void main() {
  VertexOutput v = vert_main1_inner();
  gl_Position = vec4(v.pos.x, -(v.pos.y), ((2.0f * v.pos.z) - v.pos.w), v.pos.w);
  tint_interstage_location0 = v.loc0;
  gl_PointSize = 1.0f;
}
//
// vert_main2
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  int loc0;
};

layout(location = 0) flat out int tint_interstage_location0;
VertexOutput foo(float x) {
  return VertexOutput(vec4(x, x, x, 1.0f), 42);
}
VertexOutput vert_main2_inner() {
  return foo(0.25f);
}
void main() {
  VertexOutput v = vert_main2_inner();
  gl_Position = vec4(v.pos.x, -(v.pos.y), ((2.0f * v.pos.z) - v.pos.w), v.pos.w);
  tint_interstage_location0 = v.loc0;
  gl_PointSize = 1.0f;
}
