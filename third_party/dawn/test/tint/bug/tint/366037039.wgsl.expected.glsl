#version 310 es


struct S {
  uvec3 a;
  uint b;
  uvec3 c[4];
};

layout(binding = 0, std140)
uniform ubuffer_block_1_ubo {
  S inner;
} v;
layout(binding = 1, std430)
buffer sbuffer_block_1_ssbo {
  S inner;
} v_1;
shared S wbuffer;
void tint_store_and_preserve_padding_1(uvec3 value_param[4]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      v_1.inner.c[v_3] = value_param[v_3];
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding(S value_param) {
  v_1.inner.a = value_param.a;
  v_1.inner.b = value_param.b;
  tint_store_and_preserve_padding_1(value_param.c);
}
void foo() {
  S u = v.inner;
  S s = v_1.inner;
  S w = v_1.inner;
  tint_store_and_preserve_padding(S(uvec3(0u), 0u, uvec3[4](uvec3(0u), uvec3(0u), uvec3(0u), uvec3(0u))));
  wbuffer = S(uvec3(0u), 0u, uvec3[4](uvec3(0u), uvec3(0u), uvec3(0u), uvec3(0u)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
