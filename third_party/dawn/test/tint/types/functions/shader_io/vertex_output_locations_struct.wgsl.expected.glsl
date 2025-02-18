#version 310 es


struct VertexOutputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  vec4 position;
};

layout(location = 0) flat out int tint_interstage_location0;
layout(location = 1) flat out uint tint_interstage_location1;
layout(location = 2) out float tint_interstage_location2;
layout(location = 3) out vec4 tint_interstage_location3;
VertexOutputs main_inner() {
  return VertexOutputs(1, 1u, 1.0f, vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(0.0f));
}
void main() {
  VertexOutputs v = main_inner();
  tint_interstage_location0 = v.loc0;
  tint_interstage_location1 = v.loc1;
  tint_interstage_location2 = v.loc2;
  tint_interstage_location3 = v.loc3;
  gl_Position = vec4(v.position.x, -(v.position.y), ((2.0f * v.position.z) - v.position.w), v.position.w);
  gl_PointSize = 1.0f;
}
