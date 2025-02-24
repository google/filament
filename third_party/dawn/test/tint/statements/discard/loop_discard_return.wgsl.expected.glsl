#version 310 es

bool continue_execution = true;
void f() {
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      continue_execution = false;
      return;
    }
  }
  /* unreachable */
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
