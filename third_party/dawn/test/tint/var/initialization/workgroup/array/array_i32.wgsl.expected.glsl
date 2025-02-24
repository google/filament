#version 310 es

shared int zero[2][3];
void main_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 6u)) {
        break;
      }
      zero[(v_2 / 3u)][(v_2 % 3u)] = 0;
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  barrier();
  int v[2][3] = zero;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
