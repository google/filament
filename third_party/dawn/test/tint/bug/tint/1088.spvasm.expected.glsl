#version 310 es


struct strided_arr {
  float el;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
};

struct LeftOver {
  mat4 worldViewProjection;
  float time;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  mat4 test2[2];
  strided_arr test[4];
};

struct main_out {
  vec4 member_0;
  vec2 vUV_1;
};

vec3 position_1 = vec3(0.0f);
layout(binding = 2, std140)
uniform v_x_14_block_ubo {
  LeftOver inner;
} v;
vec2 vUV = vec2(0.0f);
vec2 uv = vec2(0.0f);
vec3 normal = vec3(0.0f);
vec4 v_1 = vec4(0.0f);
layout(location = 0) in vec3 main_loc0_Input;
layout(location = 2) in vec2 main_loc2_Input;
layout(location = 1) in vec3 main_loc1_Input;
layout(location = 0) out vec2 tint_interstage_location0;
void main_1() {
  vec4 q = vec4(0.0f);
  vec3 p = vec3(0.0f);
  q = vec4(position_1.x, position_1.y, position_1.z, 1.0f);
  p = q.xyz;
  p.x = (p.x + sin(((v.inner.test[0u].el * position_1.y) + v.inner.time)));
  p.y = (p.y + sin((v.inner.time + 4.0f)));
  mat4 v_2 = v.inner.worldViewProjection;
  v_1 = (v_2 * vec4(p.x, p.y, p.z, 1.0f));
  vUV = uv;
  v_1.y = (v_1.y * -1.0f);
}
main_out main_inner(vec3 position_1_param, vec2 uv_param, vec3 normal_param) {
  position_1 = position_1_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  return main_out(v_1, vUV);
}
void main() {
  main_out v_3 = main_inner(main_loc0_Input, main_loc2_Input, main_loc1_Input);
  gl_Position = vec4(v_3.member_0.x, -(v_3.member_0.y), ((2.0f * v_3.member_0.z) - v_3.member_0.w), v_3.member_0.w);
  tint_interstage_location0 = v_3.vUV_1;
  gl_PointSize = 1.0f;
}
