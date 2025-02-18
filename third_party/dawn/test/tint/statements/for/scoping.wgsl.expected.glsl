#version 310 es

void f() {
  {
    uvec2 tint_loop_idx = uvec2(0u);
    int must_not_collide = 0;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      break;
    }
  }
  int must_not_collide = 0;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
