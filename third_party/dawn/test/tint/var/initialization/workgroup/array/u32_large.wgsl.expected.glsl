#version 310 es

shared int zero[23];
void main_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 23u)) {
        break;
      }
      zero[v_2] = 0;
      {
        v_1 = (v_2 + 13u);
      }
      continue;
    }
  }
  barrier();
  int v[23] = zero;
}
layout(local_size_x = 13, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
