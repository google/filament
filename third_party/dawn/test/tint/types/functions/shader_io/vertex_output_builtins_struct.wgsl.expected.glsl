#version 310 es


struct VertexOutputs {
  vec4 position;
};

VertexOutputs main_inner() {
  return VertexOutputs(vec4(1.0f, 2.0f, 3.0f, 4.0f));
}
void main() {
  vec4 v = main_inner().position;
  gl_Position = vec4(v.x, -(v.y), ((2.0f * v.z) - v.w), v.w);
  gl_PointSize = 1.0f;
}
