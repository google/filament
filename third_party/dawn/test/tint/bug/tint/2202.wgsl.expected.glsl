<dawn>/test/tint/bug/tint/2202.wgsl:7:9 warning: code is unreachable
        let _e9 = (vec3<i32>().y >= vec3<i32>().y);
        ^^^^^^^

#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      {
        uvec2 tint_loop_idx_1 = uvec2(0u);
        while(true) {
          if (all(equal(tint_loop_idx_1, uvec2(4294967295u)))) {
            break;
          }
          return;
        }
      }
      /* unreachable */
    }
  }
}
