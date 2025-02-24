#version 310 es


struct tint_push_constant_struct {
  uint tint_first_instance;
};

struct Output {
  vec4 Position;
  vec4 color;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
layout(location = 0) out vec4 tint_interstage_location0;
Output main_inner(uint VertexIndex, uint InstanceIndex) {
  vec2 zv[4] = vec2[4](vec2(0.20000000298023223877f), vec2(0.30000001192092895508f), vec2(-0.10000000149011611938f), vec2(1.10000002384185791016f));
  float z = zv[min(InstanceIndex, 3u)].x;
  Output v = Output(vec4(0.0f), vec4(0.0f));
  v.Position = vec4(0.5f, 0.5f, z, 1.0f);
  vec4 colors[4] = vec4[4](vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f));
  v.color = colors[min(InstanceIndex, 3u)];
  return v;
}
void main() {
  uint v_1 = uint(gl_VertexID);
  uint v_2 = uint(gl_InstanceID);
  Output v_3 = main_inner(v_1, (v_2 + tint_push_constants.tint_first_instance));
  gl_Position = vec4(v_3.Position.x, -(v_3.Position.y), ((2.0f * v_3.Position.z) - v_3.Position.w), v_3.Position.w);
  tint_interstage_location0 = v_3.color;
  gl_PointSize = 1.0f;
}
