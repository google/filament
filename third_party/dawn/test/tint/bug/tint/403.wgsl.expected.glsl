#version 310 es


struct vertexUniformBuffer1_std140 {
  vec2 transform1_col0;
  vec2 transform1_col1;
};

struct vertexUniformBuffer2_std140 {
  vec2 transform2_col0;
  vec2 transform2_col1;
};

layout(binding = 0, std140)
uniform v_x_20_block_std140_ubo {
  vertexUniformBuffer1_std140 inner;
} v;
layout(binding = 0, std140)
uniform v_x_26_block_std140_ubo {
  vertexUniformBuffer2_std140 inner;
} v_1;
vec4 main_inner(uint v_2) {
  vec2 indexable[3] = vec2[3](vec2(0.0f), vec2(0.0f), vec2(0.0f));
  mat2 x_23 = mat2(v.inner.transform1_col0, v.inner.transform1_col1);
  mat2 x_28 = mat2(v_1.inner.transform2_col0, v_1.inner.transform2_col1);
  uint x_46 = v_2;
  indexable = vec2[3](vec2(-1.0f, 1.0f), vec2(1.0f), vec2(-1.0f));
  vec2 x_51 = indexable[min(x_46, 2u)];
  vec2 x_52 = (mat2((x_23[0u] + x_28[0u]), (x_23[1u] + x_28[1u])) * x_51);
  return vec4(x_52.x, x_52.y, 0.0f, 1.0f);
}
void main() {
  vec4 v_3 = main_inner(uint(gl_VertexID));
  gl_Position = vec4(v_3.x, -(v_3.y), ((2.0f * v_3.z) - v_3.w), v_3.w);
  gl_PointSize = 1.0f;
}
