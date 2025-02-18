#version 310 es


struct str {
  int arr[4];
};

shared str S;
void func() {
  S.arr = int[4](0, 0, 0, 0);
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
      S.arr[v_1] = 0;
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  barrier();
  func();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
