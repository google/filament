#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat4 inner[4];
} v;
shared mat4 w[4];
void f_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      w[v_2] = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  barrier();
  w = v.inner;
  w[1u] = v.inner[2u];
  w[1u][0u] = v.inner[0u][1u].ywxz;
  w[1u][0u].x = v.inner[0u][1u].x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
