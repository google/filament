#version 310 es


struct str {
  int i;
};

shared str S[4];
void func(uint pointer_indices[1]) {
  S[pointer_indices[0u]] = str(0);
}
void main_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 4u)) {
        break;
      }
      S[v_1] = str(0);
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  barrier();
  func(uint[1](2u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
